/*
 * $Id: cfg.h,v 1.10 2001/08/12 19:48:12 prahl Exp $
 * History:
 * $Log: cfg.h,v $
 * Revision 1.10  2001/08/12 19:48:12  prahl
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
 * Revision 1.5  1998/11/12 15:15:42  glehner
 * Cleaned up includes, moved from .h file to .c
 * added #ifndef __CFG_H ....
 *
 * Revision 1.4  1998/11/04 13:37:46  glehner
 * removed #include <malloc.h>
 *
 * Revision 1.3  1998/07/03 07:01:28  glehner
 * added ReadLg() for language.cfg files
 * fixed open_cfg search path parsing
 *
 * Revision 1.2  1997/02/15 20:36:29  ralf
 * Almost complete rewrite of config file reading.
 * The interface was made cleaner, there are no external functions
 * that access internal data structures now.
 * The opening of config files was also cleaned up.
 * There was a bug fix for parsing of the environment settings
 * that prevented some directories from being found on second
 * parsing. This was reported by L. Mugnier and there was a proposed fix
 * by V. Menkov.
 *
 * Revision 1.1  1995/03/23 16:09:01  ralf
 * Initial revision
 *
 */

#ifndef __CFG_H
#define __CFG_H

/*** global definitons used in many(!) files ***/

typedef int (*fptr) (const void*, const void*);

typedef struct ConfigEntryT
{
   /*@shared@*/ const char  *TexCommand;
   /*@shared@*/ const char  *RtfCommand;
} ConfigEntryT;

extern void ReadCfg (void)
/*@globals configinfo@*/
/*@modifies configinfo@*/
;

extern size_t SearchRtfIndex (const char *theCommand, int WhichArray);
extern char *SearchRtfCmd (const char *theCommand, int WhichArray);
extern ConfigEntryT **CfgStartIterate (int WhichCfg);
extern ConfigEntryT **CfgNext (int WhichCfg, ConfigEntryT **last);


/* Values for WhichArray */
#define DIRECT_A	0
#define FONT_A		1
#define IGNORE_A	2
#define LANGUAGE_A      3

extern void ReadLg(char *lang);
/*@null@*/ 
extern char *TranslateName(char *name);

#ifndef LIBDIR
#define LIBDIR ""
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#endif /* ndefined __CFG_H */
