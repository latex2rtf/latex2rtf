/*  $Id: mygetopt.c,v 1.7 2001/08/12 19:32:24 prahl Exp $
 *  History:
 * $Log: mygetopt.c,v $
 * Revision 1.7  2001/08/12 19:32:24  prahl
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


