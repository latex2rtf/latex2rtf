/* $Id: tables.c,v 1.20 2002/05/02 14:47:20 prahl Exp $

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
	diagnostics(4, "Entering ConvertFormatString, input=<%s>",s);
	
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
	diagnostics(4, "Exiting ConvertFormatString, output=<%s>",simple);
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

static int
TabularColumnPosition(int n)
/******************************************************************************
 purpose:  return position of nth column 
 ******************************************************************************/
{
	int colWidth = getLength("textwidth")/colCount;
	return colWidth * (n+1);
}

static void
TabularPreamble(char *text, char *width, char *pos, char *cols)
/******************************************************************************
 purpose:  calculate column widths 
 ******************************************************************************/
{	
	colFmt = ConvertFormatString(cols);
	colCount = strlen(colFmt);
	actCol = 0;

	if (GetTexMode() != MODE_HORIZONTAL){
		CmdIndent(INDENT_NONE);
		CmdStartParagraph(0);
	}
	
	fprintRTF("\\par\n");
}

static void
TabularGetRow(char *table, char **row, char **next_row, int *height)
/******************************************************************************
 purpose:  duplicate the next row from the table and any height changes
 			e.g.   the & cell & is & here \\[2pt]
 			       but & then & it & died \\
 			will return "the & cell & is & here" and height should be 40 (twips)
 ******************************************************************************/
{
	char *s,*dimension,*dim_start;
	int slash=0;
	int row_chars=0;
	int dim_chars=0;
	
	s=table;
	*row=NULL;
	*next_row=NULL;
	*height=0;
	
	if (!s) return;
	
/* find \\ */
	while (*s != '\0' && !(*s == '\\' && slash)) {
		slash = (*s == '\\') ? 1 : 0;
		row_chars++;
		s++;
	}

	if (row_chars) row_chars--;	
	*row = (char *)malloc((row_chars+1)*sizeof(char));
	strncpy(*row, table, row_chars);
	(*row)[row_chars]='\0';
	
	diagnostics(4,"row =<%s>",*row);
	if (*s=='\0') return;
	
/* move after \\ */
	s++;
	
/* skip blanks */
	while (*s != '\0' && (*s==' ' || *s=='\n')) s++;
		
	if (*s=='\0') return;

	if (*s != '[')  {
		*next_row = s;
		return;
	}
	
/* read line space dimension */
	s++;
	dim_start=s;
	while (*s != '\0' && *s != ']') {
		s++; 
		dim_chars++;
	}
	if (*s=='\0') return;

/* figure out the row height */	
	dimension=malloc((dim_chars+2)*sizeof(char));
	strncpy(dimension,dim_start,dim_chars);
	dimension[dim_chars]='\n';  /* make sure entire string is not parsed */
	dimension[dim_chars+1]='\0';
	
	if (PushSource(NULL,dimension) == 0) {
		*height=getDimension();
		PopSource();
	}

	diagnostics(4,"height =<%s>=%d twpi",dimension,height);
	free(dimension);

	/* skip blanks */
	s++;
	while (*s != '\0' && (*s==' ' || *s=='\n')) s++;

	if (*s=='\0') return;
	*next_row = s;
}

static char *
TabularNextAmpersand(char *t)
{
	char *s;
	int slash=0;
	
	s=t;
	
	while (s && *s != '\0' && *s != '&' && !(*s=='&' && slash)) {
		slash = (*s == '\\') ? 1 : 0;
		s++;
	}
	return s;
}

static void
TabularBeginCell(int column)
{
	fprintRTF("\\pard\\intbl\\q%c ", colFmt[column]);
}

static void
TabularEndCell(int column)
{
	fprintRTF("\\cell\n");
}

static char *
TabularNextCell(char *cell_start, char **cell_end)
{	
	char *end, *dup, *dup2;
	
	end = TabularNextAmpersand(cell_start);

	if (*end=='&') {
		dup = strndup(cell_start,end-cell_start);
		*cell_end = end+1;
	} else {
		dup = strdup(cell_start);
		*cell_end = NULL;
	}		
			
	dup2 = strdup_noendblanks(dup);
	free(dup);
			
	return dup2;
}

