/*
 * $Id: l2r_fonts.h,v 1.4 2001/08/12 18:53:25 prahl Exp $
 * History:
 * $Log: l2r_fonts.h,v $
 * Revision 1.4  2001/08/12 18:53:25  prahl
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
 * Revision 1.2  1997/02/15 20:55:50  ralf
 * Some reformatting and changes suggested by lclint
 * Removed direct access to data structures in cfg.c
 *
 * Revision 1.1  1994/06/17 11:26:29  ralf
 * Initial revision
 *
 */

void WriteFontHeader(/*@dependent@*/ FILE* fRtf);
bool SetFont(char *TexFont, FILE* fRtf);
size_t GetFontNumber(char * Fname);
void RemoveFontlist(void);
size_t GetTexFontNumber(char * Fname);
