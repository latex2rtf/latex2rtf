/*
 * $Id: version.h,v 1.9 2001/08/12 19:00:04 prahl Exp $
 * History:
 * $Log: version.h,v $
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
 * Revision 1.9b  2001/05/18 13:08:44  prahl
 * Improved \begin{array} environment
 * Fixed a bug in \begin{enumerate}
 * Fixed handling of \mbox and \hbox in math environments
 *
 * Revision 1.9  2000/08/09 13:08:44  prahl
 * Added equation numbering 
 * Added including .pict files in the rtf
 * Fixed all sorts of minor bugs
 * 
 * Nicaragua in November 1998.
 * Revision 1.8  1998/11/12 13:08:44  glehner
 * The Mitch-revision, named after the Hurricane that passed over
 * Nicaragua in November 1998.
 *
 * Revision 1.7  1998/07/03 06:46:15  glehner
 * Added and Changed a lot. See file ChangeLog
 *
 * Revision 1.6  1998/06/08 19:26:59  ralf
 * Corrected some reported bugs, for details see credits.
 * Re-implemented config file parsing which was very buggy.
 *
 * Revision 1.5  1995/05/24 17:12:55  ralf
 * Some last-minute fixes (correct handling of reading from stdin)
 *
 * Revision 1.4  1995/05/24  16:12:16  ralf
 * Changes by Vladimir Menkov for DOS port
 * Added support for spanish ligatures by John E. Davis
 * Fixed some malloc errors
 * Corrected handling of .bbl and .aux files
 *
 * Revision 1.3  1995/03/24  10:22:09  ralf
 * Changed installation default to not automatically remove .cfg files
 * Some additions to .cfg files proposed by Laurent MUGNIER
 *
 * Revision 1.2  1995/03/23  16:21:29  ralf
 * Added ISO Latin-1 support (suggested by Marc Baudoin)
 * .cfg files are now read in and not consulted for every lookup
 * Citations are now supported by reading LaTeX's .aux and .bbl files
 * \title \author etc. are now recognized in several locations
 * \thanks in header is now handled
 * some common commands are now recognized
 * several missing accents are added
 * better handling of tables
 * lines containing only blanks are handled now (reported by Dean Brettle)
 * section numbering (and the support of it in Word) should work now
 * german versions of umlaut accents ("a instead of \"a) should work
 * (Both reported by Rene' Bock)
 * getopt declaration mismatch corrected
 * (Reported by Eric Picheral)
 * No extra '}' in the rtf output which confused some rtf readers
 * (Reported by chen@oracorp.com)
 * Font sizes should now be more like in LaTeX
 * (Reported by chen@oracorp.com)
 *
 * Revision 1.1  1994/07/13  09:29:37  ralf
 * Corrected SunOs missing SEEK_SET, fpos_t, f{s,g}etpos
 *
 * Revision 1.0  1994/06/21  08:15:50  ralf
 * Initial revision
 *
 */
char *Version = "$Revision: 1.9 $ $Date: 2001/08/12 19:00:04 $";

