/* $Id: tables.c,v 1.24 2002/06/23 00:10:28 prahl Exp $

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

typedef struct TabularT {
	int n;			/* number of columns                              */
	char *align;	/* align[1] is the alignment of the first column  */
	int *vert;		/* vert[0] is the number of |'s before first column
	                   vert[1] is the number of |'s after first column
	                   vert[n] is the number of |'s after last column */
	int *width;	/* width[1] is the width of the first column      */	
	int *cline;  /* cline[1] is true when line should be draws across column 1 */
} TabularT;	

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
TabularCountVert(char *s, int vert[])
/******************************************************************************
 purpose:  fill vert[] with number of '|' for each column
 ******************************************************************************/
{
	int braces,i;
	
	diagnostics(4, "Entering TabularCountVert, input=<%s>",s);
	
	i=0;
	while (*s) {

		switch (*s) {
			case 'c': case 'r': case 'l': case 'p':
				i++;
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
			case '|':
				vert[i]++;
				break;
			default:
				break;
		}
		s++;
	}
	diagnostics(4, "Exiting TabularCountVert");
}

static TabularT
TabularPreamble(char *text, char *width, char *pos, char *cols)
/******************************************************************************
 purpose:  calculate column widths 
 ******************************************************************************/
{	TabularT tab;
	int i;
	
	colFmt = ConvertFormatString(cols);
	colCount = strlen(colFmt);
	actCol = 0;
	
	tab.n = colCount;
	tab.align = (char *)calloc(sizeof(char)*(colCount+1),sizeof(char));
	tab.vert  = (int  *)calloc(sizeof(int )*(colCount+1),sizeof(int ));
	tab.width = (int  *)calloc(sizeof(int )*(colCount+1),sizeof(int ));
	tab.cline = (int  *)calloc(sizeof(int )*(colCount+1),sizeof(int ));
	
	for (i=1; i<=colCount; i++)
		tab.align[i] = colFmt[i-1];
		
	TabularCountVert(cols,tab.vert);

	if (GetTexMode() != MODE_HORIZONTAL){
		CmdIndent(INDENT_NONE);
		CmdStartParagraph(0);
	}
	
	fprintRTF("\\par\n");
	return tab; 
}

static void
TabularGetRow(char *table, char **row, char **next_row, int *height)
/******************************************************************************
 purpose:  scan and duplicate the next row from the table and any height changes
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
/******************************************************************************
 purpose:  find the next ampersand while avoiding \&
 ******************************************************************************/
{
	char *s;
	int escaped=0;
	
	s=t;
	
	while (s && *s != '\0' && (*s != '&' || (*s=='&' && escaped))) {
		escaped = (*s == '\\') ? 1 : 0;
		s++;
	}
	return s;
}

static void
TabularBeginCell(char align)
/******************************************************************************
 purpose:  emit RTF code to start each cell
 ******************************************************************************/
{
	fprintRTF("\\pard\\intbl\\q%c ", align);
}

static void
TabularEndCell(void)
/******************************************************************************
 purpose:  emit RTF code to end each cell
 ******************************************************************************/
{
	fprintRTF("\\cell\n");
}

static char *
TabularNextCell(char *cell_start, char **cell_end)
/******************************************************************************
 purpose:  scan and duplicate contents of the next cell
 ******************************************************************************/
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
TabularMultiParameters(char *cell, int *col_span, char *align, int *lvert, int *rvert)
/******************************************************************************
 purpose:  extract information from \multicolumn's on a line
           return col_span=0 if none are present
 ******************************************************************************/
{
	char *p, *format, *mformat, *num;
			
	*col_span=0;
	*align='\0';
	*lvert=0;
	*rvert=0;
	
	p=strstr(cell,"\\multicolumn");
	if (p == NULL) return;
	
	PushSource(NULL,p+strlen("\\multicolumn"));
	num     = getBraceParam();
	format  = getBraceParam();
	PopSource();
	mformat=ConvertFormatString(format);
	
	/* count '|' to the left of the column */
	p=format;
	while (p && *p && *p!='c' && *p!='l' && *p!='r'){
		if (*p=='|') (*lvert)++;
		p++;
	}
	if (p && *p) p++;

	/* count '|' to the right of the column */
	while (p && *p && *p!='c' && *p!='l' && *p!='r'){
		if (*p=='|') (*rvert)++;
		p++;
	}
			
	*col_span = atoi(num);
	*align = mformat[0];
	free(num);
	free(mformat);
	free(format);
	diagnostics(5, "TabularMultiParameters n=%d, align=%c",*col_span, *align); 
}

