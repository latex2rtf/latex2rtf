/* $Id: main.h,v 1.29 2001/10/28 04:02:44 prahl Exp $ */
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __MWERKS__
#define HAS_NO_GETOPT
#define HAS_NO_STRDUP
#define ENVSEP '^'
#define PATHSEP ':'
#include "MainMain.h"
#endif

#ifdef HAS_NO_GETOPT
#define getopt my_getopt
#endif

#ifndef ENVSEP
#define ENVSEP ':'
#endif

#ifndef PATHSEP
#define PATHSEP '/'
#endif 

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#endif

#ifdef SEMICOLONSEP
#define FORMULASEP ';'
#else
#define FORMULASEP ','
#endif

#define ERROR 0
#define WARNING 1

#define MAXCOMMANDLEN 100

/* available values for alignment */
#define LEFT      'l'
#define RIGHT     'r'
#define CENTERED  'c'
#define JUSTIFIED 'j'

#define PATHMAX 255

/*** error constants ***/
#include <assert.h>
#include <stdio.h>

typedef int     bool;

void            diagnostics(int level, char *format,...);

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

extern bool     twocolumn;
extern bool     titlepage;
extern bool     g_processing_equation;
extern bool     g_processing_preamble;
extern bool     g_processing_figure;
extern bool 	g_processing_table;
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
extern char		*g_figure_label;
extern char		*g_table_label;

void fprintRTF(char *format, ...);
void putRtfChar(char cThis);

#endif				/* __MAIN_H */
