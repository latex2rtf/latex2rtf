/*  $Id: parser.c,v 1.30 2001/11/04 19:20:44 prahl Exp $

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

typedef struct InputStackType
{
   char  *string;
   FILE  *file;
   char  *file_name;
   long   file_line;
} InputStackType;

#define PARSER_SOURCE_MAX 40
static InputStackType   g_parser_stack[PARSER_SOURCE_MAX];

static int              g_parser_depth = -1;
static char            *g_parser_string = "stdin";
static FILE            *g_parser_file   = stdin;
static int 				g_parser_line   = 1;

static long				g_parser_file_pos;
static char            *g_parser_string_pos;

static char             g_parser_currentChar;	/* Global current character */
static char             g_parser_lastChar;
static char             g_parser_penultimateChar;

static void     parseBracket();


int CurrentLineNumber(void) 
{
	return g_parser_line;
}

char *
CurrentFileName(void)
{
	return g_parser_stack[g_parser_depth].file_name;
}

/*
	The following two routines allow parsing of multiple files and strings
*/

int 
PushSource(char * filename, char * string)
{
	char       s[50];
	FILE *p    = NULL;
	char *name = NULL;
	int  line  = 1;
	
	/* save current values for linenumber and string */
	if (g_parser_depth >=0) {
		g_parser_stack[g_parser_depth].file_line = g_parser_line;
		g_parser_stack[g_parser_depth].string    = g_parser_string;
	}
		
	if (filename) {
		name = strdup(filename);
		p = fopen(filename, "rb");
		if (!p) {
           diagnostics(WARNING, "Cannot open <%s>\n", filename);
           return 0;
       }
    } else {
    	name = CurrentFileName();
    	line = CurrentLineNumber();
	}    	
	
	if (++g_parser_depth >= PARSER_SOURCE_MAX) {
		diagnostics(ERROR, "To many BeginSource() calls");
		return 0;
	}

	g_parser_stack[g_parser_depth].string      = string;
	g_parser_stack[g_parser_depth].file        = p;
	g_parser_stack[g_parser_depth].file_line   = line;
	g_parser_stack[g_parser_depth].file_name   = name;
	g_parser_file = p;
	g_parser_string = string;
	g_parser_line = line;

	if (g_parser_file)
		diagnostics(5, "Opening Source File %s", g_parser_stack[g_parser_depth].file_name);
	else {
		strncpy(s,g_parser_string,25);
		diagnostics(5, "Opening Source string <%s>",s);
	}

	return 1;
}

int 
StillSource(void)
{
	if (g_parser_file)
		return (!feof(g_parser_file));
	else
		return (*g_parser_string != '\0');
}

void 
PopSource(void)
{
	char       s[50];

	if (g_parser_file)
		diagnostics(5, "Closing Source File %s", g_parser_stack[g_parser_depth].file_name);
	else {
		strncpy(s,g_parser_string,25);
		diagnostics(5, "Closing Source string <%s>",s);
	}

	if (g_parser_depth < 0) 
		diagnostics(ERROR, "EndSource() calls exceed BeginSource() calls");

	if (g_parser_file) {
		fclose(g_parser_file);
		free(g_parser_stack[g_parser_depth].file_name);
	}
		
	g_parser_depth--;
	
	if (g_parser_depth >= 0) {
		g_parser_string = g_parser_stack[g_parser_depth].string;
		g_parser_file   = g_parser_stack[g_parser_depth].file;
		g_parser_line   = g_parser_stack[g_parser_depth].file_line;
	}

	if (g_parser_file)
		diagnostics(5, "Resuming Source File %s", g_parser_stack[g_parser_depth].file_name);
	else {
		strncpy(s,g_parser_string,25);
		diagnostics(5, "Resuming Source string <%s>",s);
	}
}

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

	if (g_parser_file) {
		thechar = getc(g_parser_file);
		if (thechar == EOF)
			if(!feof(g_parser_file)) 
				diagnostics(ERROR, "Unknown file I/O error reading latex file\n");
			else
				thechar = '\0';
		else if (thechar == CR){                  /* convert CR, CRLF, or LF to \n */
			thechar = getc(g_parser_file);
			if (thechar != LF && !feof(g_parser_file))
				ungetc(thechar, g_parser_file);
			thechar = '\n';
		} else if (thechar == LF)
			thechar = '\n';
		else if (thechar == '\t')
			thechar = ' ';
			
		g_parser_currentChar = (char) thechar;

	if (g_parser_currentChar == '\n')
		g_parser_line++;
		
	} else {							/* no need to sanitize strings! */
		if (g_parser_string && *g_parser_string) {
			g_parser_currentChar = *g_parser_string;
			g_parser_string++;
		} else 
			g_parser_currentChar = '\0';
	}
	g_parser_penultimateChar = g_parser_lastChar;
	g_parser_lastChar = g_parser_currentChar;
	return g_parser_currentChar;
}

#undef CR
#undef LF

