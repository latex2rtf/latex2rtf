/*
 * $Id: encode.h,v 1.1 2001/08/12 21:15:46 prahl Exp $
 * History:
 * $Log: encode.h,v $
 * Revision 1.1  2001/08/12 21:15:46  prahl
 *         Removed last two // comments
 *         Explicitly cast char to int in isalpha() and isdigit()
 *         Began the process of supporting Babel better
 *
 * Revision 1.1  1995/03/23 16:09:01  ralf
 * Initial revision
 *
 */

void            Write_ISO_8859_1(char theChar);
void            Write_Default_Charset(char theChar);
