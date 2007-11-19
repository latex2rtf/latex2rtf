#define FIRST_PARAGRAPH      1
#define GENERIC_PARAGRAPH        2
#define SECTION_TITLE_PARAGRAPH      3

#define INDENT_NONE    1
#define INDENT_INHIBIT 2
#define INDENT_USUAL   3

#define MODE_INTERNAL_VERTICAL     1
#define MODE_HORIZONTAL            2
#define MODE_RESTRICTED_HORIZONTAL 3
#define MODE_MATH                  4
#define MODE_DISPLAYMATH           5
#define MODE_VERTICAL              6

extern char TexModeName[7][25];


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
void startParagraph(const char *style, int indenting);
void CmdIndent(int code);
void CmdNewPage(int code);
void CmdAlign(int code);

void setLeftMarginIndent(int indent);
void setRightMarginIndent(int indent);

int  getLeftMarginIndent(void);
int  getRightMarginIndent(void);

void setAlignment(int align);
int  getAlignment(void);

int  getTexMode(void);
void setTexMode(int mode);
void changeTexMode(int mode);

void setVspace(int space);
int  getVspace(void);
