/*
 * $Id: ignore.h,v 1.9 2001/08/12 19:00:04 prahl Exp $
 * History:
 * $Log: ignore.h,v $
 * Revision 1.9  2001/08/12 19:00:04  prahl
 * 1.9e
 *         Revised all the accented character code using ideas borrowed from ltx2rtf.
 *         Comparing ltx2rtf and latex2rtf indicates that Taupin and Lehner tended to work on
 *         different areas of the latex->rtf conversion process.  Adding
 *         accented characters is the first step in the merging process.
 *
 *         Added MacRoman font handling (primarily to get the breve accent)
 *         Now supports a wide variety of accented characters.
 *         (compound characters only work under more recent versions of word)
 *         Reworked the code to change font sizes.
 *         Added latex logo code from ltx2rtf
 *         Extracted character code into separate file chars.c
 *         Fixed bug with \sf reverting to roman
 *         Added two new testing files fontsize.tex and accentchars.tex
 *
 * Revision 1.2  1997/02/15 20:53:46  ralf
 * Removed some global variable redeclarations
 * Other lclint corrections
 *
 * Revision 1.1  1994/06/17 11:26:29  ralf
 * Initial revision
 *
 */
bool TryVariableIgnore(char *command, FILE *fRtf);
