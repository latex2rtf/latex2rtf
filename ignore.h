/*
 * $Id: ignore.h,v 1.6 2001/08/12 18:25:13 prahl Exp $
 * History:
 * $Log: ignore.h,v $
 * Revision 1.6  2001/08/12 18:25:13  prahl
 * latex2rtf version 1.9c
 *
 * 	Added support for \frac
 * 	Complete support for all characters in the symbol font now
 * 	Better support for unusual ansi characters (e.g., \dag and \ddag)
 * 	Gave direct.cfg a spring cleaning
 * 	Added support for \~n and \~N
 * 	New file oddchars.tex for testing many of these translations.
 * 	New file frac.tex to test \frac and \over
 * 	Removed a lot of annoying warning messages that weren't very helpful
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
