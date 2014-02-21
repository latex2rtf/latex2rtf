
/* styles.c - Convert simple LaTeX commands using direct.cfg 

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
    2004 Scott Prahl
*/

#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "direct.h"
#include "fonts.h"
#include "cfg.h"
#include "utils.h"
#include "parser.h"
#include "styles.h"
#include "vertical.h"

static char *g_current_style = NULL;

void SetCurrentStyle(const char *style)
{
    if (g_current_style)
        free(g_current_style);
    g_current_style = strdup(style);
}

char *GetCurrentStyle(void)
{
    return g_current_style;
}

int IsSameAsCurrentStyle(const char *s)
{
    if (g_current_style==NULL)
        return FALSE;
        
    if (strcmp(s,g_current_style)==0) 
        return TRUE;
    else
        return FALSE;
}

void InsertCurrentStyle(void)
{
    if (g_current_style==NULL)
        InsertStyle("Normal");
    else
        InsertStyle(g_current_style);
}

void InsertBasicStyle(const char *rtf, int how)

/******************************************************************************
  purpose: uses data from style.cfg to try and insert RTF commands
           that correspond to the appropriate style sheet or character style
           for example
                InsertBasicStyle(rtf, INSERT_STYLE_NORMAL);
                
           where rtf="\rtfsequence,\rtfheader"
 ******************************************************************************/
{
    char *style, *comma;
    char *style_end = NULL;

    if (rtf == NULL) return;
    
/* skip over 0,0, */
    style = strchr((char *) rtf, ',') + 1;
    if (style == NULL) return;

    style = strchr(style+1, ',');
    if (style == NULL) return;

/* skip blanks */
    style++;
    while (*style == ' ')
        style++;                

/* locate end of style */
    comma = strchr(style, ',');
    if (comma == NULL) return;

    switch (how) {
    
        case INSERT_STYLE_NORMAL:
            style_end = comma;
            break;
            
        case INSERT_STYLE_FOR_HEADER:
            style_end = style + strlen(style);
            break;

        case INSERT_STYLE_NO_STYLE:
            /* move start just past style bit e.g., \s2 */
            if (strstarts(style,"\\s")) {
                style += 2;
                while (*style >= '0' && *style <= '9') 
                    style++;
            }
            style_end = comma;
            break;
    }
    
    while (style < style_end) {

        if (style == comma)
            fprintRTF(" ");
        else if (*style == '*') 
            WriteCFGFontNumber(&style);
        else
            fprintRTF("%c", *style);

        style++;
    }
    
 /* emit final blank to make sure that RTF is properly terminated */
 /*   if (how != INSERT_STYLE_FOR_HEADER)
        fprintRTF(" "); */
}

/* if the_style ends with '0' then do not insert the style sequence \s2
   and just insert the rtf commands, otherwise do both 
   This is needed so that chapter headings work properly in the table
   of contents
*/
void InsertStyle(const char *the_style)
{
    char *rtf, *last_char;
    int how = INSERT_STYLE_NORMAL;
    char *style = strdup(the_style);

    last_char = style + strlen(style) - 1;
    if (*last_char == '0') {
        how = INSERT_STYLE_NO_STYLE;
        *last_char = '\0'; 
    }
    
    if (strcmp(style,"Normal")==0) {

        if (getAlignment()==CENTERED) {
            InsertStyle("centerpar");
            free(style);
            return;
        }
        if (getAlignment()==RIGHT) {
            InsertStyle("rightpar");
            free(style);
            return;
        }
        if (getAlignment()==LEFT) {
            InsertStyle("leftpar");
            free(style);
            return;
        }
    }

    diagnostics(4, "InsertStyle(%s)", style);
    rtf = SearchCfgRtf(style, STYLE_A);
    if (rtf == NULL) {
        diagnostics(WARNING, "Cannot find '%s' style using 'Normal' style", style);
        SetCurrentStyle("Normal");
        rtf = SearchCfgRtf("Normal", STYLE_A);
        InsertBasicStyle(rtf, INSERT_STYLE_NORMAL);
    } else {
        SetCurrentStyle(style);
        InsertBasicStyle(rtf, how);
    }
    
    free(style);
}
