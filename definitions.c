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
#include "cfg.h"
#include "counters.h"
#include "funct1.h"

#define MAX_DEFINITIONS 200
#define MAX_ENVIRONMENTS 20
#define MAX_THEOREMS 20

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
} NewEnvironments[MAX_ENVIRONMENTS];

struct {
	char * name;
	char * numbered_like;
	char * caption;
	char * within;
} NewTheorems[MAX_THEOREMS];

static int iDefinitionCount = 0;
static int iNewEnvironmentCount = 0;
static int iNewTheoremCount = 0;

static int 
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

/* static void printDefinitions(void)
{
int i=0;
	fprintf(stderr, "\n");
	while(i < iDefinitionCount ) {
		fprintf(stderr, "[%d] name   =<%s>\n",i, Definitions[i].name);
		fprintf(stderr, "    def    =<%s>\n", Definitions[i].def);
		fprintf(stderr, "    params =<%d>\n", Definitions[i].params);
		i++;
	}
}

static void printTheorems(void)
{
int i=0;
	fprintf(stderr, "\n");
	for (i=0; i< iNewTheoremCount; i++) {
		fprintf(stderr, "[%d] name   =<%s>\n",i, NewTheorems[i].name);
		fprintf(stderr, "    caption    =<%s>\n", NewTheorems[i].caption);
		fprintf(stderr, "    like =<%s>\n", NewTheorems[i].numbered_like);
		fprintf(stderr, "    within    =<%s>\n", NewTheorems[i].within);
	}
}
*/

static char *
expandmacro(char *macro, int params)
/**************************************************************************
     purpose: retrieves and expands a defined macro 
**************************************************************************/
{
	int i,param;
	char * args[9], *dmacro, *macro_piece, *next_piece, *expanded, buffer[1024], *cs;

	if (params<=0) 
		return strdup(macro);
	
	for (i=0; i<params; i++) {
		args[i] = getBraceParam();
		diagnostics(3, "argument #%d <%s>", i+1, args[i]);
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
			if (*next_piece=='#')
				param = 101;				/* just a flag for below */
			else
				param = *next_piece - '1';
			next_piece++;
		} else
			param = -1;
			
		diagnostics(3, "expandmacro piece =<%s>", macro_piece);
		strcpy(expanded,macro_piece);
		expanded += strlen(macro_piece);
		if (param > -1) {
			if (param==101) {
				diagnostics(3, "expandmacro ## = #");
				strcpy(expanded,"#");
				expanded ++;
			} else if (param<params) {
				diagnostics(3, "expandmacro arg =<%s>", args[param]);
				strcpy(expanded,args[param]);
				expanded += strlen(args[param]);
			} else
				diagnostics(WARNING,"confusing definition in macro=<%s>", macro);
		}
		
		macro_piece = next_piece;
	}
	
	
/*	ConvertString(buffer);*/
	for (i=0; i< params; i++)
		if (args[i]) free(args[i]);

	if (dmacro) free(dmacro);

	diagnostics(3, "expandmacro expanded=<%s>", buffer);
	return strdup(buffer);
}

int
maybeDefinition(char * s, int n)
/**************************************************************************
     purpose: checks to see if a named TeX definition possibly exists
     returns: the array index of the named TeX definition
**************************************************************************/
{
	int i;
	
	if (n==0) return TRUE;
	
	for (i=0; i<iDefinitionCount; i++) {
		diagnostics(6, "seeking=<%s>, i=%d, current=<%s>", s,i,Definitions[i].name);
		if (strncmp(s,Definitions[i].name,n) == 0) 
			return TRUE;
	}

	return FALSE;
}

