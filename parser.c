
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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

This file is available from http://sourceforge.net/projects/latex2rtf/
 
Authors:
    1998-2000 Georg Lehner
    2001-2007 Scott Prahl
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
#include "encodings.h"

typedef struct InputStackType {
    char *string;
    char *string_start;
    FILE *file;
    char *file_name;
    int file_line;
} InputStackType;

#define PARSER_SOURCE_MAX 100
#define SCAN_BUFFER_SIZE   5000

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

int PushSource(const char *filename, const char *string)

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
        diagnostics(WARNING, "Before PushSource** line=%d, g_parser_depth=%d, g_parser_include_level=%d",
          g_parser_line, g_parser_depth, g_parser_include_level);
        for (i = 0; i <= g_parser_depth; i++) {
            if (g_parser_stack[i].file)
                diagnostics(WARNING, "i=%d file   =%s, line=%d", i, g_parser_stack[i].file_name, g_parser_stack[i].file_line);

            else {
                strncpy_printable(s, g_parser_stack[i].string, 25);
                diagnostics(WARNING, "i=%d string =%s, line=%d", i, s, g_parser_stack[i].file_line);
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
        p = my_fopen((char *)filename, "rb");
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
        diagnostics(3, "Opening Source File %s", g_parser_stack[g_parser_depth].file_name);
    } else {
        diagnostics(4, "Opening Source string");
        show_string(5, g_parser_string, "opening");
    }

    if (0) {
        diagnostics(WARNING, "After PushSource** line=%d, g_parser_depth=%d, g_parser_include_level=%d",
          g_parser_line, g_parser_depth, g_parser_include_level);
        for (i = 0; i <= g_parser_depth; i++) {
            if (g_parser_stack[i].file)
                diagnostics(WARNING, "i=%d file   =%s, line=%d", i, g_parser_stack[i].file_name, g_parser_stack[i].file_line);

            else {
                strncpy_printable(s, g_parser_stack[i].string, 25);
                diagnostics(WARNING, "i=%d string =%s, line=%d", i, s, g_parser_stack[i].file_line);
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

    if (g_parser_depth < 0) {
        diagnostics(1, "Hmmm.  More PopSource() calls than PushSource() calls");
        return;
    }

    if (0) {
        diagnostics(WARNING, "Before PopSource** line=%d, g_parser_depth=%d, g_parser_include_level=%d",
          g_parser_line, g_parser_depth, g_parser_include_level);
        for (i = 0; i <= g_parser_depth; i++) {
            if (g_parser_stack[i].file)
                diagnostics(WARNING, "i=%d file   =%s, line=%d", i, g_parser_stack[i].file_name, g_parser_stack[i].file_line);

            else {
                strncpy_printable(s, g_parser_stack[i].string, 25);
                diagnostics(WARNING, "i=%d string =%s, line=%d", i, s, g_parser_stack[i].file_line);
            }
        }
    }

    if (g_parser_file) {
        diagnostics(3, "Closing Source File '%s'", g_parser_stack[g_parser_depth].file_name);
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

        show_string(5, s, "closing");
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
        diagnostics(4, "Resuming Source File '%s'", g_parser_stack[g_parser_depth].file_name);
    else {
        diagnostics(5, "Resuming Source string");
        show_string(5,g_parser_string,"resuming");
    }

    if (0) {
        diagnostics(WARNING, "After PopSource** line=%d, g_parser_depth=%d, g_parser_include_level=%d",
          g_parser_line, g_parser_depth, g_parser_include_level);
        for (i = 0; i <= g_parser_depth; i++) {
            if (g_parser_stack[i].file)
                diagnostics(WARNING, "i=%d file   =%s, line=%d", i, g_parser_stack[i].file_name, g_parser_stack[i].file_line);

            else {
                strncpy_printable(s, g_parser_stack[i].string, 25);
                diagnostics(WARNING, "i=%d string =%s, line=%d", i, s, g_parser_stack[i].file_line);
            }
        }
    }
}

void CmdInclude(int code)

/******************************************************************************
 purpose: handles \input file, \input{file}, \include{file}
          code == 0 for \include
          code == 1 for \input
 ******************************************************************************/
{
    int cNext;
    char name[100];
    int i;
    char *basename=NULL;
    char *texname=NULL;

    cNext = getNonSpace();

    if (cNext == '{') {         /* \input{gnu} or \include{gnu} */
        ungetTexChar(cNext);
        basename = getBraceParam();

    } else {                    /* \input gnu */
        i = 0;
        while (cNext != '\0' && !isspace(cNext)) {
            if (i<99) name[i] = (char) cNext;
            i++;
            cNext = getTexChar();
        }
        
        if (i<99) 
            name[i] = '\0';
        else {
            name[99] = '\0';
            diagnostics(WARNING, "\\input filename '%s' more than 100 chars, skipping",name);
            return;
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
    
    if (basename && strstr(basename, ".tex") == NULL && strstr(basename, ".ltx") == NULL)         /* append .tex if missing */
        texname = strdup_together(basename, ".tex");

    if (texname && PushSource(texname, NULL) == 0)            /* Try the .tex name first*/
        diagnostics(WARNING, "Including file <%s> (.tex appended)", texname);
      
    else if (basename && PushSource(basename, NULL) == 0)     /* Try the basename second*/
        diagnostics(WARNING, "Including file <%s>", basename);

    /* \include{file} always starts a new page */
    if (code == 0)
        PushSource(NULL, "\\pagebreak ");
        
    if (basename) free(basename);
    if (texname)  free(texname);
}


#define CR (char) 0x0d
#define LF (char) 0x0a

int getParserDepth(void)
{
    return g_parser_depth;
}


char getRawTexChar(void)

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
        while (thechar == EOF) {
            if (!feof(g_parser_file))
                diagnostics(ERROR, "Unknown file I/O error reading latex file\n");
            else if (g_parser_include_level > 1) {
                PopSource();    /* go back to parsing parent */
                thechar = getRawTexChar();  /* get next char from parent file */
            } else
                thechar = '\0';
        }
        if (thechar == CR) {   /* convert CR, CRLF, or LF to \n */
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
    diagnostics(6, "after ungetTexChar=<%c> backslashes=%d line=%ld", c, g_parser_backslashes, g_parser_line);
}

int skipBOM(int cThis)
{
    /* UTF8 Byte Order Mark */
    if (cThis == 0xEF) {
    	cThis = getRawTexChar();
    	if (cThis == 0xBB) {
     		cThis = getRawTexChar();
    		if (cThis == 0xBF) {
    			CmdFontEncoding(ENCODING_UTF8);
     			cThis = getRawTexChar();
     			diagnostics(2, "UTF 8 BOM encountered, now assuming UTF8 input");
     		}
    	}
    }
   		
    /* UTF16 Byte Order Mark */
    if (cThis == 0xFE) {
    	cThis = getRawTexChar();
    	if (cThis == 0xFF)
     		diagnostics(2, "UTF 16 is not supported, you might try converting to UTF8");
    }
    
    return cThis;
}

char getTexChar()

/***************************************************************************
 purpose:     get the next character from the input stream
              This should be the usual place to access the LaTeX file
              It filters the input stream so that % is handled properly
****************************************************************************/
{
    int cThis;

    cThis = getRawTexChar();
    cThis = skipBOM(cThis);
    
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

void    skipWhiteSpace(void)
/***************************************************************************
 Description: skip over spaces and linefeeds
****************************************************************************/
{
    char c=getNonBlank();
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

char *getDelimitedText(char left, char right, int raw)

/******************************************************************************
  purpose: general scanning routine that allocates and returns a string
           that is between "left" and "right" that accounts for escaping by '\'
           
           Example for getDelimitedText('{','}',TRUE) 
           
           "the \{ is shown {\it by} a\\} blah blah" ----> "the \{ is shown {\it by} a\\"
            
           Note the missing opening brace in the example above
           
           It turns out that for getDelimitedText('[',']',TRUE)
           
           "the \] is shown {]} a\\] blah blah blah" ----> "the \] is shown {]} a\\"
          
 ******************************************************************************/
{
    char buffer[SCAN_BUFFER_SIZE];
    int size = -1;
    int lefts_needed = 1;
    int brace_level = 0;
    int last_char_was_backslash = FALSE;

    while (lefts_needed && size < SCAN_BUFFER_SIZE-1) {
        size++;
        buffer[size] = (raw) ? getRawTexChar() : getTexChar();

        if (last_char_was_backslash)  {            /* ignore \{ etc.           */
            if (buffer[size] == '\\') {            /* two backslashes in a row */
                last_char_was_backslash = FALSE;   /* next char is not special */
                continue;
            }
        }

        else if (buffer[size] == right && brace_level == 0) 
        	lefts_needed--;
        
        else if (buffer[size] == '{') 
        	brace_level++;
        
        else if (buffer[size] == '}') 
        	brace_level--;

        last_char_was_backslash = (buffer[size] == '\\') ? TRUE : FALSE;
    }

    buffer[size] = '\0';        /* overwrite final delimeter */
    if (size == SCAN_BUFFER_SIZE-1) {
        diagnostics(WARNING, "Could not find closing '%c' in %d chars", right, SCAN_BUFFER_SIZE);
        return strdup(" NOT FOUND ");
    }

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

static void parseBracket(void)

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
                                              \! will return \!
 **************************************************************************/
{
    char buffer[128];
    int size;

    buffer[0] = getTexChar();

    if (buffer[0] != '\\')
        return NULL;

    buffer[1] = getTexChar();

    for (size = 2; size < 127; size++) {
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
        diagnostics(6, "getBracketParam [%s]", text);

    } else {
        ungetTexChar(c);
        text = NULL;
        diagnostics(6, "getBracketParam []");
    }

    PopTrackLineNumber();
    return text;
}

static char *getBraceParam0(int raw_flag)

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
        text = getDelimitedText('{', '}', raw_flag);

    else {
        s[1] = '\0';
        text = strdup(s);
    }

    PopTrackLineNumber();
    diagnostics(6, "Leaving getBraceParam {%s}", text);
    return text;
}

char *getBraceParam(void)
{
    return getBraceParam0(FALSE);
}

char *getBraceRawParam(void)
{
    return getBraceParam0(TRUE);
}

void ignoreBraceParam(void) {
    char *p = getBraceParam();
    if (NULL != p) free(p);
}

void  ignoreBracketParam(void) {
    char *p = getBracketParam();
    if (NULL != p) free(p);
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
            my_strlcat(text + i, command, 5000);
            i += (int) strlen(command);
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
    enum { BUFFSIZE = 200000 };
    char *s;
    char buffer[BUFFSIZE];
    int last_i = -1;
    int i = 0;                  /* size of string that has been read */
    int j = 0;               /* number of found characters */
    int end_of_file_reached = FALSE;
    int len = (int) strlen(target);

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
    else {
        diagnostics(ERROR, "Could not find <%s>", target);
        exit(1);
    }

    PopTrackLineNumber();

    diagnostics(6, "buffer size =[%d], actual=[%d]", strlen(buffer), i - len);

    s = strdup(buffer);
    diagnostics(6, "getTexUntil result = %s", s);
    return s;
}

char *getSpacedTexUntil(char *target, int raw)

/**************************************************************************
     purpose: returns the portion of the file to the beginning of target

     getSpacedTexUntil("\begin|{|document|}")
     
     will match the regular expression "\\begin *{ *document *}"
 **************************************************************************/
{
    enum { BUFFSIZE = 16000 };
    char buffer[BUFFSIZE];
    char *s;
    int buffer_pos, target_pos, target_len, max_buffer_pos, start_pos;
    
    PushTrackLineNumber(FALSE);

    diagnostics(5, "getSpacedTexUntil target = <%s> raw_search = %d ", target, raw);

    buffer_pos = 0;
    target_pos = 0;
    start_pos  = 0;
    target_len = (int) strlen(target);
    max_buffer_pos = -1;
    
    do {
        
        /* the next character might already be in the buffer */
        if (buffer_pos > max_buffer_pos) {
            buffer[buffer_pos] = (raw) ? getRawTexChar() : getTexChar();
            max_buffer_pos = buffer_pos;
        } 

        if (buffer[buffer_pos] == '\0') {
            diagnostics(ERROR, "end of file reached before '%s' was found",target);
        }

        if (buffer[buffer_pos] == target[target_pos]) {
            if (target_pos == 0) 
                start_pos = buffer_pos;
            target_pos++;
        } 
        
        /* does not match next character in target ... */
        else if (target[target_pos] != '|') {

            if (target_pos > 0)        /* false start, put back what was found */
                buffer_pos = start_pos;
            target_pos = 0;
        
        /* next character in target is '|' */ 
        } else if (buffer[buffer_pos] != ' ' && buffer[buffer_pos] != '\n') {
            
            /* next char is non-blank ... either match or reset */
            target_pos++;  /* move past wildcard */
            if (buffer[buffer_pos] == target[target_pos]) {
                target_pos++;
            } else {
                buffer_pos = start_pos;
                target_pos = 0;
            }
        }
        
        if (0) {
        if (buffer[buffer_pos] != '\n')
            diagnostics(WARNING, "this char = <%c>, %d, %d, max=%d", buffer[buffer_pos], buffer_pos, target_pos, max_buffer_pos);
        else
            diagnostics(WARNING, "this char = <\\n>, %d, %d, max=%d", buffer[buffer_pos], buffer_pos, target_pos, max_buffer_pos);
        }
        
        buffer_pos++;


        if (buffer_pos == BUFFSIZE)
            diagnostics(ERROR, "Could not find <%s> in %d characters \n\
            Recompile with larger BUFFSIZE in getTexUntil() in parser.c", target, BUFFSIZE);

    } while (target_pos < target_len);

    /* terminate buffer */
    buffer[start_pos] = '\0';

    PopTrackLineNumber();

    s = strdup(buffer);
    diagnostics(6, "getSpacedTexUntil result = %s", s);
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
        getTexChar();
        cThis = getTexChar();
    }

/* skip "spread" */
    if (cThis == 's') {
        getTexChar();
        getTexChar();
        getTexChar();
        getTexChar();
        getTexChar();
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

/*  num *= 2;                    convert pts to twips */

/* obtain unit of measure */
    skipSpaces();
    buffer[0] = tolower((int) getTexChar());

    if (buffer[0] == '\0')  /* no units specified ... assume points */
        return (int) (num * 20);
    
/* skip "true" */
    if (buffer[0] == 't') {
        getTexChar();
        getTexChar();
        getTexChar();
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

/***************************************************************************
 purpose: return twips for \\, \\[1pt], \\*[1pt] 
 ***************************************************************************/
int getSlashSlashParam(void)
{
    char cThis, *vertical_space;
    int height = 0;
    
    cThis = getTexChar();
    if (cThis != '*')
        ungetTexChar(cThis);

    vertical_space = getBracketParam();
    if (vertical_space) { 
        height = getStringDimension(vertical_space);
        free(vertical_space);
    }
    
    return height;
}

