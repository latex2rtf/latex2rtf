/*
 * $Id: main.h,v 1.12 2001/08/12 19:48:12 prahl Exp $
 * History:
 * $Log: main.h,v $
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
#define HAS_NO_STRDUP
#define DEFAULT_MAC_ENCODING
#define ENVSEP '^'
#define PATHSEP ':'
char           *strdup(const char *str);
#include "MainMain.h"
#endif

#ifndef ENVSEP
#define ENVSEP ':'
#endif

#ifndef PATHSEP
#define PATHSEP '/'
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
typedef long    fpos_t;
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

typedef int     bool;

void            Convert();	/* main convert routine */
void            IgnoreTo(char cEnd);
 /* @exits@ */ void numerror(int num);
 /* @exits@ */ void error(char *text);
 /* @dependent@ */ FILE *open_cfg(const char *);
long            getLinenumber(void);
 /* @only@ */ char *EmptyString(void);
void            diagnostics(int level, char *format,...);
/* level values */

#define ERROR 0
#define WARNING 1
#define MAX_VERBOSITY 5

bool            rtf_restrict(int major, int minor);

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

enum TexCharSetKind {
	SEVEN_BIT, ISO_8859_1
};

#define PATHMAX 255

/********************************* global variables *************************/
extern long     linenumber;	/* lines in the LaTex-document */
extern				/* @null@ */
 /* @observer@ */ char *currfile;	/* current file name */
extern /* @dependent@ */ FILE *fTex;	/* file pointer to Latex file */
extern /* @dependent@ */ FILE *fRtf;	/* file pointer to RTF file */
extern				/* @null@ */
 /* @observer@ */ char *input;
extern				/* @null@ */
 /* @only@ */ char *AuxName;	/* LEG220698*** lclint error? */
extern				/* @null@ */
 /* @only@ */ char *BblName;
extern /* @observer@ */ char *progname;	/* name of the executable file */
extern /* @only@ */ char *latexname;	/* name of LaTex-File */
extern char     alignment;	/* default for justified: */
extern fpos_t   pos_begin_kill;
extern bool     bCite;		/* to produce citations */
extern bool     GermanMode;
extern int      DefFont;
extern				/* @only@ */
 /* @null@ */ char *colFmt;
/* @null@ */
extern char    *hyperref;
extern bool     pagenumbering;
extern int      headings;
//extern bool     pagestyledefined;
//extern bool     twoside;

/* Global flags of Convert routine */
extern int      RecursionLevel;
extern bool     mbox;
extern bool     bNewPar;
extern int      indent;
extern bool     bInDocument;
extern int      tabcounter;
extern bool     tabbing_on;
extern bool     g_processing_tabular;
extern bool     bBlankLine;
extern int      colCount;
extern int      actCol;
extern int      tabcounter;
extern bool     twocolumn;
extern bool     article;
extern bool     titlepage;
extern bool     g_processing_equation;
extern long     linenumber;
extern bool     tabbing_on_itself;	/* LEG220698*** lclint - really used? */
extern bool     tabbing_return;	/* LEG220698*** lclint - really used? */
extern bool     g_processing_figure;
extern bool     g_processing_include;	
extern bool     g_processing_eqnarray;
extern int      g_equation_number;
extern bool     g_show_equation_number;
extern int      g_enumerate_depth;
extern bool     g_suppress_equation_number;
extern bool     g_aux_file_missing;

/****************************************************************************/


#endif				/* __MAIN_H */
