/*
Routines to handle TeX \def and LaTeX \newcommand 

Scott Prahl, October 2001
*/

#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "convert.h"
#include "definitions.h"
#include "parser.h"
#include "funct1.h"
#include "util.h"

#define MAX_DEFINITIONS 50

struct {
	char * name;
	char * def;
	int  params;
} Definitions[MAX_DEFINITIONS];

struct {
	char * name;
	char * begdef;
	char * enddef;
	int  params;
} NewEnvironments[MAX_DEFINITIONS];

static int iDefinitionCount = 0;
static int iNewEnvironmentCount = 0;

int 
strequal(char *a, char *b)
{
	if (a==NULL || b==NULL)
		return 0;
		
	while (*a && *b && *a==*b) {a++;b++;}
	
	if (*a || *b)
		return 0;
	else
		return 1;
}

static void
expandmacro(char *macro, int params)
/**************************************************************************
     purpose: retrieves and expands a defined macro 
**************************************************************************/
{
	int i,param;
	char * args[9], *dmacro, *macro_piece, *next_piece, *expanded, buffer[1024], *cs;
		
	diagnostics(5, "expandmacro macro=<%s>, params=%d", macro, params);

	if (params == 0) {
		ConvertString(macro);
		return;
	}

	for (i=0; i<params; i++) {
		args[i] = getBraceParam();
		diagnostics(6, "argument #%d <%s>", i+1, args[i]);
	}
	
	expanded = buffer;
	dmacro = strdup(macro);
	macro_piece = dmacro;
	
	/* convert "\csname" to "\" */
	while ((cs=strstr(dmacro, "\\csname")) != NULL) strcpy(cs+1,cs+7);
		
	/* remove "\endcsname" */
	while ((cs=strstr(dmacro, "\\endcsname")) != NULL) strcpy(cs,cs+10);
	
	/* do not use strtok because it may be used elsewhere */
	while (macro_piece && *macro_piece) {

		next_piece = strchr(macro_piece, '#');
		if (next_piece) {
			*next_piece = '\0';
			next_piece++;
			param = *next_piece - '1';
			next_piece++;
		} else
			param = -1;
			
		diagnostics(6, "expandmacro piece =<%s>", macro_piece);
		strcpy(expanded,macro_piece);
		expanded += strlen(macro_piece);
		if (param > -1) {
			diagnostics(6, "expandmacro arg =<%s>", args[param]);
			strcpy(expanded,args[param]);
			expanded += strlen(args[param]);
		}
		
		macro_piece = next_piece;
	}
	
	diagnostics(4, "expandmacro expanded=<%s>", buffer);
	ConvertString(buffer);
	for (i=0; i< params; i++)
		if (args[i]) free(args[i]);

	if (dmacro) free(dmacro);
}

int
existsDefinition(char * s)
/**************************************************************************
     purpose: checks to see if a named TeX definition exists
     returns: the array index of the named TeX definition
**************************************************************************/
{
	int n, i=0;
	
	n = strlen(s);
	while(i < iDefinitionCount && !strequal(s,Definitions[i].name)) {
		diagnostics(6, "seeking=<%s>, i=%d, current=<%s>", s,i,Definitions[i].name);
		i++;
	}

	if (i==iDefinitionCount) 
		return -1;
	else
		return i;
}

void
newDefinition(char *name, char *def, int params)
/**************************************************************************
     purpose: allocates and initializes a named TeX definition 
              name should not begin with a '\'  for example to
              define \hd, name = "hd"
**************************************************************************/
{
	if (iDefinitionCount==MAX_DEFINITIONS){
		fprintf(stderr,"Too many definitions, ignoring %s", name);
		return;
	}
	
	Definitions[iDefinitionCount].params=params; 
	
	Definitions[iDefinitionCount].name=strdup(name); 
	
	if (Definitions[iDefinitionCount].name==NULL) {
		fprintf(stderr, "\nCannot allocate name for definition \\%s\n", name);
		exit(1);
	}

	Definitions[iDefinitionCount].def=strdup(def); 

	if (Definitions[iDefinitionCount].def==NULL) {
		fprintf(stderr, "\nCannot allocate def for definition \\%s\n", name);
		exit(1);
	}
	
	iDefinitionCount++;
}

