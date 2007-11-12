/*
 * $Id: util.h,v 1.2 2001/08/12 17:50:50 prahl Exp $
 * History:
 * $Log: util.h,v $
 * Revision 1.2  2001/08/12 17:50:50  prahl
 * latex2rtf version 1.9b by Scott Prahl
 * 1.9b
 * 	Improved enumerate environment so that it may be nested and
 * 	    fixed labels in nested enumerate environments
 * 	Improved handling of description and itemize environments
 * 	Improved eqnarray environment
 * 	Improved array environment
 * 	Improved \verb handling
 * 	Improved handling of \mbox and \hbox in math mode
 * 	Improved handling of \begin{array} environment
 * 	Improved handling of some math characters on the mac
 * 	Fixed handling of \( \) and \begin{math} \end{math} environments
 * 	Fixed bugs in equation numbering
 * 	Made extensive changes to character translation so that the RTF
 * 	     documents work under Word 5.1 and Word 98 on the Mac
 *
 *
 * 1.9a
 * 	Fixed bug with 'p{width}' in tabular environment
 * 		not fully implemented, but no longer creates bad RTF code
 *
 * 1.9
 * 	Fixed numbering of equations
 * 	Improved/added support for all types of equations
 * 	Now includes PICT files in RTF
 * 	Fixed \include to work (at least a single level of includes)
 *
 * 1.8
 * 	Fixed problems with \\[1mm]
 * 	Fixed handling of tabular environments
 * 	Fixed $x^\alpha$ and $x_\alpha$
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
