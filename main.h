/* $Id: main.h,v 1.26 2001/10/25 05:53:46 prahl Exp $ */
#ifndef __MAIN_H
#define __MAIN_H

#undef HAS_NO_GETOPT

#ifdef __MWERKS__
#define HAS_NO_GETOPT
#define HAS_NO_STRDUP
#define ENVSEP '^'
#define PATHSEP ':'
#include "MainMain.h"
#endif

#ifdef HAS_NO_STRDUP
char           *strdup(const char *str);
#endif

#ifndef ENVSEP
#define ENVSEP ':'
#endif

#ifndef PATHSEP
#define PATHSEP '/'
#endif 

#include <assert.h>
#include <stdio.h>

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
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

#define PATHMAX 255

/********************************* global variables *************************/
extern /* @dependent@ */ FILE *fRtf;	/* file pointer to RTF file */
extern			char *AuxName;
extern			char *BblName;
extern 			char *progname;			/* name of the executable file */

extern bool     GermanMode;
extern char    *hyperref;
extern bool     pagenumbering;
extern int      headings;

extern int      g_verbosity_level;
extern int      RecursionLevel;
extern int      indent;
extern char     alignment;

/* table/tabbing variables */
extern char 	*colFmt;
extern long   	pos_begin_kill;
extern int      tabcounter;
extern bool     tabbing_on;
extern bool     g_processing_tabular;
extern int      colCount;
extern int      actCol;
extern int      tabcounter;
extern bool     tabbing_on_itself;	/* LEG220698*** lclint - really used? */
extern bool     tabbing_return;	/* LEG220698*** lclint - really used? */


extern bool     twocolumn;
extern bool     titlepage;
extern bool     g_processing_equation;
extern bool     g_processing_preamble;
extern bool     g_processing_figure;
extern bool     g_processing_eqnarray;
extern int		g_processing_arrays;
extern int 		g_processing_fields;

extern int      g_equation_number;
extern bool     g_show_equation_number;
extern int      g_enumerate_depth;
extern bool     g_suppress_equation_number;
extern bool     g_aux_file_missing;
extern int    	g_document_type;
extern char     g_encoding[20];

void fprintRTF(char *format, ...);
void putRtfChar(char cThis);

#endif				/* __MAIN_H */
