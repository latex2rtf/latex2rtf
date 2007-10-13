
/* parser.c - parser for LaTeX code

Copyright (C) 1998-2002 The Free Software Foundation

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

This file is available from http://sourceforge.net/projects/latex2rtf/
 
Authors:
    1998-2000 Georg Lehner
    2001-2002 Scott Prahl
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "commands.h"
#include "cfg.h"
#include "stack.h"
#include "utils.h"
#include "parser.h"
#include "fonts.h"
#include "lengths.h"
#include "definitions.h"
#include "funct1.h"

typedef struct InputStackType {
    char *string;
    char *string_start;
    FILE *file;
    char *file_name;
    long file_line;
} InputStackType;

#define PARSER_SOURCE_MAX 100

static InputStackType g_parser_stack[PARSER_SOURCE_MAX];

static int g_parser_depth = -1;
static char *g_parser_string = "stdin";
static FILE *g_parser_file = NULL;
static int g_parser_line = 1;
static int g_parser_include_level = 0;

static char g_parser_currentChar;   /* Global current character */
static char g_parser_lastChar;
static char g_parser_penultimateChar;
static int g_parser_backslashes;

#define TRACK_LINE_NUMBER_MAX 10
static int g_track_line_number_stack[TRACK_LINE_NUMBER_MAX];
static int g_track_line_number = -1;

static void parseBracket();

void PushTrackLineNumber(int flag)

/***************************************************************************
 purpose:    set whether or not line numbers should be tracked in LaTeX source file
****************************************************************************/
{
    if (g_track_line_number >= TRACK_LINE_NUMBER_MAX)
        diagnostics(ERROR, "scan ahead stack too large! Sorry.");

    g_track_line_number++;
    g_track_line_number_stack[g_track_line_number] = flag;
}

void PopTrackLineNumber(void)

/***************************************************************************
 purpose:    restore last state of line numbers tracking in LaTeX source file
****************************************************************************/
{
    if (g_track_line_number < 0)
        diagnostics(ERROR, "scan ahead stack too small! Sorry.");

    g_track_line_number--;
}

/***************************************************************************
 purpose:     returns the current line number of the text being processed
****************************************************************************/
int CurrentLineNumber(void)
{
    return g_parser_line;
}

void UpdateLineNumber(char *s)

/***************************************************************************
 purpose:    advances the line number for each '\n' in s
****************************************************************************/
{
    if (s == NULL)
        return;

    while (*s != '\0') {
        if (*s == '\n')
            g_parser_line++;
        s++;
    }
}

/***************************************************************************
 purpose:     returns the current file descriptor
****************************************************************************/
int CurrentFileDescriptor(void)
{
    int fd=0;
    if (g_parser_file)
    	fd = fileno(g_parser_file);
    
    return fd;
}

char *CurrentFileName(void)

/***************************************************************************
 purpose:     returns the filename of the text being processed
****************************************************************************/
{
    char *s = "(Not set)";

    if (g_parser_stack[g_parser_depth].file_name)
        return g_parser_stack[g_parser_depth].file_name;
    else
        return s;
}

/*
	The following two routines allow parsing of multiple files and strings
*/

int PushSource(char *filename, char *string)

/***************************************************************************
 purpose:     change the source used by getRawTexChar() to either file or string
 			  --> pass NULL for unused argument (both NULL means use stdin)
 			  --> PushSource duplicates string
****************************************************************************/
{
    char s[50];
    FILE *p = NULL;
    char *name = NULL;
    int i;
    int line = 1;

    if (0) {
        diagnostics(1, "Before PushSource** line=%d, g_parser_depth=%d, g_parser_include_level=%d",
          g_parser_line, g_parser_depth, g_parser_include_level);
        for (i = 0; i <= g_parser_depth; i++) {
            if (g_parser_stack[i].file)
                diagnostics(1, "i=%d file   =%s, line=%d", i, g_parser_stack[i].file_name, g_parser_stack[i].file_line);

            else {
                strncpy_printable(s, g_parser_stack[i].string, 25);
                diagnostics(1, "i=%d string =%s, line=%d", i, s, g_parser_stack[i].file_line);
            }
        }
    }

    /* save current values for linenumber and string */
    if (g_parser_depth >= 0) {
        g_parser_stack[g_parser_depth].file_line = g_parser_line;
        g_parser_stack[g_parser_depth].string = g_parser_string;
    }

    /* first test to see if we should use stdin */
    if ((filename == NULL || strcmp(filename, "-") == 0) && string == NULL) {
        g_parser_include_level++;
        g_parser_line = 1;
        name = strdup("stdin");
        p = stdin;

        /* if not then try to open a file */
    } else if (filename) {
        p = my_fopen(filename, "rb");
        if (p == NULL)
            return 1;
        g_parser_include_level++;
        g_parser_line = 1;
        name = strdup(filename);

    } else {
        name = CurrentFileName();
        line = CurrentLineNumber();
    }

    g_parser_depth++;

    if (g_parser_depth >= PARSER_SOURCE_MAX)
        diagnostics(ERROR, "More than %d PushSource() calls", (int) PARSER_SOURCE_MAX);

    g_parser_string = (string) ? strdup(string) : NULL;
    g_parser_stack[g_parser_depth].string = g_parser_string;
    g_parser_stack[g_parser_depth].string_start = g_parser_string;
    g_parser_stack[g_parser_depth].file = p;
    g_parser_stack[g_parser_depth].file_line = line;
    g_parser_stack[g_parser_depth].file_name = name;
    g_parser_file = p;
    g_parser_string = g_parser_stack[g_parser_depth].string;

    if (g_parser_file) {
        diagnostics(5, "Opening Source File %s", g_parser_stack[g_parser_depth].file_name);
    } else {
        strncpy(s, g_parser_string, 25);
        diagnostics(5, "Opening Source string <%s>", s);
    }

    if (0) {
        diagnostics(1, "After PushSource** line=%d, g_parser_depth=%d, g_parser_include_level=%d",
          g_parser_line, g_parser_depth, g_parser_include_level);
        for (i = 0; i <= g_parser_depth; i++) {
            if (g_parser_stack[i].file)
                diagnostics(1, "i=%d file   =%s, line=%d", i, g_parser_stack[i].file_name, g_parser_stack[i].file_line);

            else {
                strncpy_printable(s, g_parser_stack[i].string, 25);
                diagnostics(1, "i=%d string =%s, line=%d", i, s, g_parser_stack[i].file_line);
            }
        }
    }
    return 0;
}

