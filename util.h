/*
 * $Id: util.h,v 1.1 2001/08/12 17:29:00 prahl Exp $
 * History:
 * $Log: util.h,v $
 * Revision 1.1  2001/08/12 17:29:00  prahl
 * latex2rtf version 1.8aa by Georg Lehner
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
