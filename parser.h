/*
 * $Id: parser.h,v 1.5 2001/08/12 18:53:25 prahl Exp $
 * History:
 * $Log: parser.h,v $
 * Revision 1.5  2001/08/12 18:53:25  prahl
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
 * Revision 1.1  1998/10/27 04:46:43  glehner
 * Initial revision
 *
 *
 * LEG 070798 adapted Frank Barnes contribution to r2l coding conventions
 */
/****************************************************************************/
/* file: parser.h                                                           */
/*                                                                          */
/* Description:                                                             */
/*    Contains declarations for a generic recursive parser the              */
/*    LaTex2RTF code.                                                       */
/*                                                                          */
/* Revision history                                                         */
/* ================                                                         */
/* 26th June 1998 - Created initial version - fb                            */
/****************************************************************************/

#define POSSTACKSIZE   256  /* Size of stack to save positions              */

void CmdIgnoreParameter(int);


/****************************************************************************/
/* End of file parser.h                                                     */
/****************************************************************************/
