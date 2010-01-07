/*
 * aux.c - handle the auxiliary files
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
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 * This file is available from http://sourceforge.net/projects/latex2rtf/
 * 
 * Authors: 1995-1997 Ralf Schlatterbeck 1998-2000 Georg Lehner 2001-2002 Scott
 * Prahl
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "utils.h"
#include "parser.h"
#include "convert.h"
/*
 * open and read an auxiliary file.
 * filter lines which start with the
 * macros contained in char *macros[]
 * and send them to ConvertString()
 */

static void FilterAuxFile(char *macros[],FILE *auxFile) {
    char linebuffer[512];
    char **candidate;
    int  tosFStack;
    FILE *fStack[16];
    
    if (auxFile == NULL)
        return;

    tosFStack         = 0;
    fStack[tosFStack] = auxFile;
    
    while (tosFStack != -1) {
        while (fgets(linebuffer,511,fStack[tosFStack]) != NULL) {
            char *lfcr = strchr(linebuffer,'\n');
            if (lfcr != NULL) *lfcr = 0;
            lfcr = strchr(linebuffer,'\r');
            if (lfcr != NULL) *lfcr = 0;
            if (strlen(linebuffer) == 0)
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
                        diagnostics(WARNING,"File not found: %s",fname);
                        tosFStack--;
                    }
                }
                continue;
            }
            for (candidate = macros; *candidate != NULL; candidate++) {
                if (strstarts(linebuffer,*candidate)) {
                    char c1 = linebuffer[strlen(*candidate)];
                    if (c1 != 0) {
                        char c2 = linebuffer[strlen(*candidate)+1];
                        if (c1 == '{' || (c1 == ' ' && c2 == '{'))
                            ConvertString(linebuffer);
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

static char **loadedAuxFiles = NULL;
static int loadedFiles = 0;

static void LoadAuxliaryFile(char *fname,char **macros)
{
    int elems;
    char **newTable;
    
    for (elems = 0; elems < loadedFiles; elems++) {
        if (streq(loadedAuxFiles[elems],fname)) {
            return;             /* already loaded */
        }
    }
    newTable = (char **)realloc(loadedAuxFiles,
                                sizeof(char *) * (loadedFiles+1));
    if (NULL != newTable) {
        FILE *auxFile =           my_fopen(fname,"rb");
        loadedAuxFiles =          newTable;
        newTable[loadedFiles++] = fname;
        if (NULL == auxFile) {
            diagnostics(WARNING, "%s not found.  Run LaTeX to create it.",fname);
        } else {
            FilterAuxFile(macros,auxFile);
            diagnostics(WARNING,"(%s)",fname);
        }
    } else {
        diagnostics(ERROR,"Memory overflow when opening '%s'",fname);
    }
}

/* List of commands to look for when handling acronyms */

char *acronymAux[] = {
    "\\newlabel",
    /*     "\\undonewlabel", */
    "\\bibcite",
    "\\harvardcite",
    "\\newacro",
    "\\newacroplural",
    NULL
};

void LoadAuxFile(void)
{
    LoadAuxliaryFile(g_aux_name,acronymAux);
}
