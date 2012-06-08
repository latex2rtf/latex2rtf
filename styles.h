#ifndef _STYLES_INCLUDED
#define _STYLES_INCLUDED 1

enum { 
    INSERT_STYLE_NORMAL = 1,
    INSERT_STYLE_FOR_HEADER,
    INSERT_STYLE_NO_STYLE
};
 
void InsertBasicStyle(const char *rtf, int how);
void InsertStyle(const char *the_style);

void SetCurrentStyle(const char *style);
char *GetCurrentStyle(void);
int IsSameAsCurrentStyle(const char *s);
void InsertCurrentStyle(void);

#endif
