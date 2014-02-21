/* utils.h - handy strings routines

Copyright (C) 1995-2002 The Free Software Foundation

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

This file is available from http://sourceforge.net/projects/latex2rtf/

Authors:
    1995-1997 Ralf Schlatterbeck
    1998-2000 Georg Lehner
    2001-2003 Scott Prahl
*/
#ifndef _UTILS_H_DEFINED
#define _UTILS_H_DEFINED 1

int     odd(long n);
int     even(long n);
double  my_rint(double nr);
int     strstr_count(const char *s, char *t);

char *  my_strcpy(char *dest, const char *src);
char *  my_strndup(const char *s, size_t n);
char *  strdup_together(const char *s, const char *t);
char *  strdup_together3(const char *s, const char *t, const char *u);
char *  strdup_together4(const char *s, const char *t, const char *u, const char *v);
char *  strdup_noblanks(const char *s);
char *  strdup_nocomments(const char *s);
char *  strdup_nobadchars(const char *s);
char *  strdup_noendblanks(const char *s);
char *  strdup_printable(const char *s);
void    strncpy_printable(char* dst, char *src, int n);
char *  ExtractLabelTag(const char *text);
char *  ExtractAndRemoveTag(char *tag, char *text);
char *  keyvalue_pair(char *t, char **key, char **value);
int     getStringDimension(char *s);
char *  getStringBraceParam(char **s);
void    show_string(int level, const char *s, const char *label);
void    str_blank_out(char *s, const char *target);
void    str_delete(char *s, const char *target);

size_t my_strlcpy(char *dst, const char *src, size_t siz);
size_t my_strlcat(char *dst, const char *src, size_t siz);
int    file_exists(char *fname);
int    my_fgetc(FILE *f);
char * my_fgets(char *buffer, int maxBuffer, FILE *f);

static inline int streq(const char *s1, const char *s2)
{
    return (strcmp(s1,s2) == 0);
}

static inline int strHasChar(const char *s1, const char c)
{
    return (NULL != strchr(s1,c));
}

static inline int strstarts(const char *s1, const char *s2)
{
    return (s1 == strstr(s1,s2));
}

static inline void safe_free(char *s)
{
    if (s) free(s); 
}
#endif
