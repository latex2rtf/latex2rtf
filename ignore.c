/*
 * $Id: ignore.c,v 1.1 2001/08/12 15:32:26 prahl Exp $
 * History:
 * $Log: ignore.c,v $
 * Revision 1.1  2001/08/12 15:32:26  prahl
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
     name : ignore.c
    autor : DORNER Fernando, GRANZER Andreas
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
/*****************************************************************************/

/************************   extern variables *********************************/
extern char *progname;
extern long linenumber;
extern char *latexname;
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
returns : TRUE if variable was ignored correctly,
	  else -> FALSE
 ****************************************************************************/
{
  FILE *fp;
  char buffer[512];
  int i;
  char *buffpoint;
  char *typepoint;
  /*char dummy;*/
  if (strlen(command) >= 100)
      {
      fprintf(stderr,"\n%s: WARNING: Word %s is too long.\n",progname,command);
      return FALSE;    /* command too long */
      }
  fp = open_cfg("ignore.cfg");
  for(;;)
  {
    if ( (fgets( buffer, 512, fp )) == NULL)
    {
      fclose(fp);
      return FALSE;
    }
    if (strlen(buffer) > 500)
    {
      fprintf(stderr, "\n%s: WARNING: line too long in IGNORE.CFG - only 500 characters\n",progname);
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
	  fprintf(stderr, "\n%s: WARNING: error in IGNORE.cfg file - missing , \n",progname);
	  printf("buffp %d",(int)*buffpoint);
	  fclose(fp);
	  return FALSE;
	}
	++buffpoint;
      }
      ++buffpoint;
      typepoint = buffpoint;
      while (*buffpoint != '.')
      {
	if ( (*buffpoint == '#') || (*buffpoint == '\0') )
	{
	  fprintf(stderr, "\n%s: WARNING: error in IGNOREs.cfg - missing , \n",progname);
	  fclose(fp);
	  return FALSE;
	}
	++buffpoint;
      }
      *buffpoint = '\0';
      fclose(fp);
      if (strcmp(typepoint,"NUMBER")==0)
      {
	IgnoreVar(fTex);
	return TRUE;
      }
      if (strcmp(typepoint,"MEASURE")==0)
      {
	IgnoreVar(fTex);
	return TRUE;
      }
      if (strcmp(typepoint,"OTHER")==0)
      {
	IgnoreVar(fTex);
	return TRUE;
      }
      if (strcmp(typepoint,"COMMAND")==0)
      {
	IgnoreCmd(fTex);
	return TRUE;
      }
      if (strcmp(typepoint,"SINGLE")==0)
      {
	return TRUE;
      }
	 return FALSE;
    } /* end found */
  }  /* end while */
}


/****************************************************************************
purpose : ignores anything till a space or a newline
params	: fTex: open Tex-File
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
  fseek(fTex,-1,SEEK_CUR);
}
