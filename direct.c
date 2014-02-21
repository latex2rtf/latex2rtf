
/* direct.c - Convert simple LaTeX commands using direct.cfg 

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
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "direct.h"
#include "fonts.h"
#include "cfg.h"
#include "utils.h"

#define MAXFONTLEN 100

void WriteCFGFontNumber(char **cfg_stream)

/******************************************************************************
  purpose: reads the fontname from cfg_stream and returns the appropriate font
           number. Supports the *FONTNAME* syntax in direct.cfg and in style.cfg
 ******************************************************************************/
{
    int n=-1;
    char *font_name, *star;

    *cfg_stream += 1;                  /* move past initial '*' */
    
    if (**cfg_stream == '*') {     /* two stars in a row ... bail */
        fprintRTF("*");
        return;
    }
    
    font_name = strdup(*cfg_stream);   
    star = strchr(font_name,'*');
    
    if (star) {
        *star = '\0';
        n = TexFontNumber(font_name);
        
        if (n < 0)
            diagnostics(ERROR, "Unknown font <%s>\nFound in cfg line <%s>", font_name, cfg_stream);

        *cfg_stream += strlen(font_name);
        fprintRTF("%d",n);
    }
    
    free(font_name);
}

int TryDirectConvert(char *command)
/******************************************************************************
  purpose: uses data from direct.cfg to try and immediately convert some
           LaTeX commands into RTF commands.  
 ******************************************************************************/
{
    char *buffpoint, *RtfCommand, *TexCommand;

    TexCommand = strdup_together("\\", command);
    RtfCommand = SearchCfgRtf(TexCommand, DIRECT_A);
    if (RtfCommand == NULL) {
        free(TexCommand);
        return FALSE;
    }
    
    buffpoint = RtfCommand;
    diagnostics(4, "Directly converting %s to %s", TexCommand, RtfCommand);
    while (buffpoint[0] != '\0') {
        if (buffpoint[0] == '*')
            WriteCFGFontNumber(&buffpoint);
        else
            fprintRTF("%c", *buffpoint);

        ++buffpoint;

    }
    free(TexCommand);
    return TRUE;
}
