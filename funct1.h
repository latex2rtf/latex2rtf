/* $Id: funct1.h,v 1.25 2001/10/22 04:33:03 prahl Exp $ */

#define THANKS 2

#define CMD_BEGIN 1
#define CMD_END 2
void            CmdBeginEnd(int code);

#define PAR_CENTER     1
#define PAR_RIGHT      2
#define PAR_LEFT       3
#define PAR_CENTERLINE 4
#define PAR_VCENTER    5

#define INDENT_NONE    1
#define INDENT_INHIBIT 2
#define INDENT_USUAL   3

#define VERBATIM_1	 1
#define VERBATIM_2   2
#define VERBATIM_3   3

#define VERB_VERB    1
#define VERB_STAR    2
#define VERB_URL     3

void            CmdStartParagraph(int code);
void            CmdEndParagraph(int code);
void            CmdIndent(int code);
void			CmdVspace(int code);
void			CmdSlashSlash(int code);

#define DEF_NEW    1
#define DEF_RENEW  2
#define DEF_DEF    3
void  			CmdNewDef(int code);
void			CmdNewEnvironment(int code);

void            CmdAlign(int code);
void            CmdToday(int code);
void            CmdIgnore(int code);
void            CmdLdots(int code);
void            Environment(int code);

#define SECT_NORM 1
#define SECT_SUB 2
#define SECT_SUBSUB 3
#define SECT_CAPTION 4
#define SECT_CHAPTER 5
#define SECT_PART 6
void            CmdSection(int code);

#define QUOTE 1
#define QUOTATION 2
void            CmdQuote(int code);

#define RESET_ITEM_COUNTER 0

void            CmdList(int code);

#define COUNTER_NEW   1
#define COUNTER_SET   2
#define COUNTER_ADD   3
#define COUNTER_VALUE 4

void            CmdCounter(int code);

#define LENGTH_NEW   1
#define LENGTH_SET   2
#define LENGTH_ADD   3

void            CmdLength(int code);
void            CmdCaption(int code);
void            CmdBox(int code);
void            CmdInclude(int code);
void            CmdVerb(int code);
void            CmdVerbatim(int code);
void            CmdVerse(int code);
void            TranslateGerman(void);
void            GermanPrint(int code);

#define GP_CK 1
#define GP_LDBL 2
#define GP_L 3
#define GP_R 4
#define GP_RDBL 5

void            CmdIgnoreLet(int code);
void            CmdIgnoreDef(int code);
void            CmdItem(int code);
void			CmdMinipage(int code);

/* LEG030598 Start */
#define RIGHT_SIDE 347
#define BOTH_SIDES  348
#define LEFT_SIDE  349
/* LEG030598 End */

#define FIGURE 1
#define FIGURE_1 5

#define IGNORE_HTMLONLY  1
#define IGNORE_PICTURE   2
#define IGNORE_MINIPAGE  3
#define IGNORE_RAWHTML   4

#define No_Opt_One_NormParam 01
#define No_Opt_Two_NormParam 02
#define No_Opt_Three_NormParam 03
#define One_Opt_No_NormParam 10
#define One_Opt_One_NormParam 11
#define One_Opt_Two_NormParam 12
#define One_Opt_Three_NormParam 13
#define Two_Opt_No_NormParam 20
#define Two_Opt_One_NormParam 21
#define Two_Opt_Two_NormParam 22
#define Two_Opt_Three_NormParam 23

#define One_Column 1
#define Two_Column 2

#define NewPage 1
#define NewColumn 2

void            CmdIgnoreEnviron(int code);

void            CmdFigure(int code);

void            Cmd_OptParam_Without_braces(int code);

void            CmdColumn(int code);

void            CmdNewPage(int code);

void            GetInputParam(char *, int);

void            CmdBottom(int code);

void            CmdAbstract(int code);
void            CmdTitlepage(int code);
void            CmdHyphenation(int code);
void            CmdFigure(int code);
void            CmdMultiCol(int code);
void            CmdAnnotation(int code);
void            CmdLink(int code);
void            CmdGraphics(int code);
void            GetRequiredParam(char *string, int size);
void 			CmdQuad(int kk);
void			CmdColsep(int code);
void		 	CmdSpace(float kk);
void 			CmdVerbosityLevel(int code);

