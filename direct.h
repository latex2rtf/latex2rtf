/*
 * $Id: direct.h,v 1.2 2001/08/12 15:47:04 prahl Exp $
 * History:
 * $Log: direct.h,v $
 * Revision 1.2  2001/08/12 15:47:04  prahl
 * latex2rtf version 1.1 by Ralf Schlatterbeck
 *
 * Revision 1.1  1994/06/17  11:26:29  ralf
 * Initial revision
 *
 */

BOOL TryDirectConvert(char *command, FILE *fRtf);
BOOL WriteFontName(char **buffpoint, FILE *fRtf);
