/* $Id: util.h,v 1.23 2003/09/02 04:10:16 prahl Exp $ */

int     odd(int n);
int     even(int n);
int     strstr_count(char *s, char *t);
char *  my_strndup(char *s, size_t n);
char *  strdup_together(char *s, char *t);
char *	strdup_noblanks(char *s);
char *	strdup_nobadchars(char *s);
char *	strdup_noendblanks(char *s);
char *	ExtractLabelTag(char *text);
char *	ExtractAndRemoveTag(char *tag, char *text);
