/*
 * $Id: funct2.h,v 1.2 2001/08/12 15:47:04 prahl Exp $
 * History:
 * $Log: funct2.h,v $
 * Revision 1.2  2001/08/12 15:47:04  prahl
 * latex2rtf version 1.1 by Ralf Schlatterbeck
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
#define EQNARRAY 2
#define EQNARRAY_1 3

#define TABULAR 1
#define TABULAR_1 2
/*--------------------------------function prototypes----------------------*/
void CmdTabset(int code);

void CmdTabjump(int code);

void CmdTabkill(int code);

void Tabbing(int code);

void CmdIgnoreFigure(int code);

void CmdIgnoreParameter(int code);

void GetOptParam(char *string, int size);

void Ignore_Environment(char *searchstring);

void CmdIgnoreEnvironment(int code);

void CmdColumn(int code);

char *GetSubString(char *s, char terminatesymbol);

void CmdNewPage(int code);

void Cmd_OptParam_Without_braces(int code);
 
void GetInputParam(char *, int);

void ConvertTabbing(void);

void Convert_Tabbing_with_kill(void);

void CmdBottom(int code);

void CmdAbstract(int code);

void CmdTitlepage(int code);

void CmdHyphenation(int code);

void CmdFormula2(int code);

void CmdAddress(int code);

void CmdSignature(int code);

void CmdOpening(int code);

void CmdClosing(int code);

void CmdPs(int code);

void CmdArray(int code);

void CmdTabular(int code);

void CmdLetter(int code);

void CmdTable(int code);
