/*
 * $Id: util.h,v 1.8 2001/08/12 19:40:25 prahl Exp $
 * History:
 * $Log: util.h,v $
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
#include <stdio.h>
#define BUFFER_INCREMENT 1024

/* @dependent@ *//* @null@ */
char           *ReadUptoMatch(FILE * infile, /* @observer@ */ const char *scanchars);
char           *StrSave( /* @observer@ */ const char *str);
/* @exits@ */
extern void     Fatal( /* @observer@ */ const char *fmt,...);
/* @exits@ */
void            ParseError( /* @observer@ */ const char *fmt,...);

void            roman_item(int n, char *s);
