/* convert.c - high level routines for LaTeX to RTF conversion

Copyright (C) 2002 The Free Software Foundation

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
*/

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include "main.h"
#include "convert.h"
#include "commands.h"
#include "chars.h"
#include "funct1.h"
#include "fonts.h"
#include "stack.h"
#include "tables.h"
#include "equations.h"
#include "direct.h"
#include "ignore.h"
#include "cfg.h"
#include "encodings.h"
#include "utils.h"
#include "parser.h"
#include "lengths.h"
#include "counters.h"
#include "preamble.h"
#include "vertical.h"
#include "fields.h"

static int ret = 0;

static void TranslateCommand(void);

void ConvertString(const char *string)

/******************************************************************************
     purpose : converts string in TeX-format to Rtf-format
 ******************************************************************************/
{
    if (string == NULL || string == '\0')
        return;

    if (PushSource(NULL, string) == 0) {
        diagnostics(5, "Entering Convert() from ConvertString()");

        show_string(4, string, "converting");

        while (StillSource())
            Convert();

        PopSource();
        diagnostics(5, "Exiting Convert() from ConvertString()");
    }
}

void ConvertAllttString(char *s)

/******************************************************************************
     purpose : converts string in TeX-format to Rtf-format
               according to the alltt environment, which is like
               verbatim environment except that \, {, and } have
               their usual meanings
******************************************************************************/
{
    char cThis;

    if (s == NULL)
        return;
    diagnostics(3, "Entering Convert() from StringAllttConvert()");

    if (PushSource(NULL, s) == 0) {

        while (StillSource()) {

            cThis = getRawTexChar();    /* it is a verbatim like environment */
            switch (cThis) {

                case '\\':
                    PushLevels();
                    TranslateCommand();
                    CleanStack();
                    break;

                case '{':
                    PushBrace();
                    fprintRTF("{");
                    break;

                case '}':
                    ret = RecursionLevel - PopBrace();
                    fprintRTF("}");
                    break;

                default:
                    fprintRTF("%c",cThis);
                    break;
            }
        }
        PopSource();
    }
    diagnostics(3, "Exiting Convert() from StringAllttConvert()");
}

void Convert(void)

