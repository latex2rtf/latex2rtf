/* $Id: tables.h,v 1.2 2001/10/22 04:33:03 prahl Exp $ */

#define TABULAR   1
#define TABULAR_1 2
#define TABULAR_2 3

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
