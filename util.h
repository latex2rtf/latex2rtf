/*
 * $Id: util.h,v 1.6 2001/08/12 19:00:04 prahl Exp $
 * History:
 * $Log: util.h,v $
 * Revision 1.6  2001/08/12 19:00:04  prahl
 * 1.9e
 *         Revised all the accented character code using ideas borrowed from ltx2rtf.
 *         Comparing ltx2rtf and latex2rtf indicates that Taupin and Lehner tended to work on
 *         different areas of the latex->rtf conversion process.  Adding
 *         accented characters is the first step in the merging process.
 *
 *         Added MacRoman font handling (primarily to get the breve accent)
 *         Now supports a wide variety of accented characters.
 *         (compound characters only work under more recent versions of word)
 *         Reworked the code to change font sizes.
 *         Added latex logo code from ltx2rtf
 *         Extracted character code into separate file chars.c
 *         Fixed bug with \sf reverting to roman
 *         Added two new testing files fontsize.tex and accentchars.tex
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

void roman_item(int n, char * s);
