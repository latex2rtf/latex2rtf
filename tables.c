/* $Id: tables.c,v 1.4 2001/09/26 03:31:56 prahl Exp $

   Translation of tabbing and tabular environments
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include "main.h"
#include "convert.h"
#include "l2r_fonts.h"
#include "commands.h"
#include "funct1.h"
#include "tables.h"
#include "stack.h"
#include "cfg.h"
#include "util.h"
#include "parser.h"
#include "counters.h"

static int      tabstoparray[100];
static int      number_of_tabstops = 0;

static void     Convert_Tabbing_with_kill(void);

void 
CmdTabbing(int code)
/******************************************************************************
  purpose: pushes all tabbing-commands on a stack
parameter: code : on/off at begin/end-environment
  globals: tabbing_on: true if tabbing-mode is on (only in this environment)
	   tabbing_return, tabbing_itself: true if environment ends
 ******************************************************************************/
{
	if (code & ON) {	/* on switch */
		code &= ~(ON);	/* mask MSB */
		diagnostics(4, "Entering CmdTabbing");
		if (code == TABBING) {
			tabbing_on = TRUE;
			/* tabbing_on_itself = FALSE; */

			fprintRTF("\\par\\line ");
			if (fgetpos(fRtf, &pos_begin_kill) != 0)
				diagnostics(ERROR, "Failed fgetpos; funct2.c (Tabbing): errno %d", errno);
			/* Test ConvertTabbing(); */
		}
	} else {		/* off switch */
		/*
		 * tabbing_return = TRUE; tabbing_on_itself = TRUE;
		 */
		tabbing_on = FALSE;

		fprintRTF("\\par\\pard\\line\\q%c ", alignment);
		diagnostics(4, "Exiting CmdTabbing");
	}
}

void 
CmdTabset()
/******************************************************************************
 purpose: sets an tabstop
globals:  tabcounter: specifies the tabstop-position
 ******************************************************************************/
{
	int             tabstop;
	tabstop = (tabcounter / 6) * 567;
	tabstoparray[number_of_tabstops] = tabstop;
	number_of_tabstops++;
	fprintRTF("\\tx%d ", tabstop);	/* Tab at tabstop/567
						 * centimeters */
}

void 
CmdTabjump()
/******************************************************************************
 purpose: jumps to an tabstop
 ******************************************************************************/
{
	fprintRTF("\\tab ");
}

void 
CmdTabkill( /* @unused@ */ int code)
/******************************************************************************
 purpose: a line in the tabbing-Environment which ends with an kill-command won't be
	 written to the rtf-FILE
 ******************************************************************************/
{
	int             i;

	if (fsetpos(fRtf, &pos_begin_kill) != 0)
		diagnostics(ERROR, "Failed fsetpos; funct2.c (CmdTabkill): errno %d", errno);

	for (i = 0; i < number_of_tabstops; i++) {
		fprintRTF("\\tx%d ", tabstoparray[i]);	/* Tab at tabstop/567 centimeters */
	}

	number_of_tabstops = 0;
}

