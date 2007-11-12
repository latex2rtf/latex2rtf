/*
 * $Id: main.h,v 1.2 2001/08/12 15:47:04 prahl Exp $
 * History:
 * $Log: main.h,v $
 * Revision 1.2  2001/08/12 15:47:04  prahl
 * latex2rtf version 1.1 by Ralf Schlatterbeck
 *
 * Revision 1.3  1994/07/13  09:27:31  ralf
 * Corrected fpos/SEEK_SET bug for SunOs 4.3.1 reported by Ulrich Schmid
 * <schmid@dkrz.d400.de>
 *
 * Revision 1.2  1994/06/17  14:19:41  ralf
 * Corrected various bugs, for example interactive read of arguments
 *
 * Revision 1.1  1994/06/17  11:26:29  ralf
 * Initial revision
 *
 */
/*** Main Includefile ***/
/*** global definitons uses in nearly all files ***/

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#endif
#ifdef HAS_NO_FPOS
typedef long fpos_t;
#define fgetpos(file, posptr) (*(posptr) = ftell(file))
#define fsetpos(file, posptr) fseek(file, *(posptr), SEEK_SET)
#endif

void numerror(int num);
void error(char * text);
FILE *open_cfg(const char *);

typedef int BOOL;
#define TRUE 1
#define FALSE 0

#define MAXCOMMANDLEN 100

BOOL Convert();  /* main convert routine */
void IgnoreTo(char cEnd);

/*** error constants ***/
#define ERR_EOF_INPUT 1
#define ERR_WRONG_COMMAND 2
#define ERR_Param 3
#define ERR_WRONG_COMMAND_IN_TABBING 4

/* available values for alignment */
#define LEFT 'l'
#define RIGHT 'r'
#define CENTERED 'c'
#define JUSTIFIED 'j'



#define PATHMAX 255
