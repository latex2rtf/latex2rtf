#include "l2rshell.h"


LPWORD lpwAlign(LPWORD);
int nCopyWCharToTemplate(LPWORD, LPCTSTR);
void DlgTemplate(PWORD&, DWORD, int, int, int,
            int, int, LPSTR);

void DlgTemplate(PWORD& p, DWORD lStyle, int items, int x, int y,
            int cx, int cy, LPCTSTR txt)
{    
    *p++=LOWORD(lStyle);
    *p++=HIWORD(lStyle);
    *p++=0;
    *p++=0;
    *p++=items;
    *p++=x;
    *p++=y;
    *p++=cx;
    *p++=cy;
    *p++=0; //menu
    *p++=0; //class name
    int nchar=nCopyWCharToTemplate(p, txt); //caption
    p+=nchar;
    *p++=8; //font size
    nchar=nCopyWCharToTemplate(p, L"MS Shell Dlg"); //font
    p+=nchar;
    p=lpwAlign((LPWORD)p);
}

void DlgItemTemplate(PWORD& p, DWORD lStyle, int x, int y,
            int cx, int cy, WORD id, LPCTSTR classname, LPCTSTR txt)
{    
    *p++=LOWORD(lStyle);
    *p++=HIWORD(lStyle);
    *p++=0;
    *p++=0;
    *p++=x;
    *p++=y;
    *p++=cx;
    *p++=cy;
    *p++=id;
    int nchar=nCopyWCharToTemplate(p, classname);
    p+=nchar;
    if (lstrlen(txt)>0) nchar=nCopyWCharToTemplate(p, txt);
    else nchar=nCopyWCharToTemplate(p, L"");
    p+=nchar; 
    *p++=0;
    p=lpwAlign((LPWORD)p);
}

LPWORD lpwAlign(LPWORD lpln)
{
    ULONG ul=(ULONG)lpln;
    ul+=3;
    ul>>=2;
    ul<<=2;
    return (LPWORD)ul;
}

int nCopyWCharToTemplate(LPWORD lpWCStr, LPCTSTR lpAnsiln)
{
    lstrcat((WCHAR*)lpWCStr, lpAnsiln);
    return lstrlen(lpAnsiln)+1;
}