static void
TabularBeginRow(char *arow)
/******************************************************************************
 purpose:  emit RTF to start one row of a table
 ******************************************************************************/
{
	int i,n,column;
	char *cell_start, *cell_end, *cell,*num,*format,*multi,*row;

	row=arow;
	fprintRTF("\\trowd");

	cell_start = row;
	column = 0;
	while (cell_start && *cell_start != '\0') {  /*for each cell */

		cell = TabularNextCell(cell_start, &cell_end);
		
		multi=strstr(cell,"\\multicolumn");
		if (multi == NULL) {
			fprintRTF("\\cellx%d", TabularColumnPosition(column));
			column++;
		} else {
			PushSource(NULL,multi+strlen("\\multicolumn"));
			num     = getBraceParam();
			format  = getBraceParam();
			PopSource();
			n = atoi(num);
			fprintRTF("\\clmgf\\cellx%d", TabularColumnPosition(column));
			for (i=1; i<n; i++)
				fprintRTF("\\clmrg\\cellx%d", TabularColumnPosition(column+i));
			column+=n;
			free(num);
			free(format);
		}	
		
		free(cell);
		cell_start = cell_end;
	}
	fprintRTF("\n");
}
static void
TabularEndRow(void)
/******************************************************************************
 purpose:  emit RTF to finish one row of a table
 ******************************************************************************/
{
	fprintRTF("\\row\n");
}

static void
TabularWriteRow(char *row, int height)
{
	char *cell, *cell_start, *cell_end;
	
	actCol=0;
	if (row==NULL || strlen(row)==0) return;
	
	diagnostics(4, "TabularWriteRow height=%d twpi, row <%s>",height, row); 
	
	TabularBeginRow(row);
	cell_start = row;
	while (cell_start && *cell_start != '\0') {

		TabularBeginCell(actCol);
		cell = TabularNextCell(cell_start, &cell_end);
		if (cell !=NULL && *cell != '\0') {
			diagnostics(4, "TabularWriteRow cell=<%s>",cell); 
			fprintRTF("{");
			ConvertString(cell);
			fprintRTF("}");
		}
		if (cell != NULL) free(cell);
		
		actCol++;
		cell_start = cell_end;
		TabularEndCell(actCol);
	}

	TabularEndRow();
}

void 
CmdTabular(int code)
/******************************************************************************
 purpose: 	\begin{tabular}[pos]{cols}          ... \end{tabular}
 			\begin{tabular*}{width}[pos]{cols}  ... \end{tabular*}
 			\begin{array}[pos]{cols}            ... \end{array}
 ******************************************************************************/
{
	int             true_code,height;
	char           *cols,*pos,*end,*width,*row, *next_row,*row_start;
	char   		   *table=NULL;

	if (!(code & ON)) {
		diagnostics(4, "Exiting CmdTabular");
		g_processing_tabular = FALSE;
		free(colFmt);
		colFmt=NULL;
		return;
	}

	width = NULL;
	true_code = code & ~(ON);
	switch (true_code) {
		case TABULAR: 
			end = strdup("\\end{tabular}");
			break;
		case TABULAR_STAR: 
			end = strdup("\\end{tabular*}");
			width = getBraceParam();
			break;
		case TABULAR_LONG: 
			end = strdup("\\end{longtable}");
			break;
		case TABULAR_LONG_STAR: 
			end = strdup("\\end{longtable*}");
			width = getBraceParam();
			break;
	}

	pos = getBracketParam();
	cols = getBraceParam();		
	
	diagnostics(3, "Entering CmdTabular() options [%s], format {%s}",pos, cols); 

	table = getTexUntil(end,FALSE);
	diagnostics(2, "table_table_table_table_table\n%stable_table_table_table_table",table);

	TabularPreamble(table,width,pos,cols);
	
	row_start=table;
	TabularGetRow(row_start,&row,&next_row,&height);
	while (row) {
		TabularWriteRow(row,height);
		free(row);
		row_start=next_row;
		TabularGetRow(row_start,&row,&next_row,&height);
	}
	
	ConvertString(end);

	if (pos) free(pos);
	if (width) free(width);
	free(cols);
	free(table);
	free(end);
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
	long            numCol, i;
	char            *num, *format, *content, *colFormat, *noblank_content;

	num     = getBraceParam();
	format  = getBraceParam();
	content = getBraceParam();
	
	noblank_content = strdup_noendblanks(content);
	free(content);
	content=noblank_content;

	diagnostics(1,"CmdMultiCol cols=%s format=<%s> content=<%s>",num,format,content);
	numCol = atoi(num);
	free(num);
	
	colFormat=ConvertFormatString(format);
	free(format);
	
	fprintRTF("}{\\q%c ", colFormat[0]);

	diagnostics(4, "Entering Convert() from CmdMultiCol()");
	ConvertString(content);
	diagnostics(4, "Exiting Convert() from CmdMultiCol()");

	fprintRTF("}\\cell\n");

	for (i = 2; i < numCol; i++)
		fprintRTF("\\pard\\intbl\\cell\n");

	fprintRTF("\\pard\\intbl{");
	actCol+=numCol-1;  /*TabularWriteRow will automatically add one */
	free(content);
	free(colFormat);
}