int StillSource(void)

/***************************************************************************
 purpose:     figure out if text remains to be processed
****************************************************************************/
{
    if (g_parser_file)
        return (!feof(g_parser_file));
    else
        return (*g_parser_string != '\0');
}

void EndSource(void)
{
    if (g_parser_file)
        fseek(g_parser_file, 0, SEEK_END);
    else
        *g_parser_string = '\0';

    return;
}

void PopSource(void)

/***************************************************************************
 purpose:     return to the previous source 
****************************************************************************/
{
    char s[50];
    int i;

    if (g_parser_depth < 0)
        diagnostics(ERROR, "More PopSource() calls than PushSource() ");

    if (0) {
        diagnostics(1, "Before PopSource** line=%d, g_parser_depth=%d, g_parser_include_level=%d",
          g_parser_line, g_parser_depth, g_parser_include_level);
        for (i = 0; i <= g_parser_depth; i++) {
            if (g_parser_stack[i].file)
                diagnostics(1, "i=%d file   =%s, line=%d", i, g_parser_stack[i].file_name, g_parser_stack[i].file_line);

            else {
                strncpy_printable(s, g_parser_stack[i].string, 25);
                diagnostics(1, "i=%d string =%s, line=%d", i, s, g_parser_stack[i].file_line);
            }
        }
    }

    if (g_parser_file) {
        diagnostics(5, "Closing Source File %s", g_parser_stack[g_parser_depth].file_name);
        fclose(g_parser_file);
        free(g_parser_stack[g_parser_depth].file_name);
        g_parser_stack[g_parser_depth].file_name = NULL;
        g_parser_include_level--;
    }

    if (g_parser_string) {
        if (strlen(g_parser_stack[g_parser_depth].string_start) < 49)
            strcpy(s, g_parser_stack[g_parser_depth].string_start);
        else {
            strncpy(s, g_parser_stack[g_parser_depth].string_start, 49);
            s[49] = '\0';
        }

        diagnostics(5, "Closing Source string <%s>", s);
        free(g_parser_stack[g_parser_depth].string_start);
        g_parser_stack[g_parser_depth].string_start = NULL;
    }

    g_parser_depth--;

    if (g_parser_depth >= 0) {
        g_parser_string = g_parser_stack[g_parser_depth].string;
        g_parser_file = g_parser_stack[g_parser_depth].file;
    }

    if (g_parser_file && 0) {
        g_parser_line = g_parser_stack[g_parser_depth].file_line;
    }

    if (g_parser_file)
        diagnostics(5, "Resuming Source File %s", g_parser_stack[g_parser_depth].file_name);
    else {
        strncpy(s, g_parser_string, 25);
        diagnostics(5, "Resuming Source string <%s>", s);
    }

    if (0) {
        diagnostics(1, "After PopSource** line=%d, g_parser_depth=%d, g_parser_include_level=%d",
          g_parser_line, g_parser_depth, g_parser_include_level);
        for (i = 0; i <= g_parser_depth; i++) {
            if (g_parser_stack[i].file)
                diagnostics(1, "i=%d file   =%s, line=%d", i, g_parser_stack[i].file_name, g_parser_stack[i].file_line);

            else {
                strncpy_printable(s, g_parser_stack[i].string, 25);
                diagnostics(1, "i=%d string =%s, line=%d", i, s, g_parser_stack[i].file_line);
            }
        }
    }
}

#define CR (char) 0x0d
#define LF (char) 0x0a

char getRawTexChar()

/***************************************************************************
 purpose:     get the next character from the input stream with minimal
              filtering  (CRLF or CR or LF ->  \n) and '\t' -> ' '
			  it also keeps track of the line number
              should only be used by \verb and \verbatim and getTexChar()
****************************************************************************/
{
    int thechar;

    if (g_parser_file) {
        thechar = getc(g_parser_file);
        if (thechar == EOF)
            if (!feof(g_parser_file))
                diagnostics(ERROR, "Unknown file I/O error reading latex file\n");
            else if (g_parser_include_level > 1) {
                PopSource();    /* go back to parsing parent */
                thechar = getRawTexChar();  /* get next char from parent file */
            } else
                thechar = '\0';
        else if (thechar == CR) {   /* convert CR, CRLF, or LF to \n */
            thechar = getc(g_parser_file);
            if (thechar != LF && !feof(g_parser_file))
                ungetc(thechar, g_parser_file);
            thechar = '\n';
        } else if (thechar == LF)
            thechar = '\n';
        else if (thechar == '\t')
            thechar = ' ';

        g_parser_currentChar = (char) thechar;

    } else {

        if (g_parser_string && *g_parser_string) {
			thechar = *g_parser_string;

			/* convert CR, CRLF, or LF to \n */			
			if (thechar == CR) {   
				g_parser_string++;
				thechar = *g_parser_string;
				if (thechar != LF)
					g_parser_string--;
				thechar = '\n';
			} else if (thechar == LF)
				thechar = '\n';
			else if (thechar == '\t')
            	thechar = ' ';
            	
            g_parser_currentChar = thechar;
            g_parser_string++;
        } 
        else if (g_parser_depth > 15) 
        {
             PopSource();    /* go back to parsing parent */
             g_parser_currentChar = getRawTexChar();  /* get next char from parent file */
        } else
            g_parser_currentChar = '\0';
    }

    if (g_parser_currentChar == '\n' && g_track_line_number_stack[g_track_line_number])
        g_parser_line++;

    g_parser_penultimateChar = g_parser_lastChar;
    g_parser_lastChar = g_parser_currentChar;
    if (0) {
		if (g_parser_currentChar=='\n')
			diagnostics(5,"getRawTexChar = <\\n>");
		else if (g_parser_currentChar=='\0')
			diagnostics(5,"getRawTexChar = <\\0> depth=%d, files=%d", g_parser_depth, g_parser_include_level);
		else
			diagnostics(5,"getRawTexChar = <%2c>",g_parser_currentChar);
	}
	/* if (g_parser_currentChar=='\0') exit(0);*/
    return g_parser_currentChar;
}

