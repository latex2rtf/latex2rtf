/*  $Id: parser.c,v 1.20 2001/10/13 19:19:10 prahl Exp $

   Contains declarations for a generic recursive parser for LaTeX code.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "cfg.h"
#include "stack.h"
#include "util.h"
#include "parser.h"
#include "l2r_fonts.h"
#include "lengths.h"

#define POSSTACKSIZE   256	/* Size of stack to save positions              */

static char     currentChar;	/* Global current character                 */
static char     lastChar;
static char     penultimateChar;

long            posStack[POSSTACKSIZE];
long            lineStack[POSSTACKSIZE];
int             stackIndex = 0;
extern FILE    *fTex;

static void     parseBracket();	/* parse an open/close bracket sequence              */

char 			ungetcharbuffer[512];
int				ungetcounter = -1;

#define CR (char) 0x0d
#define LF (char) 0x0a

char 
getRawTexChar()
/***************************************************************************
 purpose:     get the next character from the input stream with minimal
              filtering  (CRLF or CR or LF ->  \n) and '\t' -> ' '
			  it also keeps track of the line number
              should only be used by \verb and \verbatim and getTexChar()
****************************************************************************/
{
	int             thechar;

	thechar = getc(fTex);
	if (thechar == EOF)
		if(!feof(fTex)) 
			error("Unknown error reading latex file\n");
		else
			thechar = '\0';
	else if (thechar == CR){                  /* convert CR, CRLF, or LF to \n */
		thechar = getc(fTex);
		if (thechar != LF && !feof(fTex))
			ungetc(thechar, fTex);
		thechar = '\n';
	} else if (thechar == LF)
		thechar = '\n';
	else if (thechar == '\t')
		thechar = ' ';

	currentChar = (char) thechar;

	if (currentChar == '\n')
		linenumber++;

	penultimateChar = lastChar;
	lastChar = currentChar;
	return currentChar;
}

#undef CR
#undef LF

char 
getTexChar()
/***************************************************************************
 purpose:     get the next character from the input stream
              This should be the usual place to access the LaTeX file
			  It filters the input stream so that % is handled properly
****************************************************************************/
{
	char            cThis;
	char            cSave = lastChar;
	char            cSave2 = penultimateChar;

	while ((cThis = getRawTexChar()) && cThis == '%' && cSave != '\\') {
		skipToEOL();
		penultimateChar = cSave2;
		lastChar = cSave;
	}
	return cThis;
}

void 
ungetTexChar(char c)
/****************************************************************************
purpose: rewind the filepointer in the LaTeX-file by one
 ****************************************************************************/
{
	if (c != '\0') 
		ungetc(c, fTex);

	if (c == '\n')
		linenumber--;

	lastChar = penultimateChar;
	penultimateChar = '\0';	/* no longer know what that it was */
}

void 
skipToEOL(void)
/****************************************************************************
purpose: ignores anything from inputfile until the end of line.  
         uses getRawTexChar() because % are not important
 ****************************************************************************/
{
	char            cThis;
	while ((cThis=getRawTexChar()) != '\n');
}

char 
getNonBlank(void)
/***************************************************************************
 Description: get the next non-blank character from the input stream
****************************************************************************/
{
	char            currentChar;

	do {
		currentChar = getTexChar();
	} while (currentChar == ' ' || currentChar == '\n');

	return currentChar;
}

char 
getNonSpace(void)
/***************************************************************************
 Description: get the next non-space character from the input stream
****************************************************************************/
{
	char            currentChar;

	do {
		currentChar = getTexChar();
	} while (currentChar == ' ');

	return currentChar;
}

void 
skipSpaces(void)
/***************************************************************************
 Description: skip to the next non-space character from the input stream
****************************************************************************/
{
	char            c;
	while ((c = getTexChar()) && c == ' ');
	ungetTexChar(c);
}

int 
getSameChar(char c)
/***************************************************************************
 Description: returns the number of characters that are the same as c
****************************************************************************/
{
	char            currentChar;
	int 			count=-1;
	
	do {
		currentChar = getTexChar();
		count++;
	} while (currentChar == c);

	ungetTexChar(currentChar);

	return count;
}

static char *
getDelimitedText(char left, char right)
/******************************************************************************
  purpose: general scanning routine that allocates and returns a string
  		   that is between "left" and "right" that accounts for escaping by '\'
  		   
  		   Example for getDelimitedText('{','}') 
  		   
  		   "{the \{ is shown {\it by} a\\}" ----> "the \{ is shown {\it by} a\\"
  		    
  		    Note the missing opening brace in the example above
 ******************************************************************************/
{
	char            buffer[5000];
	int				size = -1;
	int				lefts_needed = 1;
	char			marker = ' ';
	char			last_char = ' ';
		
	while (lefts_needed && size < 4999) {

		size++;
		last_char = marker;
		buffer[size] = getTexChar();
		marker = buffer[size];

		if (buffer[size] != right || lastChar == '\\') {    	/* avoid \}  */
			if (buffer[size] == left && lastChar != '\\') 		/* avoid \{ */
				lefts_needed++;
			else {
				if (buffer[size] == '\\' && lastChar == '\\')   /* avoid \\} */
					marker = ' ';
			}
		} else 
			lefts_needed--;
	}

	buffer[size] = '\0';		/* overwrite final delimeter */
	if (size == 4999)
		diagnostics(ERROR, "Misplaced '%c' (Not found within 5000 chars)");
		
	return strdup(buffer);
}

