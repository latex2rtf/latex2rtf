/*
 * auxfile.c - handle .aux files
 * 
 * Copyright (C) 1995-2002 The Free Software Foundation
 * 
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 * 
 * This file is available from http://sourceforge.net/projects/latex2rtf/
 * 
 * Authors: 
 * 2009-2010 Pedro A. Aranda
 *           Scott Prahl 
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "utils.h"
#include "parser.h"
#include "convert.h"
#include "auxfile.h"

/* List of commands to look for when parsing the aux file */
/* TODO: put all commands in a CommandArray and create    */
/* an environment for .aux file handling                  */

char *acronymAux[] = {
    "\\newlabel",
    "\\newacro",
    "\\newacroplural",
    "\\harvardcite",
    "\\bibcite",
    NULL
};

/*
 * open and read an auxiliary file.
 * filter lines which start with the
 * macros contained in char *macros[]
 * and send them to ConvertString()
 */

static void FilterAuxFile(FILE *auxFile) {
    char linebuffer[1024];
    char **candidate;
    int  tosFStack;
    FILE *fStack[16];
    
    if (auxFile == NULL)
        return;

    tosFStack         = 0;
    fStack[tosFStack] = auxFile;
    
    while (tosFStack != -1) {

        while (my_fgets(linebuffer,1023,fStack[tosFStack]) != NULL) {
            
            if (strlen(linebuffer) == 0) /* should not happen */
                continue;
            
            if (strstarts(linebuffer,"\\@input{")) {
                char *fname = strchr(linebuffer,'{'); 
                char *p     = strchr(fname,'}');
                fname++;
                *p = '\0';
                if (tosFStack >= 15) {
                    diagnostics(WARNING,"AUX File stack overflow for <%s>",fname);
                } else {
                    if (NULL == (fStack[++tosFStack] = fopen(fname,"r"))) {
                        /* pop, warn and ignore */
                        diagnostics(WARNING,"File not found: %s",fname);
                        tosFStack--;
                    } else {
                        /* pushed => continue */
                        diagnostics(WARNING,"([%d] %s)",tosFStack,fname);
                    }
                }
                continue;
            }
            for (candidate = acronymAux; *candidate != NULL; candidate++) {
                if (strstarts(linebuffer,*candidate)) {
                    char c1 = linebuffer[strlen(*candidate)];
                    if (c1 != 0) {
                        char c2 = linebuffer[strlen(*candidate)+1];
                        if (c1 == '{' || (c1 == ' ' && c2 == '{'))
                            ConvertString(strdup(linebuffer));
                    }
                }
            }
        }
        if (tosFStack > 0) {
            fclose(fStack[tosFStack]);
        }
        tosFStack --;
    }
}

static int alreadyLoaded = FALSE;

void LoadAuxFile(void)
{
    FILE *auxFile;
    /* load and parse only once */
    if (alreadyLoaded == TRUE)
        return;
    alreadyLoaded = TRUE;
    /* ------------------------ */
    auxFile =  my_fopen(g_aux_name,"rb");
    if (NULL == auxFile) {
        diagnostics(WARNING, "%s not found.  Run LaTeX to create it.",g_aux_name);
    } else {
        diagnostics(WARNING,"(%s)",g_aux_name);
        FilterAuxFile(auxFile);
    }
}
