#define PARAGRAPH_FIRST           1
#define PARAGRAPH_GENERIC         2
#define PARAGRAPH_SECTION_TITLE   3
#define PARAGRAPH_EQUATION        4
#define PARAGRAPH_SLASHSLASH      5
#define PARAGRAPH_LIST            6
#define PARAGRAPH_ENVIRONMENT     7

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

#define LINE_SPACING_SINGLE         240
#define LINE_SPACING_ONE_AND_A_HALF 360
#define LINE_SPACING_DOUBLE         480

void CmdBeginEnd(int code);
void CmdEndParagraph(int code);
void CmdVspace(int code);
void CmdLineSpacing(int code);
void CmdSpacingEnviron(int code);
void startParagraph(const char *style, int indenting);
void CmdIndent(int code);
void CmdNewPage(int code);
void CmdAlign(int code);
void CmdHfill(int code);

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
