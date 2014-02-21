/* acronym.c - handle label table

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
    1995      Fernando Dorner, Andreas Granzer, Freidrich Polzer, Gerhard Trisko
    1995-1997 Ralf Schlatterbeck
    1998-2000 Georg Lehner
    2001-2002 Scott Prahl
    
    2009-2010 Pedro A. Aranda
*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "parser.h"
#include "utils.h"
#include "auxfile.h"
#include "labels.h"

static labelElem *labelTable = NULL;
static int labelCount = 0;
static int labelReserved = 0;
#define labelDelta 8

/* reserve 8 labels in a row when we grow the label table */
/* return pointer to new label or NULL if memory overflow */

static labelElem *newLabel(char *name,char *def)
{
    labelElem *result = labelTable;

    if (labelCount == labelReserved) {
        /* time to reserve another chunk */
        /* of 8 labels                   */
        result =
            (labelElem *)realloc(labelTable,
                                (labelReserved+labelDelta)*sizeof(labelElem));
        if (NULL != result) {
            labelTable = result;
            labelReserved += labelDelta;
        }
    }
    
    if (NULL != result) {
        result = &labelTable[labelCount++];
        result->labelName = name;
        result->labelDef  = def;
    }
    return result;
}

labelElem *getLabel(char *name)
{
    labelElem *result = NULL;
    int        label;
    LoadAuxFile();      /* LoadAuxFile if not already loaded */
    
    for (label = 0; label < labelCount; label++) {
        if (streq(name,labelTable[label].labelName)) {
            result = &labelTable[label];
            break;
        }   
    }
    return result;
}

/* support for labels created by \usepackage{nameref}     */
/* \newlabel{acro:undef}{{1}{1}{Test section\relax }{}{}} */
/* returns the nameref                                    */
char *getLabelNameref(char *name)
{
    char      *result = NULL;
    labelElem *label = getLabel(name);
    if (NULL != label) {
        char *fullDef = strdup(label->labelDef);
        PushSource(NULL, fullDef);
        ignoreBraceParam(); /* section */
        ignoreBraceParam(); /* page */
        result = getBraceParam();
        PopSource();
        free(fullDef); /* getBraceParam() strdup's, free this */
    }
    return result;
}

/*  from \newlabel{sec:test}{{1.1}{1}} */
/*  returns the section 1.1 */

char *getLabelSection(char *name)
{
    char      *result = NULL;
    labelElem *label = getLabel(name);
    if (NULL != label) {
        char *fullDef = strdup(label->labelDef);
        PushSource(NULL, fullDef);
        result = getBraceParam();
        PopSource();
        free(fullDef);
    }
    return result;
}

/*  from \newlabel{sec:test}{{1.1}{1}} */
/*  returns the page  1 */

char *getLabelPage(char *name)
{
    char      *result = NULL;
    labelElem *label = getLabel(name);
    if (NULL != label) {
        char *fullDef = strdup(label->labelDef);
        PushSource(NULL, fullDef);
        ignoreBraceParam();
        result = getBraceParam();
        PopSource();
        free(fullDef);
    }
    return result;
}
/*  from \newlabel{sec:test}{{1.1}{1}} */
/*  returns the definition {1.1}{1} */

char *getLabelDefinition(char *name)
{
    char      *result = NULL;
    labelElem *label = getLabel(name);
    if (NULL != label) {
        result = strdup(label->labelDef);
    }
    return result;
}

/*  \newlabel{sec:test}{{1.1}{1}} */

void CmdNewLabel(int code)
{
    if (LABEL_UNDONEW == code) {
        char *p = getBraceParam();
        diagnostics(WARNING,"undoing label '%s'",p);
        free(p);
    } else {
        char *p1 = getBraceParam();
        char *p2 = getBraceParam();

        if (NULL == newLabel(p1,p2)) {
            diagnostics(WARNING,"memory overflow defining label '%s' as '%s'",p1,p2);
            free(p1);
            free(p2);
        }
    }
}
