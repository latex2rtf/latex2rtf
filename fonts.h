/*
 * $Id: fonts.h,v 1.4 2001/08/12 17:29:00 prahl Exp $
 * History:
 * $Log: fonts.h,v $
 * Revision 1.4  2001/08/12 17:29:00  prahl
 * latex2rtf version 1.8aa by Georg Lehner
 *
 * Revision 1.2  1997/02/15 20:55:50  ralf
 * Some reformatting and changes suggested by lclint
 * Removed direct access to data structures in cfg.c
 *
 * Revision 1.1  1994/06/17 11:26:29  ralf
 * Initial revision
 *
 */

void WriteFontHeader(/*@dependent@*/ FILE* fRtf);
bool SetFont(char *TexFont, FILE* fRtf);
size_t GetFontNumber(char * Fname);
void RemoveFontlist(void);
size_t GetTexFontNumber(char * Fname);