#undef CR
#undef LF

void ungetTexChar(char c)

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

    if (c == '\n' && g_track_line_number_stack[g_track_line_number])
        g_parser_line--;

    g_parser_currentChar = g_parser_lastChar;
    g_parser_lastChar = g_parser_penultimateChar;
    g_parser_penultimateChar = '\0';    /* no longer know what that it was */
    g_parser_backslashes = 0;
    diagnostics(5, "after ungetTexChar=<%c> backslashes=%d line=%ld", c, g_parser_backslashes, g_parser_line);
}

char getTexChar()

/***************************************************************************
 purpose:     get the next character from the input stream
              This should be the usual place to access the LaTeX file
			  It filters the input stream so that % is handled properly
****************************************************************************/
{
    char cThis;
    char cSave = g_parser_lastChar;
    char cSave2 = g_parser_penultimateChar;

    cThis = getRawTexChar();
    while (cThis == '%' && even(g_parser_backslashes)) {
        skipToEOL();
        g_parser_penultimateChar = cSave2;
        g_parser_lastChar = cSave;
        cThis = getRawTexChar();
    }

    if (cThis == '\\')
        g_parser_backslashes++;
    else
        g_parser_backslashes = 0;
	if (0) {
		if (cThis=='\n')
			diagnostics(6,"getRawTexChar = <\\n> backslashes=%d line=%ld", g_parser_backslashes, g_parser_line);
		else if (cThis=='\0')
			diagnostics(6,"getRawTexChar = <\\0> backslashes=%d line=%ld", g_parser_backslashes, g_parser_line);
		else
			diagnostics(6,"getRawTexChar = <%2c> backslashes=%d line=%ld",cThis, g_parser_backslashes, g_parser_line);
	}
    return cThis;
}

void skipToEOL(void)

/****************************************************************************
purpose: ignores anything from inputfile until the end of line.  
         uses getRawTexChar() because % are not important
 ****************************************************************************/
{
    char cThis;

    while ((cThis = getRawTexChar()) && cThis != '\n') {
    }
}

char getNonBlank(void)

/***************************************************************************
 Description: get the next non-blank character from the input stream
****************************************************************************/
{
    char c;

	c = getTexChar();
    while (c == ' ' || c == '\n') {
    	c = getTexChar();
    }
    return c;
}

char getNonSpace(void)

/***************************************************************************
 Description: get the next non-space character from the input stream
****************************************************************************/
{
    char c;

    while ((c = getTexChar()) && c == ' ') {
    }
    return c;
}

void skipSpaces(void)

/***************************************************************************
 Description: skip to the next non-space character from the input stream
****************************************************************************/
{
    char c;

    while ((c = getTexChar()) && c == ' ') {
    }
    ungetTexChar(c);
}

int getSameChar(char c)

/***************************************************************************
 Description: returns the number of characters that are the same as c
****************************************************************************/
{
    char cThis;
    int count = -1;

    do {
        cThis = getTexChar();
        count++;
    } while (cThis == c);

    ungetTexChar(cThis);

    return count;
}

char *getDelimitedText(char left, char right, bool raw)

/******************************************************************************
  purpose: general scanning routine that allocates and returns a string
  		   that is between "left" and "right" that accounts for escaping by '\'
  		   
  		   Example for getDelimitedText('{','}',TRUE) 
  		   
  		   "{the \{ is shown {\it by} a\\}" ----> "the \{ is shown {\it by} a\\"
  		    
  		    Note the missing opening brace in the example above
 ******************************************************************************/
{
    char buffer[5000];
    int size = -1;
    int lefts_needed = 1;
    char marker = ' ';
    char last_char = ' ';

    while (lefts_needed && size < 4999) {

        size++;
        last_char = marker;
        buffer[size] = (raw) ? getRawTexChar() : getTexChar();
        marker = buffer[size];

        if (buffer[size] != right || last_char == '\\') {   /* avoid \} */
            if (buffer[size] == left && last_char != '\\')  /* avoid \{ */
                lefts_needed++;
            else {
                if (buffer[size] == '\\' && last_char == '\\')  /* avoid \\} */
                    marker = ' ';
            }
        } else
            lefts_needed--;
    }

    buffer[size] = '\0';        /* overwrite final delimeter */
    if (size == 4999)
        diagnostics(ERROR, "Misplaced '%c' (Not found within 5000 chars)");

    return strdup(buffer);
}

void parseBrace(void)

/****************************************************************************
  Description: Skip text to balancing close brace                          
 ****************************************************************************/
{
    char *s = getDelimitedText('{', '}', FALSE);

    free(s);
}

void parseBracket(void)

/****************************************************************************
  Description: Skip text to balancing close bracket
 ****************************************************************************/
{
    char *s = getDelimitedText('[', ']', FALSE);

    free(s);
}

void CmdIgnoreParameter(int code)

