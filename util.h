/*
 * $Id: util.h,v 1.10 2001/08/12 21:15:46 prahl Exp $
 * History:
 * $Log: util.h,v $
 * Revision 1.10  2001/08/12 21:15:46  prahl
 *         Removed last two // comments
 *         Explicitly cast char to int in isalpha() and isdigit()
 *         Began the process of supporting Babel better
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
