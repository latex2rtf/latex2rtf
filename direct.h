/*
 * $Id: direct.h,v 1.13 2001/08/12 21:15:46 prahl Exp $
 * History:
 * $Log: direct.h,v $
 * Revision 1.13  2001/08/12 21:15:46  prahl
 *         Removed last two // comments
 *         Explicitly cast char to int in isalpha() and isdigit()
 *         Began the process of supporting Babel better
 *
 * Revision 1.3  1998/07/03 07:03:16  glehner
 * lclint cleaning
 *
 * Revision 1.2  1997/02/15 20:45:41  ralf
 * Some lclint changes and corrected variable declarations
 *
 * Revision 1.1  1994/06/17 11:26:29  ralf
 * Initial revision
 *
 */

bool            TryDirectConvert(char *command, FILE * fRtf);
