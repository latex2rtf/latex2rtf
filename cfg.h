/*
 * $Id: cfg.h,v 1.9 2001/08/12 19:40:25 prahl Exp $
 * History:
 * $Log: cfg.h,v $
 * Revision 1.9  2001/08/12 19:40:25  prahl
 * 1.9g
 *         Added commands to read and set TeX lengths
 *         Added commands to read and set TeX counters
 *         Fixed bug in handling of \item[text]
 *         Eliminated comparison of fpos_t variables
 *         Revised getLinenumber ... this is not perfect
 *         Fixed bug in getTexChar() routine
 *         Clearly separate preamble from the document in hopes that
 *           one day more appropriate values for page size, margins,
 *           paragraph spacing etc, will be used in the RTF header
 *         I have added initial support for page sizes still needs testing
 *         added two more test files misc3.tex and misc4.tex
 *         misc4.tex produces a bad rtf file currently
 *         separated all letter commands into letterformat.c
 *         cleaned up warning calls throughout code
 *         added \neq \leq \geq \mid commands to direct.cfg
 *         collected and added commands to write RTF header in preamble.c
 *         broke isolatin1 and hyperlatex support, these will be fixed next version
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

/* Default to ':' for environment separator and something unlikely for classic MacOS */
#ifndef ENVSEP
  #ifdef __MWERKS__
    #define ENVSEP '^'
  #else
    #define ENVSEP ':'
  #endif
#endif

/* Defaults to '/' , use ':' for classic MacOS */
#ifndef PATHSEP
  #ifdef __MWERKS__
    #define PATHSEP ':'
  #else
    #define PATHSEP '/'
  #endif
#endif

#endif /* ndefined __CFG_H */
