/*
 * $Id: funct1.h,v 1.4 2001/08/12 17:29:00 prahl Exp $
 * History:
 * $Log: funct1.h,v $
 * Revision 1.4  2001/08/12 17:29:00  prahl
 * latex2rtf version 1.8aa by Georg Lehner
 *
 * Revision 1.5  1998/11/04 13:39:40  glehner
 * Changed ON-Flag to 0x4000 for little int compilers.
 *
 * Revision 1.4  1998/07/03 07:10:10  glehner
 * updated for latex2rtf V1.7
 *
 * Revision 1.3  1997/02/15 20:59:48  ralf
 * Mainly lclint-suggested changes
 *
 * Revision 1.2  1995/03/23 15:58:08  ralf
 * Reworked version by Friedrich Polzer and Gerhard Trisko
 *
 * Revision 1.1  1994/06/17  11:26:29  ralf
 * Initial revision
 *
 */
/*** prototypes fof functions in funct1 ***/


#define AST_FORM 100

#define CMD_BOLD 1
#define CMD_ITALIC 2
#define CMD_UNDERLINE 3
#define CMD_CAPS 4
/* ----------------------------------- */
#define CMD_CENTERED 5
/* ----------------------------------- */
void CmdCharFormat(int);

#define EMPHASIZE 1
void Format(int code);

#define FOOTN 1
#define THANKS 2

#define CMD_BEGIN 1
#define CMD_END 2
void CmdBeginEnd(int code);

#define PAR_CENTER 1
#define PAR_RIGHT 2
#define PAR_LEFT 3
#define PAR_CENTERLINE 4
#define ON 0x4000
#define OFF 0x0000
void Paragraph(int code);

void CmdToday(int code);

void CmdFontSize(int code);

void CmdC(int code);
void CmdUmlaute(void);
void CmdLApostrophChar(int code);
void CmdRApostrophChar(int code);
void CmdSpitzeChar();
void CmdTildeChar(int code);

#define CMD_TEX 1
#define CMD_LATEX 2
#define CMD_SLITEX 3
#define CMD_BIBTEX 4
void CmdLogo(int code);

void CmdIgnore(int code);
void CmdLdots(int code);
void CmdEmphasize(int code);

void Environment(int code);

#define TITLE_TITLE 1
#define TITLE_AUTHOR 2
#define TITLE_DATE 3
#define TITLE_MAKE 4
void CmdTitle(int code);

void CmdDocumentStyle(int code);

#define SECT_NORM 1
#define SECT_SUB 2
#define SECT_SUBSUB 3
#define SECT_CAPTION 4
#define SECT_CHAPTER 5
#define SECT_PART 6
void CmdSection(int code);

void CmdFootNote(int code);

#define FORM_DOLLAR 2	/* ('$')  */
#define FORM_RND_OPEN 3      /* ('/(') */
#define FORM_ECK_OPEN 4      /* ('/[') */
#define FORM_RND_CLOSE 5      /* ('/)') */
#define FORM_ECK_CLOSE 6      /* ('/]') */

void CmdFormula(int code);

#define QUOTE 1
#define QUOTATION 2
void CmdQuote(int code);

#define RESET 0

void CmdList(int code);

void CmdMbox(int code);

#define F_ROMAN 1
#define F_SLANTED 2
#define F_SANSSERIF 3
#define F_TYPEWRITER 4
void CmdSetFont(int code);

void CmdInclude(int code);

void CmdVerb(int code);

void CmdVerbatim(int code);

void CmdVerse(int code);

void TranslateGerman(void);
void CmdPrintRtf(int code);

/*@only@*/ char *GetParam();

void GermanPrint(int code);
#define GP_CK 1
#define GP_LDBL 2
#define GP_L 3
#define GP_R 4
#define GP_RDBL 5

void CmdIgnoreLet(int code);

void IgnoreNewCmd(int code);

#define LABEL 1
#define REF 2
#define PAGEREF 3
/*LEG190498 Start*/
#define HYPER 100
#define HYPERREF HYPER+REF
#define HYPERPAGEREF HYPER+PAGEREF
/*LEG190498 End*/

void CmdLabel(int code);

void CmdUsepackage(int code);

void CmdIgnoreDef(int code);

void CmdItem(int code);

void ConvertString(char *string);

/*LEG030598 Start*/
#define RIGHT_SIDE 347
#define BOTH_SIDES  348
#define LEFT_SIDE  349
/*LEG030598 End*/

