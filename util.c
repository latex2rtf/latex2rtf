/*
 * $Id: util.c,v 1.13 2001/10/14 18:24:10 prahl Exp $ 
 */
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "main.h"
#include "util.h"


#ifdef HAS_NO_STRDUP
char           *
strdup(const char *str)
{
	char           *s;

	if ((s = malloc(strlen(str) + 1)) == NULL) {
		error("Cannot allocate memory for string\n");
	}
	strcpy(s, str);
	return s;
}
#endif