void
parseBrace(void)
/****************************************************************************
  Description: Skip text to balancing close brace                          
 ****************************************************************************/
{
	char *s = getDelimitedText('{','}');	
	free(s);
}

void
parseBracket(void)
/****************************************************************************
  Description: Skip text to balancing close bracket
 ****************************************************************************/
{
	char *s = getDelimitedText('[',']');
	free(s);
}

void 
CmdIgnoreParameter(int code)
/****************************************************************************
   Description: Ignore the parameters of a command 
   Example    : CmdIgnoreParameter(21) for \command[opt1]{reg1}{reg2}

   code is a decimal # of the form "op" where `o' is the number of
   optional parameters (0-9) and `p' is the # of required parameters.    
                                                
   The specified number of parameters is ignored.  The order of the parameters
   in the LaTeX file does not matter.                      
****************************************************************************/
{
	int             optParmCount = code / 10;
	int             regParmCount = code % 10;
	char            cThis;
	
	diagnostics(4, "CmdIgnoreParameter [%d] {%d}", optParmCount, regParmCount);

	while (regParmCount) {
		cThis = getNonBlank();
		switch (currentChar) {
		case '{':

			regParmCount--;
			parseBrace();
			break;

		case '[':

			optParmCount--;
			parseBracket();
			break;

		default:
			diagnostics(WARNING,"Ignored command missing {} expected %d - found %d", code%10, code%10-regParmCount);
			ungetTexChar(cThis);
			return;
		}
	}

	/* Check for trailing optional parameter e.g., \item[label] */

	if (optParmCount > 0) {
		cThis=getNonSpace();
		if (cThis == '[') 
			parseBracket();
		else {
			ungetTexChar(cThis);
			return;
		}
	}
	return;
}

char * 
getBracketParam(void)
/******************************************************************************
  purpose: return bracketed parameter
  			
  \item[1]   --->  "1"        \item[]   --->  ""        \item the  --->  NULL
       ^                           ^                         ^

  \item [1]  --->  "1"        \item []  --->  ""        \item  the --->  NULL
       ^                           ^                         ^
 ******************************************************************************/
{
	char            c, *text;

	c = getNonBlank();

	if (c == '[') {
		text = getDelimitedText('[',']');
		diagnostics(5, "getBracketParam [%s]", text);

	} else {
		ungetTexChar(c);
		text = NULL;
		diagnostics(5, "getBracketParam []");
	}
	
	return text;
}

char    *
getSimpleCommand(void)
/**************************************************************************
     purpose: returns a simple command e.g., \alpha\beta will return "alpha"
 **************************************************************************/
{
	char            buffer[128];
	int             size;

	buffer[0] = getTexChar();

	if (buffer[0] != '\\')
		return NULL;

	for (size = 1; size < 127; size++) {
		buffer[size] = getTexChar();

		if (!isalpha((int)buffer[size])) {
			ungetTexChar(buffer[size]);
			break;
		}
	}

	buffer[size] = '\0';
	if (size == 127)
		error(" Misplaced brace in command.  Scanned 127 chars looking for end\n");

	diagnostics(5, "getSimpleCommand result <%s>", buffer);
	return strdup(buffer);
}

char           *
getParam(void)
/**************************************************************************
     purpose: allocates and returns the next parameter in the LaTeX file
              if ^ indicates the current file position then
              
     \alpha\beta   --->  "\beta"             \bar \alpha   --->  "\alpha"
           ^                                     ^
     \bar{text}    --->  "text"              \bar text     --->  "t"
         ^                                       ^
 **************************************************************************/
{
	char            s[2], *text;

	s[0] = getNonSpace();			/*skip spaces and one possible newline */
	if (s[0] == '\n')
		s[0] = getNonSpace();	
	
	if (s[0] == '\\') {
		ungetTexChar(s[0]);
		text=getSimpleCommand();

	} else 	if (s[0] == '{') 
		text = getDelimitedText('{','}');
	
	else {
		s[1] = '\0';
		text = strdup(s);
	}
		
	diagnostics(5, "Leaving getBraceParam {%s}", text);
	return text;
}

long ftellTex()
{
	return ftell(fTex);
}

