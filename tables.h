/* $Id: tables.h,v 1.1 2001/09/06 04:43:04 prahl Exp $ */

#define TABULAR   1
#define TABULAR_1 2
#define TABULAR_2 3

void            CmdTabset(void);
void            CmdTabjump(void);
void            CmdTabkill(int code);
void            CmdTabbing(int code);
void            ConvertTabbing(void);
void            CmdTabular(int code);
void            CmdTable(int code);
void            CmdMultiCol(int code);