void 
ConvertTabbing(void)
/******************************************************************************
 purpose: routine which converts the tabbing-commands from LaTex to Rtf
 ******************************************************************************/
{
	int             read_end = 1024;
	char            cCommand[MAXCOMMANDLEN];
	int             i;
	long            j = 0;
	char            cThis;
	bool            getcommand;
	bool            command_end_line_found;
	bool            command_kill_found;

	while (tabbing_on) {
		command_end_line_found = FALSE;
		command_kill_found = FALSE;

		while (command_end_line_found) {
			for (;;) {	/* do forever */
				getcommand = FALSE;
				cThis = getTexChar();
				j++;

				if (cThis == '\\') {
					getcommand = TRUE;
					strcpy(cCommand, "");

					for (i = 0;; i++) {	/* get command from
								 * input stream */
						cThis = getTexChar();
						j++;

						if (i == 0) {	/* test for special
								 * characters */
							switch (cThis) {
							case '\\':
								command_end_line_found = TRUE;
								break;
							}	/* switch */
						}	/* if */
						if (!isalpha((unsigned char) cThis)) {
							while (cThis == ' ') {	/* all spaces after
										 * commands are ignored */
								cThis = getTexChar();
								j++;
							}

							ungetTexChar(cThis);	/* position of next
										 * character after
										 * command except space */
							j--;
							break;	/* for */
						}
						cCommand[i] = cThis;
					}	/* for */

					cCommand[i] = '\0';	/* mark end of string
								 * with zero */
				}	/* if \\ */
				if ((getcommand) &&
				    ((command_end_line_found) ||
				     (strcmp(cCommand, "kill") == 0) ||
				     (strcmp(cCommand, "end") == 0))) {
					command_end_line_found = TRUE;
					if (strcmp(cCommand, "kill") == 0)
						command_kill_found = TRUE;
					break;
				}
				if (j >= read_end) {
					command_end_line_found = TRUE;
					break;
				}
			}	/* for */
		}		/* while command_end_line_found */

		ungetTexChar(cThis);	/* re_read line  PROBABLY WRONG! */
		if (command_kill_found)
			Convert_Tabbing_with_kill();
		else
		{
			diagnostics(4, "Entering Convert() from within tabbing");
			Convert();
			diagnostics(4, "Exiting Convert() from within tabbing");
		}
	}			/* while Tabbing_ON */

	tabbing_on = FALSE;
}				/* ConvertTabbing */


void 
Convert_Tabbing_with_kill(void)
/******************************************************************************
 purpose: routine which converts the tabbing-kill-option from LaTex to Rtf
 globals: tabcounter:
 ******************************************************************************/
{
	int             i = 0;
	bool            command_kill_found = FALSE;
	char            cThis;
	char            cCommand[MAXCOMMANDLEN];

	tabcounter = 0;

	while (command_kill_found) {
		cThis = getTexChar();

		strcpy(cCommand, "");

		if (cThis == '\\') {

			for (i = 0;; i++) {	/* get command from input
						 * stream */
				cThis = getTexChar();

				if (i == 0) {	/* test for special
						 * characters */
					switch (cThis) {
					case '=':
						CmdTabset();
						break;
					default:
						if (!isalpha((unsigned char) cThis))
							numerror(ERR_WRONG_COMMAND_IN_TABBING);
					}	/* switch */
				}	/* if */
				if (!isalpha((unsigned char) cThis)) {
					while (cThis == ' ')	/* all spaces after
								 * commands are ignored */
						cThis = getTexChar();

					ungetTexChar(cThis);	/* position of next
								 * character after
								 * command except space */
					break;	/* for */
				}
				cCommand[i] = cThis;
			}	/* for */

			cCommand[i] = '\0';	/* mark end of string with
						 * zero */
		}
		 /* if \\ */ 
		else
			tabcounter++;

		if (strcmp(cCommand, "kill") == 0) {
			command_kill_found = TRUE;
			tabcounter = 0;
			break;
		}
	}			/* while command_kill_found */
}				/* Convert_Tabbing_with_kill */

