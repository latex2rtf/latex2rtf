/*
 * $Id: util.c,v 1.15 2001/10/25 14:41:51 prahl Exp $ 
 */
#include <stdlib.h>
#include <string.h>
#include "main.h"

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


