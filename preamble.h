#define FORMAT_ARTICLE 1
#define FORMAT_REPORT 2
#define FORMAT_BOOK 3
#define FORMAT_SLIDES 4
#define FORMAT_LETTER 5

#define TITLE_TITLE 1
#define TITLE_AUTHOR 2
#define TITLE_DATE 3
#define TITLE_TITLEPAGE 4

/* 
   No Lang    = \lang1024
   US ENGLISH = \lang1033
   UK ENGLISH = \lang2057
*/

#define HEADER11 "\\s1\\sb240\\sa60\\keepn{\\*\\pn\\pnlvl1\\pndec\\pnprev1\\pnstart1\\pnsp144{\\pntxta .}}\\b"
#define HEADER12 "\\fs32\\lang1024\\kerning28"

#define HEADER21 "\\s2\\sb240\\sa60\\keepn{\\*\\pn\\pnlvl2\\pndec\\pnprev1\\pnstart1\\pnsp144}\\b"
#define HEADER22 "\\fs24\\lang1024"

#define HEADER31 "\\s3\\sb240\\sa60\\keepn{\\*\\pn\\pnlvl3\\pndec\\pnprev1\\pnstart1\\pnsp144{\\pntxtb .}}\\b"
#define HEADER32 "\\fs24\\lang1024"

#define HEADER41 "\\s4\\sb240\\sa60\\keepn{\\*\\pn\\pnlvl4\\pndec\\pnprev1\\pnstart1\\pnsp144{\\pntxtb .}}\\b"
#define HEADER42 "\\fs24\\lang1024"

#define HEADER03 "{\\*\\cs10 \\additive Default Paragraph Font;}\n}"
#define HEADER13 "{\\*\\pnseclvl1\\pnucrm\\pnstart1\\pnindent720\\pnhang{\\pntxta .}}"
#define HEADER23 "{\\*\\pnseclvl2\\pnucltr\\pnstart1\\pnindent720\\pnhang{\\pntxta .}}"
#define HEADER33 "{\\*\\pnseclvl3\\pndec\\pnstart1\\pnindent720\\pnhang{\\pntxta .}}"
#define HEADER43 "{\\*\\pnseclvl4\\pnlcltr\\pnstart1\\pnindent720\\pnhang{\\pntxta .}}"

#define CFOOT 1
#define LFOOT 2
#define RFOOT 3
#define LHEAD 4
#define CHEAD 5
#define RHEAD 6

void CmdDocumentStyle(int code);
void CmdUsepackage(int code);
void CmdTitle(int code);
void CmdMakeTitle(int code);
void CmdPreambleBeginEnd(int code);
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
