/* $Id: util.h,v 1.13 2001/10/27 06:13:58 prahl Exp $ */

#ifdef HAS_NO_STRDUP
char           *strdup(const char *str);
#endif

char *	strdup_noblanks(char *s);
char *	strdup_nobadchars(char *s);
