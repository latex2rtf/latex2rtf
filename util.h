/* $Id: util.h,v 1.11 2001/10/13 19:19:10 prahl Exp $ */

#include <stdio.h>
#define BUFFER_INCREMENT 1024

/* @dependent@ *//* @null@ */
char           *ReadUptoMatch(FILE * infile, /* @observer@ */ const char *scanchars);
char           *StrSave( /* @observer@ */ const char *str);
/* @exits@ */
extern void     Fatal( /* @observer@ */ const char *fmt,...);
/* @exits@ */
void            ParseError( /* @observer@ */ const char *fmt,...);

char *            roman_item(int n);
