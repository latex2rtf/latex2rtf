/*
 * $Id: util.h,v 1.5 2001/08/12 18:53:25 prahl Exp $
 * History:
 * $Log: util.h,v $
 * Revision 1.5  2001/08/12 18:53:25  prahl
 * 1.9d
 *         Rewrote the \cite code.
 *         No crashes when .aux missing.
 *         Inserts '?' for unknown citations
 *         Added cite.tex and cite.bib to for testing \cite commands
 *         hyperref not tested since I don't use it.
 *         A small hyperref test file would be nice
 *         Revised treatment of \oe and \OE per Wilfried Hennings suggestions
 *         Added support for MT Extra in direct.cfg and fonts.cfg so that
 *         more math characters will be translated e.g., \ell (see oddchars.tex)
 *         added and improved font changing commands e.g., \texttt, \it
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
