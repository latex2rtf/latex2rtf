/* $Id: util.h,v 1.19 2002/09/25 01:34:07 prahl Exp $ */

char *  my_strndup(char *s, int n);
char *  strdup_together(char *s, char *t);
char *	strdup_noblanks(char *s);
char *	strdup_nobadchars(char *s);
char *	strdup_noendblanks(char *s);
char *	ExtractLabelTag(char *text);

