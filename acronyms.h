#ifndef _ACRONYM_INCLUDED
#define _ACRONYM_INCLUDED 1

enum { 
    ACRONYM_NEWACRO = 1,
    ACRONYM_NEWACROPLURAL,
    ACRONYM_ACRO,
    ACRONYM_ACROPLURAL,
    ACRONYM_ACRODEF,
    ACRONYM_ACRODEFPLURAL
};
    
enum {
    ACRONYM_AC = 1,
    ACRONYM_ACL,
    ACRONYM_ACS,
    ACRONYM_ACF,
    ACRONYM_ACFI,

    ACRONYM_IGNORE = 0x0ff,
    ACRONYM_PLURAL = 0x100,
    ACRONYM_STAR   = 0x200,
    ACRONYM_USED   = 0x400
};

extern int acroPrintWithPage;
extern int acroPrintOnlyUsed;
extern char *acronymAux[];

void UsePackageAcronym(char *options);
void CmdBeginAcronym(int code);
void CmdAcrodef(int code);
void CmdAcroExtra(int code);
void CmdAc(int code);
void CmdAcResetAll(int code);
void CmdAcUsed(int code);
void CmdAC(int code);

int acronymHint(int maxWidth);

extern CommandArray acronymCommands[];

#endif