/****************************************************************************
   Description: Ignore the parameters of a command 
   Example    : CmdIgnoreParameter(21) for \command[opt1]{reg1}{reg2}

   code is a decimal # of the form "op" where `o' is the number of
   optional parameters (0-9) and `p' is the # of required parameters.    
                                                
   The specified number of parameters is ignored.  The order of the parameters
   in the LaTeX file does not matter.                      
****************************************************************************/
{
    int optParmCount = code / 10;
    int regParmCount = code % 10;
    char cThis;

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
                diagnostics(WARNING, "Ignored command missing {} expected %d - found %d", code % 10,
                  code % 10 - regParmCount);
                ungetTexChar(cThis);
                return;
        }
    }

    /* Check for trailing optional parameter e.g., \item[label] */

    if (optParmCount > 0) {
        cThis = getNonSpace();
        if (cThis == '[')
            parseBracket();
        else {
            ungetTexChar(cThis);
            return;
        }
    }
    return;
}

char *getSimpleCommand(void)

/**************************************************************************
     purpose: returns a simple command e.g., \alpha\beta will return "\beta"
                                                   ^
 **************************************************************************/
{
    char buffer[128];
    int size;

    buffer[0] = getTexChar();

    if (buffer[0] != '\\')
        return NULL;

    for (size = 1; size < 127; size++) {
        buffer[size] = getRawTexChar(); /* \t \r '%' all end command */

        if (!isalpha((int) buffer[size])) {
            ungetTexChar(buffer[size]);
            break;
        }
    }

    buffer[size] = '\0';
    if (size == 127) {
        diagnostics(WARNING, "Misplaced brace.");
        diagnostics(ERROR, "Cannot find close brace in 127 characters");
    }

    diagnostics(5, "getSimpleCommand result <%s>", buffer);
    return strdup(buffer);
}

char *getBracketParam(void)

/******************************************************************************
  purpose: return bracketed parameter
  			
  \item[1]   --->  "1"        \item[]   --->  ""        \item the  --->  NULL
       ^                           ^                         ^
  \item [1]  --->  "1"        \item []  --->  ""        \item  the --->  NULL
       ^                           ^                         ^
 ******************************************************************************/
{
    char c, *text;

    c = getNonBlank();
    PushTrackLineNumber(FALSE);

    if (c == '[') {
        text = getDelimitedText('[', ']', FALSE);
        diagnostics(5, "getBracketParam [%s]", text);

    } else {
        ungetTexChar(c);
        text = NULL;
        diagnostics(5, "getBracketParam []");
    }

    PopTrackLineNumber();
    return text;
}

char *getBraceParam(void)

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
    char s[2], *text;

    s[0] = getNonSpace();       /* skip spaces and one possible newline */
    if (s[0] == '\n')
        s[0] = getNonSpace();

    PushTrackLineNumber(FALSE);

    if (s[0] == '\\') {
        ungetTexChar(s[0]);
        text = getSimpleCommand();

    } else if (s[0] == '{')
        text = getDelimitedText('{', '}', FALSE);

    else {
        s[1] = '\0';
        text = strdup(s);
    }

    PopTrackLineNumber();
    diagnostics(5, "Leaving getBraceParam {%s}", text);
    return text;
}

char *getBeginEndParam(void)

/**************************************************************************
     purpose: allocates and returns the next parameter in the LaTeX file
              Examples:  (^ indicates the current file position)
              
     \beginning    --->  NULL                \begin \alpha --->  NULL
           ^                                       ^
     \begin{text}  --->  "text"              \begin { text } --->  "text"
           ^                                       ^
 **************************************************************************/
{
    char s, *text, *raw;

    s = getNonSpace();       /* skip spaces and one possible newline */
    if (s == '\n')
        s = getNonSpace();

    PushTrackLineNumber(FALSE);

    if (s != '{') {
        ungetTexChar(s);
        return NULL;
    }
    
    raw = getDelimitedText('{', '}', FALSE);
    text = strdup_noendblanks(raw);
    free(raw);

    PopTrackLineNumber();
    diagnostics(5, "Leaving getBeginEndParam {%s}", text);
    return text;
}

char *getLeftRightParam(void)

/**************************************************************************
     purpose: get text between \left ... \right
 **************************************************************************/
{
    char text[5000], s, *command;
    int i = 0;
    int lrdepth = 1;

    text[0] = '\0';

    for (;;) {
        s = getTexChar();
        if (s == '\\') {
            ungetTexChar(s);
            command = getSimpleCommand();
            if (strcmp(command, "\\right") == 0) {
                lrdepth--;
                if (lrdepth == 0) {
                    free(command);
                    return strdup(text);
                }
            }
            strcat(text + i, command);
            i += strlen(command);
            if (i > 4950)
                diagnostics(ERROR, "Contents of \\left .. \\right too large.");
            if (strcmp(command, "\\left") == 0)
                lrdepth++;
            free(command);
        } else {
            text[i] = s;
            i++;
            text[i] = '\0';
        }
    }
    return NULL;
}




char *getTexUntil(char *target, int raw)

/**************************************************************************
     purpose: returns the portion of the file to the beginning of target
     returns: NULL if not found
 **************************************************************************/
{
    enum { BUFFSIZE = 16000 };
    char *s;
    char buffer[BUFFSIZE];
    int last_i = -1;
    int i = 0;                  /* size of string that has been read */
    size_t j = 0;               /* number of found characters */
    bool end_of_file_reached = FALSE;
    size_t len = strlen(target);

    PushTrackLineNumber(FALSE);

    diagnostics(5, "getTexUntil target = <%s> raw_search = %d ", target, raw);

    while (j < len && i < BUFFSIZE) {

        if (i > last_i) {
            buffer[i] = (raw) ? getRawTexChar() : getTexChar();
            last_i = i;
            if (buffer[i] != '\n')
                diagnostics(7, "next char = <%c>, %d, %d, %d", buffer[i], i, j, last_i);
            else
                diagnostics(7, "next char = <\\n>");

        }

        if (buffer[i] == '\0') {
            end_of_file_reached = TRUE;
            diagnostics(7, "end of file reached");
            break;
        }

        if (buffer[i] != target[j]) {
            if (j > 0) {        /* false start, put back what was found */
                diagnostics(8, "failed to match target[%d]=<%c> != buffer[%d]=<%c>", j, target[j], i, buffer[i]);
                i -= j;
                j = 0;
            }
        } else
            j++;

        i++;
    }

    if (i == BUFFSIZE)
        diagnostics(ERROR, "Could not find <%s> in %d characters \n\
        Recompile with larger BUFFSIZE in getTexUntil() in parser.c", target, BUFFSIZE);

    if (!end_of_file_reached)   /* do not include target in returned string */
        buffer[i - len] = '\0';

    PopTrackLineNumber();

    diagnostics(3, "buffer size =[%d], actual=[%d]", strlen(buffer), i - len);

    s = strdup(buffer);
    diagnostics(3, "strdup result = %s", s);
    return s;
}

