/* Generalised bibliography handling

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
*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "utils.h"
#include "parser.h"
#include "auxfile.h"
#include "biblio.h"

/*
 * *********************************************
 * A try at unifying the citation infrastructure
 * *********************************************
 */
static biblioElem *biblioTable = NULL;
static int         biblioCount = 0;

static biblioElem *newBiblio(char *newKey)
{
    biblioElem *result =
        (biblioElem *)realloc(biblioTable,
                              sizeof(biblioElem) * (biblioCount+1));
    
    if (NULL != result) {
        biblioTable = result;
        result = &result[biblioCount++];
        result->biblioKey  = newKey;
        result->biblioN    = NULL;
        result->biblioFull = NULL;
        result->biblioAbbr = NULL;
        result->biblioYear = NULL;
    }
    return result;
}

static void printTable(void)
{
#ifdef VERBOSE_BIBLIO
    int i;
    for (i=0; i< biblioCount; i++) {
        biblioElem *p = &biblioTable[i];
        diagnostics(WARNING,"biblioTable[%d]",i);
        diagnostics(WARNING,"%s %s %s %s %s %d",
                    p->biblioKey,
                    p->biblioN,
                    p->biblioFull,
                    p->biblioAbbr,
                    p->biblioYear,
                    p->biblioType);
    }
#endif
}

biblioElem *getBiblio(char *key)
{
    int i;
    LoadAuxFile();      /* load auxfile if not already loaded */
    for (i=0; i< biblioCount; i++) {
        if (streq(biblioTable[i].biblioKey,key)) {
            return &biblioTable[i];
        }
    }
    return NULL;
}

/* for normal bibliographic references: */
/* \bibcite{key}{ref}                   */
/* returns ref                          */
char *getBiblioRef(char *key) {
    biblioElem *result = getBiblio(key);
    if (result != NULL) {
        return strdup(result->biblioN);
    }
    return NULL;
}

/* for extended bibliographic references: */
/* \bibcite{key}{{first}{}{}...}          */
/* returns first                          */

char *getBiblioFirst(char *key) {
    biblioElem *result = getBiblio(key);
    if (result != NULL) {
        char *ref = strdup(result->biblioN);
        char *res = NULL;
        if (NULL != ref) {
            PushSource(NULL,ref);
            res = getBraceParam();
            PopSource();
        }
        safe_free(ref);
        return res;
    }
    return NULL;
}
biblioElem *newBibCite(char *cite, char *tag)
{
    biblioElem *newCite = newBiblio(cite);
    if (NULL != newCite) {
        newCite->biblioN    = tag;
        newCite->biblioType = BIBLIO_BIBCITE;
        printTable();
    }
    return newCite;
}

biblioElem *newHarvardCite(char *cite,char *full,char* abbr,char *year)
{
    biblioElem *newCite = newBiblio(cite);
    if (NULL != newCite) {
        newCite->biblioFull = full;
        newCite->biblioAbbr = abbr;
        newCite->biblioYear = year;
        newCite->biblioType = BIBLIO_HARVARD;
    }
    return newCite;
}

biblioElem *newNatBibCite(char *cite,char *full,char* abbr,char *year,char *n)
{
    biblioElem *newCite = newBiblio(cite);
    if (NULL != newCite) {
        newCite->biblioFull = full;
        newCite->biblioAbbr = abbr;
        newCite->biblioYear = year;
        newCite->biblioN    = n;
        newCite->biblioType = BIBLIO_NATBIB;
    }
    return newCite;
}

/*
 * Interface to the AUX file parser
 */

/*  \bibcite{rfc2328}{18} */

void CmdBibCite(int code)
{
    char *p1 = getBraceParam();
    char *p2 = getBraceParam();

    if (NULL == newBibCite(p1,p2)) {
        diagnostics(WARNING,"memory exhausted for \\bibcite{%s}{%s}",p1,p2);
        free(p1);
        free(p2);
    } 
}

/*  \harvardcite{latex}{Lamport}{Lamport}{1986} */

void CmdAuxHarvardCite(int code)
{
    char *key = getBraceParam();
    char *full = getBraceParam();
    char *abbr = getBraceParam();
    char *year = getBraceParam();
   
    if (NULL == newHarvardCite(key,full,abbr,year)) {
        diagnostics(ERROR,"Memory overflow defining \\harvardcite(%s)",key);
    }
}
