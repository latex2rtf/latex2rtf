/*
 * $Id: cfg.h,v 1.3 2001/08/12 17:50:50 prahl Exp $
 * History:
 * $Log: cfg.h,v $
 * Revision 1.3  2001/08/12 17:50:50  prahl
 * latex2rtf version 1.9b by Scott Prahl
 * 1.9b
 * 	Improved enumerate environment so that it may be nested and
 * 	    fixed labels in nested enumerate environments
 * 	Improved handling of description and itemize environments
 * 	Improved eqnarray environment
 * 	Improved array environment
 * 	Improved \verb handling
 * 	Improved handling of \mbox and \hbox in math mode
 * 	Improved handling of \begin{array} environment
 * 	Improved handling of some math characters on the mac
 * 	Fixed handling of \( \) and \begin{math} \end{math} environments
 * 	Fixed bugs in equation numbering
 * 	Made extensive changes to character translation so that the RTF
 * 	     documents work under Word 5.1 and Word 98 on the Mac
 *
 *
 * 1.9a
 * 	Fixed bug with 'p{width}' in tabular environment
 * 		not fully implemented, but no longer creates bad RTF code
 *
 * 1.9
 * 	Fixed numbering of equations
 * 	Improved/added support for all types of equations
 * 	Now includes PICT files in RTF
 * 	Fixed \include to work (at least a single level of includes)
 *
 * 1.8
 * 	Fixed problems with \\[1mm]
 * 	Fixed handling of tabular environments
 * 	Fixed $x^\alpha$ and $x_\alpha$
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

/*@null@*//*@dependent@*/
extern size_t SearchRtfIndex ( /*@in@*/ const char *theCommand
                              ,                int WhichArray
                              );
/*@null@*/ 
extern const char *SearchRtfCmd ( /*@in@*/ const char *theCommand
                                ,                int WhichArray
	                        );
/*@null@*/
extern const ConfigEntryT **CfgStartIterate (/*@unused@*/ int WhichCfg);

/*@null@*/
extern const ConfigEntryT **CfgNext (                  int            WhichCfg
                                    , /*@null@*/ const ConfigEntryT **last
				    );


/* Values for WhichArray */
#define DIRECT_A	0
#define FONT_A		1
#define IGNORE_A	2
#define LANGUAGE_A      3

extern void ReadLg(char *lang);
/*@null@*/ 
extern const char *TranslateName(char *name);

#define LIBDIR ""
#define TRUE 1
#define FALSE 0
#endif /* ndefined __CFG_H */
