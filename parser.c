/*
 * $Id: parser.c,v 1.8 2001/08/12 19:40:25 prahl Exp $
 * History:
 * $Log: parser.c,v $
 * Revision 1.8  2001/08/12 19:40:25  prahl
 * 1.9g
 *         Added commands to read and set TeX lengths
 *         Added commands to read and set TeX counters
 *         Fixed bug in handling of \item[text]
 *         Eliminated comparison of fpos_t variables
 *         Revised getLinenumber ... this is not perfect
 *         Fixed bug in getTexChar() routine
 *         Clearly separate preamble from the document in hopes that
 *           one day more appropriate values for page size, margins,
 *           paragraph spacing etc, will be used in the RTF header
 *         I have added initial support for page sizes still needs testing
 *         added two more test files misc3.tex and misc4.tex
 *         misc4.tex produces a bad rtf file currently
 *         separated all letter commands into letterformat.c
 *         cleaned up warning calls throughout code
 *         added \neq \leq \geq \mid commands to direct.cfg
 *         collected and added commands to write RTF header in preamble.c
 *         broke isolatin1 and hyperlatex support, these will be fixed next version
 *
 * Revision 1.2  1998/10/27 04:51:38  glehner
 * Changed function prototype of parseBrace & parseBracket to
 * void parse...  (from char parse...)
 *
 * Revision 1.1  1998/10/27 04:46:43  glehner
 * Initial revision
 *
 *
 * LEG 070798 adapted Frank Barnes contribution to r2l coding conventions
 *

/* Contains declarations for a generic recursive parser the  LaTex2RTF code.                                                       */
/* */
/* Revision history                                                         */
/* ================                                                         */
/* 26th June 1998 - Created initial version - fb                            */
/* 24th May  2001 - Now includes getParam, getbracketparam, getbraceparam    */
/****************************************************************************/

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

static void     parseBrace();	/* parse an open/close brace sequence                */
static void     parseBracket();	/* parse an open/close bracket sequence              */


#define CR (char) 0x0d
#define LF (char) 0x0a

char 
getRawTexChar()
/***************************************************************************
 Description: get the next character from the input stream with minimal
              filtering  (CRLF or CR or LF ->  \n) and '\t' -> ' '
			  it also keeps track of the line number
              should only be used for \verb and \verbatim and getTexChar()
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

/***************************************************************************
 function: getTexChar()
 Description: get the next character from the input stream
              This should be the usual place to access the LaTeX file
			  It should handle % properly
****************************************************************************/
char 
getTexChar()
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
 Description: get the next non-space character from the input stream
****************************************************************************/
{
	char            c;
	while ((c = getTexChar()) && c == ' ');
	ungetTexChar(c);
}

void
parseBrace()
/****************************************************************************
  Description: Skip text to balancing close brace                          
/****************************************************************************/
{
	while (getTexChar() != '}') {
		switch (currentChar) {
			case '{':

			parseBrace();
			break;

		case '[':

			parseBracket();
			break;

		default: /* Skip other characters */ ;
		}
	}
}

