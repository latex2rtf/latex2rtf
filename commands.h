/*
 * $Id: commands.h,v 1.5 2001/08/12 17:50:50 prahl Exp $
 * History:
 * $Log: commands.h,v $
 * Revision 1.5  2001/08/12 17:50:50  prahl
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
void PushEnvironment(int code);
void PopEnvironment();
void ClearEnvironment();
bool CallCommandFunc(char *cCommand);
bool CallParamFunc(char *cCommand, int AddParam);
