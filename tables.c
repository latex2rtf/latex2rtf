/* $Id: tables.c,v 1.16 2002/04/13 19:59:27 prahl Exp $

   Translation of tabbing and tabular environments
*/

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "main.h"
#include "convert.h"
#include "l2r_fonts.h"
#include "commands.h"
#include "funct1.h"
#include "tables.h"
#include "stack.h"
#include "cfg.h"
#include "parser.h"
#include "counters.h"
#include "util.h"
#include "lengths.h"

int             tabcounter = 0;
bool            tabbing_on = FALSE;
bool            tabbing_return = FALSE;
bool            tabbing_on_itself = FALSE;
long          	pos_begin_kill;
bool			g_processing_table = FALSE;

int             colCount;			/* number of columns in a tabular environment */
int             actCol;				/* actual column in the tabular environment */
char           *colFmt = NULL;

static int      tabstoparray[100];
static int      number_of_tabstops = 0;

static void     Convert_Tabbing_with_kill(void);

static char *
ConvertFormatString(char *s)
/******************************************************************************
 purpose: convert latex formatting to something simpler
 ******************************************************************************/
{
	int braces,i;
	char *simple;
	
	simple = strdup(s);  /* largest possible */
	diagnostics(1, "Entering ConvertFormatString, input=<%s>",s);
	
	i=0;
	while (*s) {

		switch (*s) {
			case 'c':
			case 'r':
			case 'l':
				simple[i++] = *s;
				break;
			case '{':		/* skip to balancing brace */
				braces=1;
				s++;
				while (*s && (braces>1 || *s!='}')) {
					if (*s=='{') braces++;
					if (*s=='}') braces--;
					s++;
				}
				break;
			case 'p':
				diagnostics(WARNING, " 'p{width}' not fully supported");
				simple[i++] = 'l';
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
	s++;
	}
	simple[i]='\0';
	diagnostics(1, "Exiting ConvertFormatString, output=<%s>",simple);
	return simple;
}

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
/*			pos_begin_kill= ftell(fRtf);*/
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
	 \kill does not work until tables.c gets rewritten
 ******************************************************************************/
{
	int             i;

/*	fseek(fRtf, pos_begin_kill, SEEK_SET);*/

	for (i = 0; i < number_of_tabstops; i++) {
		fprintRTF("\\tx%d ", tabstoparray[i]);	/* Tab at tabstop/567 centimeters */
	}

	number_of_tabstops = 0;
}

void 
ConvertTabbing(void)
/******************************************************************************
 purpose: converts tabbing commands from LaTeX to Rtf
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

			for (i = 0; ; i++) {	/* get command from input stream */
				cThis = getTexChar();

				if (i == 0) {	/* test for special characters */
					switch (cThis) {
					case '=':
						CmdTabset();
						break;
					default:
						if (!isalpha((unsigned char) cThis))
							diagnostics(ERROR, "Wrong command in tabbing environment");
					}
				}
				if (!isalpha((unsigned char) cThis)) {
					cThis = getNonSpace();
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
 purpose: 	\begin{tabular}[pos]{cols}          ... \end{tabular}
 			\begin{tabular*}{width}[pos]{cols}  ... \end{tabular*}
 			\begin{array}[pos]{cols}            ... \end{array}

          colFmt: contains alignment of the columns in the tabular
          colCount: number of columns in tabular is set
          actCol: actual treated column
 ******************************************************************************/
{
	int             true_code,i,colWidth;
	char           *cols,*pos;

	true_code = code & ~(ON);
	if (code & ON) {

		if (g_processing_tabular)
			diagnostics(ERROR, "Nested tabular and array environments not supported! Giving up! \n");
		g_processing_tabular = TRUE;

		pos = getBracketParam();
		cols = getBraceParam();		

		diagnostics(4, "Entering CmdTabular() options [%s], format {%s}\n",pos, cols); 

		if (pos) free(pos);

		colFmt = ConvertFormatString(cols);
		free(cols);
		colCount = strlen(colFmt);
		actCol = 0;
		colWidth = getLength("textwidth")/colCount;

		if (true_code == TABULAR_2) {
			fprintRTF("\n\\par\\pard");
			fprintRTF("\\tqc\\tx1000\\tx2000\\tx3000\\tx4000\\tx5000\\tx6000\\tx7000\n\\tab");
			return;
		}
		fprintRTF("\\par\n\\trowd\\trrh0");

		for (i = 0; i < colCount; i++) {
			fprintRTF("\\cellx%d", colWidth * (i+1));
		}
		fprintRTF("\n\\pard\\intbl\\q%c ", colFmt[actCol]);
	
		CmdIndent(INDENT_NONE);
	} else {
		diagnostics(4, "Exiting CmdTabular");
		g_processing_tabular = FALSE;

		assert(colFmt != NULL);
		free(colFmt);
		colFmt = NULL;

		if (code == TABULAR_2)
			return;

		for (; actCol < colCount; actCol++) {
			fprintRTF("\\cell\\pard\\intbl ");
		}
		fprintRTF("\\row\\pard\\par\\pard\\q%c\n", alignment);

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
	char            *location, *table_contents;
	char	endtable[] = "\\end{table}";

	if (code & ON) {

		location = getBracketParam();
		if (location) free(location);

		CmdEndParagraph(0);
		CmdIndent(INDENT_NONE);

		g_processing_table = TRUE;
		table_contents = getTexUntil(endtable, TRUE);
		g_table_label = ExtractLabelTag(table_contents);
		ConvertString(table_contents);	
		ConvertString(endtable);
		free(table_contents);		
	} else {
		g_processing_table = FALSE;
		CmdEndParagraph(0);
		if (g_table_label) free(g_table_label);
	}
}

void 
CmdMultiCol(int code)
/******************************************************************************
 purpose: handles \multicolumn{n}{format}{content}
 ******************************************************************************/
{
	long            numCol, i, toBeInserted;
	char            *num, *format, *content, *colFormat;

	num     = getBraceParam();
	format  = getBraceParam();
	content = getBraceParam();
	
	diagnostics(1,"CmdMultiCol cols=%s format=<%s> content=<%s>",num,format,content);
	numCol = atoi(num);
	free(num);
	
	colFormat=ConvertFormatString(format);
	free(format);

	diagnostics(1,"CmdMultiCol cols=%d format=<%s> content=<%s>",numCol,colFormat,content);
	
	switch (colFormat[0]) {
		case 'r':
			toBeInserted = numCol;
			break;
		case 'c':
			toBeInserted = (numCol + 1) / 2;
			break;
		default:
			toBeInserted = 1;
			break;
	}

	for (i = 1; i < toBeInserted; i++, actCol++) 
		fprintRTF(" \\cell \\pard \\intbl ");
	
	fprintRTF("\\q%c ", colFormat[0]);

	diagnostics(4, "Entering Convert() from CmdMultiCol()");
	ConvertString(content);
	free(content);
	diagnostics(4, "Exiting Convert() from CmdMultiCol()");

	for (i = toBeInserted; (i < numCol) && (actCol < colCount); i++, actCol++) 
		fprintRTF(" \\cell \\pard \\intbl ");

}