void
parseBracket()
/****************************************************************************
  Description: Skip text to balancing close bracket
/****************************************************************************/
{
	while (getTexChar() != ']') {
		switch (currentChar) {
			case '{':

			parseBrace();
			break;

		case '[':

			parseBracket();
			break;

		default: /* Skip other characters */ ;
		}
	}
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
	/**********************************************************/
	/* Check for trailing optional parameter                  */
	/* (I dont think optionals come last - but just in case!) */
	/* Of course, optionals very often come last. e.g.: the   */
	/* \item[label] of item in a description list.           */
	/**********************************************************/
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

bool 
getBracketParam(char *string, int size)
/******************************************************************************
  purpose: function to get an optional parameter
parameter: string: returnvalue of optional parameter
	   size: max. size of returnvalue
	   returns true if a brackets are found
	   allows us to figure out if \item[] is found for example
 ******************************************************************************/
{
	char            c;
	int             i = 0;
	int             bracketlevel = 0;

	diagnostics(5, "Entering getBracketParam()");
	c = getNonBlank();

	if (c != '[') {		/* does not start with a bracket, abort */
		ungetTexChar(c);
		string[0] = '\0';
		return FALSE;
	}
	for (i = 0;; i++) {
		c = getTexChar();

		if ((c == ']') && (bracketlevel == 0))
			break;

		if (c == '[')
			bracketlevel++;

		if (c == ']')
			bracketlevel--;

		if (i < size - 1)	/* throw away excess */
			string[i] = c;
	}

	if (bracketlevel > 0) {
		fprintf(stderr, "**Error - Bracketed string is longer than %d characters\n", size - 1);
		i = size - 1;
	}
	string[i] = '\0';
	diagnostics(5, "Leaving getBracketParam() [%s]", string);
	return TRUE;
}

void 
getBraceParam(char *string, int size)
/******************************************************************************
  purpose: function to get a parameter between {}
parameter: string: returnvalue of optional parameter
	   size: max. size of returnvalue

If it a {} expression does not follow, then return an empty expression
with fTex pointing to the first non-space character
 ******************************************************************************/
{
	char            c;
	int             i = 0;
	int             bracelevel = 0;

	*string = '\0';

	c = getNonBlank();

	if (c != '{') {		/* does not start with a brace, abort */
		ungetTexChar(c);
		return;
	}
	while ((c = getTexChar())) {

		if ((c == '}') && (bracelevel == 0))
			break;

		if (c == '{')
			bracelevel++;

		if (c == '}')
			bracelevel--;

		if (i < size - 1)	/* throw away excess */
			string[i++] = c;
	}
	string[i] = '\0';

	/* fprintf(stderr, "\nthe string in braces is %s\n", string); */
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

	for (size = 0; size < 127; size++) {
		buffer[size] = getTexChar();

		if (!isalpha(buffer[size])) {
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
     purpose: returns the parameter after the \begin-command
	      for instance: \begin{environment}
		    return: -> string = "environment"
   parameter: string: pointer to string, which returns the parameter
     globals: BracketLevel: counts open braces
              fTex	  : Tex-file-pointer
     returns: success: string
	      miss : string = ""
 **************************************************************************/
{
	char            cThis, buffer[4094];
	int             PopLevel, PopBrack, bracket, size;

	if ((cThis = getTexChar()) != '{') {
		buffer[0] = cThis;
		buffer[1] = '\0';
		return strdup(buffer);
	}
	++BracketLevel;
	(void) Push(RecursLevel, BracketLevel);

	size = 0;
	bracket = 1;
	while (bracket > 0 && size < 4093) {
		buffer[size] = getTexChar();

		if (buffer[size] == '}') {
			bracket--;
			if (bracket == 0) {
				--BracketLevel;
				(void) Pop(&PopLevel, &PopBrack);
				break;
			}
		}
		if (buffer[size] == '{')
			bracket++;

		size++;
	}

	buffer[size] = '\0';
	if (size == 4093)
		error(" Misplaced brace in command.  Scanned 4093 chars looking for end\n");

	diagnostics(5, "getParam result <%s>", buffer);

	return strdup(buffer);
}

char *
getTexUntil(char * target)
/**************************************************************************
     purpose: returns the portion of the file to the beginning of target
     returns: NULL if not found
 **************************************************************************/
{
	char            buffer[4096];
	int             i   = 0;                /* size of string that has been read */
	int             j   = 0;                /* number of found characters */
	int             len = strlen(target);
	
	while (j < len && i < 4095) {
	
		buffer[i] = getTexChar();
		
		if (buffer[i] != target[j]) {
			while (j > 0) {			        /* false start, put back what was found */
				ungetTexChar(buffer[i]);
				j--;
				i--;
			}
		} else 
			j++;
		
		i++;
	}
	
	if (i == 4096)
		error("Could not find target in 4096 characters");
	
	buffer[i-len] = '\0';

	while(j>0) {					/* put the target back */
		j--;
		ungetTexChar(target[j]);
	}
		
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

	buffer[0] = getTexChar();

	if (buffer[0] == '{') {
		ungetTexChar(buffer[0]);
		return getParam();
	} else if (buffer[0] == '\\') {
		if (buffer[0] != ' ')	/* TeX eats the space after control words */
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
	while (i<19 && (isdigit(cThis) || cThis=='.' || cThis == ',')){
		if (cThis==',') cThis = '.';
		buffer[i++] = cThis;
		cThis = getTexChar();
	}
	ungetTexChar(cThis);
	buffer[i]='\0';
	diagnostics(4,"getDimension() number is <%s>", buffer);
	
	if (i==19 || sscanf(buffer, "%f", &num) != 1)
		error("Screwy number in TeX dimension");
	num *= 2;                   /* convert pts to twips */
	
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
		char * s;
		ungetTexChar(buffer[0]);
		s = getSimpleCommand();
		diagnostics(4,"getDimension() dimension is <%s>", s);
		num *= getLength(s);
		free(s);
		return num;
	}
		
}

