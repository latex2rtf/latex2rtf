/* $Id: tables.h,v 1.4 2002/05/04 18:12:14 prahl Exp $ */

#define TABULAR   1
#define TABULAR_STAR 2
#define TABULAR_LONG 3
#define TABULAR_LONG_STAR 4

#define TABLE 2
#define TABLE_1 3

void            CmdTabset(void);
void            CmdTabjump(void);
void            CmdTabkill(int code);
void            CmdTabbing(int code);
void            ConvertTabbing(void);
void            CmdTabular(int code);
void            CmdTable(int code);
void            CmdMultiCol(int code);
void 			CmdHline(int code);
