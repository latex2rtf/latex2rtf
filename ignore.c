/* $Id: ignore.c,v 1.15 2001/09/16 05:11:19 prahl Exp $

  purpose : ignores variable-name-commands which can't be converted from LaTeX2Rtf
	    (variable-command-formats must be added by the user in the file
	     "ignore.cfg")
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "main.h"
#include "direct.h"
#include "l2r_fonts.h"
#include "cfg.h"
#include "ignore.h"
#include "funct1.h"
#include "commands.h"
#include "util.h"
#include "parser.h"

static void     IgnoreVar(void);
static void     IgnoreCmd(void);
static void 	Ignore_Environment(char *searchstring);

bool 
TryVariableIgnore(char *command)
/****************************************************************************
purpose : ignores variable-formats shown in file "ignore.cfg"
returns : TRUE if variable was ignored correctly, otherwise FALSE

#  NUMBER        simple numeric value
#  MEASURE : numeric value with following unit of measure
#  OTHER: ignores anything to the first character after '='
#	 and from there to next space. eg. \setbox\bak=\hbox
#  COMMAND       ignores anything to next '\' and from there to occurence
#	             of anything but a letter. eg. \newbox\bak
#  SINGLE        ignore single command. eg. \noindent
#  PARAMETER	 ignores a command with one paramter
#  PACKAGE		 does not produce a Warning message if PACKAGE is encountered
#  ENVCMD		 proceses contents of unknown environment as if it were plain latex
#  ENVIRONMENT   ignores contentents of that environment
 ****************************************************************************/
{
	const char     *RtfCommand;
	char            TexCommand[128];
	bool            result = TRUE;

	if (strlen(command) >= 100) {
		diagnostics(WARNING, "Command <%s> is too long", command);
		return FALSE;	/* command too long */
	}
	TexCommand[0] = '\\';
	TexCommand[1] = '\0';
	strcat(TexCommand, command);

	RtfCommand = SearchRtfCmd(TexCommand, IGNORE_A);
	if (RtfCommand == NULL)
		result = FALSE;
	else if (strcmp(RtfCommand, "NUMBER") == 0)
		IgnoreVar();
	else if (strcmp(RtfCommand, "MEASURE") == 0)
		IgnoreVar();
	else if (strcmp(RtfCommand, "OTHER") == 0)
		IgnoreVar();
	else if (strcmp(RtfCommand, "COMMAND") == 0)
		IgnoreCmd();
	else if (strcmp(RtfCommand, "SINGLE") == 0);
	else if (strcmp(RtfCommand, "PARAMETER") == 0)
		CmdIgnoreParameter(No_Opt_One_NormParam);
/*	else if (strcmp(RtfCommand, "LINE") == 0) skipToEOL(); */
	else if (strcmp(RtfCommand, "ENVIRONMENT") == 0) {
		char           *str;
		str = malloc(strlen(command) + 5);	/* envelope: end{..} */
		if (str == NULL)
			error(" malloc error -> out of memory!\n");
		strcpy(str, "end{");
		strcat(str, command);
		strcat(str, "}");
		Ignore_Environment(str);
		free(str);
	} else if (strcmp(RtfCommand, "ENVCMD") == 0)
		PushEnvironment(IGN_ENV_CMD);
	else if (strcmp(RtfCommand, "PACKAGE") == 0);
	else
		result = FALSE;
	return (result);
}


void 
IgnoreVar(void)
/****************************************************************************
purpose : ignores anything till a space or a newline
 ****************************************************************************/
{
	char            c;
	while ((c = getTexChar()) && c != '\n' && c != ' ');
}


void 
IgnoreCmd(void)
/****************************************************************************
purpose : ignores anything till an alphanumeric character
 ****************************************************************************/
{
	char            c;
	while ((c = getTexChar()) && c != '\\');
	while ((c = getTexChar()) && !isalpha((int)c));
	ungetTexChar(c);
}

void 
Ignore_Environment(char *searchstring)
/******************************************************************************
  purpose: function, which ignores an unconvertable environment in LaTex
           and writes text unchanged into the Rtf-file.
parameter: searchstring : includes the string to search for
	   example: \begin{unknown} ... \end{unknown}
		    searchstring="end{unknown}"
 ******************************************************************************/
{
	char            thechar;
	bool            found = FALSE;
	int             i, j, endstring;
	endstring = strlen(searchstring) - 1;
	while ((thechar = getTexChar()) && !found) {
		if (thechar == '\\') {
			for (i = 0; i <= endstring; i++) {
				thechar = getTexChar();

				if (thechar != searchstring[i])
					break;
				if (i == endstring)	/* end-environment-found */
					found = TRUE;
			}	/* for */

			if (!found) {
				fprintRTF("\\\\");
				for (j = 0; j < i; j++)
					switch (searchstring[j]) {
					case '\n':
						fprintRTF("\\par \n");
						break;
					case '\\':
					case '{':
					case '}':
						fprintRTF("\\%c",searchstring[j]);
						break;
					default:
						fprintRTF("%c", searchstring[j]);
						break;
					}
			}
		}		/* if */
		
		if ((thechar != '%') && !found)
			switch (thechar) {
			case '\n':
				fprintRTF("\\par \n");
				break;
			case '\\':
			case '{':
			case '}':
				fprintRTF("\\%c", thechar);
				break;
			default:
				fprintRTF("%c", thechar);
				break;
			}
	}
	return;
}

