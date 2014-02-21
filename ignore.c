
/* ignore.c - ignore commands found in ignore.cfg

Copyright (C) 1995-2002 The Free Software Foundation

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
    1995-1997 Ralf Schlatterbeck
    1998-2000 Georg Lehner
    2001-2002 Scott Prahl
*/

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "main.h"
#include "direct.h"
#include "fonts.h"
#include "cfg.h"
#include "ignore.h"
#include "funct1.h"
#include "commands.h"
#include "parser.h"
#include "convert.h"
#include "utils.h"
#include "vertical.h"

/****************************************************************************
purpose : ignores anything till an alphanumeric character
 ****************************************************************************/
static void IgnoreCmd(void)
{
    char c;

    do { c = getTexChar(); } while (c && c != '\\');
    do { c = getTexChar(); } while (c && isalpha((int) c));
    
    ungetTexChar(c);
}

/****************************************************************************
purpose : ignores a number 
          \tolerance = 10000
          \tolerance 10000
          \tolerance10000
 ****************************************************************************/
static void IgnoreNumber(void)
{
    char c;
    
    c = getNonSpace();
    if (c == '=')
        c = getNonSpace();

    while (c=='-' || c=='+' || c== '.' || isdigit(c))
        c = getTexChar();
    
    ungetTexChar(c);
}

/****************************************************************************
purpose : ignores a dimension
 ****************************************************************************/
static void IgnoreMeasure(void)
{
    char c = getNonSpace();
    if (c == '=')
        c = getNonSpace();
    
    ungetTexChar(c);

    getDimension();
}

/****************************************************************************
purpose : ignores variable-formats shown in file "ignore.cfg"
returns : TRUE if variable was ignored correctly, otherwise FALSE

#  SINGLE        ignore single command. e.g., \noindent
#  NUMBER        simple numeric value e.g., \tolerance10000
#  MEASURE       numeric value with following unit of measure
#  OTHER         ignores anything to the first character after '='
#                and from there to next space. eg. \setbox\bak=\hbox
#  COMMAND       ignores anything to next '\' and from there to occurence
#                of anything but a letter. eg. \newbox\bak
#  PARAMETER     ignores a command with one paramter
#  PACKAGE       does not produce a Warning message if PACKAGE is encountered
#  ENVCMD        proceses contents of unknown environment as if it were plain latex
#  ENVIRONMENT   ignores contents of that environment
 ****************************************************************************/
int TryVariableIgnore(const char *command)
{
    const char *RtfCommand;
    char *TexCommand, c;
        
    TexCommand = strdup_together("\\", command);
    RtfCommand = SearchCfgRtf(TexCommand, IGNORE_A);
    free(TexCommand);
    
    diagnostics(4, "Ignoring '%s' as '%s'", command, RtfCommand);
    if (RtfCommand == NULL) return FALSE;
    
    if (strcmp(RtfCommand, "SINGLE") == 0) {
    
        /* don't need to do anything */
        
    } else if (strcmp(RtfCommand, "NUMBER") == 0) {
    
        IgnoreNumber();
        
    } else if (strcmp(RtfCommand, "MEASURE") == 0) {
    
        IgnoreMeasure();
        
    } else if (strcmp(RtfCommand, "OTHER") == 0) {
    
        c = getNonSpace();
        if (c == '=')
            c = getNonSpace();
        
        while (c && !isspace(c))
            c = getTexChar();

        if (c) {
            c = getNonSpace();
            ungetTexChar(c);
        }
        
    } else if (strcmp(RtfCommand, "COMMAND") == 0) {
    
        IgnoreCmd();
        
    } else  if (strcmp(RtfCommand, "PARAMETER") == 0) {
        CmdIgnoreParameter(No_Opt_One_NormParam);
        
    } else if (strcmp(RtfCommand, "TWOPARAMETER") == 0) {
        CmdIgnoreParameter(No_Opt_Two_NormParam);
        
    } else if (strcmp(RtfCommand, "ENVIRONMENT") == 0) {
        char *str = strdup_together3("end{", command, "}");
        Ignore_Environment(str);
        free(str);
        
    } else if (strcmp(RtfCommand, "ENVCMD") == 0) {
        PushEnvironment(IGNORE_MODE);
    } 
    
    return TRUE;
}

int TryPackageIgnore(const char *package) {
    const char *RtfCommand;
    char *TexCommand;
    int  result = FALSE;
    
    diagnostics(4, "trying to ignore '%s'", package);
    
    TexCommand = strdup_together("\\", package);
    RtfCommand = SearchCfgRtf(TexCommand, IGNORE_A);
    free(TexCommand);

    if (NULL != RtfCommand)
        result = (0 == strcmp(RtfCommand,"PACKAGE"));
    return result;
}
/******************************************************************************
  purpose: function, which ignores an unconvertable environment in LaTex
           and writes text unchanged into the Rtf-file.
parameter: searchstring : includes the string to search for
       example: \begin{unknown} ... \end{unknown}
            searchstring="end{unknown}"
 ******************************************************************************/
void Ignore_Environment(char *cCommand)
{
    char unknown_environment[100];
    char *buffer;
    int font;

    diagnostics(4, "Entering IgnoreEnvironment <%s>", cCommand);

    snprintf(unknown_environment, 100, "\\%s%s%s", "end{", cCommand, "}");
    font = TexFontNumber("Typewriter");
    CmdEndParagraph(0);
    CmdIndent(INDENT_NONE);
    startParagraph("Normal", PARAGRAPH_GENERIC);
    fprintRTF("\\qc [Sorry. Ignored ");
    fprintRTF("{\\plain\\f%d\\\\begin\\{%s\\} ... \\\\end\\{%s\\}}]", font, cCommand, cCommand);
    CmdEndParagraph(0);
    CmdIndent(INDENT_INHIBIT);

    buffer = getTexUntil(unknown_environment, 0);
    ConvertString(unknown_environment);
    free(buffer);

    diagnostics(4, "Exiting IgnoreEnvironment");
}
