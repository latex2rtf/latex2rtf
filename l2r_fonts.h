/*
 * $Id: l2r_fonts.h,v 1.6 2001/08/12 19:32:24 prahl Exp $
 * History:
 * $Log: l2r_fonts.h,v $
 * Revision 1.6  2001/08/12 19:32:24  prahl
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
 * Revision 1.2  1997/02/15 20:55:50  ralf
 * Some reformatting and changes suggested by lclint
 * Removed direct access to data structures in cfg.c
 *
 * Revision 1.1  1994/06/17 11:26:29  ralf
 * Initial revision
 *
 */

#define F_ROMAN        1
#define F_SLANTED      2
#define F_SANSSERIF    3
#define F_TYPEWRITER   4
#define F_ROMAN_1      5
#define F_SLANTED_1    6
#define F_SANSSERIF_1  7
#define F_TYPEWRITER_1 8

#define CMD_BOLD 1
#define CMD_ITALIC 2
#define CMD_UNDERLINE 3
#define CMD_CAPS 4
#define CMD_BOLD_1 5
#define CMD_ITALIC_1 6
#define CMD_CAPS_1 7

/* ----------------------------------- */
#define CMD_CENTERED 8
/* ----------------------------------- */

void WriteFontHeader(/*@dependent@*/ FILE* fRtf);
bool SetFont(char *TexFont, FILE* fRtf);
size_t GetFontNumber(char * Fname);
void RemoveFontlist(void);
size_t GetTexFontNumber(char * Fname);

void CmdSetFontStyle(int code);
void CmdSetFont(int code);
void CmdSetFontSize(int code);
void SetDocumentFontSize(int code);
int CurrentFontSize(void);
void BasicSetFontSize(int code);

