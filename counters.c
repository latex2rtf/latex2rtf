/*
Routines to access TeX variables that contain TeX counters

Scott Prahl, June 2001
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "main.h"
#include "counters.h"

#define MAX_COUNTERS 50

struct {
	char * name;
	int number;
} Counters[MAX_COUNTERS];

static int iCounterCount = 0;

static int
existsCounter(char * s)
/**************************************************************************
     purpose: checks to see if a named TeX counter exists
     returns: the array index of the named TeX counter
**************************************************************************/
{
	int i=0;
	
	while(i < iCounterCount && strstr(Counters[i].name,s)==NULL)
		i++;

	if (i==iCounterCount) 
		return -1;
	else
		return i;
}

static void
newCounter(char *s, int n)
/**************************************************************************
     purpose: allocates and initializes a named TeX counter 
**************************************************************************/
{
	if (iCounterCount==MAX_COUNTERS){
		fprintf(stderr,"Too many counters, ignoring %s", s);
		return;
	}
	
	Counters[iCounterCount].number=n;
	Counters[iCounterCount].name=strdup(s); 
	
	if (Counters[iCounterCount].name==NULL) {
		fprintf(stderr, "\nCannot allocate name for counter \\%s\n", s);
		exit(1);
	}

	iCounterCount++;
}

void
incrementCounter(char * s)
/**************************************************************************
     purpose: increments a TeX counter (or initializes to 1) 
**************************************************************************/
{
	int i;
	i = existsCounter(s);
	
	if (i<0) 
		newCounter(s, 1);
	else
		Counters[i].number++;
}

void
setCounter(char * s, int n)
/**************************************************************************
     purpose: allocates (if necessary) and sets a named TeX counter 
**************************************************************************/
{
	int i;
	i = existsCounter(s);
	
	if (i<0) 
		newCounter(s, n);
	else
		Counters[i].number = n;
}

int
getCounter(char * s)
/**************************************************************************
     purpose: retrieves a named TeX counter 
**************************************************************************/
{
	int i;
	i = existsCounter(s);
	
	if (i<0) {
		fprintf(stderr, "No counter of type <%s>", s);
		return 0;
	}
		
	return Counters[i].number;
}

