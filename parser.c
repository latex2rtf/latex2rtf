/*  $Id: parser.c,v 1.37 2001/11/23 21:43:48 prahl Exp $

   Contains declarations for a generic recursive parser for LaTeX code.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "commands.h"
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
static int 				g_parser_include_level=0;

static long				g_parser_file_pos;
static char            *g_parser_string_pos;

static char             g_parser_currentChar;	/* Global current character */
static char             g_parser_lastChar;
static char             g_parser_penultimateChar;
static int				g_parser_backslashes;
static bool				g_parser_scan_ahead;

static void     parseBracket();

static void
SetScanAhead(int flag)
/***************************************************************************
 purpose:     allows getSection to avoid incrementing line number
****************************************************************************/
{
	g_parser_scan_ahead = flag;
}

int 
CurrentLineNumber(void) 
/***************************************************************************
 purpose:     returns the current line number of the text being processed
****************************************************************************/
{
	return g_parser_line;
}

char *
CurrentFileName(void)
/***************************************************************************
 purpose:     returns the filename of the text being processed
****************************************************************************/
{
	return g_parser_stack[g_parser_depth].file_name;
}

/*
	The following two routines allow parsing of multiple files and strings
*/

int 
PushSource(char * filename, char * string)
/***************************************************************************
 purpose:     change the source used by getRawTexChar() to either file or string
 			  pass NULL for unused argument
****************************************************************************/
{
	char       s[50];
	FILE *p    = NULL;
	char *name = NULL;
	int i;
	int line   = 1;
	
	if (0) {
		diagnostics(1,"Before PushSource** line=%d, g_parser_depth=%d, g_parser_include_level=%d",
		g_parser_line,g_parser_depth,g_parser_include_level);
		for (i=0; i<=g_parser_depth; i++) {
			if (g_parser_stack[i].file)
				diagnostics(1,"i=%d file   =%s, line=%d", i, g_parser_stack[i].file_name,
					g_parser_stack[i].file_line);

			else {
				strncpy(s,g_parser_stack[i].string,25);
				diagnostics(1,"i=%d string =%s, line=%d", i, s, g_parser_stack[i].file_line);
			}
		}
	}
	
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
       g_parser_include_level++;
       g_parser_line=1;
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
/*
	g_parser_line = line;
*/
	if (g_parser_file) {
		diagnostics(5, "Opening Source File %s", g_parser_stack[g_parser_depth].file_name);
	}else {
		strncpy(s,g_parser_string,25);
		diagnostics(5, "Opening Source string <%s>",s);
	}

	if (0) {
		diagnostics(1,"After PushSource** line=%d, g_parser_depth=%d, g_parser_include_level=%d",
		g_parser_line,g_parser_depth,g_parser_include_level);
		for (i=0; i<=g_parser_depth; i++) {
			if (g_parser_stack[i].file)
				diagnostics(1,"i=%d file   =%s, line=%d", i, g_parser_stack[i].file_name,
					g_parser_stack[i].file_line);

			else {
				strncpy(s,g_parser_stack[i].string,25);
				diagnostics(1,"i=%d string =%s, line=%d", i, s, g_parser_stack[i].file_line);
			}
		}
	}
	return 1;
}

int 
StillSource(void)
/***************************************************************************
 purpose:     figure out if text remains to be processed
****************************************************************************/
{
	if (g_parser_file)
		return (!feof(g_parser_file));
	else
		return (*g_parser_string != '\0');
}

