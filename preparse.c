/* preparser.c - parser for LaTeX code

Copyright (C) 2007 The Free Software Foundation

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
    2007 Scott Prahl
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "preparse.h"
#include "cfg.h"
#include "main.h"
#include "utils.h"
#include "definitions.h"
#include "parser.h"
#include "funct1.h"

#define SECTION_BUFFER_SIZE 2048
static char *section_buffer = NULL;
static long section_buffer_size = SECTION_BUFFER_SIZE;
static long section_buffer_end = 0;

static void increase_buffer_size(void)
{
    char *new_section_buffer;

    new_section_buffer = (char *) malloc(2 * section_buffer_size + 1);
    if (new_section_buffer == NULL)
        diagnostics(ERROR, "Could not allocate enough memory to process file. Sorry.");
    memmove(new_section_buffer, section_buffer, section_buffer_size);
    section_buffer_size *= 2;
    free(section_buffer);
    section_buffer = new_section_buffer;
    diagnostics(4, "Expanded buffer size is now %ld", section_buffer_size);
}

static void show_buffer(char *s)
{
    long i;
    char c;
    for (i=0; i<=section_buffer_end; i++) {
    
        if (i==0) 
            diagnostics(WARNING, "\n%-*ld: ", (int) strlen(s), section_buffer_end);
        else if (i % 100 == 0) 
            diagnostics(WARNING, "\n%s: ",s);
        c = section_buffer[i];
        if (c == '\n') c = '=';
        if (c == '\0') c = '*';
        diagnostics(WARNING,"%c",c);
    }
}

static void add_chr_to_buffer(char c)
{
    if (section_buffer == NULL) {
        section_buffer = (char *) malloc(section_buffer_size + 1);
        if (section_buffer == NULL)
            diagnostics(ERROR, "Could not allocate enough memory to process file. Sorry.");
    }

    section_buffer_end++;
    *(section_buffer + section_buffer_end) = c;

    if (section_buffer_end + 2 >= section_buffer_size)
        increase_buffer_size();

    if (0) {
        if (c == '\0')
            diagnostics(WARNING, "[\\0]");
        else if (c == '\n')
            diagnostics(WARNING, "[\\n]");
        else
            diagnostics(WARNING, "[%c]", (int) c);
        diagnostics(WARNING, "<%ld>", section_buffer_end);
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
    long len;

    if (s==NULL) return FALSE;
    len = (long) strlen(s);
    
    if (len > section_buffer_end+1) return FALSE;
    
    if (strncmp(s,section_buffer+section_buffer_end-len,len)==0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static void reset_buffer(void)
{
    section_buffer_end = -1;
}

static void move_end_of_buffer(long n)
{
    section_buffer_end += n;

    if (0) {
        if (n<0)
            diagnostics(WARNING, "[removing %ld chars]", -n);
        else 
            diagnostics(WARNING, "[adding %ld chars]", n);
    }

    if (0) {
        diagnostics(WARNING,"last 5 characters are [%c][%c][%c][%c][%c]\n", 
        *(section_buffer+section_buffer_end-4),
        *(section_buffer+section_buffer_end-3),
        *(section_buffer+section_buffer_end-2),
        *(section_buffer+section_buffer_end-1),
        *(section_buffer+section_buffer_end)
        );
    }
}

void preParse(char **body, char **header, char **label)

/**************************************************************************
    purpose: obtain the next section of the latex file
    
    This is now a preparsing routine that breaks a file up into sections.  
    Macro expansion happens here as well.  \input and \include are also
    handled here.  The reason for this routine is allow \labels to refer
    to sections.  
    
    This routine reads text until a new section heading is found.  The text 
    is returned in body and the *next* header is returned in header.  If 
    no header follows then NULL is returned.
    
**************************************************************************/
{
    int any_possible_match, found;
    char cNext, cThis, *s, *text, *next_header, *str, *p;
    int i;
    int possible_match[43];
    char *command[43] = { "",   /* 0 entry is for user definitions */
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
        "\\label", "\\input", "\\include", "\\verb", "\\url", "\\nolinkurl",
        "\\newcommand", "\\def", "\\renewcommand", "\\endinput", "\\end{document}",
    };

    int ncommands = 43;

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
    const int nolinkurl_item = 37;
    const int new_item = 38;
    const int def_item = 39;
    const int renew_item = 40;
    const int endinput_item = 41;
    const int e_document_item = 42;

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
        while (cThis == '\0' && getParserDepth() > 0) {
            PopSource();
            diagnostics(4,"parser depth is now %d", getParserDepth());
            cThis = getRawTexChar();
        }

        if (cThis == '\0')
            diagnostics(6, "[%ld] xchar=000 '\\0' (backslash count=%d)", section_buffer_end, bs_count);
        else if (cThis == '\n')
            diagnostics(6, "[%ld] xchar=012 '\\n' (backslash count=%d)", section_buffer_end, bs_count);
        else
            diagnostics(6, "[%ld] xchar=%03d '%c' (backslash count=%d)", section_buffer_end, (int) cThis, cThis, bs_count);

		cThis = skipBOM(cThis);

        add_chr_to_buffer(cThis);

        if (cThis == '\0') break;

        if (cThis == '%' && even(bs_count)) {   
            int n = 1;  /* remove initial % */
            do {
                cNext = getRawTexChar();
                add_chr_to_buffer(cNext);
                n++;
            } while (cNext != '\n' && cNext != '\0');        
            move_end_of_buffer(-n);
            if (0) show_buffer("percent");
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

        /* replace "\ " and "\\n" with a space */
        if (cmd_pos == 1 &&  (cThis == '\n' || cThis == ' ') ) {
            move_end_of_buffer(-1);
            add_chr_to_buffer(' ');
            cmd_pos = 0;
            continue;
        }
        
        /* at this point we are have encountered a command and may have to do something.
           if it is a user definition, then the user definition is expanded here
           if it is a user environment, then expansion also happens here
           it it is something else then we may or may not have to mess with it. */

        /* hack to convert '\begin   {' --> '\begin{' */
        if (cThis==' ' && (matches_buffer_tail("\\begin") || matches_buffer_tail("\\end")) ) {
            diagnostics(5, "matched '\\begin ' or '\\end '");
            do {cThis = getRawTexChar();} while (cThis == ' ');
        }
        
        /* hack to convert '\begin{   ' --> '\begin{' */
        if (cThis==' ' && (matches_buffer_tail("\\begin{") || matches_buffer_tail("\\end{")) ) {
            diagnostics(5, "matched '\\begin{ ' or '\\end{ '");
            do {cThis = getRawTexChar();} while (cThis == ' ');
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
                        diagnostics(4, "matched <%s> ", section_buffer + section_buffer_end - cmd_pos);
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
            char *pp = section_buffer + section_buffer_end - cmd_pos;
            possible_match[1] = maybeEnvironment(pp, cmd_pos);
            
            if (possible_match[1] == TRUE) {
    
                any_possible_match = TRUE;
                cNext = getRawTexChar();    /* wrong when cNext == '%' */
    
                /* \begin{name} or \end{name} will end with '}' */
                if (cNext == '}') {
                    char *ss = NULL;
                    
                    *(pp + cmd_pos + 1) = '\0';
                    if (*(pp + 1) == 'e') {  
                        i = existsEnvironment(pp + strlen("\\end{"));
                        ss = expandEnvironment(i, CMD_END);
                    } else { 
                        i = existsEnvironment(pp + strlen("\\begin{"));
                        ss = expandEnvironment(i, CMD_BEGIN);
                    }
    
                    if (ss) {          /* found */
                        diagnostics(5, "matched <%s}>", pp);
                        diagnostics(5, "expanded to <%s>", ss);
    
                        PushSource(NULL, ss);
                        move_end_of_buffer(-cmd_pos-1); /* remove \begin{userenvironment} */
                        
                        free(ss);
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

                diagnostics(6,"cmd_pos = %d, char = %c, possible match %s, size=%d, possible=%d", \
                    cmd_pos,cThis,command[i],strlen(command[i]),possible_match[i]);
            }
        }



        i_match = -1;
        for (i = 2; i < ncommands; i++) {   /* discover any exact matches */
            if (possible_match[i]) {
                diagnostics(6, "testing for <%s>", command[i]);
                
                /* right length? */
                
                if (cmd_pos+1 == strlen(command[i])) {

                    if (i<=e_comment_item || i==e_comment_item) { 
                        /* these entries are complete matches */ 
                        found = TRUE;
                    } else {
                        /* these entries we need to be a bit more careful and check next character */
                        cNext = getRawTexChar();
                        ungetTexChar(cNext);
                        
                        /* this test for the end of commands may still need tweaking */
                        if (!isalpha(cNext) && cNext != '*') {
                            found = TRUE;
                        }
                    }
                }
            }
            
            if (found == TRUE) {
                diagnostics(6,"preparse matched '%s'",command[i]);
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
            diagnostics(6, "\\endinput");
            move_end_of_buffer(-9);         /* remove \endinput */
            PopSource();
            cmd_pos = 0;          /* keep looking */
            continue;
        }

        /* \end{document} reached! Stop processing */
        if (i_match == e_document_item) {
            diagnostics(6, "\\end{document}");
            move_end_of_buffer(-strlen(command[e_document_item]));
            add_chr_to_buffer('\0');
            safe_free(*header);
            *header = strdup(command[e_document_item]);
            p = section_buffer;
            while (*p==' ' || *p == '\n') p++;
            *body = strdup(p);
            PopTrackLineNumber();
            diagnostics(6, "body = %s", section_buffer);
            diagnostics(6, "next header = '%s'", command[e_document_item]);
            return;
        }

        if (i_match == verb_item) {  /* slurp \verb#text# */
            char cc;
            
            cNext = getRawTexChar();
            add_chr_to_buffer(cNext);
            diagnostics(6, "verb char = %c", cNext);
 
            do {
                cc = getRawTexChar();
                add_chr_to_buffer(cc);
            } while (cc != cNext && cc != '\0');
              
            cmd_pos = 0;          /* reset the command position */
            continue;
        }

        /* cannot ignore this because it may contain unescaped '%' */
        if (i_match == url_item || i_match == nolinkurl_item) {  
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
            cmd_pos = 0;                 /* reset the command position */
            continue;
        }

        if (i_match == input_item) {
            CmdInclude(1);
            move_end_of_buffer(-strlen(command[i]));    
            cmd_pos = 0;                 /* reset the command position */
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

        diagnostics(4, "possible end of section");
        diagnostics(4, "label_depth = %d", label_depth);

        if (label_depth > 0)    /* still in a \begin{xxx} environment? */
            continue;

        /* actually found command to end the section */
        diagnostics(4, "preParse() found command to end section");
        s = getBraceParam();
        next_header = strdup_together4(command[i], "{", s, "}");
        free(s);

        move_end_of_buffer(-strlen(command[i]));
        add_chr_to_buffer('\0');
        break;
    }
    
    /*eliminate white space at beginning of buffer */
    p = section_buffer;
    while (*p==' ' || *p == '\n') p++;
    
    *body = strdup(p);
    safe_free(*header);
    *header = next_header;
    PopTrackLineNumber();
}

