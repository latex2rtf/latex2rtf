/* $Id: util.h,v 1.21 2003/01/06 01:26:06 prahl Exp $ */

char *  my_strndup(char *s, size_t n);
char *  strdup_together(char *s, char *t);
char *	strdup_noblanks(char *s);
char *	strdup_nobadchars(char *s);
char *	strdup_noendblanks(char *s);
char *	ExtractLabelTag(char *text);
char *	ExtractAndRemoveTag(char *tag, char *text);
