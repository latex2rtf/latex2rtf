/*
 * $Id: direct.c,v 1.3 2001/08/12 15:56:56 prahl Exp $
 * History:
 * $Log: direct.c,v $
 * Revision 1.3  2001/08/12 15:56:56  prahl
 * latex2rtf version 1.5 by Ralf Schlatterbeck
 *
 * Revision 1.4  1995/03/23  15:58:08  ralf
 * Reworked version by Friedrich Polzer and Gerhard Trisko
 *
 *
 * Revision 1.3  1994/06/21  08:14:11  ralf
 * Corrected Bug in keyword search
 *
 * Revision 1.2  1994/06/17  14:19:41  ralf
 * Corrected various bugs, for example interactive read of arguments
 *
 * Revision 1.1  1994/06/17  11:26:29  ralf
 * Initial revision
 *
 */
/***************************************************************************
     name : direct.c
   author : DORNER Fernando, GRANZER Andreas
            POLZER Friedrich,TRISKO Gerhard
  * changed TryDirectConvert: use search on sorted array
  purpose : This file is used for converting LaTeX commands by simply text exchange
 ******************************************************************************/

/**********************************  includes ***********************************/
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "direct.h"
#include "fonts.h"
#include "cfg.h"
/******************************************************************************/

/*******************************  extern variables ****************************/
extern char *progname;
/******************************************************************************/

/******************************* defines *************************************/
#define MAXFONTLEN 100
/******************************************************************************/


/******************************************************************************/
BOOL WriteFontName(char **buffpoint, FILE *fRtf)
/******************************************************************************
  purpose: reads from the font-array to write correct font-number into
           Rtf-File
parameter: buffpoint: font and number
	   fRtf: File-Pointer to Rtf-File
globals:   progname
 ******************************************************************************/
{
  char buffer[MAXFONTLEN+1];
  int i;
  int fnumber;

  if (**buffpoint == '*')
  {
    fprintf(fRtf,"*");
    return TRUE;
  }
  i = 0;
  while(**buffpoint != '*')
  {
    if ((i >= MAXFONTLEN) || (**buffpoint == '\0'))
    {
      fprintf(stderr, "\n%s: ERROR: Invalid fontname in direct command",progname);
      exit(-1);
    }
    buffer[i] = **buffpoint;
    i++;
    (*buffpoint)++;
  }
  buffer[i] = '\0';
  if ((fnumber = GetFontNumber(buffer)) < 0)
  {
    fprintf(stderr, "\n%s: ERROR: Unknown fontname in direct command",progname);
    fprintf(stderr, "\nprogram aborted\n");
    exit(-1);
  }
  else
  {
    fprintf(fRtf,"%d",fnumber);
    return TRUE;
  }
}



/******************************************************************************/
BOOL TryDirectConvert(char *command, FILE *fRtf)
/******************************************************************************
  purpose: reads from the direct-array how some easy LaTex-commands can be
	   converted into Rtf-commands by text exchange
parameter: command: LaTex-command and Rtf-command
	   fRtf: File-Pointer to Rtf-File
globals:   progname
 ******************************************************************************/
{
  int i;
  char *buffpoint;
  char *RtfCommand;
  char TexCommand[128];

  if (strlen(command) >= 100)
  {
      fprintf(stderr,"\n%s: WARNING: Command %s is too long in LaTeX-File.\n",progname,command);
      return FALSE;    /* command too long */
  }

  TexCommand[0] = '\\';
  TexCommand[1] = '\0';
  strcat (TexCommand, command);

  RtfCommand = search (TexCommand, DIRECT_A);
  if (RtfCommand == NULL)
     return FALSE;

  buffpoint = RtfCommand;

  while (buffpoint[0] != '\0')
  {
     if (buffpoint[0] == '*')
     {
        ++buffpoint;
        if (WriteFontName(&buffpoint, fRtf)==FALSE)
        {
           fprintf(stderr, "\n%s: WARNING: error in direct command file - invalid font name , \n",progname);
           return FALSE;
        }
     }
     else
     {
        fprintf(fRtf,"%c",*buffpoint);
     }

     ++buffpoint;
     
  }  /* end while */

  return TRUE;
}


