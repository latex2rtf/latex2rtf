/* $Id: util.h,v 1.18 2002/05/02 14:47:20 prahl Exp $ */

char *  strndup(char *s, int n);
char *  strdup_together(char *s, char *t);
char *	strdup_noblanks(char *s);
char *	strdup_nobadchars(char *s);
char *	strdup_noendblanks(char *s);
char *	ExtractLabelTag(char *text);