void 
PopSource(void)
/***************************************************************************
 purpose:     return to the previous source 
****************************************************************************/
{
	char       s[50];
	int i;

	if (0) {
		diagnostics(1,"Before PopSource** line=%d, g_parser_depth=%d, g_parser_include_level=%d",
		g_parser_line,g_parser_depth,g_parser_include_level);
		for (i=0; i<=g_parser_depth; i++) {
			if (g_parser_stack[i].file)
				diagnostics(1,"i=%d file   =%s, line=%d", i, g_parser_stack[i].file_name,
					g_parser_stack[i].file_line);

			else {
				strncpy(s,g_parser_stack[i].string,25);
				diagnostics(1,"i=%d string =%s, line=%d", i, s, g_parser_stack[i].file_line);
			}
		}
	}
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
		g_parser_include_level--;
	}
		
	g_parser_depth--;
	
	if (g_parser_depth >= 0) {
		g_parser_string = g_parser_stack[g_parser_depth].string;
		g_parser_file   = g_parser_stack[g_parser_depth].file;
	}

	if (g_parser_file && 0) {
		g_parser_line   = g_parser_stack[g_parser_depth].file_line;
	}

	if (g_parser_file)
		diagnostics(5, "Resuming Source File %s", g_parser_stack[g_parser_depth].file_name);
	else {
		strncpy(s,g_parser_string,25);
		diagnostics(5, "Resuming Source string <%s>",s);
	}

	if (0) {
		diagnostics(1,"After PopSource** line=%d, g_parser_depth=%d, g_parser_include_level=%d",
		g_parser_line,g_parser_depth,g_parser_include_level);
		for (i=0; i<=g_parser_depth; i++) {
			if (g_parser_stack[i].file)
				diagnostics(1,"i=%d file   =%s, line=%d", i, g_parser_stack[i].file_name,
					g_parser_stack[i].file_line);

			else {
				strncpy(s,g_parser_stack[i].string,25);
				diagnostics(1,"i=%d string =%s, line=%d", i, s, g_parser_stack[i].file_line);
			}
		}
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
			else if (g_parser_include_level > 1) {
				PopSource();				/* go back to parsing parent */
				thechar = getRawTexChar();  /* get next char from parent file */
			} else
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

	} else {							/* no need to sanitize strings! */
		if (g_parser_string && *g_parser_string) {
			g_parser_currentChar = *g_parser_string;
			g_parser_string++;
		} else 
			g_parser_currentChar = '\0';
	}

	if (g_parser_currentChar == '\n' && !g_parser_scan_ahead) 
			g_parser_line++;
	
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
		
	} else {
		g_parser_string--; 
		if (g_parser_string && *g_parser_string) {
			*g_parser_string = c;
		}
	}
	
	if (c == '\n' && !g_parser_scan_ahead)
		g_parser_line--;

	g_parser_currentChar = g_parser_lastChar;
	g_parser_lastChar = g_parser_penultimateChar;
	g_parser_penultimateChar = '\0';	/* no longer know what that it was */
	g_parser_backslashes = 0;
	diagnostics(6,"after ungetTexChar=<%c> backslashes=%d line=%ld",c,g_parser_backslashes,g_parser_line);
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

	cThis = getRawTexChar();
	while (cThis == '%' && g_parser_backslashes % 2 == 0) {
		skipToEOL();
		g_parser_penultimateChar = cSave2;
		g_parser_lastChar = cSave;
		cThis = getRawTexChar();
	}
	
	if (cThis == '\\') 
		g_parser_backslashes++;
	else
		g_parser_backslashes=0;
	diagnostics(6,"after getTexChar=<%c> backslashes=%d line=%d",cThis,
	              g_parser_backslashes,g_parser_line);
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
	while ((cThis=getRawTexChar()) && cThis != '\n');
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
	if (g_parser_file) {
		diagnostics(3, "Saving current file pos %ld",ftell(g_parser_file));
		g_parser_file_pos = ftell(g_parser_file);
	} else {
		diagnostics(3, "Saving current string pos %ld char='%c'",g_parser_string, *g_parser_string);
		g_parser_string_pos = g_parser_string;
	}
}

