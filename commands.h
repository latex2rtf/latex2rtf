/*
 * $Id: commands.h,v 1.3 2001/08/12 15:56:56 prahl Exp $
 * History:
 * $Log: commands.h,v $
 * Revision 1.3  2001/08/12 15:56:56  prahl
 * latex2rtf version 1.5 by Ralf Schlatterbeck
 *
 * Revision 1.1  1994/06/17  11:26:29  ralf
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
void PushEnvironment(int code);
void PopEnvironment();
void ClearEnvironment();
BOOL CallCommandFunc(char *cCommand);
BOOL CallParamFunc(char *cCommand, int AddParam);
