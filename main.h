/*
 * $Id: main.h,v 1.3 2001/08/12 15:56:56 prahl Exp $
 * History:
 * $Log: main.h,v $
 * Revision 1.3  2001/08/12 15:56:56  prahl
 * latex2rtf version 1.5 by Ralf Schlatterbeck
 *
 * Revision 1.4  1995/03/23  15:58:08  ralf
 * Reworked version by Friedrich Polzer and Gerhard Trisko
 *
 *
 * Revision 1.3  1994/07/13  09:27:31  ralf
 * Corrected fpos/SEEK_SET bug for SunOs 4.3.1 reported by Ulrich Schmid
 * <schmid@dkrz.d400.de>
 *
 * Revision 1.2  1994/06/17  14:19:41  ralf
 * Corrected various bugs, for example interactive read of arguments
 *
 * Revision 1.1  1994/06/17  11:26:29  ralf
 * Initial revision
 *
 */
/*** Main Includefile ***/
/*** global definitons uses in nearly all files ***/


#ifndef __MAIN_H
#define __MAIN_H

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#endif
#ifdef HAS_NO_FPOS
typedef long fpos_t;
#define fgetpos(file, posptr) (*(posptr) = ftell(file))
#define fsetpos(file, posptr) fseek(file, *(posptr), SEEK_SET)
#endif

typedef int BOOL;

BOOL Convert();  /* main convert routine */
void IgnoreTo(char cEnd);
void numerror(int num);
void error(char * text);
FILE *open_cfg(const char *);
size_t fTexRead (void *ptr, size_t size, size_t nitems, FILE *stream);
int getLinenumber(void);


#define TRUE 1
#define FALSE 0
#define UNDEFINED -1

#define MAXCOMMANDLEN 100


/*** error constants ***/
#define ERR_EOF_INPUT 1
#define ERR_WRONG_COMMAND 2
#define ERR_Param 3
#define ERR_WRONG_COMMAND_IN_TABBING 4

/* available values for alignment */
#define LEFT 'l'
#define RIGHT 'r'
#define CENTERED 'c'
#define JUSTIFIED 'j'

enum TexCharSetKind { SEVEN_BIT, ISO_8859_1 };

#define PATHMAX 255

#define HEADER11 "\\s1\\sb240\\sa60\\keepn{\\*\\pn \\pnlvl1\\pndec\\pnprev1\\pnstart1\\pnsp144 {\\pntxta .}}\\b\\f"
#define HEADER12 "\\fs32\\lang2057\\kerning28"

#define HEADER21 "\\s2\\sb240\\sa60\\keepn{\\*\\pn \\pnlvl2\\pndec\\pnprev1\\pnstart1\\pnsp144 }\\b\\f"
#define HEADER22 "\\fs24\\lang2057"

#define HEADER31 "\\s3\\sb240\\sa60\\keepn{\\*\\pn \\pnlvl3\\pndec\\pnprev1\\pnstart1\\pnsp144 {\\pntxtb .}}\\b\\f"
#define HEADER32 "\\fs24\\lang2057"

#define HEADER41 "\\s4\\sb240\\sa60\\keepn{\\*\\pn \\pnlvl4\\pndec\\pnprev1\\pnstart1\\pnsp144 {\\pntxtb .}}\\b\\f"
#define HEADER42 "\\fs24\\lang2057"

#define HEADER03 "{\\*\\cs10 \\additive Default Paragraph Font;}}"
#define HEADER13 "{\\*\\pnseclvl1\\pnucrm\\pnstart1\\pnindent720\\pnhang{\\pntxta .}}"
#define HEADER23 "{\\*\\pnseclvl2\\pnucltr\\pnstart1\\pnindent720\\pnhang{\\pntxta .}}"
#define HEADER33 "{\\*\\pnseclvl3\\pndec\\pnstart1\\pnindent720\\pnhang{\\pntxta .}}"
#define HEADER43 "{\\*\\pnseclvl4\\pnlcltr\\pnstart1\\pnindent720\\pnhang{\\pntxta .}}"

#endif     /* __MAIN_H */