int getDimension(void)

/**************************************************************************
     purpose: reads a TeX dimension and returns size it twips
          eg: 3 in, -.013mm, 29 pc, + 42,1 dd, 1234sp
**************************************************************************/
{
    char cThis, buffer[20];
    int i = 0;
    float num;

    skipSpaces();

/* obtain optional sign */
    cThis = getTexChar();

/* skip "to" */
    if (cThis == 't') {
        cThis = getTexChar();
        cThis = getTexChar();
    }

/* skip "spread" */
    if (cThis == 's') {
        cThis = getTexChar();
        cThis = getTexChar();
        cThis = getTexChar();
        cThis = getTexChar();
        cThis = getTexChar();
        cThis = getTexChar();
    }

    if (cThis == '-' || cThis == '+') {
        buffer[i++] = cThis;
        skipSpaces();
        cThis = getTexChar();
    }

/* obtain number */
    if (cThis == '\\')
        buffer[i++] = '1';
    else {
        while (i < 19 && (isdigit((int) cThis) || cThis == '.' || cThis == ',')) {
            if (cThis == ',')
                cThis = '.';
            buffer[i++] = cThis;
            cThis = getTexChar();
        }
    }
    ungetTexChar(cThis);
    buffer[i] = '\0';
    diagnostics(4, "getDimension() raw number is <%s>", buffer);

    if (i == 19 || sscanf(buffer, "%f", &num) != 1) {
        diagnostics(WARNING, "Screwy number in TeX dimension");
        diagnostics(WARNING, "getDimension() number is <%s>", buffer);
        return 0;
    }

/*	num *= 2;                    convert pts to twips */

/* obtain unit of measure */
    skipSpaces();
    buffer[0] = tolower((int) getTexChar());

	if (buffer[0] == '\0')  /* no units specified ... assume points */
        return (int) (num * 20);
	
/* skip "true" */
    if (buffer[0] == 't') {
        cThis = getTexChar();
        cThis = getTexChar();
        cThis = getTexChar();
        skipSpaces();
        buffer[0] = tolower((int) getTexChar());
    }

    if (buffer[0] != '\\') {
        buffer[1] = tolower((int) getTexChar());
        buffer[2] = '\0';

        diagnostics(4, "getDimension() dimension is <%s>", buffer);
        if (strstr(buffer, "pt"))
            return (int) (num * 20);
        else if (strstr(buffer, "pc"))
            return (int) (num * 12 * 20);
        else if (strstr(buffer, "in"))
            return (int) (num * 72.27 * 20);
        else if (strstr(buffer, "bp"))
            return (int) (num * 72.27 / 72 * 20);
        else if (strstr(buffer, "cm"))
            return (int) (num * 72.27 / 2.54 * 20);
        else if (strstr(buffer, "mm"))
            return (int) (num * 72.27 / 25.4 * 20);
        else if (strstr(buffer, "dd"))
            return (int) (num * 1238.0 / 1157.0 * 20);
        else if (strstr(buffer, "dd"))
            return (int) (num * 1238.0 / 1157 * 20);
        else if (strstr(buffer, "cc"))
            return (int) (num * 1238.0 / 1157.0 * 12.0 * 20);
        else if (strstr(buffer, "sp"))
            return (int) (num / 65536.0 * 20);
        else if (strstr(buffer, "ex"))
            return (int) (num * CurrentFontSize() * 0.5);
        else if (strstr(buffer, "em"))
            return (int) (num * CurrentFontSize());
        else if (strstr(buffer, "in"))
            return (int) (num * 72.27 * 20);
        else {
            ungetTexChar(buffer[1]);
            ungetTexChar(buffer[0]);
            return (int) num;
        }
    } else {
        char *s, *t;

        ungetTexChar(buffer[0]);
        s = getSimpleCommand();
        t = s + 1;              /* skip initial backslash */
        diagnostics(4, "getDimension() dimension is <%s>", t);
        num *= getLength(t);
        free(s);
        return (int) num;
    }

}

void CmdInclude(int code)

/******************************************************************************
 purpose: handles \input file, \input{file}, \include{file}
          code == 0 for \include
          code == 1 for \input
 ******************************************************************************/
{
    char name[50], cNext;
    int i;
    char *basename=NULL;
    char *texname=NULL;

    cNext = getNonSpace();

    if (cNext == '{') {         /* \input{gnu} or \include{gnu} */
        ungetTexChar(cNext);
        basename = getBraceParam();

    } else {                    /* \input gnu */
        name[0] = cNext;
        for (i = 1; i < 50; i++) {
            name[i] = getTexChar();
            if (isspace((int) name[i])) {
                name[i] = '\0';
                break;
            }
        }
        basename = strdup(name);
    }

	if (strstr(basename, "german.sty") != NULL) {
    	GermanMode = TRUE;
     	PushEnvironment(GERMAN_MODE);
     	free(basename);
     	return;

    } else if (strstr(basename, "french.sty") != NULL) {
        FrenchMode = TRUE;
        PushEnvironment(FRENCH_MODE);
     	free(basename);
     	return;
	}
	
    if (basename && strstr(basename, ".tex") == NULL)         /* append .tex if missing */
        texname = strdup_together(basename, ".tex");

    if (texname && PushSource(texname, NULL) == 0)            /* Try the .tex name first*/
        diagnostics(WARNING, "Including file <%s>", texname);
      
    else if (basename && PushSource(basename, NULL) == 0)     /* Try the basename second*/
        diagnostics(WARNING, "Including file <%s>", basename);

    if (basename) free(basename);
    if (texname)  free(texname);
}

