/* tables.c - Translation of tabbing and tabular environments

Copyright (C) 2002 The Free Software Foundation

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

This file is available from http://sourceforge.net/projects/latex2rtf/
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
bool            tabbing_return = FALSE;
bool            tabbing_on_itself = FALSE;
long          	pos_begin_kill;
bool            g_processing_tabbing = FALSE;
bool			g_processing_table = FALSE;

int             colCount;			/* number of columns in a tabular environment */
int             actCol;				/* actual column in the tabular environment */
char           *colFmt = NULL;

void            CmdTabjump(void)
{
	fprintRTF("\\tab ");
}

void            CmdTabset(void)
{
}

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
TabularGetRow(char *table, char **row, char **next_row, int *height, int tabbing)
/******************************************************************************
 purpose:  scan and duplicate the next row from the table and any height changes
 			e.g.   the & cell & is & here \\[2pt]
 			       but & then & it & died \\
 			will return "the & cell & is & here" and height should be 40 (twips)
 if tabbing then \kill will end a line
 ******************************************************************************/
{
	char *s,*dimension,*dim_start;
	int slash=0;
	int row_chars=0;
	int dim_chars=0;
	bool slashslash=FALSE;
	
	s=table;
	*row=NULL;
	*next_row=NULL;
	*height=0;
	
	if (!s) return;

	/* the end of a row is caused by one of three things
	   1) the buffer ends
       2) two slashes in a row are found
       3) \kill is encountered                            */
	while (!(*s == '\0') &&
		   !(*s == '\\' && slash) &&
		   !(tabbing && (row_chars>6) && strncmp(s-6,"\\kill",5) == 0 && !isalpha((int) *s))
		  ) {
		slash = (*s == '\\') ? 1 : 0;
		row_chars++;
		s++;
	}

	if (*s == '\\' && slash) {
		row_chars--;
		slashslash=TRUE;
	}
	
	*row = (char *)malloc((row_chars+1)*sizeof(char));
	strncpy(*row, table, row_chars);
	(*row)[row_chars]='\0';

	diagnostics(4,"row =<%s>",*row);
	if (!slashslash) return;
	
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

static void
TabbingWriteRow(TabularT tabular, char *this_row, char *next_row, int height, int first_row)
{
	if (this_row==NULL || strlen(this_row)==0) return;
	
	diagnostics(4, "TabbingWriteRow height=%d twpi, row <%s>",height, this_row); 
	if (strstr(this_row,"\\kill")) return;
	ConvertString(this_row);
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
	bool			tabbing;
	TabularT        tabular;

	if (!(code & ON)) {
		diagnostics(4, "Exiting CmdTabular");
		g_processing_tabular = FALSE;
		g_processing_tabbing = FALSE;
		if (colFmt) free(colFmt);
		colFmt=NULL;
		return;
	}

	width = NULL;
	pos=NULL;
	cols=NULL;
	tabbing=FALSE;
	true_code = code & ~(ON);
	switch (true_code) {
		case TABBING: 
			end = strdup("\\end{tabbing}");
			tabbing = TRUE;
			break;
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

	if (tabbing){
		table = getTexUntil(end,FALSE);
		diagnostics(3, "Entering CmdTabular() with tabbing"); 
	} else {
		pos = getBracketParam();
		cols = getBraceParam();		
		table = getTexUntil(end,FALSE);
		diagnostics(3, "Entering CmdTabular() options [%s], format {%s}",pos, cols); 
		tabular=TabularPreamble(table,width,pos,cols);
	}

	diagnostics(2, "table_table_table_table_table\n%stable_table_table_table_table",table);
	
	row_start=table;
	TabularGetRow(row_start,&this_row,&next_row_start,&this_height,tabbing);
	first_row = TRUE;
	while (this_row) {
		row_start=next_row_start;
		TabularGetRow(row_start,&next_row,&next_row_start,&next_height,tabbing);
		if (tabbing) {
			diagnostics(1,"this row=<%s>",this_row);
			diagnostics(1,"next row=<%s>",next_row);
			TabbingWriteRow(tabular, this_row, next_row, this_height, first_row);
		}
		else
			TabularWriteRow(tabular, this_row, next_row, this_height, first_row);
		free(this_row);
		this_row = next_row;
		this_height = next_height;
		first_row = FALSE;
	}
	
	ConvertString(end);

	if (pos  ) free(pos);
	if (width) free(width);
	if (cols ) free(cols);
	free(table);
	free(end);
	if (!tabbing) {
		free(tabular.cline);
		free(tabular.align);
		free(tabular.vert);
		free(tabular.width);
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
