/*
 * $Id: commands.h,v 1.8 2001/08/12 18:53:25 prahl Exp $
 * History:
 * $Log: commands.h,v $
 * Revision 1.8  2001/08/12 18:53:25  prahl
 * 1.9d
 *         Rewrote the \cite code.
 *         No crashes when .aux missing.
 *         Inserts '?' for unknown citations
 *         Added cite.tex and cite.bib to for testing \cite commands
 *         hyperref not tested since I don't use it.
 *         A small hyperref test file would be nice
 *         Revised treatment of \oe and \OE per Wilfried Hennings suggestions
 *         Added support for MT Extra in direct.cfg and fonts.cfg so that
 *         more math characters will be translated e.g., \ell (see oddchars.tex)
 *         added and improved font changing commands e.g., \texttt, \it
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
