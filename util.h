/*
 * $Id: util.h,v 1.3 2001/08/12 18:25:13 prahl Exp $
 * History:
 * $Log: util.h,v $
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
#include <stdio.h>
#define BUFFER_INCREMENT 1024

/*@dependent@*/ /*@null@*/
char *ReadUptoMatch (FILE *infile, /*@observer@*/ const char *scanchars);
char *StrSave       (/*@observer@*/ const char *str);
/*@exits@*/
extern void  Fatal         (/*@observer@*/ const char *fmt, ...);
/*@exits@*/
void  ParseError    (/*@observer@*/ const char *fmt, ...);

void rewind_one(void);
void roman_item(int n, char * s);