void
renewDefinition(char * name, char * def, int params)
/**************************************************************************
     purpose: allocates (if necessary) and sets a named TeX definition 
**************************************************************************/
{
	int i;
	i = existsDefinition(name);
	
	if (i<0) {
		newDefinition(name, def, params);
		diagnostics(WARNING, "No existing definition for \\%s", name);
		
	} else {
		free(Definitions[i].def);
		Definitions[i].params = params;
		Definitions[i].def = strdup(def);
		if (Definitions[i].def==NULL) {
			diagnostics(WARNING, "\nCannot allocate def for definition \\%s\n", name);
			exit(1);
		}
	}
}

void
expandDefinition(int thedef)
/**************************************************************************
     purpose: retrieves and expands a \newcommand macro 
**************************************************************************/
{
	diagnostics(5, "expandDefinition name=<%s>", Definitions[thedef].name);
	diagnostics(5, "expandDefinition def =<%s>", Definitions[thedef].def);

	expandmacro(Definitions[thedef].def, Definitions[thedef].params);
}

int
existsEnvironment(char * s)
/**************************************************************************
     purpose: checks to see if a user created environment exists
     returns: the array index of the \newenvironment
**************************************************************************/
{
	int n, i=0;
	
	n = strlen(s);
	while(i < iNewEnvironmentCount && !strequal(s,NewEnvironments[i].name)) {
		diagnostics(6, "seeking=<%s>, i=%d, current=<%s>", s,i,NewEnvironments[i].name);
		i++;
	}

	if (i==iNewEnvironmentCount) 
		return -1;
	else
		return i;
}

void
newEnvironment(char *name, char *begdef, char *enddef, int params)
/**************************************************************************
     purpose: allocates and initializes a \newenvironment 
              name should not begin with a '\' 
**************************************************************************/
{
	if (iNewEnvironmentCount==MAX_DEFINITIONS){
		diagnostics(WARNING,"Too many newenvironments, ignoring %s", name);
		return;
	}
	
	NewEnvironments[iNewEnvironmentCount].name=strdup(name); 
	NewEnvironments[iNewEnvironmentCount].begdef=strdup(begdef); 
	NewEnvironments[iNewEnvironmentCount].enddef=strdup(enddef); 
	NewEnvironments[iNewEnvironmentCount].params=params; 

	if (NewEnvironments[iNewEnvironmentCount].name==NULL ||
		NewEnvironments[iNewEnvironmentCount].begdef==NULL ||
	    NewEnvironments[iNewEnvironmentCount].enddef==NULL) {
		diagnostics(ERROR, "Cannot allocate memory for \\newenvironment{%s}", name);
	}
	
	iNewEnvironmentCount++;
}

void
renewEnvironment(char *name, char *begdef, char *enddef, int params)
/**************************************************************************
     purpose: allocates and initializes a \renewenvironment 
**************************************************************************/
{
	int i;
	i = existsEnvironment(name);
	
	if (i<0) {
		newEnvironment(name, begdef, enddef, params);
		diagnostics(WARNING, "No existing \\newevironment{%s}", name);
		
	} else {
		free(NewEnvironments[i].begdef);
		free(NewEnvironments[i].enddef);
		NewEnvironments[i].params = params;
		NewEnvironments[i].begdef = strdup(begdef);
		NewEnvironments[i].enddef = strdup(enddef);
		if (NewEnvironments[i].begdef==NULL || NewEnvironments[i].enddef==NULL) {
			diagnostics(ERROR, "Cannot allocate memory for \\renewenvironment{%s}", name);
		}
	}
}

void
expandEnvironment(int thedef, int code)
/**************************************************************************
     purpose: retrieves and expands a \newenvironment 
**************************************************************************/
{
	if (code == CMD_BEGIN) {
	
		diagnostics(4, "\\begin{%s} <%s>", NewEnvironments[thedef].name, \
										   NewEnvironments[thedef].begdef);
		expandmacro(NewEnvironments[thedef].begdef, NewEnvironments[thedef].params);
	
	} else {

		diagnostics(4, "\\end{%s} <%s>", NewEnvironments[thedef].name, \
										 NewEnvironments[thedef].enddef);
		expandmacro(NewEnvironments[thedef].enddef, 0);
	}
}



