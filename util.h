/* $Id: util.h,v 1.17 2002/04/03 15:44:19 prahl Exp $ */

char *  strdup_together(char *s, char *t);
char *	strdup_noblanks(char *s);
char *	strdup_nobadchars(char *s);
char *	strdup_noendblanks(char *s);
char *	ExtractLabelTag(char *text);
