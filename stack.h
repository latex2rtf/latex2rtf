/*
 * $Id: stack.h,v 1.4 2001/08/12 17:29:00 prahl Exp $
 * History:
 * $Log: stack.h,v $
 * Revision 1.4  2001/08/12 17:29:00  prahl
 * latex2rtf version 1.8aa by Georg Lehner
 *
 * Revision 1.2  1997/02/15 20:29:18  ralf
 * Added lclint comments.
 *
 * Revision 1.1  1994/06/17 11:26:29  ralf
 * Initial revision
 *
 */
int Push(int lev, int brack);
int Pop(/*@out@*/int *lev, /*@out@*/int *brack);

