/*
 * $Id: direct.c,v 1.1 2001/08/12 15:32:18 prahl Exp $
 * History:
 * $Log: direct.c,v $
 * Revision 1.1  2001/08/12 15:32:18  prahl
 * Initial revision
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
  purpose : This file is used for converting LaTeX commands by simply text exchange
 ******************************************************************************/

/**********************************  includes ***********************************/
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "direct.h"
#include "fonts.h"
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
  purpose: reads from the file fonts.cfg how some easy LaTex-commands can be
	   converted into Rtf-commands by text exchange
	   especially font-number-type-convertions
parameter: buffpoint: font and number
	   fRtf: File-Pointer to Rtf-File
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
  purpose: reads from the file direct.cfg how some easy LaTex-commands can be
	   converted into Rtf-commands by text exchange
parameter: command: LaTex-command and Rtf-command
	   fRtf: File-Pointer to Rtf-File
 ******************************************************************************/
{
  FILE *fp;
  char buffer[512];
  int i;
  char *buffpoint;

  if (strlen(command) >= 100)
      {
      fprintf(stderr,"\n%s: WARNING: Command %s is too long in direct.cfg.\n",progname,command);
      return FALSE;    /* command too long */
      }
  fp = open_cfg("direct.cfg");
  for(;;)
  {
    if ( (fgets( buffer, 512, fp )) == NULL)
    {
      fclose(fp);
      return FALSE;
    }
    if (strlen(buffer) > 500)
    {
      fprintf(stderr, "\n%s: WARNING: line too long in DIRECT.CFG - only 500 characters\n",progname);
      fclose(fp);
      return FALSE;
    }
    buffpoint = buffer;
    while (*buffpoint != '\\')
    {
      if ( (*buffpoint == '#') || (*buffpoint == '\0') )
	break;
      ++buffpoint;
    }
    if (*buffpoint != '\\')
      continue;
    ++buffpoint;
    for ((int)i = 0; (int)i < (int)strlen(command); i++)
    {
      if (buffpoint[i] == '\0')
	break;
      if (buffpoint[i] != command[i])
	break;
    }
    if ((int)i == (int)strlen(command) && buffpoint[i] == ',')   /* found */
    {
      buffpoint += i;
      while (*buffpoint != ',')
      {
	if ( (*buffpoint == '#') || (*buffpoint == '\0') )
	{
	  fprintf(stderr, "\n%s: WARNING: error in direct command file - missing , \n",progname);
	  fclose(fp);
	  return FALSE;
	}
	++buffpoint;
      }
      ++buffpoint;
      while (*buffpoint != '.')
      {
	if ( (*buffpoint == '#') || (*buffpoint == '\0') )
	{
	  fprintf(stderr, "\n%s: WARNING: error in direct command file - missing , \n",progname);
	  fclose(fp);
	  return FALSE;
	}
	if (*buffpoint == '*')
	{
	  ++buffpoint;
	  if (WriteFontName(&buffpoint, fRtf)==FALSE)
	  {
	    fprintf(stderr, "\n%s: WARNING: error in direct command file - invalid font name , \n",progname);
	    fclose(fp);
	    return FALSE;
	  }
	}
	else
	  fprintf(fRtf,"%c",*buffpoint);
	++buffpoint;
      }
      fclose(fp);
      return TRUE;
    } /* end found */
  }  /* end while */
}
