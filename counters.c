
/* counters.c - Routines to access TeX variables that contain TeX counters

Copyright (C) 2001-2002 The Free Software Foundation

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

#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "utils.h"
#include "counters.h"

#define MAX_COUNTERS 500

typedef struct {
    char *name;
    int number;
} counter_type;

counter_type Counters[MAX_COUNTERS];

static int iCounterCount = 0;

static int existsCounter(char *s)

/**************************************************************************
     purpose: checks to see if a named TeX counter exists
     returns: the array index of the named TeX counter
**************************************************************************/
{
    int i = 0;

    while (i < iCounterCount && strstr(Counters[i].name, s) == NULL)
        i++;

    if (i == iCounterCount)
        return -1;
    else
        return i;
}

static void newCounter(char *s, int n)

/**************************************************************************
     purpose: allocates and initializes a named TeX counter 
**************************************************************************/
{
    if (iCounterCount == MAX_COUNTERS) {
        diagnostics(WARNING, "Too many counters, ignoring %s", s);
        return;
    }

    Counters[iCounterCount].number = n;
    Counters[iCounterCount].name = strdup(s);

    if (Counters[iCounterCount].name == NULL) {
        diagnostics(WARNING, "\nCannot allocate name for counter \\%s\n", s);
        exit(1);
    }

    iCounterCount++;
}

void incrementCounter(char *s)

/**************************************************************************
     purpose: increments a TeX counter (or initializes to 1) 
**************************************************************************/
{
    int i;

    i = existsCounter(s);

    if (i < 0)
        newCounter(s, 1);
    else
        Counters[i].number++;
}

void setCounter(char *s, int n)

/**************************************************************************
     purpose: allocates (if necessary) and sets a named TeX counter 
**************************************************************************/
{
    int i;

    i = existsCounter(s);

    if (i < 0)
        newCounter(s, n);
    else
        Counters[i].number = n;
}

int getCounter(char *s)

/**************************************************************************
     purpose: retrieves a named TeX counter 
**************************************************************************/
{
    int i;

    i = existsCounter(s);

    if (i < 0) {
        diagnostics(2, "No counter of type <%s>", s);
        return 0;
    }

    return Counters[i].number;
}

void zeroKeyCounters(char *key)

/**************************************************************************
     purpose: zeros all the acronym counters
**************************************************************************/
{    
    int i;
    for (i=0; i < iCounterCount; i++) {
        if (strstr(Counters[i].name,key)!=Counters[i].name)
            Counters[i].number = 0;
    }
}

int TryTheCounter(char *theCommand)
/*
  Purpose: see if this is a \the<counter> for a defined counter
*/
{
	char *varCandidate = NULL;
	int varValue;

	if (strlen(theCommand) <= 3)
		return FALSE;
	if (strncmp(theCommand, "the", 3) != 0)
		return FALSE;
	varCandidate = &theCommand[3];
	varValue = existsCounter(varCandidate);

	if (varValue == -1) {
		diagnostics(WARNING, "Can '%s' reference a variable?", varCandidate);
	} else {
		varValue = getCounter(varCandidate);
		/* diagnostics(WARNING, "Assuming '%s' is a variable: [%d]", varCandidate, varValue); */
		fprintRTF("%d", varValue);
	}
	return TRUE;
}
