/* $Id: util.h,v 1.16 2002/03/11 04:41:40 prahl Exp $ */

#ifdef HAS_NO_STRDUP
char           *strdup(const char *str);
#endif

char *  strdup_together(char *s, char *t);
char *	strdup_noblanks(char *s);
char *	strdup_nobadchars(char *s);
char *	strdup_noendblanks(char *s);
char *	ExtractLabelTag(char *text);
