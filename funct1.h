/*
 * $Id: funct1.h,v 1.13 2001/08/12 21:15:46 prahl Exp $
 * History:
 * $Log: funct1.h,v $
 * Revision 1.13  2001/08/12 21:15:46  prahl
 *         Removed last two // comments
 *         Explicitly cast char to int in isalpha() and isdigit()
 *         Began the process of supporting Babel better
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


#define EMPHASIZE 1
void            Format(int code);

#define FOOTN 1
#define THANKS 2

#define CMD_BEGIN 1
#define CMD_END 2
void            CmdBeginEnd(int code);

#define PAR_CENTER 1
#define PAR_RIGHT 2
#define PAR_LEFT 3
#define PAR_CENTERLINE 4
void            Paragraph(int code);

void            CmdToday(int code);


void            CmdIgnore(int code);
void            CmdLdots(int code);
void            CmdEmphasize(int code);

void            Environment(int code);

#define SECT_NORM 1
#define SECT_SUB 2
#define SECT_SUBSUB 3
#define SECT_CAPTION 4
#define SECT_CHAPTER 5
#define SECT_PART 6
void            CmdSection(int code);

void            CmdFootNote(int code);

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
void            CmdPrintRtf(int code);

void            GermanPrint(int code);
#define GP_CK 1
#define GP_LDBL 2
#define GP_L 3
#define GP_R 4
#define GP_RDBL 5

void            CmdIgnoreLet(int code);

void            IgnoreNewCmd(int code);

#define LABEL 1
#define REF 2
#define PAGEREF 3
/* LEG190498 Start */
#define HYPER 100
#define HYPERREF HYPER+REF
#define HYPERPAGEREF HYPER+PAGEREF
/* LEG190498 End */

void            CmdLabel(int code);

void            CmdUsepackage(int code);

void            CmdIgnoreDef(int code);

void            CmdItem(int code);

void            ConvertString(char *string);

/* LEG030598 Start */
#define RIGHT_SIDE 347
#define BOTH_SIDES  348
#define LEFT_SIDE  349
/* LEG030598 End */

int             ScanAux(char *token, char *reference, int code);