int
existsDefinition(char * s)
/**************************************************************************
     purpose: checks to see if a named TeX definition exists
     returns: the array index of the named TeX definition
**************************************************************************/
{
	int i;
	
	for (i=0; i<iDefinitionCount; i++) {
		diagnostics(6, "seeking=<%s>, i=%d, current=<%s>", s,i,Definitions[i].name);
		if (strcmp(s,Definitions[i].name) == 0) break;
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
	diagnostics(3,"Adding macro <%s>=<%s>",name,def);

	if (strcmp(name,"LaTeX")==0) return;
	if (strcmp(name,"TeX")==0) return;
	if (strcmp(name,"AmSTeX")==0) return;
	if (strcmp(name,"BibTex")==0) return;
	if (strcmp(name,"LaTeXe")==0) return;
	if (strcmp(name,"AmSLaTeX")==0) return;
	
	if (iDefinitionCount==MAX_DEFINITIONS){
		diagnostics(WARNING,"Too many definitions, ignoring %s", name);
		return;
	}
	
	Definitions[iDefinitionCount].params=params; 
	
	Definitions[iDefinitionCount].name=strdup(name); 
	
	if (Definitions[iDefinitionCount].name==NULL) {
		diagnostics(ERROR, "\nCannot allocate name for definition \\%s\n", name);
	}

	Definitions[iDefinitionCount].def=strdup(def); 

	if (Definitions[iDefinitionCount].def==NULL) {
		diagnostics(ERROR, "\nCannot allocate def for definition \\%s\n", name);
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

	diagnostics(3,"renewDefinition seeking <%s>\n",name);
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

char *
expandDefinition(int thedef)
/**************************************************************************
     purpose: retrieves and expands a \newcommand macro 
**************************************************************************/
{

	diagnostics(3, "expandDefinition name   =<%s>", Definitions[thedef].name);
	diagnostics(3, "expandDefinition def    =<%s>", Definitions[thedef].def);
	diagnostics(3, "expandDefinition params =<%d>", Definitions[thedef].params);

	return expandmacro(Definitions[thedef].def, Definitions[thedef].params);
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
	if (iNewEnvironmentCount==MAX_ENVIRONMENTS){
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

char *
expandEnvironment(int thedef, int code)
/**************************************************************************
     purpose: retrieves and expands a \newenvironment 
**************************************************************************/
{
	if (code == CMD_BEGIN) {
	
		diagnostics(4, "\\begin{%s} <%s>", NewEnvironments[thedef].name, \
										   NewEnvironments[thedef].begdef);
		return expandmacro(NewEnvironments[thedef].begdef, NewEnvironments[thedef].params);
	
	} else {

		diagnostics(4, "\\end{%s} <%s>", NewEnvironments[thedef].name, \
										 NewEnvironments[thedef].enddef);
		return expandmacro(NewEnvironments[thedef].enddef, 0);
	}
}

void
newTheorem(char *name, char *caption, char *numbered_like, char *within)
/**************************************************************************
     purpose: allocates and initializes a \newtheorem 
**************************************************************************/
{
	if (iNewTheoremCount==MAX_THEOREMS){
		diagnostics(WARNING,"Too many \\newtheorems, ignoring %s", name);
		return;
	}
	
	NewTheorems[iNewTheoremCount].name=strdup(name); 
	
	NewTheorems[iNewTheoremCount].caption=strdup(caption); 

	if (numbered_like)
		NewTheorems[iNewTheoremCount].numbered_like=strdup(numbered_like);
	else 
		NewTheorems[iNewTheoremCount].numbered_like=strdup(name);

	if (within)
		NewTheorems[iNewTheoremCount].within=strdup(within);
	else 
		NewTheorems[iNewTheoremCount].within=NULL;
		
	setCounter(NewTheorems[iNewTheoremCount].numbered_like,0);

	iNewTheoremCount++;
}

int
existsTheorem(char * s)
/**************************************************************************
     purpose: checks to see if a user created environment exists
     returns: the array index of the \newtheorem
**************************************************************************/
{
	int i=0;
	
	while(i < iNewTheoremCount && !strequal(s,NewTheorems[i].name)) {
		diagnostics(6, "seeking=<%s>, i=%d, current=<%s>", s,i,NewTheorems[i].name);
		i++;
	}

	if (i==iNewTheoremCount) 
		return -1;
	else
		return i;
}

char *
expandTheorem(int i, char *option)
/**************************************************************************
     purpose: retrieves and expands a \newtheorem into a string
**************************************************************************/
{	
	char s[128], *num;
	int ithm;
	
	if (i<0 || i>=iNewTheoremCount)
		return strdup("");
	
	incrementCounter(NewTheorems[i].numbered_like);
	ithm = getCounter(NewTheorems[i].numbered_like);
	
	if (NewTheorems[i].within) {
		num = FormatUnitNumber(NewTheorems[i].within);
		if (option)
			sprintf(s,"%s %s.%d (%s)", NewTheorems[i].caption, num, ithm, option);
		else
			sprintf(s,"%s %s.%d", NewTheorems[i].caption, num, ithm);
		free(num);
	} else {
		if (option)
			sprintf(s,"%s %d (%s)", NewTheorems[i].caption, ithm, option);
		else
			sprintf(s,"%s %d", NewTheorems[i].caption, ithm);
	}
			
	return strdup(s);
}

void
resetTheoremCounter(char *unit)
/**************************************************************************
     purpose: resets theorem counters based on unit
**************************************************************************/
{	
	int i;
	
	for (i=0; i<iNewTheoremCount; i++) {
		if (strequal(unit,NewTheorems[i].within))
			setCounter(NewTheorems[i].numbered_like, 0);
	}
}

