/*
 * $Id: version.h,v 1.3 2001/08/12 15:56:56 prahl Exp $
 * History:
 * $Log: version.h,v $
 * Revision 1.3  2001/08/12 15:56:56  prahl
 * latex2rtf version 1.5 by Ralf Schlatterbeck
 *
 * Revision 1.5  1995/05/24  17:12:55  ralf
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
char *Version = "$Revision: 1.3 $ $Date: 2001/08/12 15:56:56 $";