/****************************************************************************
purpose: converts inputfile and writes result to outputfile
 ****************************************************************************/
{
    char cThis = '\n';
    char cLast = '\0';
    char cNext;
    int count;

    diagnostics(3, "Entering Convert ret = %d", ret);
    RecursionLevel++;
    PushLevels();

    while ((cThis = getTexChar()) && cThis != '\0') {

        if (cThis == '\n')
            diagnostics(6, "Current character is '\\n' mode = %d ret = %d level = %d", getTexMode(), ret,
              RecursionLevel);
        else
            diagnostics(6, "Current character is '%c' mode = %d ret = %d level = %d", cThis, getTexMode(), ret,
              RecursionLevel);

        /* preliminary support for utf8 sequences.  Thanks to CSH */
        if ((cThis & 0x8000) && (CurrentFontEncoding() == ENCODING_UTF8)) {
            uint8_t byte;
            uint16_t len, value, i;

            /* Get the number of bytes in the sequence        */
            /* Must use an unsigned character for comparisons */
            byte = cThis;
            len = 0;
            value = 0;
            if (byte >= 0xF0) { 
                len = 3; 
                value = byte & ~0xF0; 
            } 
            else if (byte >= 0xE0) { 
                len = 2; 
                value = byte & ~0xE0; 
            }       
            else if (byte >= 0xC0) { 
                len = 1; 
                value = byte & ~0xC0; 
            }
            
            /* reassemble the character */
            for ( i=0; i<len; i++) {
                byte = getTexChar() & ~0xC0;
                value = (value << 6) + byte;
            }
            
            diagnostics(4,"(flag = 0x%X) char value = 0X%04X or %u (%u bytes)", (unsigned char) cThis, value, value, len);
            CmdUnicodeChar(value);
        }                       
        else

        switch (cThis) {

            case '\\':
                PushLevels();
                TranslateCommand();
                CleanStack();

                if (ret > 1) {
                    diagnostics(3, "Exiting Convert via TranslateCommand ret = %d level = %d", ret, RecursionLevel);
                    ret--;
                    RecursionLevel--;
                    return;
                }
                break;


            case '{':
                if (getTexMode() == MODE_VERTICAL)
                    changeTexMode(MODE_HORIZONTAL);

                CleanStack();
                PushBrace();
                fprintRTF("{");
                break;

            case '}':
                CleanStack();
                ret = RecursionLevel - PopBrace();
                fprintRTF("}");
                if (ret > 1) {
                    diagnostics(3, "Exiting Convert via '}' ret = %d level = %d", ret, RecursionLevel);
                    ret--;
                    RecursionLevel--;
                    return;
                }
                break;

            case ' ':
                if (getTexMode() == MODE_VERTICAL || getTexMode() == MODE_MATH || getTexMode() == MODE_DISPLAYMATH)
                    cThis = cLast;

                else if (cLast != ' ' && cLast != '\n') {

                    if (getTexMode() == MODE_RESTRICTED_HORIZONTAL)
                        fprintRTF("\\~");
                    else
                        fprintRTF(" ");
                }

                break;

            case '\n':
                g_tab_counter = 0;

                if (getTexMode() == MODE_MATH || getTexMode() == MODE_DISPLAYMATH) {

                    cNext = getNonBlank();
                    ungetTexChar(cNext);

                } else {
                    cNext = getNonSpace();

                    if (cNext == '\n') {    /* new paragraph ... skip all ' ' and '\n' */
                        CmdEndParagraph(0);

                    } else {    /* add a space if needed */
                        ungetTexChar(cNext);
                        if (getTexMode() != MODE_VERTICAL && cLast != ' ')
                            fprintRTF(" ");
                    }
                }
                break;


            case '$':
                cNext = getTexChar();
                diagnostics(5, "Processing $, next char <%c>", cNext);

                if (cNext == '$' && getTexMode() != MODE_MATH)
                    CmdEquation(EQN_DOLLAR_DOLLAR | ON);
                else {
                    ungetTexChar(cNext);
                    CmdEquation(EQN_DOLLAR | ON);
                }
                break;

            case '&':         
                /* should occur in array, align, or eqnarray environments  */

                /* are we are in the middle of \begin{array} ... \end{array} */
                if (g_processing_arrays) {
                    fprintRTF("%c", g_field_separator);
                    break;
                }
                
                /* must be in 'eqnarray' or 'align' environment */
                if (EQ_field_active()) {
                    diagnostics(4,"ending/restarting field before alignment '&'");
                    endCurrentField();
                    fprintRTF("\\tab\n");
                    startField(FIELD_EQ);
                } else {
                    diagnostics(4,"tabbing to match '&'");
                    fprintRTF("\\tab\n");
                }
                g_equation_column++;               
                break;

            case '~':
                fprintRTF("\\~");
                break;

            case '^':
                CmdSuperscript(0);
                break;

            case '_':
                CmdSubscript(0);
                break;

            case '-':
                if (getTexMode() == MODE_MATH || getTexMode() == MODE_DISPLAYMATH)
                    CmdUnicodeChar(8722); /* MINUS SIGN */
                else {
                    changeTexMode(MODE_HORIZONTAL);

                    count = getSameChar('-') + 1;

                    if (count == 1)
                        fprintRTF("-");
                    else if (count == 2)
                        fprintRTF("\\endash ");
                    else if (count == 3)
                        fprintRTF("\\emdash ");
                    else
                        while (count--)
                            fprintRTF("-");
                }
                break;

            case '|':
                if (getTexMode() == MODE_MATH || getTexMode() == MODE_DISPLAYMATH)
                    fprintRTF("|");
                else
                    fprintRTF("\\emdash ");
                break;

            case '\'':
                if (getTexMode() == MODE_MATH || getTexMode() == MODE_DISPLAYMATH)
                    fprintRTF("'");
                else {
                    changeTexMode(MODE_HORIZONTAL);
                    count = getSameChar('\'') + 1;
                    if (count == 2)
                        fprintRTF("\\rdblquote ");
                    else
                        while (count--)
                            fprintRTF("\\rquote ");
                }
                break;

            case '`':
                if (getTexMode() == MODE_VERTICAL)
                    changeTexMode(MODE_HORIZONTAL);
                count = getSameChar('`') + 1;
                if (count == 2)
                    fprintRTF("\\ldblquote ");
                else
                    while (count--)
                        fprintRTF("\\lquote ");
                break;

            case '\"':
                changeTexMode(MODE_HORIZONTAL);
                if (GermanMode)
                    TranslateGerman();
                else
                    fprintRTF("\"");
                break;

            case '<':
                if (getTexMode() == MODE_VERTICAL)
                    changeTexMode(MODE_HORIZONTAL);
                if (getTexMode() == MODE_HORIZONTAL) {
                    cNext = getTexChar();
                    if (cNext == '<') {
                        fprintRTF("\\'ab");
                    } else {
                        ungetTexChar(cNext);
                        fprintRTF("<");
                    }
                } else
                    fprintRTF("<");

                break;

            case '>':
                if (getTexMode() == MODE_VERTICAL)
                    changeTexMode(MODE_HORIZONTAL);
                if (getTexMode() == MODE_HORIZONTAL) {
                    cNext = getTexChar();
                    if (cNext == '>')
                        fprintRTF("\\'bb");
                    else {
                        ungetTexChar(cNext);
                        fprintRTF(">");
                    }
                } else
                    fprintRTF(">");
                break;

            case '!':
                if (getTexMode() == MODE_MATH || getTexMode() == MODE_DISPLAYMATH)
                    fprintRTF("!");
                else {
                    changeTexMode(MODE_HORIZONTAL);
                    if ((cNext = getTexChar()) && cNext == '`') {
                        fprintRTF("\\'a1 ");
                    } else {
                        fprintRTF("!");      /* WH 2014-06-16: Blank removed, was fprintRTF("! ") */
                        ungetTexChar(cNext);
                    }
                }
                break;

            case '?':
                changeTexMode(MODE_HORIZONTAL);
                if ((cNext = getTexChar()) && cNext == '`') {
                    fprintRTF("\\'bf ");
                } else {
                    fprintRTF("?");
                    ungetTexChar(cNext);     /* WH 2014-06-16: Blank removed, was fprintRTF("? ") */
                }
                break;

            case ':':
                if (getTexMode() == MODE_MATH || getTexMode() == MODE_DISPLAYMATH)
                    fprintRTF(":");
                else {
                    changeTexMode(MODE_HORIZONTAL);
                    if (FrenchMode)
                        fprintRTF("\\~:");
                    else
                        fprintRTF(":");
                }
                break;

            case '.':
                if (getTexMode() == MODE_MATH || getTexMode() == MODE_DISPLAYMATH)
                    fprintRTF(".");
                else {
                    changeTexMode(MODE_HORIZONTAL);
                    fprintRTF(".");

                    /* try to simulate double spaces after sentences */
                    cNext = getTexChar();
                    if (0 && cNext == ' ' && (isalpha((int) cLast) && !isupper((int) cLast)))
                        fprintRTF(" ");
                    ungetTexChar(cNext);
                }
                break;

            case '\t':
                diagnostics(WARNING, "This should not happen, ignoring \\t");
                cThis = ' ';
                break;

            case '\r':
                diagnostics(WARNING, "This should not happen, ignoring \\r");
                cThis = ' ';
                break;

            case '%':
                /*diagnostics(WARNING, "This should not happen, ignoring %%");*/
                skipToEOL();
                cThis = ' ';
                break;

            case '(':
                if (processing_fields() && g_escape_parens)
                    fprintRTF("\\\\(");
                else
                    fprintRTF("(");
                break;

            case ')':
                if (processing_fields() && g_escape_parens)
                    fprintRTF("\\\\)");
                else
                    fprintRTF(")");
                break;

            case ';':
                if (g_field_separator == ';' && processing_fields())
                    fprintRTF("\\\\;");
                else if (FrenchMode)
                    fprintRTF("\\~;");
                else
                    fprintRTF(";");
                break;

            case ',':
            
                if (g_field_separator == ',' && processing_fields()) {
                    if (g_processing_arrays) {
                    /* this is crazy, fields fail if \, is present in EQ array! */
                    /* substitute semi-colon because it is least worst option   */
                        fprintRTF(";"); 
                    } else {
                        fprintRTF("\\\\,"); 
                    }
                } else {
                /* ,, is \quotedblbase nearly always */
                    if ((cNext = getTexChar()) && cNext == ',') {
                        fprintRTF("\\'84");
                    } else {
                        fprintRTF(",");
                        ungetTexChar(cNext);
                    }
                }
                break;

            default:
                if (getTexMode() == MODE_MATH || getTexMode() == MODE_DISPLAYMATH) {
                    if (('a' <= cThis && cThis <= 'z') || ('A' <= cThis && cThis <= 'Z')) {
                        if (CurrentFontSeries() == F_SERIES_BOLD)    /* do not italicize */
                            fprintRTF("%c", cThis);
                        else if (CurrentFontShape() == F_SHAPE_MATH_UPRIGHT)
                            fprintRTF("%c", cThis);
                        else
                            fprintRTF("{\\i %c}", cThis);
                    } else
                        fprintRTF("%c", cThis);
                

                } else {
                    if (getTexMode()==MODE_VERTICAL) 
                        changeTexMode(MODE_HORIZONTAL);
                        
                    if (CurrentFontEncoding() != ENCODING_OT2)
                        fprintRTF("%c", cThis);
                    else
                        CmdOT2Transliteration(cThis);
                }
                break;
        }

        g_tab_counter++;
        cLast = cThis;
    }
    RecursionLevel--;
    diagnostics(3, "Exiting Convert via exhaustion ret = %d", ret);
}