void fseekTex(long pos)
{
	fseek(fTex, pos, SEEK_SET);
}

char *
getTexUntil(char * target, int raw)
/**************************************************************************
     purpose: returns the portion of the file to the beginning of target
     returns: NULL if not found
     
 **************************************************************************/
{
	char            buffer[4096];
	int             i   = 0;                /* size of string that has been read */
	int             j   = 0;                /* number of found characters */
	int             len = strlen(target);
	long			start_of_target=0;
	
	diagnostics(4, "getTexUntil target = <%s>", target);
	
	while (j < len && i < 4095) {
	
		buffer[i] = (raw) ? getRawTexChar() : getTexChar();
		
		if (buffer[i] != target[j]) {
			if (j > 0) {			        /* false start, put back what was found */
				fseekTex(start_of_target);
				i-=j;
				j=0;
			}
		} else {
			if (j==0)
				start_of_target = ftellTex();
			j++;
		}
		i++;
	}
	
	if (i == 4096) {
		sprintf(buffer,"Could not find <%s> in 4096 characters", target);
		error(buffer);
	}
	
	buffer[i-len] = '\0';

	fseekTex(start_of_target-1);	/* move to start of target */
	
	return strdup(buffer);
}

char           *
getMathParam(void)
/**************************************************************************
     purpose: returns the parameter after ^ or _
     example  ^\alpha and ^{\alpha} both return \alpha
              ^2      and ^{2} both return 2
**************************************************************************/
{
	char            buffer[2];

	diagnostics(5,"entering getMathParam");

	buffer[0] = getNonSpace();			/*skip spaces and one possible newline */
	if (buffer[0] == '\n')
		buffer[0] = getNonSpace();	

	if (buffer[0] == '{') {
		return getDelimitedText('{','}');
	} else if (buffer[0] == '\\') {
		ungetTexChar(buffer[0]);
		return getSimpleCommand();
	} else {
		buffer[1] = '\0';
		return strdup(buffer);
	}

}

int 
getDimension(void)
/**************************************************************************
     purpose: reads a TeX dimension and returns size it twips
          eg: 3 in, -.013mm, 29 pc, + 42,1 dd, 1234sp
**************************************************************************/
{
	char            cThis, buffer[20];
	int             i=0;
	float           num;

	skipSpaces();	

/* obtain optional sign */
	cThis=getTexChar();
	if (cThis=='-' || cThis == '+')
	{
		buffer[i++]=cThis;
		skipSpaces();
		cThis=getTexChar();
	}
	
/* obtain number */
	while (i<19 && (isdigit((int)cThis) || cThis=='.' || cThis == ',')){
		if (cThis==',') cThis = '.';
		buffer[i++] = cThis;
		cThis = getTexChar();
	}
	ungetTexChar(cThis);
	buffer[i]='\0';
	diagnostics(4,"getDimension() number is <%s>", buffer);
	
	if (i==19 || sscanf(buffer, "%f", &num) != 1) {
		diagnostics(WARNING, "Screwy number in TeX dimension");
		return 0;
	}
/*	num *= 2;                    convert pts to twips */
	
/* obtain unit of measure */
	skipSpaces();
	buffer[0] = tolower(getTexChar());
	
	if (buffer[0] != '\\') {
		buffer[1] = tolower(getTexChar());
		buffer[2] = '\0';
		
		diagnostics(4,"getDimension() dimension is <%s>", buffer);
		if (strstr(buffer,"pt"))
			return num*20;
		else if (strstr(buffer,"pc"))
			return num*12*20;
		else if (strstr(buffer,"in"))
			return num*72.27*20;
		else if (strstr(buffer,"bp"))
			return num*72.27/72*20;
		else if (strstr(buffer,"cm"))
			return num*72.27/2.54*20;
		else if (strstr(buffer,"mm"))
			return num*72.27/25.4*20;
		else if (strstr(buffer,"dd"))
			return num*1238.0/1157.0*20;
		else if (strstr(buffer,"dd"))
			return num*1238.0/1157*20;
		else if (strstr(buffer,"cc"))
			return num*1238.0/1157.0*12.0*20;
		else if (strstr(buffer,"sp"))
			return num/65536.0*20;
		else if (strstr(buffer,"ex")) 
			return num*CurrentFontSize()*0.5;
		else if (strstr(buffer,"em")) 
			return num*CurrentFontSize();
		else if (strstr(buffer,"in"))
			return num*72.27*20;
		else {
			ungetTexChar(buffer[1]);
			ungetTexChar(buffer[0]);
			return num;
		}
	} else {
		char * s, *t;
		ungetTexChar(buffer[0]);
		s = getSimpleCommand();
		t = s+1;                  /* skip initial backslash */
		diagnostics(4,"getDimension() dimension is <%s>", t);
		num *= getLength(t);
		free(s);
		return num;
	}
		
}

