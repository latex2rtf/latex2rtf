/* $Id: util.h,v 1.14 2001/10/28 04:02:44 prahl Exp $ */

#ifdef HAS_NO_STRDUP
char           *strdup(const char *str);
#endif

char *	strdup_noblanks(char *s);
char *	strdup_nobadchars(char *s);
char *	ExtractLabelTag(char *text);
