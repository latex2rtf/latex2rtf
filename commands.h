/*
 * $Id: commands.h,v 1.9 2001/08/12 19:00:04 prahl Exp $
 * History:
 * $Log: commands.h,v $
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