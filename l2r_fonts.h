/*
 * $Id: l2r_fonts.h,v 1.7 2001/08/12 19:40:25 prahl Exp $
 * History:
 * $Log: l2r_fonts.h,v $
 * Revision 1.7  2001/08/12 19:40:25  prahl
 * 1.9g
 *         Added commands to read and set TeX lengths
 *         Added commands to read and set TeX counters
 *         Fixed bug in handling of \item[text]
 *         Eliminated comparison of fpos_t variables
 *         Revised getLinenumber ... this is not perfect
 *         Fixed bug in getTexChar() routine
 *         Clearly separate preamble from the document in hopes that
 *           one day more appropriate values for page size, margins,
 *           paragraph spacing etc, will be used in the RTF header
 *         I have added initial support for page sizes still needs testing
 *         added two more test files misc3.tex and misc4.tex
 *         misc4.tex produces a bad rtf file currently
 *         separated all letter commands into letterformat.c
 *         cleaned up warning calls throughout code
 *         added \neq \leq \geq \mid commands to direct.cfg
 *         collected and added commands to write RTF header in preamble.c
 *         broke isolatin1 and hyperlatex support, these will be fixed next version
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