static int
TabularHline(char *row)
/******************************************************************************
 purpose:  count the number of \hline's in a line
 ******************************************************************************/
{
	char *s,*t;
	if (row==NULL) return 0;
	
	s=strstr(row,"\\hline");
	if (s==NULL) return 0;
	
	t = strstr(s+6,"\\hline");
	
	if (t==NULL) return 1;
	return 2;
}

char *
TabularCline(char *row, int columns)
/******************************************************************************
 purpose:  mark columns needing underline by \cline{ 1 - 2 }
           limited to 1-9 columns
 ******************************************************************************/
{
	char *s,*x;
	int i,n,m;
	char *cline=NULL;
		
	cline = (char *) malloc((columns+2)*sizeof(char));
	for (i=0;i<=columns+1;i++) 
		cline[i]='\0';
	
	if (row==NULL) return cline;

	/* hline commands override cline commands */
	n=TabularHline(row);
	
	for (i=0;i<=columns;i++) 
		cline[i]= n;	

	if (n) return cline;
	
	s = row;
	x = row + strlen(row);
	while (s<x) {
		s=strstr(s,"\\cline{");
		
		if (s==NULL) return cline;
		
		s+=7;

		while (*s && (*s==' ' || *s=='\n')) s++; 	/* skip spaces after { */
		
		if (*s<='0' || *s>='9') return cline;
		n=atoi(s);									/* first column */

		while (*s && *s!='-') s++;					/* skip past '-' */
		s++;
		
		while (*s && (*s==' ' || *s=='\n')) s++; 	/* skip spaces after '-' */
				
		if (*s<='0' || *s>='9') return cline;
		m=atoi(s);									/* second column */
		
		for (i=n; i<=m; i++) {
			if (i<=columns && i>=1) cline[i]=1;
		}		
		while (*s && *s!='}') s++;					/* skip to last '}' */
	}
	
	return cline;
}

static void
TabularBeginRow(TabularT tabular, char *this_row, char *next_row, int first_row)
/******************************************************************************
 purpose:  emit RTF to start one row of a table
 
           the vertical bar formatting codes for the entire row must be present at
           the start of the row.  The same holds for horizontal lines.  Only the first
           line may have an hlines above it.  Drawing hlines below depends on the
           next line containing an hline (i.e., after \\ that ends the row)
           
           
 ******************************************************************************/
{
	int i,n,column,rvert,lvert;
	char align;
	char *cell_start, *cell_end, *cell, *row;
	int top, left, bottom, right;  /* cell borders */
	char *cline;
	
	fprintRTF("\\trowd");

	row=this_row;
	cell_start = this_row;
	column = 0;
	
	cline = TabularCline(next_row,tabular.n);

	while (cell_start) {  /*for each cell */

		top    = 0;
		left   = 0;
		right  = tabular.vert[column+1];
		bottom = cline[column+1];
		if (first_row) top  = TabularHline(this_row);
		if (column==0) left = tabular.vert[0];
		
		cell = TabularNextCell(cell_start, &cell_end);
		TabularMultiParameters(cell,&n,&align,&lvert,&rvert);
		if (n>1) fprintRTF("\\clmgf");
			
		if (left   == 1) fprintRTF("\\clbrdrl\\brdrs" );
		if (left   == 2) fprintRTF("\\clbrdrl\\brdrdb");
		if (top    == 1) fprintRTF("\\clbrdrt\\brdrs");
		if (top    == 2) fprintRTF("\\clbrdrt\\brdrdb");
		if (bottom == 1) fprintRTF("\\clbrdrb\\brdrs");
		if (bottom == 2) fprintRTF("\\clbrdrb\\brdrdb");
		if (right  == 1 || (n == 1 && rvert == 1)) fprintRTF("\\clbrdrr\\brdrs" );
		if (right  == 2 || (n == 1 && rvert == 2)) fprintRTF("\\clbrdrr\\brdrdb");
		fprintRTF("\\cellx%d", TabularColumnPosition(column));
		column++;
		
		for (i=2; i<=n; i++) {
			fprintRTF("\\clmrg");
			if (top == 1) fprintRTF("\\clbrdrt\\brdrs");
			if (top == 2) fprintRTF("\\clbrdrt\\brdrdb");
			if (bottom == 1) fprintRTF("\\clbrdrb\\brdrs");
			if (bottom == 2) fprintRTF("\\clbrdrb\\brdrdb");
			if (i == n && rvert == 1) fprintRTF("\\clbrdrr\\brdrs");
			if (i == n && rvert == 2) fprintRTF("\\clbrdrr\\brdrdb");
			fprintRTF("\\cellx%d", TabularColumnPosition(column));
			column++;
		}	
		
		free(cell);
		cell_start = cell_end;
	}
	fprintRTF("\n");
	free(cline);
}

