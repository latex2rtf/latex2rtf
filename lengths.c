/*
Routines to access TeX variables that contain TeX lengths
By convention all the values stored should be twips  20 twips = 1 pt

Scott Prahl, June 2001
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "main.h"
#include "lengths.h"

#define MAX_LENGTHS 50

struct {
	char * name;
	int distance;
} Lengths[MAX_LENGTHS];

static int iLengthCount = 0;

static int
existsLength(char * s)
/**************************************************************************
     purpose: checks to see if a named TeX dimension exists
     returns: the array index of the named TeX dimension
**************************************************************************/
{
	int i=0;
	
	while(i < iLengthCount && strstr(Lengths[i].name,s)==NULL)
		i++;

	if (i==iLengthCount) 
		return -1;
	else
		return i;
}

static void
newLength(char *s, int d)
/**************************************************************************
     purpose: allocates and initializes a named TeX dimension 
**************************************************************************/
{
	if (iLengthCount==MAX_LENGTHS){
		diagnostics(WARNING,"Too many lengths, ignoring %s", s);
		return;
	}
	
	Lengths[iLengthCount].distance=d;
	Lengths[iLengthCount].name=strdup(s); 
	
	if (Lengths[iLengthCount].name==NULL) {
		fprintf(stderr, "\nCannot allocate name for length \\%s\n", s);
		exit(1);
	}

	iLengthCount++;
}

void
setLength(char * s, int d)
/**************************************************************************
     purpose: allocates (if necessary) and sets a named TeX dimension 
**************************************************************************/
{
	int i;
	i = existsLength(s);
	
	if (i<0) 
		newLength(s, d);
	else
		Lengths[i].distance = d;
}

int
getLength(char * s)
/**************************************************************************
     purpose: retrieves a named TeX dimension 
**************************************************************************/
{
	int i;
	i = existsLength(s);
	
	if (i<0) {
		diagnostics(WARNING, "No length of type %s", s);
		return 0;
	}
		
	return Lengths[i].distance;
}

