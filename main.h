/* $Id: main.h,v 1.21 2001/10/07 05:42:18 prahl Exp $ */

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

void            IgnoreTo(char cEnd);
 /* @exits@ */ void numerror(int num);
 /* @exits@ */ void error(char *text);
 /* @dependent@ */ FILE *open_cfg(const char *);
long            getLinenumber(void);
void            diagnostics(int level, char *format,...);
/* level values */

#define ERROR 0
#define WARNING 1

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
extern bool     GermanMode;
extern				/* @only@ */
 /* @null@ */ char *colFmt;
/* @null@ */
extern char    *hyperref;
extern bool     pagenumbering;
extern int      headings;

/* Global flags of Convert routine */
extern int      RecursionLevel;
extern int      indent;
extern int      tabcounter;
extern bool     tabbing_on;
extern bool     g_processing_tabular;
extern int      colCount;
extern int      actCol;
extern int      tabcounter;
extern bool     twocolumn;
extern bool     titlepage;
extern bool     g_processing_equation;
extern long     linenumber;
extern bool     tabbing_on_itself;	/* LEG220698*** lclint - really used? */
extern bool     tabbing_return;	/* LEG220698*** lclint - really used? */
extern bool     g_processing_preamble;
extern bool     g_processing_figure;
extern bool     g_processing_include;	
extern bool     g_processing_eqnarray;
extern int      g_equation_number;
extern bool     g_show_equation_number;
extern int      g_enumerate_depth;
extern bool     g_suppress_equation_number;
extern bool     g_aux_file_missing;
extern int    	g_document_type;
extern char     g_language[20];
extern char     g_encoding[20];
extern int      g_verbosity_level;

void fprintRTF(char *format, ...);
void putRtfChar(char cThis);

#endif				/* __MAIN_H */
