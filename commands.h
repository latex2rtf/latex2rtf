/*
 * $Id: commands.h,v 1.10 2001/08/12 19:32:24 prahl Exp $
 * History:
 * $Log: commands.h,v $
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
 * Revision 1.3  1998/07/03 06:57:28  glehner
 * added command number to ignore unwanted environments
 *
 * Revision 1.2  1997/02/15 20:42:58  ralf
 * Corrected some declarations found by lclint.
 *
 * Revision 1.1  1994/06/17 11:26:29  ralf
 * Initial revision
 *
 */
#define HEADER 1
#define DOCUMENT 2
#define ITEMIZE 3
#define ENUMERATE 4
#define DESCRIPTION 5
#define TABBING 6
#define GERMANMODE 7
#define LETTER 8
#define IGN_ENV_CMD 9
#define HYPERLATEX 10

#define ON 0x4000
#define OFF 0x0000

void PushEnvironment(int code);
void PopEnvironment();
void ClearEnvironment();
bool CallCommandFunc(char *cCommand);
bool CallParamFunc(char *cCommand, int AddParam);
int CurrentEnvironmentCount(void);