/*
 * $Id: funct2.h,v 1.12 2001/08/12 19:48:12 prahl Exp $
 * History:
 * $Log: funct2.h,v $
 * Revision 1.12  2001/08/12 19:48:12  prahl
 * 1.9h
 * 	Turned hyperlatex back on.  Still not tested
 * 	Turned isolatin1 back on.  Still not tested.
 * 	Eliminated use of \\ in code for comments
 * 	Eliminated \* within comments
 * 	Eliminated silly char comparison to EOF
 * 	Revised README to eliminate DOS stuff
 * 	Added support for \pagebreak
 * 	Added support for \quad, \qquad, \, \; and \> (as spaces)
 * 	Improved support for \r accent
 * 	Made minor changes to accentchars.tex
 * 	fixed bugs in \textit{s_$c$} and $\bf R$
 * 	fixed longstanding bugs in stack cleaning
 * 	fixed ' in math mode
 * 	log-like functions now typeset in roman
 * 	Added test cases to eqns.tex
 * 	default compiler options empty until code is more portable
 *
 * Revision 1.5  1998/10/28 06:08:06  glehner
 * Changed ON Flag to 0x4000 for not to use MSB of int on some cc
 *
 * Revision 1.4  1998/07/03 07:00:13  glehner
 * added hyperlatex-support, CmdColsep
 *
 * Revision 1.3  1997/02/15 20:59:16  ralf
 * Corrected core-dump bug in tabular environment (gcc only)
 * Some lclint changes
 *
 * Revision 1.2  1995/03/23 15:58:08  ralf
 * Reworked version by Friedrich Polzer and Gerhard Trisko
 *
 * Revision 1.1  1994/06/17  11:26:29  ralf
 * Initial revision
 *
 */

#define FIGURE 1
#define PICTURE 2
#define MINIPAGE 3
#define THEBIBLIOGRAPHY 4
#define FIGURE_1 5

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

#define BIBLIOGRAPHY 1
#define TABLE 2
#define TABLE_1 3

#define ARRAY 1

#define TABULAR   1
#define TABULAR_1 2
#define TABULAR_2 3

void            CmdTabset(void);

void            CmdTabjump(void);

void            CmdTabkill(int code);

void            CmdTabbing(int code);

void            CmdIgnoreFigure(int code);

void            CmdFigure(int code);

void            Cmd_OptParam_Without_braces(int code);

void            Ignore_Environment(char *searchstring);	/* LEG210698*** lclint -
							 * consider passing this
							 * to ignore.c */

void            CmdIgnoreEnvironment(int code);

void            CmdColumn(int code);

void            CmdNewPage(int code);


void            GetInputParam(char *, int);

void            ConvertTabbing(void);

void            CmdBottom(int code);

void            CmdAbstract(int code);

void            CmdTitlepage(int code);

void            CmdHyphenation(int code);

void            CmdAddress(int code);

void            CmdSignature(int code);

void            CmdOpening(int code);

void            CmdClosing(int code);

void            CmdPs(int code);

void            CmdTabular(int code);

void            CmdLetter(int code);

void            CmdFigure(int code);

void            CmdTable(int code);

void            CmdMultiCol(int code);

void            CmdNoCite(int code);

void            CmdCite(int code);

void 
WriteRefList(void)
/* @globals input, bCite @ */
;

void            CmdAnnotation(int code);

void            CmdColsep( /* @unused@ */ int code);
void            CmdLink( /* @unused@ */ int code);

void            CmdConvertBiblio( /* @unused@ */ int code);

void            CmdGraphics(int code);
void            GetRequiredParam(char *string, int size);


void CmdQuad(int kk);
void CmdSpace(float kk);

