#define TABULAR           1
#define TABULAR_STAR      2
#define TABULAR_LONG      3
#define TABULAR_LONG_STAR 4
#define TABBING           5

#define TABLE             2
#define TABLE_STAR        3

void CmdTabjump(void);
void CmdTabset(void);
void CmdTabular(int code);
void CmdTabbing(int code);
void CmdTable(int code);
void CmdMultiCol(int code);
void CmdHline(int code);
void CmdHAlign(int code);
