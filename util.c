/*
 * $Id: util.c,v 1.11 2001/10/12 05:45:07 prahl Exp $ 
 */
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "main.h"
#include "util.h"

 /* @null@ *//* @owned@ */ static char *buffer;
static size_t   bufsize = 0;

#define CR (char) 0x0d
#define LF (char) 0x0a
/*
 * This function assumes there are no '\0' characters in the input.
 * if there are any, they are ignored.
 */
char           *
ReadUptoMatch(FILE * infile, /* @observer@ */ const char *scanchars)
{
	size_t          bufindex = 0;
	int             c;

	if (feof(infile) != 0) {
		return NULL;
	}
	if (buffer == NULL) {
		if ((buffer = malloc(BUFFER_INCREMENT)) == NULL) {
			Fatal("Cannot allocate memory for input buffer\n");
		}
		bufsize = BUFFER_INCREMENT;
	}
	while ((c = getc(infile)) != EOF ) {
	
		if (c == CR || c == LF)
			c = '\n';
		
		if (strchr(scanchars, c))
			break;
			
		if (c == (int) '\0') {
			continue;
		}
/*		if (c == (int) '\n') {
			linenumber++;
		}
*/		buffer[bufindex++] = (char) c;
		if (bufindex >= bufsize) {
			if ((buffer = realloc(buffer, bufsize += BUFFER_INCREMENT)) == NULL) {
				Fatal("Cannot allocate memory for input buffer\n");
			}
		}
	}
	buffer[bufindex] = '\0';
	if (c != EOF) {
		ungetc(c, infile);	/* LEG210698*** lclint, GNU libc
					 * doesn't say what's the return
					 * value */
	}
	return buffer;
}

#ifdef HAS_NO_STRDUP
char           *
strdup(const char *str)
{
	char           *s;

	if ((s = malloc(strlen(str) + 1)) == NULL) {
		Fatal("Cannot allocate memory for string\n");
	}
	strcpy(s, str);
	return s;
}
#endif

/* @exits@ */
void 
ParseError(const char *fmt,...)
{
	va_list         ap;

	fprintf(stderr, "%s: %s %4ld: ", progname, currfile, linenumber);
	va_start(ap, fmt);
	(void) vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

void 
Fatal(const char *fmt,...)
{
	va_list         ap;

	fprintf(stderr, "%s: Fatal error: ", progname);
	va_start(ap, fmt);
	(void) vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}


/* convert integer to roman number --- only works up correctly up to 39 */

void 
roman_item(int n, char *s)
{
	int             i = 0;

	while (n >= 10) {
		n -= 10;
		s[i] = 'x';
		i++;
	}

	if (n == 9) {
		s[i] = 'i';
		i++;
		s[i] = 'x';
		i++;
		s[i] = '\0';
		return;
	}
	if (n >= 5) {
		n -= 5;
		s[i] = 'v';
		i++;
	}
	if (n == 4) {
		s[i] = 'i';
		i++;
		s[i] = 'v';
		i++;
		s[i] = '\0';
		return;
	}
	while (n >= 1) {
		n -= 1;
		s[i] = 'i';
		i++;
	}

	s[i] = '\0';
}
