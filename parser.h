/*
 * $Id: parser.h,v 1.6 2001/08/12 19:00:04 prahl Exp $
 * History:
 * $Log: parser.h,v $
 * Revision 1.6  2001/08/12 19:00:04  prahl
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
char *GetParam();
bool GetBracketParam(char *string, int size);
void GetBraceParam(char *string, int size);
void rewind_one(void);

/****************************************************************************/
/* End of file parser.h                                                     */
/****************************************************************************/
