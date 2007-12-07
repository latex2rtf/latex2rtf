
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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

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

void InsertBasicStyle(const char *rtf, bool include_header_info)

/******************************************************************************
  purpose: uses data from style.cfg to try and insert RTF commands
           that correspond to the appropriate style sheet or character style
           for example
           		ApplyStyle("section", FALSE);
           		
        rtf="\rtfsequence,\rtfheader"
 ******************************************************************************/
{
    char *style, *style_end, *comma;
    int font_number;

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

    if (include_header_info)
        style_end = style + strlen(style);
    else
        style_end = comma;
        
    while (style < style_end) {

        if (style == comma)
        	fprintRTF(" ");
        else if (*style == '*')
            font_number = WriteFontName((const char **)&style);
        else
            fprintRTF("%c", *style);

        style++;
    }
    
 /* emit final blank to make sure that RTF is properly terminated */
    if (!include_header_info)
    	fprintRTF(" ");
}

static void StyleCount(char *rtfline, int *optional, int *mandated)
{
    int n;

    *optional = 0;
    *mandated = 0;

    n = sscanf(rtfline, " %d , %d ,", optional, mandated);

    if (n != 2)
        diagnostics(ERROR, "bad rtf line <%s> in style.cfg", rtfline);

    if (*optional < 0 || *optional > 9)
        diagnostics(ERROR, "bad number of optional parameters in rtf command <%s> style.cfg", rtfline);

    if (*mandated < 0 || *mandated > 9)
        diagnostics(ERROR, "bad number of mandatory parameters in rtf command <%s> style.cfg", rtfline);
}

void InsertStyle(const char *command)
{
    const char *rtf;

    diagnostics(5, "Inserting style '%s'", command);
    rtf = SearchCfgRtf(command, STYLE_A);
    if (rtf == NULL) {
        diagnostics(WARNING, "Cannot find '%s' style using 'Normal' style", command);
    	SetCurrentStyle("Normal");
    	rtf = SearchCfgRtf("Normal", STYLE_A);
        InsertBasicStyle(rtf, FALSE);
    } else {
    	SetCurrentStyle(command);
        InsertBasicStyle(rtf, FALSE);
    }
}


bool TryStyleConvert(char *command)

/******************************************************************************
  purpose: uses data from style.cfg to try and convert some
           LaTeX commands into RTF commands using stylesheet info.  
 ******************************************************************************/
{
    char *rtf;
    char *RtfCommand;
    char *TexCommand;
    char *option[9];

/*	char *	option_header[9];*/
    char *mandatory[9];

/*	char *  mandatory_header[9];*/
    char *rtf_piece[40];
    char *comma;
    int optional;
    int mandated;
    int i;

    TexCommand = strdup_together("\\", command);
    RtfCommand = SearchCfgRtf(TexCommand, STYLE_A);
    if (RtfCommand == NULL)
        return FALSE;

    rtf = RtfCommand;
    StyleCount(rtf, &optional, &mandated);

    /* read all the optional and mandatory parameters */
    for (i = 0; i < optional; i++) {
        option[i] = getBracketParam();
    }

    for (i = 0; i < mandated; i++) {
        mandatory[i] = getBraceParam();
    }

    /* read and duplicate the RTF pieces into an array */
    for (i = 0; i < mandated + optional + 1; i++) {
        comma = strchr(rtf, ',');
        if (comma == NULL)
            diagnostics(ERROR, "Not enough commas in style command <%s>", RtfCommand);

        *comma = '\0';
        rtf_piece[i] = strdup(rtf);
        diagnostics(WARNING, "piece %d is %s", i, rtf_piece[i]);
        *comma = ',';
        rtf = comma + 1;
    }


/* free all the pieces */
    for (i = 0; i < optional; i++) {
        if (option[i])
            free(option[i]);
    }

    for (i = 0; i < mandated; i++) {
        if (mandatory[i])
            free(mandatory[i]);
    }

    for (i = 0; i < mandated + optional + 1; i++) {
        if (rtf_piece[i])
            free(rtf_piece[i]);
    }

    free(TexCommand);
    return TRUE;
}
