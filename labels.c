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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

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
#include "convert.h"
#include "funct1.h"
#include "commands.h"
#include "stack.h"
#include "fonts.h"
#include "cfg.h"
#include "ignore.h"
#include "utils.h"
#include "encodings.h"
#include "parser.h"
#include "counters.h"
#include "lengths.h"
#include "definitions.h"
#include "preamble.h"
#include "xrefs.h"
#include "equations.h"
#include "direct.h"
#include "styles.h"
#include "graphics.h"
#include "vertical.h"
#include "acronym.h"
#include "biblio.h"
#include "labels.h"

static labelElem *labelTable = NULL;
static int labelCount = 0;

static labelElem *newLabel(char *name,char *def)
{
    labelElem *result;

    labelTable =
        (labelElem *)realloc(labelTable,
                             (labelCount+1)*sizeof(labelElem));
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
    for (label = 0; label < labelCount; label++) {
        if (streq(name,labelTable[label].labelName)) {
            result = &labelTable[label];
            break;
        }   
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
        char *p;
        PushSource(NULL, fullDef);
        p = getBraceParam();
        result = getBraceParam();
        if (NULL != p) free(p);
        PopSource();
        free(fullDef);
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
