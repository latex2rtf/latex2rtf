/*
 * $Id: main.h,v 1.10 2001/08/12 19:32:24 prahl Exp $
 * History:
 * $Log: main.h,v $
 * Revision 1.10  2001/08/12 19:32:24  prahl
 * 1.9f
 * 	Reformatted all source files ---
 * 	    previous hodge-podge replaced by standard GNU style
 * 	Compiles cleanly using -Wall under gcc
 *
 * 	added better translation of \frac, \sqrt, and \int
 * 	forced all access to the LaTeX file to use getTexChar() or ungetTexChar()
 * 	    allows better handling of %
 * 	    simplified and improved error checking
 * 	    eliminates the need for WriteTemp
 * 	    potentially allows elimination of getLineNumber()
 *
 * 	added new verbosity level -v5 for more detail
 * 	fixed bug with in handling documentclass options
 * 	consolidated package and documentclass options
 * 	fixed several memory leaks
 * 	enabled the use of the babel package *needs testing*
 * 	fixed bug in font used in header and footers
 * 	minuscule better support for french
 * 	Added testing file for % comment support
 * 	Enhanced frac.tex to include \sqrt and \int tests also
 * 	Fixed bugs associated with containing font changes in
 * 	    equations, tabbing, and quote environments
 * 	Added essential.tex to the testing suite --- pretty comprehensive test.
 * 	Perhaps fix missing .bbl crashing bug
 * 	Fixed ?` and !`
 *
 * Revision 1.7  1998/11/04 13:40:57  glehner
 * Added HAS_NO_GETOPT preprocessor flag
 *
 * Revision 1.6  1998/07/03 07:01:00  glehner
 * added diagnostics()
 *
 * Revision 1.5  1997/02/15 20:30:47  ralf
 * Added lclint comments and corrected some types.
 *
 * Revision 1.4  1995/03/23 15:58:08  ralf
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
/*** global definitons used in nearly all files ***/

#undef HAS_NO_GETOPT

#ifdef __MWERKS__
#define HAS_NO_GETOPT
#define DEFAULT_MAC_ENCODING
#endif

#include <assert.h>
#include <stdio.h>

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

#ifdef HAS_NO_GETOPT
#define getopt my_getopt
#endif

#ifdef SEMICOLONSEP
#define FORMULASEP ';'
#else
#define FORMULASEP ','
#endif

typedef int bool;

void Convert();  /* main convert routine */
void IgnoreTo(char cEnd);
/*@exits@*/     void numerror(int num);
/*@exits@*/     void error(char * text);
/*@dependent@*/ FILE *open_cfg(const char *);
size_t fTexRead (/*@out@*/void *ptr, size_t size, size_t nitems, FILE *stream);
long getLinenumber(void);
/*@only@*/	char *EmptyString(void);
void diagnostics(int level, char *format, ...);
/* level values */

#define ERROR 0
#define WARNING 1
#define MAX_VERBOSITY 5

bool rtf_restrict(int major, int minor);

#define MAXCOMMANDLEN 100
#define MAXENVIRONS 100


/*** error constants ***/
#define ERR_EOF_INPUT 1
#define ERR_WRONG_COMMAND 2
#define ERR_Param 3
#define ERR_WRONG_COMMAND_IN_TABBING 4
#define ERR_NOT_IN_DOCUMENT 5

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

/********************************* global variables *************************/
extern long linenumber;                      /* lines in the LaTex-document */
extern /*@null@*//*@observer@*/ char *currfile; /* current file name */
extern /*@dependent@*/ FILE *fTex;           /* file pointer to Latex file */
extern /*@dependent@*/ FILE *fRtf;           /* file pointer to RTF file */
extern /*@null@*//*@observer@*/ char *input;
extern /*@null@*//*@only@*/     char *AuxName; /*LEG220698*** lclint error?*/
extern /*@null@*//*@only@*/     char *BblName;
extern /*@observer@*/ char *progname;        /* name of the executable file */
extern /*@only@*/     char *latexname;       /* name of LaTex-File */
extern char alignment;                       /* default for justified: */
extern fpos_t pos_begin_kill;
extern bool bCite;                           /* to produce citations */
extern bool GermanMode;
extern size_t DefFont;
extern /*@only@*//*@null@*/ char* colFmt;
/*@null@*/ 
extern char *hyperref;
extern bool pagenumbering;
extern int headings;
extern bool pagestyledefined;
extern bool  twoside;

/* Global flags of Convert routine */
extern int RecursLevel;
extern int BracketLevel;
extern bool mbox;
extern bool bNewPar;
extern int indent;
extern bool bInDocument;
extern int tabcounter;
extern bool tabbing_on;
extern bool g_processing_tabular;
extern bool bBlankLine;
extern int colCount;
extern int actCol;
extern int tabcounter;
extern bool twocolumn;
extern bool article;
extern bool titlepage;
extern bool g_processing_equation;
extern long linenumber;
extern bool tabbing_on_itself; /*LEG220698*** lclint - really used? */
extern bool tabbing_return; /*LEG220698*** lclint - really used? */
extern bool g_processing_figure;   /*SAP Change to produce count figures and tables separately*/
extern bool g_processing_include;  /*SAP Change to process include files separately*/
extern bool g_processing_eqnarray;
extern int g_equation_number;
extern bool g_show_equation_number;
extern int g_enumerate_depth;
extern bool g_suppress_equation_number;
extern bool g_aux_file_missing;

/****************************************************************************/


#endif     /* __MAIN_H */


