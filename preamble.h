#define FORMAT_ARTICLE 1
#define FORMAT_REPORT  2
#define FORMAT_BOOK    3
#define FORMAT_SLIDES  4
#define FORMAT_LETTER  5
#define FORMAT_APA     6

#define TITLE_TITLE       1
#define TITLE_AUTHOR      2
#define TITLE_DATE        3
#define TITLE_TITLEPAGE   4
#define TITLE_AFFILIATION 5
#define TITLE_ABSTRACT    6
#define TITLE_ACKNOWLEDGE 7

#define GRAPHICS_NONE     0
#define GRAPHICS_GRAPHICS 1
#define GRAPHICS_GRAPHICX 2

#define CFOOT 1
#define LFOOT 2
#define RFOOT 3
#define LHEAD 4
#define CHEAD 5
#define RHEAD 6

#define RIGHT_SIDE 347
#define BOTH_SIDES 348
#define LEFT_SIDE  349

void CmdDocumentStyle(int code);
void CmdUsepackage(int code);
void CmdTitle(int code);
void CmdAnd(int code);
void CmdMakeTitle(int code);
void CmdPreambleBeginEnd(int code);
void CmdTableOfContents(int code);
void PlainPagestyle(void);
void CmdPagestyle(int code);
void CmdHeader(int code);
void RtfHeader(int where, char *what);
void CmdHyphenation(int code);
void WriteRtfHeader(void );
void CmdHeadFoot(int code);
void CmdThePage(int code);
void setPackageInputenc(char * option);
void setPackageBabel(char * option);
void CmdGeometry(int code);