static void
TabularEndRow(void)
/******************************************************************************
 purpose:  emit RTF to finish one row of a table
 ******************************************************************************/
{
	fprintRTF("\\row\n");
}

char 
TabularColumnAlignment(int column)
/******************************************************************************
 purpose:  alignment for a column
 ******************************************************************************/
{
	return colFmt[column];
}

static void
TabularWriteRow(TabularT tabular, char *this_row, char *next_row, int height, int first_row)
{
	char *cell, *cell_start, *cell_end;
	char align;
	int n, lvert, rvert;
	
	actCol=0;
	if (this_row==NULL || strlen(this_row)==0) return;
	
	diagnostics(4, "TabularWriteRow height=%d twpi, row <%s>",height, this_row); 
	
	/* avoid writing anything for empty last row */
	if (next_row == NULL && !strchr(this_row, '&')) return;
	
	TabularBeginRow(tabular, this_row, next_row, first_row);
	cell_start = this_row;
	while (cell_start) {

		cell = TabularNextCell(cell_start, &cell_end);
		
		/* establish cell alignment */
		TabularMultiParameters(cell,&n,&align,&lvert,&rvert);
		if (n == 0) {
			align=TabularColumnAlignment(actCol);
			n = 1;
		}

		TabularBeginCell(align);
		if (cell !=NULL) {
			diagnostics(5, "TabularWriteRow align=%c n=%d cell=<%s>",align,n,cell); 
			fprintRTF("{");
			ConvertString(cell);
			fprintRTF("}");
		}
		TabularEndCell();
				
		actCol+=n;
		cell_start = cell_end;
		if (cell != NULL) free(cell);
	}

	if (actCol < colCount) {
		align=TabularColumnAlignment(actCol);
		TabularBeginCell(align);
		TabularEndCell();
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
	int             true_code,this_height, next_height, first_row;
	char           *cols,*pos,*end,*width,*this_row, *next_row, *next_row_start, *row_start;
	char   		   *table=NULL;
	TabularT        tabular;

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

	tabular=TabularPreamble(table,width,pos,cols);
	
	row_start=table;
	TabularGetRow(row_start,&this_row,&next_row_start,&this_height);
	first_row = TRUE;
	while (this_row) {
		row_start=next_row_start;
		TabularGetRow(row_start,&next_row,&next_row_start,&next_height);
		TabularWriteRow(tabular, this_row, next_row, this_height, first_row);
		free(this_row);
		this_row = next_row;
		this_height = next_height;
		first_row = FALSE;
	}
	
	ConvertString(end);

	if (pos) free(pos);
	if (width) free(width);
	free(cols);
	free(table);
	free(end);
	free(tabular.cline);
	free(tabular.align);
	free(tabular.vert);
	free(tabular.width);
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
	char            *num, *format, *content, *bcontent;

	num     = getBraceParam();
	format  = getBraceParam();
	bcontent = getBraceParam();
	
	content = strdup_noendblanks(bcontent);
	free(bcontent);

	diagnostics(4,"CmdMultiCol cols=%s format=<%s> content=<%s>",num,format,content);
	numCol = atoi(num);
	free(num);
	
	diagnostics(4, "Entering Convert() from CmdMultiCol()");
	ConvertString(content);
	diagnostics(4, "Exiting Convert() from CmdMultiCol()");

	for (i = 1; i < numCol; i++) {
		fprintRTF("}\\cell\n");
		fprintRTF("\\pard\\intbl{");
	}

	free(content);
}

void 
CmdHline(int code)
{
/* cline and hline are already handled by tabular code 
   here we just properly skip the commands             */
	char *s;
	
	if (code==1) {
		s = getBraceParam();
		free(s);
		skipSpaces();
	}
}
