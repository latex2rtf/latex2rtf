/*
 * $Id: util.c,v 1.8 2001/08/12 19:40:25 prahl Exp $
 * History:
 * $Log: util.c,v $
 * Revision 1.8  2001/08/12 19:40:25  prahl
 * 1.9g
 *         Added commands to read and set TeX lengths
 *         Added commands to read and set TeX counters
 *         Fixed bug in handling of \item[text]
 *         Eliminated comparison of fpos_t variables
 *         Revised getLinenumber ... this is not perfect
 *         Fixed bug in getTexChar() routine
 *         Clearly separate preamble from the document in hopes that
 *           one day more appropriate values for page size, margins,
 *           paragraph spacing etc, will be used in the RTF header
 *         I have added initial support for page sizes still needs testing
 *         added two more test files misc3.tex and misc4.tex
 *         misc4.tex produces a bad rtf file currently
 *         separated all letter commands into letterformat.c
 *         cleaned up warning calls throughout code
 *         added \neq \leq \geq \mid commands to direct.cfg
 *         collected and added commands to write RTF header in preamble.c
 *         broke isolatin1 and hyperlatex support, these will be fixed next version
 *
 * Revision 1.2  1998/07/03 07:03:16  glehner
 * lclint cleaning
 *
 * Revision 1.1  1997/02/15 21:09:16  ralf
 * Initial revision
 *
 */
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "main.h"
#include "util.h"

 /* @null@ *//* @owned@ */ static char *buffer;
static size_t   bufsize = 0;

/*
 * This function assumes there are no ´\0´ characters in the input.
 * if there are any, they are ignored.
 */
/* @dependent@ *//* @null@ */
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
	while ((c = getc(infile)) != EOF && strchr(scanchars, c) == NULL) {
		if (c == (int) '\0') {
			continue;
		}
		if (c == (int) '\n') {
			linenumber++;
		}
		buffer[bufindex++] = (char) c;
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