void 
ungetTexChar(char c)
/****************************************************************************
purpose: rewind the filepointer in the LaTeX-file by one
 ****************************************************************************/
{
	if (c == '\0')
		return;
		
	if (g_parser_file) {
	
		ungetc(c, g_parser_file);
		if (c == '\n') g_parser_line--;
		
	} else {
		g_parser_string--; 
		if (g_parser_string && *g_parser_string) {
			*g_parser_string = c;
		}
	}
	
	g_parser_currentChar = g_parser_lastChar;
	g_parser_lastChar = g_parser_penultimateChar;
	g_parser_penultimateChar = '\0';	/* no longer know what that it was */
}

char 
getTexChar()
/***************************************************************************
 purpose:     get the next character from the input stream
              This should be the usual place to access the LaTeX file
			  It filters the input stream so that % is handled properly
****************************************************************************/
{
	char            cThis;
	char            cSave = g_parser_lastChar;
	char            cSave2 = g_parser_penultimateChar;

	while ((cThis = getRawTexChar()) && cThis == '%' && cSave != '\\') {
		skipToEOL();
		g_parser_penultimateChar = cSave2;
		g_parser_lastChar = cSave;
	}
	return cThis;
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
	char            c;
	while ((c = getTexChar()) && (c == ' ' || c == '\n'));
	return c;
}

char 
getNonSpace(void)
/***************************************************************************
 Description: get the next non-space character from the input stream
****************************************************************************/
{
	char            c;
	while ((c = getTexChar()) && c == ' ');
	return c;
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
	char            cThis;
	int 			count=-1;
	
	do {
		cThis = getTexChar();
		count++;
	} while (cThis == c);

	ungetTexChar(cThis);

	return count;
}

char *
getDelimitedText(char left, char right, bool raw)
/******************************************************************************
  purpose: general scanning routine that allocates and returns a string
  		   that is between "left" and "right" that accounts for escaping by '\'
  		   
  		   Example for getDelimitedText('{','}',TRUE) 
  		   
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
		buffer[size] = (raw) ? getRawTexChar() : getTexChar();
		marker = buffer[size];

		if (buffer[size] != right || last_char == '\\') {    	/* avoid \}  */
			if (buffer[size] == left && last_char != '\\') 		/* avoid \{ */
				lefts_needed++;
			else {
				if (buffer[size] == '\\' && last_char == '\\')   /* avoid \\} */
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
	char *s = getDelimitedText('{','}',FALSE);	
	free(s);
}

void
parseBracket(void)
/****************************************************************************
  Description: Skip text to balancing close bracket
 ****************************************************************************/
{
	char *s = getDelimitedText('[',']',FALSE);
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
		switch (cThis) {
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

char    *
getSimpleCommand(void)
/**************************************************************************
     purpose: returns a simple command e.g., \alpha\beta will return "\beta"
                                                   ^
 **************************************************************************/
{
	char            buffer[128];
	int             size;

	buffer[0] = getTexChar();

	if (buffer[0] != '\\')
		return NULL;

	for (size = 1; size < 127; size++) {
		buffer[size] = getRawTexChar();    /* \t \r '%' all end command */

		if (!isalpha((int)buffer[size])) {
			ungetTexChar(buffer[size]);
			break;
		}
	}

	buffer[size] = '\0';
	if (size == 127){
		diagnostics(WARNING, "Misplaced brace.");
		diagnostics(ERROR, "Cannot find close brace in 127 characters");
	}
	
	diagnostics(5, "getSimpleCommand result <%s>", buffer);
	return strdup(buffer);
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
		text = getDelimitedText('[',']',FALSE);
		diagnostics(5, "getBracketParam [%s]", text);

	} else {
		ungetTexChar(c);
		text = NULL;
		diagnostics(5, "getBracketParam []");
	}
	
	return text;
}

char           *
getBraceParam(void)
/**************************************************************************
     purpose: allocates and returns the next parameter in the LaTeX file
              Examples:  (^ indicates the current file position)
              
     \alpha\beta   --->  "\beta"             \bar \alpha   --->  "\alpha"
           ^                                     ^
     \bar{text}    --->  "text"              \bar text     --->  "t"
         ^                                       ^
	_\alpha        ---> "\alpha"             _{\alpha}     ---> "\alpha"
	 ^                                        ^
	_2             ---> "2"                  _{2}          ---> "2"
	 ^                                        ^
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
		text = getDelimitedText('{','}',FALSE);
	
	else {
		s[1] = '\0';
		text = strdup(s);
	}
		
	diagnostics(5, "Leaving getBraceParam {%s}", text);
	return text;
}

static void 
SaveFilePosition(void)
{
	if (g_parser_file)
		g_parser_file_pos = ftell(g_parser_file);
	else {
		diagnostics(3, "Saving current string pos %ld char='%c'",g_parser_string, *g_parser_string);
		g_parser_string_pos = g_parser_string;
	}
}

static void 
RestoreFilePosition(int offset)
{
	if (g_parser_file)
		fseek(g_parser_file, g_parser_file_pos+offset, SEEK_SET);
	else {
		diagnostics(3, "Restoring before pos %ld char='%c'",g_parser_string, *g_parser_string);
		g_parser_string = g_parser_string_pos+offset;
		diagnostics(3, "Restoring after  pos %ld char='%c'",g_parser_string, *g_parser_string);
	}
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
	
	diagnostics(3, "getTexUntil target = <%s>", target);
	
	while (j < len && i < 4095) {
	
		buffer[i] = (raw) ? getRawTexChar() : getTexChar();
		
		if (buffer[i] != target[j]) {
			if (j > 0) {			        /* false start, put back what was found */
				RestoreFilePosition(0);
				i-=j;
				j=0;
			}
		} else {
			if (j==0)
				SaveFilePosition();
			j++;
		}
		i++;
	}
	
	if (i == 4096) 
		diagnostics(ERROR, "Could not find <%s> in 4096 characters", target);
	
	buffer[i-len] = '\0';

	RestoreFilePosition(-1);	/* move to start of target */
	
	return strdup(buffer);
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

