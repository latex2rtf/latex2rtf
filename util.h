/* $Id: util.h,v 1.20 2002/11/07 02:06:56 prahl Exp $ */

char *  my_strndup(char *s, int n);
char *  strdup_together(char *s, char *t);
char *	strdup_noblanks(char *s);
char *	strdup_nobadchars(char *s);
char *	strdup_noendblanks(char *s);
char *	ExtractLabelTag(char *text);
char *	ExtractAndRemoveTag(char *tag, char *text);