static void 
RestoreFilePosition(int offset)
{
	if (g_parser_file) {
		diagnostics(3, "Restoring before pos %ld",ftell(g_parser_file));
		fseek(g_parser_file, g_parser_file_pos+offset, SEEK_SET);
		diagnostics(3, "Restoring after  pos %ld",ftell(g_parser_file));
	} else {
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
	char            *s, buffer[4096];
	int             i   = 0;                /* size of string that has been read */
	int             j   = 0;                /* number of found characters */
	bool			end_of_file_reached = FALSE;
	int             len = strlen(target);

	SetScanAhead(TRUE);
	diagnostics(3, "getTexUntil target = <%s> raw_search = %d ", target, raw);
	
	while (j < len && i < 4095) {
	
		buffer[i] = (raw) ? getRawTexChar() : getTexChar();
		
		if (!buffer[i]) {
			end_of_file_reached = TRUE;
			break;
		}
		
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
	
	if (!end_of_file_reached) {
		buffer[i-len] = '\0';
		RestoreFilePosition(-1);	/* move to start of target */
	}
	
	SetScanAhead(FALSE);
	s = strdup(buffer);
	diagnostics(6,"strdup result = %s",s);
	return s;
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

#define SECTION_BUFFER_SIZE 2048
static char* section_buffer = NULL;
static long section_buffer_size = SECTION_BUFFER_SIZE;

static void increase_buffer_size(void)
{
	char * new_section_buffer;
	
	new_section_buffer = malloc(2*section_buffer_size+1);
	if (new_section_buffer == NULL)
		diagnostics(ERROR, "Could not allocate enough memory to process file. Sorry.");
	memcpy(new_section_buffer, section_buffer, section_buffer_size);
	section_buffer_size *=2;
	free(section_buffer);
	section_buffer = new_section_buffer;
	diagnostics(4, "Expanded buffer size is now %ld", section_buffer_size);
}

void
getSection(char **body, char **header, char **label)
/**************************************************************************
	purpose: obtain the next section of the latex file
	
	This is now the preparsing routine.  Ideally, macro expansion would
	occur here as well.  The whole reason for this routine was to properly
	handle \label for a sections.  This routine reads text until a new
	section heading is found.  The text is returned in body and the *next*
	header is returned in header.  If header is NULL then there is no next
	header.  
	
	\include{file} is handled in this section here as well.
**************************************************************************/
{
	int possible_match, found;
	char cNext, *s,*text,*next_header;
	int i;
	long delta;
	int  match[19];
	char * command[19] = {"\\begin{verbatim}", "\\begin{figure}", "\\begin{equation}", 
						  "\\begin{eqnarray}", "\\begin{table}", "\\begin{description}",
	                     "\\part", "\\chapter", "\\section", "\\subsection", "\\subsubsection", 
	                     "\\section*", "\\subsection*", "\\subsubsection*", 
	                     "\\label", "\\input", "\\include", "\\verb", "\\url" };

	char * ecommand[6] = {"\\end{verbatim}", "\\end{figure}", "\\end{equation}", 
						  "\\end{eqnarray}", "\\end{table}", "\\end{description}"};
	int ncommands = 19;
	int ecommands = 6;

	const int label_item   = 14;
	const int input_item   = 15;
	const int include_item = 16;
	const int verb_item    = 17;
	const int url_item     = 18;

	int bs_count = 0;
	int index = 0;
	
	if (section_buffer == NULL) {
		section_buffer = malloc(section_buffer_size+1);
		if (section_buffer == NULL)
			diagnostics(ERROR, "Could not allocate enough memory to process file. Sorry.");
	}
	
	text = NULL;			
	next_header = NULL;  /* typically becomes \subsection{Cows eat grass}  */
	*body=NULL;
	*header=NULL;
	*label=NULL;
	
	SetScanAhead(TRUE);
	for (delta=0;;delta++) {
	
		if (delta+2 >= section_buffer_size) increase_buffer_size();
		
		*(section_buffer+delta) = getRawTexChar();
		if (*(section_buffer+delta)=='\0')
			diagnostics(2,"char=\\0");
		else if (*(section_buffer+delta)=='\n')
			diagnostics(2,"char=\\n");
		else
			diagnostics(2,"char=%d %c",(int)*(section_buffer+delta),*(section_buffer+delta));
		
		if (*(section_buffer+delta) == '\0') break;
		
		if (*(section_buffer+delta) == '%' && bs_count % 2) {	/* slurp TeX comments */
			delta++;
			while ((cNext=getRawTexChar()) != '\n') {
				if (delta+2 >= section_buffer_size) increase_buffer_size();
				*(section_buffer+delta) = cNext;
				delta++;
			}
			*(section_buffer+delta) = cNext;
		}

		if (*(section_buffer+delta) == '\\') {				/* begin search if backslash found */
			bs_count++;
			if (bs_count % 2) {		/* avoid "\\section" and "\\\\section" */
				for(i=0; i<ncommands; i++) match[i]=TRUE;
				index = 1;
				continue;
			}
		} else 
			bs_count = 0;
		
		if (index == 0) continue;
			
		possible_match = FALSE;	
		for (i=0; i<ncommands; i++) {	/* test each command for match */
			if (!match[i]) continue;
				
			if (*(section_buffer+delta)!=command[i][index]) {
				match[i] = FALSE;
/*				diagnostics(2,"index = %d, char = %c, failed to match %s, size=%d", \
				index,*p,command[i],strlen(command[i]));
*/				continue;
			}
			possible_match = TRUE;
		}

		found = FALSE;
		for (i=0; i<ncommands; i++) {	/* discover any exact matches */
			if (!match[i]) continue;
			if (index+1 == strlen(command[i])) {
				found = TRUE;
				break;
			}
		}
		
		if (found) {				/* make sure the next char is the right sort */
			diagnostics(5, "matched %s", command[i]);
			cNext = getRawTexChar();
			ungetTexChar(cNext);

			if (i >= ecommands && i <= include_item && cNext != ' ' && cNext != '{') {
				found = FALSE;
				match[i] = FALSE;
				diagnostics(5, "oops! did not match %s", command[i]);
			}
		}
		
		if (!possible_match) {		/* no possible matches, reset and wait for next '\\'*/
			index = 0;
			continue;
		} else
			index++;
					
		if (!found) continue;
			
		if (i==verb_item || i==url_item) {				/* slurp \verb#text# */
			if (i==url_item && cNext=='{') cNext = '}';
			delta++;
			*(section_buffer+delta)= getRawTexChar();
			delta++;
			while ((*(section_buffer+delta)=getRawTexChar()) != '\0' && *(section_buffer+delta) != cNext) {
				delta++;
				if (delta>=section_buffer_size) increase_buffer_size();
			}
			index = 0;				/* keep looking */
			continue;
		}

		if (i==input_item || i==include_item) {
			char *s, *s2;
			
			s = getBraceParam();
			if (i==input_item) 
				diagnostics(4,"\\input{%s}",s);
			else
				diagnostics(4,"\\include{%s}",s);
			
			if (strstr(s, "german.sty") != NULL) {
				GermanMode = TRUE;
				PushEnvironment(GERMAN_MODE);

			} else if (strstr(s, "french.sty") != NULL) {
				FrenchMode = TRUE;
				PushEnvironment(FRENCH_MODE);
				
			} else if (strcmp(s, "") == 0) {
				diagnostics(WARNING, "Empty or invalid filename in \\include{}");

			} else {
			
				if (strstr(s, ".tex") == NULL) {
					/* extension .tex is appended automatically if missing*/
					s2 = strdup_together(s, ".tex");
					free(s);
					s = s2;
				}
	
				/* Classic MacOS because of how DropUnix handles directories */
				#ifdef __MWERKS__
					{
						char            fullpath[1024];
						char           *dp;
						strcpy(fullpath, latexname);
						dp = strrchr(fullpath, ':');
						if (dp != NULL) {
							dp++;
							*dp = '\0';
						} else
							strcpy(fullpath, "");
						s2 = strdup_together(fullpath, s);
						free(s);
						s = s2;
					}
				#endif
				
				PushSource(s, NULL);
			}
			delta -=  (i==input_item) ? 6 : 8;  /* remove \input or \include */
			free(s);
			index = 0;				/* keep looking */
			continue;
		}
		
		if (i==label_item) {
			s = getBraceParam();
			diagnostics(4,"\\label{%s}",s);

			/* append \label{tag} to the buffer */
			delta++;
			*(section_buffer+delta) = '{';
			while (delta+strlen(s)+1 >= section_buffer_size) increase_buffer_size();
			strcpy(section_buffer+delta+1,s);
			delta += strlen(s)+1;
			*(section_buffer+delta) = '}';

			if (!(*label) && strlen(s)) 
				*label = strdup_nobadchars(s);
				
			free(s);
			index = 0;				/* keep looking */
			continue;
		}
		
		if (i<ecommands) {			/* slurp environment to avoid \labels within */
			delta++;
			s=getTexUntil(ecommand[i],TRUE);

			while (delta+strlen(s)+1 >= section_buffer_size)
				increase_buffer_size();

			strcpy(section_buffer+delta,s);
			delta += strlen(s)-1;
			free(s);
			index = 0;				/* keep looking */
			continue;
		} 
		
		/* actually found command to end the section */
		diagnostics(2,"getSection found command to end section");
		s = getBraceParam();
		next_header = malloc(strlen(command[i])+strlen(s)+3);
		strcpy(next_header, command[i]);
		strcpy(next_header+strlen(command[i]), "{");
		strcpy(next_header+strlen(command[i])+1,s);
		strcpy(next_header+strlen(command[i])+1+strlen(s),"}");
		free(s);
		delta -= strlen(command[i])-1;
		*(section_buffer+delta)='\0';
		break;
	}
	text = strdup(section_buffer);
	*body = text;
	*header = next_header;
	SetScanAhead(FALSE);
}
