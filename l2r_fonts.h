/*
 * $Id: l2r_fonts.h,v 1.8 2001/08/12 19:48:12 prahl Exp $
 * History:
 * $Log: l2r_fonts.h,v $
 * Revision 1.8  2001/08/12 19:48:12  prahl
 * 1.9h
 * 	Turned hyperlatex back on.  Still not tested
 * 	Turned isolatin1 back on.  Still not tested.
 * 	Eliminated use of \\ in code for comments
 * 	Eliminated \* within comments
 * 	Eliminated silly char comparison to EOF
 * 	Revised README to eliminate DOS stuff
 * 	Added support for \pagebreak
 * 	Added support for \quad, \qquad, \, \; and \> (as spaces)
 * 	Improved support for \r accent
 * 	Made minor changes to accentchars.tex
 * 	fixed bugs in \textit{s_$c$} and $\bf R$
 * 	fixed longstanding bugs in stack cleaning
 * 	fixed ' in math mode
 * 	log-like functions now typeset in roman
 * 	Added test cases to eqns.tex
 * 	default compiler options empty until code is more portable
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
#define F_ROMAN_2      9
#define F_SLANTED_2    10
#define F_SANSSERIF_2  11
#define F_TYPEWRITER_2 12

#define CMD_BOLD 1
#define CMD_ITALIC 2
#define CMD_UNDERLINE 3
#define CMD_CAPS 4
#define CMD_BOLD_1 5
#define CMD_ITALIC_1 6
#define CMD_CAPS_1 7
#define CMD_BOLD_2 8
#define CMD_ITALIC_2 9
#define CMD_CAPS_2 10

/* ----------------------------------- */
#define CMD_CENTERED 13
/* ----------------------------------- */

bool            SetFont(char *TexFont, FILE * fRtf);
int             GetFontNumber(char *Fname);
void            RemoveFontlist(void);
int             getTexFontNumber(char *Fname);

void            CmdSetFontStyle(int code);
void            CmdSetFont(int code);
void            CmdSetFontSize(int code);
void            SetDocumentFontSize(int code);
int             CurrentFontSize(void);
void            BasicSetFontSize(int code);
