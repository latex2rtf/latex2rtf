/*
 * $Id: util.h,v 1.9 2001/08/12 19:48:12 prahl Exp $
 * History:
 * $Log: util.h,v $
 * Revision 1.9  2001/08/12 19:48:12  prahl
 * 1.9h
 * 	Turned hyperlatex back on.  Still not tested
 * 	Turned isolatin1 back on.  Still not tested.
 * 	Eliminated use of \\ in code for comments
 * 	Eliminated \* within comments
 * 	Eliminated silly char comparison to EOF
 * 	Revised README to eliminate DOS stuff
 * 	Added support for \pagebreak
 * 	Added support for \quad, \qquad, \, \; and \> (as spaces)
 * 	Improved support for \r accent
 * 	Made minor changes to accentchars.tex
 * 	fixed bugs in \textit{s_$c$} and $\bf R$
 * 	fixed longstanding bugs in stack cleaning
 * 	fixed ' in math mode
 * 	log-like functions now typeset in roman
 * 	Added test cases to eqns.tex
 * 	default compiler options empty until code is more portable
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
