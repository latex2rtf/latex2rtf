/*
 * $Id: util.c,v 1.23 2002/05/08 05:07:29 prahl Exp $ 
 */
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "util.h"
#include "parser.h"

char *  
strndup(char *src, int n)
/******************************************************************************
 purpose:  returns a new string with n characters from s (with '\0' at the end)
******************************************************************************/
{
	char *dst;
	
	dst = (char *) calloc(n+1,sizeof(char));
	if (dst==NULL) return NULL;
		
	strncpy(dst,src,n);

	return dst;
}

char *  
strdup_together(char *s, char *t)
/******************************************************************************
 purpose:  returns a new string consisting of s+t
******************************************************************************/
{
	char * both;
	
	if (s==NULL) {
		if (t==NULL) return NULL;
		return strdup(t);
	}
	
	if (t==NULL)
		return strdup(s);
		
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
	if (s==NULL) return NULL;
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
strdup_noendblanks(char * s)
/******************************************************************************
 purpose:  duplicates a string without spaces or newlines at front or end
******************************************************************************/
{
char *p, *t;

	if (s==NULL) return NULL;
	if (*s=='\0') return strdup("");
	
	t=s;
	while (*t == ' ' || *t == '\n') t++;	/* first non blank char*/
	
	p = s + strlen(s) -1;
	while (p >= t && (*p == ' ' || *p == '\n')) p--;	/* last non blank char */
	
	if (t>p) return strdup("");
	return strndup(t,p-t+1);
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


