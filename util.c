/*
 * $Id: util.c,v 1.3 2001/08/12 18:25:13 prahl Exp $
 * History:
 * $Log: util.c,v $
 * Revision 1.3  2001/08/12 18:25:13  prahl
 * latex2rtf version 1.9c
 *
 * 	Added support for \frac
 * 	Complete support for all characters in the symbol font now
 * 	Better support for unusual ansi characters (e.g., \dag and \ddag)
 * 	Gave direct.cfg a spring cleaning
 * 	Added support for \~n and \~N
 * 	New file oddchars.tex for testing many of these translations.
 * 	New file frac.tex to test \frac and \over
 * 	Removed a lot of annoying warning messages that weren't very helpful
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

char *StrSave(/*@observer@*/ const char *str)
{
    char *s;

    if((s = malloc(strlen(str) + 1)) == NULL)
    {
	Fatal("Cannot allocate memory for string\n");
    }
    strcpy (s, str);
    return s;
}

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

/*********************************************************
 * LEG210698
 * rewind the filepointer in the LaTeX-file by one
 * globals: fTex, LaTeX-filepointer
 *********************************************************/
void
rewind_one(void)
{
  if(fseek(fTex,-1L,SEEK_CUR) != 0)
    diagnostics(ERROR, "Seek failed in LaTeX-file");
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


