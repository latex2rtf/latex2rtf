/*
 * $Id: ignore.h,v 1.13 2001/08/12 21:15:46 prahl Exp $
 * History:
 * $Log: ignore.h,v $
 * Revision 1.13  2001/08/12 21:15:46  prahl
 *         Removed last two // comments
 *         Explicitly cast char to int in isalpha() and isdigit()
 *         Began the process of supporting Babel better
 *
 * Revision 1.2  1997/02/15 20:53:46  ralf
 * Removed some global variable redeclarations
 * Other lclint corrections
 *
 * Revision 1.1  1994/06/17 11:26:29  ralf
 * Initial revision
 *
 */
bool            TryVariableIgnore(char *command, FILE * fRtf);
