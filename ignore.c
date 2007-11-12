/*
 * $Id: ignore.c,v 1.10 2001/08/12 19:32:24 prahl Exp $
 * History:
 * $Log: ignore.c,v $
 * Revision 1.10  2001/08/12 19:32:24  prahl
 * 1.9f
 * 	Reformatted all source files ---
 * 	    previous hodge-podge replaced by standard GNU style
 * 	Compiles cleanly using -Wall under gcc
 *
 * 	added better translation of \frac, \sqrt, and \int
 * 	forced all access to the LaTeX file to use getTexChar() or ungetTexChar()
 * 	    allows better handling of %
 * 	    simplified and improved error checking
 * 	    eliminates the need for WriteTemp
 * 	    potentially allows elimination of getLineNumber()
 *
 * 	added new verbosity level -v5 for more detail
 * 	fixed bug with in handling documentclass options
 * 	consolidated package and documentclass options
 * 	fixed several memory leaks
 * 	enabled the use of the babel package *needs testing*
 * 	fixed bug in font used in header and footers
 * 	minuscule better support for french
 * 	Added testing file for % comment support
 * 	Enhanced frac.tex to include \sqrt and \int tests also
 * 	Fixed bugs associated with containing font changes in
 * 	    equations, tabbing, and quote environments
 * 	Added essential.tex to the testing suite --- pretty comprehensive test.
 * 	Perhaps fix missing .bbl crashing bug
 * 	Fixed ?` and !`
 *
 * Revision 1.7  1998/07/03 06:56:08  glehner
 * adde PARAMETER, PACKAGE, ENVCMD, ENVIRONMENT
 *
 * Revision 1.6  1997/02/15 20:53:46  ralf
 * Removed some global variable redeclarations
 * Other lclint corrections
 *
 * Revision 1.5  1995/05/10 06:37:43  ralf
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
#include <string.h>
#include "main.h"
#include "direct.h"
#include "l2r_fonts.h"
#include "cfg.h"
#include "ignore.h"
#include "funct2.h"
#include "commands.h"
#include "util.h"
#include "parser.h"
/*****************************************************************************/

/***********************      prototypes   ***********************************/
static void IgnoreVar(FILE *fRtf);
static void IgnoreCmd(FILE *fTex);
/*****************************************************************************/


bool TryVariableIgnore(char *command, FILE *fTex)
/****************************************************************************
purpose : ignores variable-formats shown in file "ignore.cfg"
params	:    fTex: open Tex-File
	  command: variable-name-command
globals : progname
returns : TRUE if variable was ignored correctly,
	  else -> FALSE
LEG190498
expanded the syntax with PARAMTER, PACKAGE, ENVCMD, ENVIRONMENT:
PARAMETER	ignores a command with one paramter
PACKAGE		does not produce a Warning message if PACKAGE is
		encountered
ENVCMD		proceses contents of unknown environment as if it were
		plain latex
ENVIRONMENT     ignores contentents of that environment
 ****************************************************************************/
{
  const char *RtfCommand;
  char TexCommand[128];
  bool result = TRUE;

  if (strlen(command) >= 100)
  {
      fprintf(stderr,"\n%s: WARNING: Command %s is too long in LaTeX-File.\n",progname,command)
;
      return FALSE;    /* command too long */
  }

  TexCommand[0] = '\\';
  TexCommand[1] = '\0';
  strcat (TexCommand, command);

  RtfCommand = SearchRtfCmd (TexCommand, IGNORE_A);
  if (RtfCommand == NULL)
    result = FALSE;
  else if (strcmp(RtfCommand,"NUMBER")==0)
    IgnoreVar(fTex);
  else if (strcmp(RtfCommand,"MEASURE")==0)
    IgnoreVar(fTex);
  else if (strcmp(RtfCommand,"OTHER")==0)
    IgnoreVar(fTex);
  else if (strcmp(RtfCommand,"COMMAND")==0)
    IgnoreCmd(fTex);
  else if (strcmp(RtfCommand,"SINGLE")==0)
      ;
  else if (strcmp(RtfCommand,"PARAMETER")==0)
    CmdIgnoreParameter(No_Opt_One_NormParam);
//  else if (strcmp(RtfCommand,"LINE")==0)
//    skipToEOL();
  else if (strcmp(RtfCommand,"ENVIRONMENT")==0)
    {
      char *str;
      str = malloc(strlen(command)+5); /* envelope: end{..} */
      if (str == NULL)
	error(" malloc error -> out of memory!\n");
      strcpy(str, "end{");
      strcat(str, command);
      strcat(str, "}");
      Ignore_Environment(str);
      free(str);
    }
  else if (strcmp(RtfCommand,"ENVCMD")==0)
    PushEnvironment(IGN_ENV_CMD);
  else if (strcmp(RtfCommand,"PACKAGE")==0)
      ;
  else
    result = FALSE;
  /*LEG210698*** lclint ?  free(RtfCommand);*/
  return(result);
}


/****************************************************************************
purpose : ignores anything till a space or a newline
params	: fTex: open Tex-File
globals : linenumber
 ****************************************************************************/
void IgnoreVar(FILE *fTex)
{
  char c;
  while ( (c=getTexChar()) && c != '\n' && c != ' ') ;
}


/****************************************************************************
purpose : ignores anything till an alphanumeric character 
params	: fTex: open Tex-File
globals : linenumber
 ****************************************************************************/
void IgnoreCmd(FILE *fTex)
{
  char c;
  while ( (c=getTexChar()) && c != '\\') ;
  while ( (c=getTexChar()) && !isalpha(c) ) ;
  rewind_one(c);
}
