/*
 * $Id: commands.h,v 1.4 2001/08/12 17:29:00 prahl Exp $
 * History:
 * $Log: commands.h,v $
 * Revision 1.4  2001/08/12 17:29:00  prahl
 * latex2rtf version 1.8aa by Georg Lehner
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
void PushEnvironment(int code);
void PopEnvironment();
void ClearEnvironment();
bool CallCommandFunc(char *cCommand);
bool CallParamFunc(char *cCommand, int AddParam);
