/*
 * $Id: util.c,v 1.7 2001/08/12 19:32:24 prahl Exp $
 * History:
 * $Log: util.c,v $
 * Revision 1.7  2001/08/12 19:32:24  prahl
 * 1.9f
 * 	Reformatted all source files ---
 * 	    previous hodge-podge replaced by standard GNU style
 * 	Compiles cleanly using -Wall under gcc
 *
 * 	added better translation of \frac, \sqrt, and \int
 * 	forced all access to the LaTeX file to use getTexChar() or ungetTexChar()
 * 	    allows better handling of %
 * 	    simplified and improved error checking
 * 	    eliminates the need for WriteTemp
 * 	    potentially allows elimination of getLineNumber()
 *
 * 	added new verbosity level -v5 for more detail
 * 	fixed bug with in handling documentclass options
 * 	consolidated package and documentclass options
 * 	fixed several memory leaks
 * 	enabled the use of the babel package *needs testing*
 * 	fixed bug in font used in header and footers
 * 	minuscule better support for french
 * 	Added testing file for % comment support
 * 	Enhanced frac.tex to include \sqrt and \int tests also
 * 	Fixed bugs associated with containing font changes in
 * 	    equations, tabbing, and quote environments
 * 	Added essential.tex to the testing suite --- pretty comprehensive test.
 * 	Perhaps fix missing .bbl crashing bug
 * 	Fixed ?` and !`
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

/*@null@*/ /*@owned@*/ static char *buffer;
                       static size_t   bufsize = 0;

/*
 * This function assumes there are no ´\0´ characters in the input.
 * if there are any, they are ignored.
 */
/*@dependent@*/ /*@null@*/
char *ReadUptoMatch (FILE *infile, /*@observer@*/ const char *scanchars)
{
    size_t bufindex = 0;
    int c;

    if (feof (infile) != 0)
    {
	return NULL;
    }

    if(buffer == NULL)
    {
	if((buffer = malloc(BUFFER_INCREMENT)) == NULL)
	{
	    Fatal("Cannot allocate memory for input buffer\n");
	}
	bufsize = BUFFER_INCREMENT;
    }

    while ((c = getc (infile)) != EOF && strchr (scanchars, c) == NULL)
    {
	if (c == (int) '\0')
	{
	    continue;
	}
	if (c == (int) '\n')
	{
	    linenumber++;
	}
	buffer[bufindex++] = (char) c;
	if(bufindex >= bufsize)
	{
	    if((buffer = realloc(buffer, bufsize += BUFFER_INCREMENT)) == NULL)
	    {
		Fatal("Cannot allocate memory for input buffer\n");
	    }
	}
    }
    buffer[bufindex] = '\0';
    if (c != EOF)
    {
	ungetc(c, infile); /*LEG210698*** lclint, GNU libc doesn't say
			     what's the return value */
    }
    return buffer;
}

#ifdef HAS_NO_STRDUP
char *strdup(const char *str)
{
    char *s;

    if((s = malloc(strlen(str) + 1)) == NULL)
    {
	Fatal("Cannot allocate memory for string\n");
    }
    strcpy (s, str);
    return s;
}
#endif

/*@exits@*/
void ParseError (const char *fmt, ...)
{
    va_list ap;

    fprintf(stderr,"%s: %s %4ld: ", progname, currfile, linenumber);
    va_start(ap, fmt);
    (void)vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr,"\n");
    exit (EXIT_FAILURE);
}

void Fatal(const char *fmt, ...)
{
    va_list ap;

    fprintf(stderr,"%s: Fatal error: ", progname);
    va_start(ap, fmt);
    (void)vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr,"\n");
    exit(EXIT_FAILURE);
}


/* convert integer to roman number --- only works up correctly up to 39 */

void roman_item(int n, char * s)
{
	int i=0;
	
	while (n>=10)
	{
		n -= 10;
		s[i] = 'x';
		i++;
	}
	
	if (n==9)
	{
		s[i] = 'i';
		i++;
		s[i] = 'x';
		i++;
		s[i] = '\0';
		return;
	}

	if (n>=5)
	{
		n -= 5;
		s[i] = 'v';
		i++;
	}

	if (n==4)
	{
		s[i] = 'i';
		i++;
		s[i] = 'v';
		i++;
		s[i] = '\0';
		return;
	}

	while (n>=1)
	{
		n -= 1;
		s[i] = 'i';
		i++;
	}
	
	s[i] = '\0';
}