#define BUFFER_SIZE 32000

void
getSection(char **body, char **header)
/*
	purpose: obtain next chunk of latex file that has the same label
	
	along the way, beware of \verb and \begin{verbatim} ... \end{verbatim}
	this will make linenumbers less useful.  Aargh.
*/

{
	int possible_match, found;
	char cNext, *p, *s, buffer[BUFFER_SIZE],*text,*next_header;
	int i;
	int  match[9];
	char * command[9] = {"\\verb", "\\begin{verbatim}", "\\url", "\\begin{figure}",
	                     "\\section", "\\subsection", "\\subsubsection", 
	                     "\\chapter", "\\part"};
	int ncommands = 9;
	const int verb = 0;
	const int verbatim = 1;
	const int url = 2;
	const int figure = 3;
	int bs_count = 0;
	int index = 0;
	char * bufend = buffer + BUFFER_SIZE;

	text = NULL;			
	next_header = NULL;  /* typically becomes \subsection{Cows eat grass}  */
	*body=NULL;
	*header=NULL;
	
	for (p = buffer; p < bufend; p++) {
		
		*p = getTexChar();
		diagnostics(6,"char=%d %c",(int)*p,*p);
		
		if (*p == '\0') break;

		if (*p == '\\') {				/* begin search if backslash found */
			bs_count ++;
			if (bs_count % 2) {		/* avoid "\\section" and "\\\\section" */
				for(i=0; i<ncommands; i++) match[i]=TRUE;
				index = 1;
				continue;
			}
		} else 
			bs_count = 0;
		
		if (index == 0) continue;
			
		possible_match = FALSE;
		found = FALSE;
		
		for (i=0; i<ncommands; i++) {	/* check each command for match */
			if (!match[i]) continue;
				
			if (*p!=command[i][index]) {
				match[i] = FALSE;
/*				diagnostics(5,"index = %d, char = %c, failed to match %s, size=%d", \
				index,*p,command[i],strlen(command[i]));
*/				continue;
			}
			possible_match = TRUE;
			if (index == strlen(command[i])-1) {
				found = TRUE;
				break;
			}
		}
		
		if (found) {				/* make sure the next char is the right sort */
			diagnostics(5,"matched %s", command[i]);
			cNext = getTexChar();
			ungetTexChar(cNext);
			if (!(cNext == ' ' || cNext == '{' || (i==verb) || (i==url) || (i==verbatim && !isalpha(cNext)))) {
				possible_match = FALSE;
			}
		}
		
		if (!possible_match) {		/* no possible matches, reset and wait for next '\\'*/
			index = 0;
			continue;
		} else
			index++;
					
		if (!found) continue;
			
		if (i==verb || i==url) {				/* slurp \verb#text# */
			if (i==url && cNext=='{') cNext = '}';
			p++;
			*p++= getTexChar();
			while ((*p=getRawTexChar()) != '\0' && *p != cNext && p <bufend) p++;
			index = 0;				/* keep looking */
			continue;
		}

		if (i==verbatim || i==figure) {			/* slurp \begin{verbatim} ... \end{verbatim} */
			p++;
			if (i==verbatim)
				s=getTexUntil("\\end{verbatim}",TRUE);
			else if (i==figure)
				s=getTexUntil("\\end{figure}",TRUE);

			if (p+strlen(s)  >=bufend)
				diagnostics(ERROR, "out of buffer memory in getSection");
			strcpy(p,s);
			p += strlen(s);
			free(s);
			p--;
			index = 0;				/* keep looking */
			continue;
		} 
		
		/* actually found command to end the section */
		s = getBraceParam();
		next_header = malloc(strlen(command[i])+strlen(s)+3);
		strcpy(next_header, command[i]);
		strcpy(next_header+strlen(command[i]), "{");
		strcpy(next_header+strlen(command[i])+1,s);
		strcpy(next_header+strlen(command[i])+1+strlen(s),"}");
		free(s);
		p= p - strlen(command[i])+1;
		*p='\0';
		break;
	}
	text = strdup(buffer);
	*body = text;
	*header = next_header;
}
