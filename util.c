/*
 * $Id: util.c,v 1.16 2001/10/27 06:13:58 prahl Exp $ 
 */
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "util.h"

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
	
	dup = strdup(text);
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