#define SECTION_BUFFER_SIZE 2048
static char *section_buffer = NULL;
static size_t section_buffer_size = SECTION_BUFFER_SIZE;
static size_t section_buffer_end = 0;

static void increase_buffer_size(void)
{
    char *new_section_buffer;

    new_section_buffer = malloc(2 * section_buffer_size + 1);
    if (new_section_buffer == NULL)
        diagnostics(ERROR, "Could not allocate enough memory to process file. Sorry.");
    memmove(new_section_buffer, section_buffer, section_buffer_size);
    section_buffer_size *= 2;
    free(section_buffer);
    section_buffer = new_section_buffer;
    diagnostics(4, "Expanded buffer size is now %ld", section_buffer_size);
}

static void add_chr_to_buffer(char c)
{
    if (section_buffer == NULL) {
        section_buffer = malloc(section_buffer_size + 1);
        if (section_buffer == NULL)
            diagnostics(ERROR, "Could not allocate enough memory to process file. Sorry.");
    }

	section_buffer_end++;
	*(section_buffer + section_buffer_end) = c;

	if (section_buffer_end + 2 >= section_buffer_size)
		increase_buffer_size();

	if (0) {
		if (c == '\0')
			fprintf(stderr, "[\\0]");
		else if (c == '\n')
			fprintf(stderr, "[\\n]");
		else
			fprintf(stderr, "[%c]", (int) c);
	}
}

static void add_str_to_buffer(const char *s)
{
	while (s && *s) {
		add_chr_to_buffer(*s);
		s++;
	}
}

static int matches_buffer_tail(const char *s)
{
	size_t len;
	return FALSE;
	if (s==NULL) return FALSE;
	
	len = strlen(s);
	if (len>section_buffer_end) return FALSE;
	
	if (strncmp(s,section_buffer+section_buffer_end-len,len))
		return TRUE;

	return FALSE;
}

static void reset_buffer(void)
{
	section_buffer_end = -1;
}

static void move_end_of_buffer(size_t n)
{
	section_buffer_end += n;

	if (0) {
		diagnostics(1,"last 5 characters are [%c][%c][%c][%c][%c]\n", 
		*(section_buffer+section_buffer_end-4),
		*(section_buffer+section_buffer_end-3),
		*(section_buffer+section_buffer_end-2),
		*(section_buffer+section_buffer_end-1),
		*(section_buffer+section_buffer_end)
		);
	}
}

void getSection(char **body, char **header, char **label)