static void TranslateCommand(void)

/****************************************************************************
purpose: The function is called on a backslash in input file and
     tries to call the command-function for the following command.
returns: success or not
 ****************************************************************************/
{
    char cCommand[MAXCOMMANDLEN];
    int i, mode, height;
    int cThis,cNext;

    cThis = getTexChar();
    mode = getTexMode();

    diagnostics(4, "Beginning TranslateCommand() \\%c", cThis);

    switch (cThis) {
        case 'a':
            if (!g_processing_tabbing) break;
            cNext = getTexChar();
            if (cNext=='=' ) {CmdMacronChar(0); return;}
            if (cNext=='\'') {CmdAcuteChar(0); return;}
            if (cNext=='`' ) {CmdAcuteChar(0); return;}
            ungetTexChar(cNext);
            break;
        case '}':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            fprintRTF("\\}");
            return;
        case '{':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            fprintRTF("\\{");
            return;
        case '#':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            fprintRTF("#");
            return;
        case '$':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            fprintRTF("$");
            return;
        case '&':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            fprintRTF("&");
            return;
        case '%':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            fprintRTF("%%");
            return;
        case '_':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            fprintRTF("_");
            return;

        case '\\':             /* \\[1mm] or \\*[1mm] possible */
            diagnostics(5,"here we are in TranslateCommand with '\\\\'");
            height = getSlashSlashParam();
            if (g_processing_arrays)            /* \begin{array} ... \end{array} */
                CmdArraySlashSlash(height);     /* may be contained in eqnarray environment */
            else if (g_processing_eqnarray)
                CmdEqnArraySlashSlash(height);
            else  if (g_processing_tabbing)
                diagnostics(1,"should not be processing \\\\ in tabbing here");
            else if (g_processing_tabular)
                diagnostics(1,"should not be processing \\\\ in tabular here");
            else
                CmdSlashSlash(height);
            return;

        case 0x0D:  /*handle any CRLF that might have snuck in*/
            cNext = getTexChar();
            if (cNext != 0x0A)
                ungetTexChar(cNext);
            /* fall through */

        case '\t':  /* tabs should already be spaces */
        case ' ':
        case 0x0A:
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            fprintRTF(" ");     /* ordinary interword space */
            skipSpaces();
            return;

/* \= \> \< \+ \- \' \` all have different meanings in a tabbing environment */

        case '-':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            if (g_processing_tabbing) {
                (void) PopBrace();
                PushBrace();
            } else
                fprintRTF("\\-");
            return;

        case '+':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            if (g_processing_tabbing) {
                (void) PopBrace();
                PushBrace();
            }
            return;

        case '<':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            if (g_processing_tabbing) {
                (void) PopBrace();
                PushBrace();
            }
            return;

        case '>':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            if (g_processing_tabbing) {
                (void) PopBrace();
                CmdTabjump();
                PushBrace();
            } else
                CmdSpace((float)0.50); /* medium space */
            return;

        case '`':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            if (g_processing_tabbing) {
                (void) PopBrace();
                PushBrace();
            } else
                CmdGraveChar(0);
            return;

        case '\'':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            if (g_processing_tabbing) {
                (void) PopBrace();
                PushBrace();
                return;
            } else
                CmdAcuteChar(0);   /* char ' =?= \' */
            return;

        case '=':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            if (g_processing_tabbing) {
                (void) PopBrace();
                CmdTabset();
                PushBrace();
            } else
                CmdMacronChar(0);
            return;

        case '~':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            CmdTildeChar(0);
            return;
            
        case '^':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            
            cThis = getTexChar();
            if (cThis=='^') {
                /* usually \^^M ... just replace with a blank */
                getTexChar();
                fprintRTF(" ");
            } else {
                ungetTexChar(cThis);
                CmdHatChar(0);
            }
            return;
        case '.':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            CmdDotChar(0);
            return;
        case '\"':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            CmdUmlauteChar(0);
            return;
        case '(':
            CmdEquation(EQN_RND_OPEN | ON);
            /* PushBrace(); */
            return;
        case '[':
            CmdEquation(EQN_BRACKET_OPEN | ON);
            /* PushBrace(); */
            return;
        case ')':
            CmdEquation(EQN_RND_CLOSE | OFF);
            /* ret = RecursionLevel - PopBrace(); */
            return;
        case ']':
            CmdEquation(EQN_BRACKET_CLOSE | OFF);
            /* ret = RecursionLevel - PopBrace(); */
            return;
        case '/':
            CmdIgnore(0);       /* italic correction */
            return;
        case '!':
            CmdIgnore(0);       /* \! negative thin space */
            return;
        case ',':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            CmdNonBreakSpace(50);     /* \, produces a small space = 50% */
            return;
        case ';':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            CmdSpace((float)0.75);     /* \; produces a thick space */
            return;
        case '@':
            CmdIgnore(0);       /* \@ produces an "end of sentence" space */
            return;
        case '3':
            if (mode == MODE_VERTICAL)
                changeTexMode(MODE_HORIZONTAL);
            fprintRTF("{\\'df}");   /* german symbol 'á' */
            return;
    }


    /* Commands consist of letters and can have an optional * at the end */
    for (i = 0; i < MAXCOMMANDLEN-1; i++) {
        if (!isalpha((int) cThis) && (cThis != '*')) {
            int found_nl = FALSE;

            if (cThis == '%') { /* put the % back and get the next char */
                ungetTexChar('%');
                cThis = getTexChar();
            }

            /* all spaces after commands are ignored, a single \n may occur */
            while (cThis == ' ' || (0 && cThis == '\n' && !found_nl)) {
                if (cThis == '\n')
                    found_nl = TRUE;
                cThis = getTexChar();
            }

            ungetTexChar(cThis);    /* put back first non-space char after command */
            break;              /* done skipping spaces */
        } else
            cCommand[i] = cThis;

        cThis = getRawTexChar();    /* Necessary because % ends a command */
    }

    cCommand[i] = '\0';         /* mark end of string with zero */
    diagnostics(4, "TranslateCommand() <%s>", cCommand);

    if (i==MAXCOMMANDLEN-1) {
        diagnostics(WARNING, "Skipping absurdly long command <%s>", cCommand);
        return;
    }

    if (i == 0)
        return;

    if (strcmp(cCommand, "begin") == 0) {
        fprintRTF("{");
        PushBrace();
    }

    if (CallCommandFunc(cCommand)) {    /* call handling function for command */
        if (strcmp(cCommand, "end") == 0) {
            diagnostics(4, "before PopBrace()");
            ret = RecursionLevel - PopBrace();
            diagnostics(4, "after PopBrace(), ret=%d",ret);
            fprintRTF("}");
        }
        return;
    }

    if (TryDirectConvert(cCommand))
        return;

    if (TryVariableIgnore(cCommand))
        return;

    if (TryTheCounter(cCommand))
        return;

    diagnostics(WARNING, "Unknown command '\\%s'", cCommand);
}