void 
CmdTabular(int code)
/******************************************************************************
 purpose: converts the LaTex-Tabular to a similar Rtf-style
          the size of the table should be adjusted by hand
parameter: type of array-environment
 globals: fTex: Tex-file-pointer
          fRtf: Rtf-file-pointer
          g_processing_tabular: TRUE if EnvironmenTabular is converted
          colFmt: contains alignment of the columns in the tabular
          colCount: number of columns in tabular is set
          actCol: actual treated column

   Does not handle \begin{tabular*}{width}[h]{ccc}
   but it does do \begin{tabular*}[h]{ccc}
              and \begin{tabular*}{ccc}

 ******************************************************************************/
{
	char            dummy[51];
	int             i, n;
	static bool     bWarningDisplayed = FALSE;
	int             openBracesInParam = 1;
	char           *colParams;


	if (code & ON) {	/* on switch */
		code &= ~(ON);	/* mask MSB */

		if (g_processing_tabular)
			error(" Nested tabular and array environments not supported! Giving up! \n");
		g_processing_tabular = TRUE;

		getBracketParam(dummy, 50);	/* throw it away */
		diagnostics(5, "Discarding bracket string in tabular [%s]\n",dummy); 
		colParams = getParam();	/* colParams should now the column instructions */

		diagnostics(4, "Entering CmdTabular() with options {%s}\n",colParams); 

		if (!bWarningDisplayed) {
			diagnostics(WARNING, "Tabular or array environment: May need resizing");
			bWarningDisplayed = TRUE;
		}
		assert(colFmt == NULL);
		colFmt = (char *) malloc(sizeof(char) * 20);
		if (colFmt == NULL)
			error(" malloc error -> out of memory!\n");
		n = 0;
		colFmt[n++] = ' ';	/* colFmt[0] unused */
		i = 0;
		while (colParams[i]) {
			/* fprintf(stderr,"char='%c'\n",colParams[i]); */
			switch (colParams[i++]) {
			case 'c':
				colFmt[n++] = 'c';
				break;
			case 'r':
				colFmt[n++] = 'r';
				break;
			case 'l':
				colFmt[n++] = 'l';
				break;
			case '{':
				openBracesInParam++;
				break;
			case '}':
				openBracesInParam--;
				break;
			case 'p':
				diagnostics(WARNING, " 'p{width}' not fully supported");
				colFmt[n++] = 'l';
				break;
			case '*':
				diagnostics(WARNING, " '*{num}{cols}' not supported.\n");
				break;
			case '@':
				diagnostics(WARNING, " '@{text}' not supported.\n");
				break;
			default:
				break;
			}
		}

		free(colParams);
		colFmt[n] = '\0';
		colCount = n - 1;
		actCol = 1;

		if (code == TABULAR_2) {
			fprintRTF("\n\\par\\pard");
			fprintRTF("\\tqc\\tx1000\\tx2000\\tx3000\\tx4000\\tx5000\\tx6000\\tx7000\n\\tab");
			return;
		}
		fprintRTF("\\par\\trowd\\trqc\\trrh0");
		for (i = 1; i <= colCount; i++) {
			fprintRTF("\\cellx%d ", (7236 / colCount) * i);	/* 7236 twips in A4 page */
		}
		fprintRTF("\n\\pard\\intbl\\q%c ", colFmt[1]);
	} else {		/* off switch */
		diagnostics(4, "Exiting CmdTabular");
		code &= ~(OFF);	/* mask MSB */
		g_processing_tabular = FALSE;

		assert(colFmt != NULL);
		free(colFmt);
		colFmt = NULL;

		if (code == TABULAR_2)
			return;

		for (; actCol < colCount; actCol++) {
			fprintRTF("\\cell\\pard\\intbl ");
		}
		fprintRTF("\\cell\\pard\\intbl\\row\n");
		fprintRTF("\n\\pard\\par\\pard\\q%c\n", alignment);

	}
}

void 
CmdTable(int code)
/******************************************************************************
 purpose: converts the LaTex-Table to a similar Rtf-style
	  this converting is only partially
	  so the user has to convert some part of the Table-environment by hand
parameter: type of array-environment
 ******************************************************************************/
{
	char            location[10];

	if (code & ON) {	/* on switch */
		code &= ~(ON);	/* mask MSB */

		CmdEndParagraph(0);
		CmdIndent(INDENT_NONE);
		
		if ((code == FIGURE) || (code == FIGURE_1))
			g_processing_figure = TRUE;

		getBracketParam(location, 10);
	} else {		/* off switch */
		code &= ~(OFF);	/* mask MSB */
		g_processing_figure = FALSE;
		CmdEndParagraph(0);
	}
}

