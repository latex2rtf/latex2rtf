/*
 * $Id: util.c,v 1.18 2001/11/23 21:43:48 prahl Exp $ 
 */
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "util.h"
#include "parser.h"

#ifdef HAS_NO_STRDUP
char           *
strdup(const char *str)
{
	char           *s;

	if ((s = malloc(strlen(str) + 1)) == NULL) {
		diagnostics(ERROR,"Cannot allocate memory for string\n");
	}
	strcpy(s, str);
	return s;
}
#endif

char *  
strdup_together(char *s, char *t)
/******************************************************************************
 purpose:  returns a new string consisting of s+t
******************************************************************************/
{
	char * both;
	
	both = malloc(strlen(s) + strlen(t) + 1);
	if (both == NULL)
		diagnostics(ERROR, "Could not allocate memory for both strings.");

	strcpy(both, s);
	strcat(both, t);
	return both;
}

char *
strdup_noblanks(char *s)
/******************************************************************************
 purpose:  duplicates a string without including spaces or newlines
******************************************************************************/
{
char *p, *dup;
	while (*s == ' ' || *s == '\n') s++;	/* skip to non blank */
	dup = malloc(strlen(s) + 1);
	p = dup;
	while (*s) {
		*p = *s;
		if (*p != ' ' && *p != '\n') p++;	/* increment if non-blank */
		s++;
	}
	*p = '\0';		
	return dup;
}

char * 
strdup_nobadchars(char * text)
/*************************************************************************
purpose: duplicate text with only a..z A..Z 0..9 and _
 ************************************************************************/
{
	char *dup, *s;
	
	dup = strdup_noblanks(text);
	s = dup;
	
	while (*s) {
		if (!('a' <= *s && *s <= 'z') &&
		    !('A' <= *s && *s <= 'Z') &&
		    !('0' <= *s && *s <= '9'))
			*s = '_';
		s++;
	}

	return dup;
}

char *
ExtractLabelTag(char *text)
/******************************************************************************
  purpose: return a copy of tag from \label{tag} in the string text
 ******************************************************************************/
{
	char *s, *label_with_spaces, *label;
	
	s = strstr(text,"\\label{");
	if (!s) s = strstr(text,"\\label ");
	if (!s) return NULL;
	
	s += strlen("\\label");
	PushSource(NULL,s);
	label_with_spaces = getBraceParam();
	PopSource();
	label = strdup_nobadchars(label_with_spaces);
	free(label_with_spaces);

	diagnostics(4, "LabelTag = <%s>", (label) ? label : "missing");
	return label;
}


