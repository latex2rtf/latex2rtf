/*
 * $Id: direct.c,v 1.5 2001/08/12 17:50:50 prahl Exp $
 * History:
 * $Log: direct.c,v $
 * Revision 1.5  2001/08/12 17:50:50  prahl
 * latex2rtf version 1.9b by Scott Prahl
 * 1.9b
 * 	Improved enumerate environment so that it may be nested and
 * 	    fixed labels in nested enumerate environments
 * 	Improved handling of description and itemize environments
 * 	Improved eqnarray environment
 * 	Improved array environment
 * 	Improved \verb handling
 * 	Improved handling of \mbox and \hbox in math mode
 * 	Improved handling of \begin{array} environment
 * 	Improved handling of some math characters on the mac
 * 	Fixed handling of \( \) and \begin{math} \end{math} environments
 * 	Fixed bugs in equation numbering
 * 	Made extensive changes to character translation so that the RTF
 * 	     documents work under Word 5.1 and Word 98 on the Mac
 *
 *
 * 1.9a
 * 	Fixed bug with 'p{width}' in tabular environment
 * 		not fully implemented, but no longer creates bad RTF code
 *
 * 1.9
 * 	Fixed numbering of equations
 * 	Improved/added support for all types of equations
 * 	Now includes PICT files in RTF
 * 	Fixed \include to work (at least a single level of includes)
 *
 * 1.8
 * 	Fixed problems with \\[1mm]
 * 	Fixed handling of tabular environments
 * 	Fixed $x^\alpha$ and $x_\alpha$
 *
 * Revision 1.7  1998/10/28 04:09:56  glehner
 * (WriteFontName): Cleaned up. Eliminated unecessary warning
 * and not completed rtf-output when using *Font*.
 *
 * Revision 1.6  1998/07/03 07:03:16  glehner
 * lclint cleaning
 *
 * Revision 1.5  1997/02/15 20:45:41  ralf
 * Some lclint changes and corrected variable declarations
 *
 * Revision 1.4  1995/03/23 15:58:08  ralf
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
#include <string.h>
#include "main.h"
#include "direct.h"
#include "l2r_fonts.h"
#include "cfg.h"
/******************************************************************************/

/*************************** prototypes **************************************/

static bool WriteFontName(const char **buffpoint, FILE *fRtf);

/******************************* defines *************************************/
#define MAXFONTLEN 100
/******************************************************************************/


/******************************************************************************/
bool WriteFontName(const char **buffpoint, FILE *fRtf)
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
  size_t fnumber;

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
      fprintf(stderr, "\n%s: ERROR: Invalid fontname in direct command",
	      progname);
      exit(EXIT_FAILURE);
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
    exit(EXIT_FAILURE);
  }
  else
  {
    fprintf(fRtf,"%u",(unsigned int)fnumber);
    return TRUE;
  }
}


/******************************************************************************
  purpose: reads from the direct-array how some easy LaTex-commands can be
	   converted into Rtf-commands by text exchange
parameter: command: LaTex-command and Rtf-command
	   fRtf: File-Pointer to Rtf-File
globals:   progname
 ******************************************************************************/
bool
TryDirectConvert(char *command, FILE *fRtf)
{
  const char *buffpoint;
  const char *RtfCommand;
  char TexCommand[128];

  if (strlen(command) >= 100)
    {
      fprintf(stderr,"\n%s: WARNING: Command %s is too long in LaTeX-File.\n",progname,command);
      return FALSE;    /* command too long */
    }
  
  TexCommand[0] = '\\';
  TexCommand[1] = '\0';
  strcat (TexCommand, command);
  
  RtfCommand = SearchRtfCmd (TexCommand, DIRECT_A);
  if (RtfCommand == NULL)
    return FALSE;
  
  buffpoint = RtfCommand;
  diagnostics(4, "Direct converting `%s' command to `%s'.",
	      TexCommand, RtfCommand);
  while (buffpoint[0] != '\0')
    {
      if (buffpoint[0] == '*')
	{
	  ++buffpoint;
	  (void)WriteFontName(&buffpoint, fRtf);

	  /* From here on it is not necesarry
	     if (WriteFontName(&buffpoint, fRtf))
	     {
	     fprintf(stderr,
	     "\n%s: WARNING: error in direct command file"
	     " - invalid font name , \n",
	     progname);
	     return FALSE;
	     }
	     */
	}
      else
	{
	  fprintf(fRtf,"%c",*buffpoint);
	}
      
      ++buffpoint;
      
    }  /* end while */
  return TRUE;
}


