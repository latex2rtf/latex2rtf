/*
 * $Id: ignore.c,v 1.3 2001/08/12 15:56:56 prahl Exp $
 * History:
 * $Log: ignore.c,v $
 * Revision 1.3  2001/08/12 15:56:56  prahl
 * latex2rtf version 1.5 by Ralf Schlatterbeck
 *
 * Revision 1.5  1995/05/10  06:37:43  ralf
 * Added own includefile (for consistency checking of decls)
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
     name : ignore.c
    autor : DORNER Fernando, GRANZER Andreas
            POLZER Friedrich,TRISKO Gerhard
 * changed TryVariableIgnore: use search on sorted array
  purpose : ignores variable-name-commands which can't be converted from LaTeX2Rtf
	    (variable-command-formats must be added by the user in the file
	     "ignore.cfg")
 *****************************************************************************/

/****************************  includes *************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "main.h"
#include "direct.h"
#include "fonts.h"
#include "cfg.h"
#include "ignore.h"
/*****************************************************************************/

/************************   extern variables *********************************/
extern char *progname;
extern long linenumber;
/*****************************************************************************/

/***********************      prototypes   ***********************************/
void IgnoreVar(FILE *fRtf);
void IgnoreCmd(FILE *fTex);
/*****************************************************************************/


BOOL TryVariableIgnore(char *command, FILE *fTex)
/****************************************************************************
purpose : ignores variable-formats shown in file "ignore.cfg"
params	:    fTex: open Tex-File
	  command: variable-name-command
globals : progname
returns : TRUE if variable was ignored correctly,
	  else -> FALSE
 ****************************************************************************/
{
  int i;
  char *buffpoint;
  char *RtfCommand;
  char TexCommand[128];

  if (strlen(command) >= 100)
  {
      fprintf(stderr,"\n%s: WARNING: Command %s is too long in LaTeX-File.\n",progname,command)
;
      return FALSE;    /* command too long */
  }

  TexCommand[0] = '\\';
  TexCommand[1] = '\0';
  strcat (TexCommand, command);

  RtfCommand = search (TexCommand, IGNORE_A);
  if (RtfCommand == NULL)
     return FALSE;

  if (strcmp(RtfCommand,"NUMBER")==0)
  {
     IgnoreVar(fTex);
     return TRUE;
  }
  if (strcmp(RtfCommand,"MEASURE")==0)
  {
     IgnoreVar(fTex);
     return TRUE;
  }
  if (strcmp(RtfCommand,"OTHER")==0)
  {
     IgnoreVar(fTex);
     return TRUE;
  }
  if (strcmp(RtfCommand,"COMMAND")==0)
  {
     IgnoreCmd(fTex);
     return TRUE;
  }
  if (strcmp(RtfCommand,"SINGLE")==0)
  {
     return TRUE;
  }

  return FALSE;

}


/****************************************************************************
purpose : ignores anything till a space or a newline
params	: fTex: open Tex-File
globals : linenumber
 ****************************************************************************/
void IgnoreVar(FILE *fTex)
{
  char dummy;
  fread(&dummy,1,1,fTex);
  if (dummy == '\n')
      linenumber++;
  do
  {
    fread(&dummy,1,1,fTex);
    if (dummy == '\n')
	linenumber++;
  } while ((dummy != ' ') && (dummy != '\n'));
}


/****************************************************************************
purpose : ignores anything till an alphanumeric character 
params	: fTex: open Tex-File
globals : linenumber
 ****************************************************************************/
void IgnoreCmd(FILE *fTex)
{
  char dummy;
  fread(&dummy,1,1,fTex);
  if (dummy == '\n')
      linenumber++;
  do
  {
    fread(&dummy,1,1,fTex);
    if (dummy == '\n')
	linenumber++;
  } while (dummy != '\\');
  do
  {
    fread(&dummy,1,1,fTex);
    if (dummy == '\n')
	linenumber++;
  }
  while (!isalpha(dummy));
  if (dummy == '\n')
     linenumber--;
  fseek(fTex,-1,SEEK_CUR);
}
