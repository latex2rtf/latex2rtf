/* $Id: util.h,v 1.15 2001/11/23 21:43:48 prahl Exp $ */

#ifdef HAS_NO_STRDUP
char           *strdup(const char *str);
#endif

char *  strdup_together(char *s, char *t);
char *	strdup_noblanks(char *s);
char *	strdup_nobadchars(char *s);
char *	ExtractLabelTag(char *text);
