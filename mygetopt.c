/*  $Id: mygetopt.c,v 1.2 2001/08/12 17:50:50 prahl Exp $
 *  History:
 * $Log: mygetopt.c,v $
 * Revision 1.2  2001/08/12 17:50:50  prahl
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
 * Revision 1.1  1998/11/12 13:05:43  glehner
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

/* my_getopt  is supposed to emulate the C Library getopt
   (which, according to the man pages, is written by Henry Spencer
   to emulate the Bell Lab version).

   my_getopt is scanning argv[optind] (and, perhaps, following
   arguments), looking for the first option starting with `-' and a
   character from optstring[]. Therefore, if you are looking
   for options in argv[1] etc., you should initialize optind
   with 1 (not 0, as the manual erroneously claims).

   Experiments with getopt() established that when an argument
   consists of more than one option, getopt() stores the
   pointer to the beginning of the argument as a static
   variable, for re-use later.

   See the getopt manual pages for more information on getopt.



   Written by V.Menkov, IU, 1995

*/

extern  char * optarg;
extern  int optind;

typedef int logical;

int my_getopt(int argc, char ** argv, char * optstring);

int my_getopt(int argc, char ** argv, char * optstring) {
  char *q;
  static char * rem = NULL;
  int c;
  logical needarg = 0;

  optarg = NULL;

  diagnostics(4, "Processing option `%s'", argv[optind]);

  /*  printf("optind = %d\n", optind);  if (rem) printf("rem=`%s'\n", rem);*/

    if (!rem) {
    if (optind < argc && argv[optind][0] == '-') {
      rem = argv[optind]+1;
      if (*rem == 0) return EOF;     /* Treat lone "-" as a non-option arg */
      if (*rem == '-') { optind++; return EOF;}  /* skip "--" and terminate */
    }
    else return EOF;
    }

  c = *rem;
  q = strchr(optstring, c);
  if (q && c != ':') {              /* matched */
    needarg = (q[1] == ':');
    if (needarg) {
      if (rem[1] != 0) optarg = rem + 1;
      else {
	optind ++;
	if (optind < argc) optarg = argv[optind];
	else {
	  fprintf(stderr, "Missing argument after -%c\n", c);
	  exit(1);
	}
      }
    } else rem ++;
  } else {
    fprintf(stderr, "%s: illegal option -- %c\n", argv[0], c);
    c = '?';
    rem ++;
  }
  if (needarg || *rem == 0) { rem = NULL; optind++;}
  return c;
}