/**************************************************************************
	purpose: obtain the next section of the latex file
	
	This is now a preparsing routine that breaks a file up into sections.  
	Macro expansion happens here as well.  \input and \include is also
	handled here.  The reason for this routine is allow \labels to refer
	to sections.  
	
	This routine reads text until a new section heading is found.  The text 
	is returned in body and the *next* header is returned in header.  If 
	no header follows then NULL is returned.
	
**************************************************************************/
{
    int any_possible_match, found;
    char cNext, cThis, *s, *text, *next_header, *str;
    int i;
    int possible_match[42];
    char *command[42] = { "",   /* 0 entry is for user definitions */
        "",                     /* 1 entry is for user environments */
        "\\begin{verbatim}", 
        "\\begin{figure}",      "\\begin{figure*}", 
        "\\begin{equation}",    "\\begin{equation*}",
        "\\begin{eqnarray}",    "\\begin{eqnarray*}",
        "\\begin{table}",       "\\begin{table*}",
        "\\begin{description}", "\\begin{comment}",
        "\\end{verbatim}", 
        "\\end{figure}",        "\\end{figure*}", 
        "\\end{equation}",      "\\end{equation*}",
        "\\end{eqnarray}",      "\\end{eqnarray*}",
        "\\end{table}",         "\\end{table*}",
        "\\end{description}",   "\\end{comment}",
        "\\part", "\\chapter",  "\\section", "\\subsection", "\\subsubsection",
        "\\section*", "\\subsection*", "\\subsubsection*",
        "\\label", "\\input", "\\include", "\\verb", "\\url",
        "\\newcommand", "\\def", "\\renewcommand", "\\endinput", "\\end{document}",
    };

    int ncommands = 42;

    const int b_verbatim_item = 2;
    const int b_figure_item = 3;
    const int b_figure_item2 = 4;
    const int b_equation_item = 5;
    const int b_equation_item2 = 6;
    const int b_eqnarray_item = 7;
    const int b_eqnarray_item2 = 8;
    const int b_table_item = 9;
    const int b_table_item2 = 10;
    const int b_description_item = 11;
    const int b_comment_item = 12;
    const int e_verbatim_item = 13;
    const int e_figure_item = 14;
    const int e_figure_item2 = 15;
    const int e_equation_item = 16;
    const int e_equation_item2 = 17;
    const int e_eqnarray_item = 18;
    const int e_eqnarray_item2 = 19;
    const int e_table_item = 20;
    const int e_table_item2 = 21;
    
    const int e_description_item = 22;
    const int e_comment_item = 23;

    const int label_item = 32;
    const int input_item = 33;
    const int include_item = 34;
    
    const int verb_item = 35;
    const int url_item = 36;
    const int new_item = 37;
    const int def_item = 38;
    const int renew_item = 39;
    const int endinput_item = 40;
    const int e_document_item = 41;

    int bs_count = 0;          /* number of backslashes encountered in a row */
    size_t cmd_pos = 0;        /* position of start of command relative to end of buffer */
    int label_depth = 0;
	int i_match = 0;
	
    text = NULL;
    next_header = NULL;         /* typically becomes \subsection{Cows eat grass} */
    *body = NULL;
    *header = NULL;
    *label = NULL;

    PushTrackLineNumber(FALSE);
    reset_buffer();
    while (1) {

        cThis = getRawTexChar();
        while (cThis == '\0' && g_parser_depth > 0) {
            PopSource();
            cThis = getRawTexChar();
        }

        if (cThis == '\0')
            diagnostics(5, "[%ld] xchar=000 '\\0' (backslash count=%d)", section_buffer_end, bs_count);
        else if (cThis == '\n')
            diagnostics(5, "[%ld] xchar=012 '\\n' (backslash count=%d)", section_buffer_end, bs_count);
        else
            diagnostics(5, "[%ld] xchar=%03d '%c' (backslash count=%d)", section_buffer_end, (int) cThis, cThis, bs_count);

        add_chr_to_buffer(cThis);

        if (cThis == '\0') break;

        /* slurp TeX comments but retain stuff following %latex2rtf: */
        /* if we did not have the %latex2rtf: feature then we could just */
        /* discard the entire line following a '%'.  Instead we must collect */
        /* the line and then see if there is a %latex2rtf:, if there is not */
        /* then just discard the entire line, otherwise just the %latex2rtf: */
        if (cThis == '%' && even(bs_count)) {   
            int n = 1;
            do {
            	cNext = getRawTexChar();
        		add_chr_to_buffer(cNext);
            	n++;
            	if (matches_buffer_tail(InterpretCommentString)) {
            		n = strlen(InterpretCommentString)+1;
            		break;
            	}
            } while (cNext != '\n' && cNext != '\0');
            
            move_end_of_buffer(-n);
            continue;
        }

        /* cmd_pos > 0 means that we have encountered a '\' and some
           fraction of a command, e.g., '\sec', therefore the first
           time that a backslash is found then cmd_pos becomes 1,      */
        if (cThis == '\\') {
            bs_count++;
            if (odd(bs_count)) {    /* avoid "\\section" and "\\\\section" */
                for (i = 0; i < ncommands; i++)
                    possible_match[i] = TRUE;
                cmd_pos = 1;
                continue;
            }
        } else
            bs_count = 0;

        if (cmd_pos == 0) continue;

		/* at this point we are have encountered a command and may have to do something.
		   if it is a user definition, then the user definition is expanded here
		   if it is a user environment, then expansion also happens here
		   it it is something else then we may or may not have to mess with it. */

		/* hack to convert '\begin { environment}' --> '\begin{environ}' */
        if (matches_buffer_tail("\\begin") || matches_buffer_tail("\\end") ) {
        	char *env = getBeginEndParam();
        	if (env != NULL) {    /* now add '{environ}' to buffer */
        		add_chr_to_buffer('{');
        		add_str_to_buffer(env);
        		add_chr_to_buffer('}');
       			free(env);
        	}
        }
        
        any_possible_match = FALSE;
        found = FALSE;

        /* is is possibly a user defined command? */
        if (possible_match[0]) {
            possible_match[0] = maybeDefinition(section_buffer + section_buffer_end - cmd_pos + 1, cmd_pos - 1);
			
			if (possible_match[0]) {        /* test to make sure \userdef is complete */
				any_possible_match = TRUE;
				cNext = getRawTexChar();    /* wrong when cNext == '%' */
				ungetTexChar(cNext);
				
				if (!isalpha((int) cNext)) {   /* is macro name complete? */
	
					*(section_buffer + section_buffer_end + 1) = '\0';
					i = existsDefinition(section_buffer + section_buffer_end - cmd_pos + 1);
					if (i > -1) {
						diagnostics(2, "matched <%s> ", section_buffer + section_buffer_end - cmd_pos);
						if (cNext == ' ') {
							cNext = getNonSpace();
							ungetTexChar(cNext);
						}
	
						move_end_of_buffer(-cmd_pos-1);  /* remove \userdef */
						
						str = expandDefinition(i);
						PushSource(NULL, str);
						free(str);
						cmd_pos = 0;
            			bs_count = 0;
						continue;
					}
				}
			}
        }

        /* is it a user defined environment? */
        if (possible_match[1]) {
			char *p = section_buffer + section_buffer_end - cmd_pos;
            possible_match[1] = maybeEnvironment(p, cmd_pos);
            
        	if (possible_match[1] == TRUE) {
	
				any_possible_match = TRUE;
				cNext = getRawTexChar();    /* wrong when cNext == '%' */
	
				/* \begin{name} or \end{name} will end with '}' */
				if (cNext == '}') {
					char *str = NULL;
					
					*(p + cmd_pos + 1) = '\0';
					if (*(p + 1) == 'e') {  
						i = existsEnvironment(p + strlen("\\end{"));
						str = expandEnvironment(i, CMD_END);
					} else { 
						i = existsEnvironment(p + strlen("\\begin{"));
						str = expandEnvironment(i, CMD_BEGIN);
					}
	
					if (str) {          /* found */
						diagnostics(2, "matched <%s}>", p);
						diagnostics(3, "expanded to <%s>", str);
	
						PushSource(NULL, str);
						move_end_of_buffer(-cmd_pos-1); /* remove \begin{userenvironment} */
						
						free(str);
						cmd_pos = 0;
						continue;
					}
				}
				
				ungetTexChar(cNext);    /* put the character back */
        	}
        }

		/* is it one of the commands listed above? */
        for (i = 2; i < ncommands; i++) {
            if (possible_match[i]) {
				if (cThis == command[i][cmd_pos])
					any_possible_match = TRUE;
				else
					possible_match[i] = FALSE;

				diagnostics(2,"cmd_pos = %d, char = %c, possible match %s, size=%d, possible=%d", \
					cmd_pos,cThis,command[i],strlen(command[i]),possible_match[i]);
            }
        }



        i_match = -1;
        for (i = 2; i < ncommands; i++) {   /* discover any exact matches */
            if (possible_match[i]) {
				diagnostics(5, "testing for <%s>", command[i]);
				
				/* right length? */
				if (cmd_pos + 1 == strlen(command[i])) {

					if (i<=e_comment_item || i==e_comment_item) { 
						/* these entries are complete matches */ 
						found = TRUE;
					} else {
						/* these entries we need to be a bit more careful and check next character */
						cNext = getRawTexChar();
						ungetTexChar(cNext);
			
						if (cNext == ' ' || cNext == '{' || cNext == '\n' || cNext == '\0') {
							found = TRUE;
						}
					}
				}
            }
            
            if (found == TRUE) {
				diagnostics(5,"matched <%s> entry number %d",command[i],i);
				i_match = i;
				break;
			}
        }


        if (any_possible_match)
            cmd_pos++;
        else
            cmd_pos = 0;    /* no possible matches, reset and wait for next '\\' */
        

        if (!found)
            continue;

        if (i_match == endinput_item) {
            diagnostics(5, "\\endinput");
            move_end_of_buffer(-9);         /* remove \endinput */
            PopSource();
            cmd_pos = 0;          /* keep looking */
            continue;
        }

		/* \end{document} reached! Stop processing */
        if (i_match == e_document_item) {
            diagnostics(2, "\\end{document}");
        	move_end_of_buffer(-strlen(command[e_document_item]));
        	add_chr_to_buffer('\0');
        	*header = strdup(command[e_document_item]);
	    	*body = strdup(section_buffer);
    		PopTrackLineNumber();
			diagnostics(3, "body = %s", section_buffer);
			diagnostics(3, "next header = '%s'", command[e_document_item]);
	    	return;
        }

        if (i_match == verb_item) {  /* slurp \verb#text# */
        	char cc;
 
            do {
            	cc = getRawTexChar();
            	add_chr_to_buffer(cc);
            } while (cc != '\0' && cc != cThis);
            
            cmd_pos = 0;          /* reset the command position */
            continue;
        }

		/* cannot ignore this because it may contain unescaped '%' */
        if (i_match == url_item) {  
        	char cc;
 
            do {
            	cc = getRawTexChar();
            	add_chr_to_buffer(cc);
            } while (cc != '\0' && cc != '}');

            cmd_pos = 0;          /* reset the command position */
            continue;
        }

        if (i_match == include_item) {
            CmdInclude(0);
            move_end_of_buffer(-strlen(command[i]));    
            cmd_pos = 0;          		 /* reset the command position */
            continue;
        }

        if (i_match == input_item) {
            CmdInclude(1);
            move_end_of_buffer(-strlen(command[i]));    
            cmd_pos = 0;          		 /* reset the command position */
            continue;
        }

        if (i_match == label_item) {
            char *tag;
            tag = getBraceParam();

            /* append \label{tag} to the buffer */
			add_chr_to_buffer('{');
			add_str_to_buffer(tag);
			add_chr_to_buffer('}');

            if (!(*label) && strlen(tag) && label_depth == 0)
                *label = strdup_nobadchars(tag);

            free(tag);
            cmd_pos = 0;          /* keep looking */
            continue;
        }

        /* process any new definitions */
        if (i_match == def_item || i_match == new_item || i_match == renew_item) {
        
            cNext = getRawTexChar();    /* wrong when cNext == '%' */
            ungetTexChar(cNext);
            
            if (isalpha((int) cNext))   /* is macro name complete? */
                continue;

            move_end_of_buffer(-strlen(command[i]));    /* do not include in buffer */

            if (i_match == def_item)
                CmdNewDef(DEF_DEF);
            else if (i_match == new_item)
                CmdNewDef(DEF_NEW);
            else
                CmdNewDef(DEF_RENEW);

            cmd_pos = 0;          /* keep looking */
            continue;
        }

        if (i_match == b_figure_item   || i_match == b_figure_item2   || 
            i_match == b_equation_item || i_match == b_equation_item2 || 
            i_match == b_eqnarray_item || i_match == b_eqnarray_item2 ||
            i_match == b_table_item    || i_match == b_table_item2    ||
            i_match == b_description_item) {
            label_depth++;      /* labels now will not be the section label */
            cmd_pos = 0;
            continue;
        }

        if (i_match == e_figure_item   || i_match == e_figure_item2   || 
            i_match == e_equation_item || i_match == e_equation_item2 || 
            i_match == e_eqnarray_item || i_match == e_eqnarray_item2 ||
            i_match == e_table_item    || i_match == e_table_item2    ||
            i_match == e_description_item)  {
            label_depth--;      /* labels may now be the section label */
            cmd_pos = 0;
            continue;
        }

        if (i_match == b_verbatim_item) { /* slurp environment ... toxic contents! */
            s = getTexUntil(command[e_verbatim_item], TRUE);
			add_str_to_buffer(s);
            free(s);
			add_str_to_buffer(command[e_verbatim_item]);
            diagnostics(4,"matched \\end{verbatim}");
            cmd_pos = 0;          /* keep looking */
            continue;
        }

        if (i_match == b_comment_item) {  /* slurp environment ... toxic contents! */
            s = getTexUntil(command[e_comment_item], TRUE);
			add_str_to_buffer(s);
            free(s);
			add_str_to_buffer(command[e_comment_item]);
            cmd_pos = 0;          /* keep looking */
            continue;
        }

        diagnostics(5, "possible end of section");
        diagnostics(5, "label_depth = %d", label_depth);

        if (label_depth > 0)    /* still in a \begin{xxx} environment? */
            continue;

        /* actually found command to end the section */
        diagnostics(4, "getSection found command to end section");
        s = getBraceParam();
        next_header = strdup_together4(command[i], "{", s, "}");
        free(s);

        move_end_of_buffer(-strlen(command[i]));
		add_chr_to_buffer('\0');
        break;
    }
    text = strdup(section_buffer);
    *body = text;
    *header = next_header;
    PopTrackLineNumber();
    
    diagnostics(3, "body = %s", text);
    diagnostics(2, "next header = '%s'", next_header);
}
