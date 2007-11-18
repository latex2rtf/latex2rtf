#define MODE_INTERNAL_VERTICAL     1
#define MODE_HORIZONTAL            2
#define MODE_RESTRICTED_HORIZONTAL 3
#define MODE_MATH                  4
#define MODE_DISPLAYMATH           5
#define MODE_VERTICAL              6

extern char TexModeName[7][25];

void SetTexMode(int mode, int just_set_it);
int  GetTexMode(void);

#define VSPACE_VSPACE     -1
#define VSPACE_VSKIP       0
#define VSPACE_SMALL_SKIP  1
#define VSPACE_MEDIUM_SKIP 2
#define VSPACE_BIG_SKIP    3

void CmdBeginEnd(int code);
void CmdEndParagraph(int code);
void CmdIndent(int code);
void CmdVspace(int code);
void CmdSlashSlash(int code);
void CmdDoubleSpacing(int code);
void CmdStartParagraph(const char *style, int indenting);
void CmdIndent(int code);
void SetVspaceDirectly(int space);
void CmdNewPage(int code);