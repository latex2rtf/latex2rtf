
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
#include "fonts.h"
#include "commands.h"
#include "funct1.h"
#include "tables.h"
#include "stack.h"
#include "cfg.h"
#include "parser.h"
#include "counters.h"
#include "utils.h"
#include "lengths.h"
#include "preamble.h"
#include "graphics.h"
#include "vertical.h"

int g_tabbing_left_position = 0;
int g_tabbing_current_position = 0;
int *g_tabbing_locations = NULL;
int tabcounter = 0;
bool tabbing_return = FALSE;
bool tabbing_on_itself = FALSE;
long pos_begin_kill;
bool g_processing_tabbing = FALSE;
bool g_processing_tabular = FALSE;
bool g_processing_table = FALSE;

typedef struct TabularT {
    int n;                /* number of columns */
    int i;                /* current column */
    char *align;          /* align[1] is the alignment of the first column */
    int *vert;            /* vert[0] is the number of |'s before first column 
                             vert[1] is the number of |'s after first column
                             vert[n] is the number of |'s after last column */
    int *width;           /* width[1] is the width of the first column */
    int *cline;           /* cline[1] is true when line should be draws across column 1 */
} TabularT;

void CmdTabjump(void)
{
    fprintRTF("\\tab ");
}

void CmdTabset(void)
{
}

static void BeginCellRtf(char align)

/******************************************************************************
 purpose:  emit RTF code to start each cell
 ******************************************************************************/
{
    fprintRTF("\\pard\\intbl\\q%c ", align);
}

static void EndCellRtf(void)

/******************************************************************************
 purpose:  emit RTF code to end each cell
 ******************************************************************************/
{
    fprintRTF("\\cell\n");
}

static char *ConvertFormatString(char *s)

/******************************************************************************
 purpose: convert latex formatting to something simpler
 ******************************************************************************/
{
    int iCol, width;
    char *simple, *t;
    static int warned_once = FALSE;

    simple = strdup(s);         /* largest possible */
    diagnostics(5, "Entering ConvertFormatString, input=<%s>", s);

    iCol = 0;
    while (*s) {

        switch (*s) {
            case 'c':
            case 'r':
            case 'l':
                simple[iCol] = *s;
                iCol++;
                break;
            case '{':          /* skip to balancing brace */
            	t=getStringBraceParam(&s);
            	free(t);
                break;
            case 'p':
                simple[iCol] = 'l';
            	t=getStringBraceParam(&s);
            	width = getStringDimension(t);
            	free(t);
                iCol++;
                break;
            case '*':
                diagnostics(WARNING, " '*{num}{cols}' not supported.");
                break;
            case '@':
                if (!warned_once) diagnostics(WARNING, " '@{text}' not supported.");
                warned_once = TRUE;
                break;
            default:
                break;
        }
        s++;
    }
    simple[iCol] = '\0';
    diagnostics(5, "Exiting ConvertFormatString, output=<%s>", simple);
    return simple;
}

static int TabularColumnPosition(TabularT tabular, int n)

/******************************************************************************
 purpose:  return position of nth column 
 ******************************************************************************/
{
    int colWidth = getLength("textwidth") / tabular.n;

    return colWidth * (n + 1);
}

static void TabularCountVert(char *s, int vert[])

/******************************************************************************
 purpose:  fill vert[] with number of '|' for each column
 ******************************************************************************/
{
    int braces, i;

    diagnostics(5, "Entering TabularCountVert, input=<%s>", s);

    i = 0;
    while (*s) {

        switch (*s) {
            case 'c':
            case 'r':
            case 'l':
            case 'p':
                i++;
                break;
            case '{':          /* skip to balancing brace */
                braces = 1;
                s++;
                while (*s && (braces > 1 || *s != '}')) {
                    if (*s == '{')
                        braces++;
                    if (*s == '}')
                        braces--;
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
    diagnostics(5, "Exiting TabularCountVert");
}

static TabularT TabularPreamble(char *text, char *width, char *pos, char *cols)

/******************************************************************************
 purpose:  calculate column widths 
 ******************************************************************************/
{
    TabularT tabular;
    int i, colCount;
    char *colFmt;

    colFmt = ConvertFormatString(cols);
    colCount = (int) strlen(colFmt);

    tabular.i = 0;
    tabular.n = colCount;
    tabular.align = (char *) calloc(sizeof(char) * (colCount + 1), sizeof(char));
    tabular.vert  = (int *)  calloc(sizeof(int)  * (colCount + 1), sizeof(int));
    tabular.width = (int *)  calloc(sizeof(int)  * (colCount + 1), sizeof(int));
    tabular.cline = (int *)  calloc(sizeof(int)  * (colCount + 1), sizeof(int));

    for (i = 1; i <= colCount; i++)
        tabular.align[i] = colFmt[i - 1];

    TabularCountVert(cols, tabular.vert);

    if (getTexMode() != MODE_HORIZONTAL) {
        CmdIndent(INDENT_NONE);
        startParagraph("tabular", FIRST_PARAGRAPH);
    }

    fprintRTF("\\par\n");
    
    return tabular;
}

static void TabularGetRow(char *table, char **row, char **next_row, int *height)

/******************************************************************************
 purpose:  scan and duplicate the next row from a tabular environment and any height changes
            e.g.   the & cell & is & here \\[2pt]
                   but & then & it & died \\
            will return "the & cell & is & here" and height should be 40 (twips)
 ******************************************************************************/
{
    char *s, *dimension, *dim_start;
    int slash = 0;
    size_t row_chars = 0;
    size_t dim_chars = 0;
    bool slashslash = FALSE;

    s = table;
    *row = NULL;
    *next_row = NULL;
    *height = 0;

    if (!s)
        return;

    while (!(*s == '\0') && !(*s == '\\' && slash)) {
        slash = (*s == '\\') ? 1 : 0;
        row_chars++;
        s++;
    }

    if (*s == '\\' && slash) {
        row_chars--;
        slashslash = TRUE;
    }

    *row = (char *) malloc((row_chars + 1) * sizeof(char));
    strncpy(*row, table, row_chars);
    (*row)[row_chars] = '\0';

    show_string(5, *row, "row");
    if (!slashslash)
        return;

/* move after \\ */
    s++;

/* skip blanks */
    while (*s != '\0' && (*s == ' ' || *s == '\n'))
        s++;

    if (*s == '\0')
        return;

    if (*s != '[') {
        *next_row = s;
        return;
    }

/* read line space dimension */
    s++;
    dim_start = s;
    while (*s != '\0' && *s != ']') {
        s++;
        dim_chars++;
    }
    if (*s == '\0')
        return;

/* figure out the row height */
    dimension = malloc((dim_chars + 2) * sizeof(char));
    strncpy(dimension, dim_start, dim_chars);
    dimension[dim_chars] = '\n';    /* make sure entire string is not parsed */
    dimension[dim_chars + 1] = '\0';

    *height = getStringDimension(dimension);

    diagnostics(5, "height =<%s>=%d twpi", dimension, height);
    free(dimension);

    /* skip blanks */
    s++;
    while (*s != '\0' && (*s == ' ' || *s == '\n'))
        s++;

    if (*s == '\0')
        return;
    *next_row = s;
}

static char *TabularNextAmpersand(char *t)

/******************************************************************************
 purpose:  find the next ampersand while avoiding \&
 ******************************************************************************/
{
    char *s;
    int escaped = 0;

    s = t;

    while (s && *s != '\0' && (*s != '&' || (*s == '&' && escaped))) {
        escaped = (*s == '\\') ? 1 : 0;
        s++;
    }
    return s;
}

static char *TabularNextCell(char *cell_start, char **cell_end)

/******************************************************************************
 purpose:  scan and duplicate contents of the next cell
 ******************************************************************************/
{
    char *end, *dup, *dup2;

    end = TabularNextAmpersand(cell_start);

    if (*end == '&') {
        dup = my_strndup(cell_start, (size_t) (end - cell_start));
        *cell_end = end + 1;
    } else {
        dup = strdup(cell_start);
        *cell_end = NULL;
    }

    dup2 = strdup_noendblanks(dup);
    free(dup);

    return dup2;
}

static void TabularMultiParameters(char *cell, int *col_span, char *align, int *lvert, int *rvert)

/******************************************************************************
 purpose:  extract information from \multicolumn's on a line
           return col_span=0 if none are present
 ******************************************************************************/
{
    char *p, *format, *mformat, *num;
    diagnostics(5, "TabularMultiParameters cell=\"%s\" ", cell);

    *col_span = 0;
    *align = '\0';
    *lvert = 0;
    *rvert = 0;

    p = strstr(cell, "\\multicolumn");
    if (p == NULL)
        return;

	p      += strlen("\\multicolumn");
	num     = getStringBraceParam(&p);
	format  = getStringBraceParam(&p);
    mformat = ConvertFormatString(format);

    /* count '|' to the left of the column */
    p = format;
    while (p && *p && *p != 'c' && *p != 'l' && *p != 'r') {
        if (*p == '|')
            (*lvert)++;
        p++;
    }
    if (p && *p)
        p++;

    /* count '|' to the right of the column */
    while (p && *p && *p != 'c' && *p != 'l' && *p != 'r') {
        if (*p == '|')
            (*rvert)++;
        p++;
    }

    *col_span = atoi(num);
    *align = mformat[0];
    free(num);
    free(mformat);
    free(format);
    diagnostics(5, "TabularMultiParameters n=%d, align=%c", *col_span, *align);
}

static int TabularHline(char *row)

/******************************************************************************
 purpose:  count the number of \hline's in a line
 ******************************************************************************/
{
    char *s, *t;

    if (row == NULL)
        return 0;

    s = strstr(row, "\\hline");
    if (s == NULL)
        return 0;

    t = strstr(s + 6, "\\hline");

    if (t == NULL)
        return 1;
    return 2;
}

char *TabularCline(char *row, int columns)

/******************************************************************************
 purpose:  mark columns needing underline by \cline{ 1 - 2 }
           limited to 1-9 columns
 ******************************************************************************/
{
    char *s, *x;
    int i, n, m;
    char *cline = NULL;

    cline = (char *) malloc((columns + 2) * sizeof(char));
    for (i = 0; i <= columns + 1; i++)
        cline[i] = '\0';

    if (row == NULL)
        return cline;

    /* hline commands override cline commands */
    n = TabularHline(row);

    for (i = 0; i <= columns; i++)
        cline[i] = n;

    if (n)
        return cline;

    s = row;
    x = row + strlen(row);
    while (s < x) {
        s = strstr(s, "\\cline{");

        if (s == NULL)
            return cline;

        s += 7;

        while (*s && (*s == ' ' || *s == '\n'))
            s++;                /* skip spaces after { */

        if (*s <= '0' || *s >= '9')
            return cline;
        n = atoi(s);            /* first column */

        while (*s && *s != '-')
            s++;                /* skip past '-' */
        s++;

        while (*s && (*s == ' ' || *s == '\n'))
            s++;                /* skip spaces after '-' */

        if (*s <= '0' || *s >= '9')
            return cline;
        m = atoi(s);            /* second column */

        for (i = n; i <= m; i++) {
            if (i <= columns && i >= 1)
                cline[i] = 1;
        }
        while (*s && *s != '}')
            s++;                /* skip to last '}' */
    }

    return cline;
}

static void TabularBeginRow(TabularT tabular, char *this_row, char *next_row, int first_row)

/******************************************************************************
 purpose:  emit RTF to start one row of a table
 
           the vertical bar formatting codes for the entire row must be present at
           the start of the row.  The same holds for horizontal lines.  Only the first
           line may have an hlines above it.  Drawing hlines below depends on the
           next line containing an hline (i.e., after \\ that ends the row)
           
 ******************************************************************************/
{
    int i, n, column, rvert, lvert;
    char align;
    char *cell_start, *cell_end, *cell, *row;
    int top, left, bottom, right;   /* cell borders */
    char *cline;

    fprintRTF("\\trowd");

    row = this_row;
    cell_start = this_row;
    column = 0;

    cline = TabularCline(next_row, tabular.n);


    while (cell_start) {        /* for each cell */

        top = 0;
        left = 0;
        right = tabular.vert[column + 1];
        bottom = cline[column + 1];
        if (first_row)
            top = TabularHline(this_row);
        if (column == 0)
            left = tabular.vert[0];

        cell = TabularNextCell(cell_start, &cell_end);
        TabularMultiParameters(cell, &n, &align, &lvert, &rvert);
        
        if (n > 1)
            fprintRTF("\\clmgf");

        if (left == 1)
            fprintRTF("\\clbrdrl\\brdrs");
        if (left == 2)
            fprintRTF("\\clbrdrl\\brdrdb");
        if (top == 1)
            fprintRTF("\\clbrdrt\\brdrs");
        if (top == 2)
            fprintRTF("\\clbrdrt\\brdrdb");
        if (bottom == 1)
            fprintRTF("\\clbrdrb\\brdrs");
        if (bottom == 2)
            fprintRTF("\\clbrdrb\\brdrdb");
        if (right == 1 || (n == 1 && rvert == 1))
            fprintRTF("\\clbrdrr\\brdrs");
        if (right == 2 || (n == 1 && rvert == 2))
            fprintRTF("\\clbrdrr\\brdrdb");
        fprintRTF("\\cellx%d", TabularColumnPosition(tabular, column));
        column++;

        for (i = 2; i <= n; i++) {
            fprintRTF("\\clmrg");
            if (top == 1)
                fprintRTF("\\clbrdrt\\brdrs");
            if (top == 2)
                fprintRTF("\\clbrdrt\\brdrdb");
            if (bottom == 1)
                fprintRTF("\\clbrdrb\\brdrs");
            if (bottom == 2)
                fprintRTF("\\clbrdrb\\brdrdb");
            if (i == n && rvert == 1)
                fprintRTF("\\clbrdrr\\brdrs");
            if (i == n && rvert == 2)
                fprintRTF("\\clbrdrr\\brdrdb");
            fprintRTF("\\cellx%d", TabularColumnPosition(tabular, column));
            column++;
        }

        free(cell);
        cell_start = cell_end;
    }
    fprintRTF("\n");
    free(cline);
}

void TabularEndRow(void)

/******************************************************************************
 purpose:  emit RTF to finish one row of a table
 ******************************************************************************/
{
    fprintRTF("\\row\n");
}

char TabularColumnAlignment(TabularT tabular, int column)

/******************************************************************************
 purpose:  alignment for a column
 ******************************************************************************/
{
    return tabular.align[column+1];
}

static void TabularWriteRow(TabularT tabular, char *this_row, char *next_row, int height, int first_row)
{
    char *cell, *cell_start, *cell_end;
    char align;
    int n, lvert, rvert;

    tabular.i = 0;
    if (this_row == NULL || strlen(this_row) == 0)
        return;

    diagnostics(5, "TabularWriteRow height=%d twpi, row <%s>", height, this_row);

    /* avoid writing anything for empty last row */
    if (next_row == NULL) {
    	/* do nothing if the row is empty */
        if (tabular.n == 1 && strlen(this_row) == 0)
            return;
            
        /* do nothing if there is more than one column, but no '&' */
        if (tabular.n > 1 && !strchr(this_row, '&')) {
        
        	/* except if the last line is multicolumn! */
            TabularMultiParameters(this_row, &n, &align, &lvert, &rvert);
            if (n==0) return;
		}
    }

    TabularBeginRow(tabular, this_row, next_row, first_row);
    cell_start = this_row;
    while (cell_start) {

        cell = TabularNextCell(cell_start, &cell_end);

        /* establish cell alignment */
        TabularMultiParameters(cell, &n, &align, &lvert, &rvert);
        if (n == 0) {
            align = TabularColumnAlignment(tabular, tabular.i);
            n = 1;
        }

        BeginCellRtf(align);
        if (cell != NULL) {
            diagnostics(5, "TabularWriteRow align=%c n=%d cell=<%s>", align, n, cell);
            fprintRTF("{");
            ConvertString(cell);
            fprintRTF("}");
        }
        EndCellRtf();

        tabular.i += n;
        cell_start = cell_end;
        if (cell != NULL)
            free(cell);
    }

    if (tabular.i < tabular.n) {
        align = TabularColumnAlignment(tabular, tabular.i);
        BeginCellRtf(align);
        EndCellRtf();
    }

    TabularEndRow();
}

int TabularMeasureCell(char *cell)

/******************************************************************************
 purpose:  come up with a rough number for the number of characters in a cell
           this is pretty ridiculous, but just counting characters is better
           than nothing.
 ******************************************************************************/
{
    char *s;
    int len;

    if (cell == NULL || *cell == '\0')
        return 0;

    s = strstr(cell, "multicolumn");

    if (s) {
        char *num, *format, *text, *xtext;

        PushSource(NULL, s + strlen("multicolumn"));
        num = getBraceParam();
        format = getBraceParam();
        text = getBraceParam();
        PopSource();
        xtext = strdup_noendblanks(text);

        len = strlen(xtext);
        free(num);
        free(format);
        free(text);
        free(xtext);
        return len;

    }

    return strlen(cell);
}

int TabularMeasureCellx(char *cell)

/******************************************************************************
 purpose:  come up with a rough number for the number of characters in a cell
           this is pretty ridiculous, but just counting characters is better
           than nothing.
 ******************************************************************************/
{
    char *t;
    char *s;

    if (cell == NULL || *cell == '\0')
        return 0;

    s = strstr(cell, "multicolumn");

    if (s) {
        int i;

        /* find beginning of text in \multicolumn{1}{c}{ text } */
        for (i = 0; i < 3; i++) {   /* skip over multicolumn{1}{c}{ text } */
            while (*s && *s != '{')
                s++;
            if (*s == '\0')
                return 0;
            s++;
        }
        while (*s && *s == ' ')
            s++;                /* skip over spaces */

        /* find last non-space character in text */
        t = strrchr(s, '}');
        if (!t)
            return 0;
        t--;
        while (t >= s && *t == ' ')
            t--;                /* skip over spaces */

        return t - s + 1;
    }

    return strlen(cell);
}

static void TabularMeasureRow(TabularT tabular, char *this_row, char *next_row, int height, int first_row)

/******************************************************************************
 purpose:  come up with relative widths for all cells in a row
  ******************************************************************************/
{
    char *cell, *cell_start, *cell_end;
    char align;
    int n, lvert, rvert;

    tabular.i = 0;
    if (this_row == NULL || strlen(this_row) == 0)
        return;

    diagnostics(5, "TabularMeasureRow height=%d twpi, row <%s>", height, this_row);

    cell_start = this_row;
    while (cell_start) {

        cell = TabularNextCell(cell_start, &cell_end);

        TabularMultiParameters(cell, &n, &align, &lvert, &rvert);
        if (n == 0) {
            n = 1;
        }

        if (cell != NULL) {
            int len;

            len = TabularMeasureCell(cell);
            diagnostics(6, "col=%d n=%d len=%d cell=<%s>", tabular.i, n, len, cell);
        }

        tabular.i += n;
        cell_start = cell_end;
        if (cell != NULL)
            free(cell);
    }
}

void CmdTabular(int code)

/******************************************************************************
 purpose:   \begin{tabular}[pos]{cols}          ... \end{tabular}
            \begin{tabular*}{width}[pos]{cols}  ... \end{tabular*}
            \begin{array}[pos]{cols}            ... \end{array}
 ******************************************************************************/
{
    int true_code, this_height, next_height, first_row, begins, ends;
    char *end=NULL, *begin=NULL, *this_row, *next_row, *next_row_start, *row_start;
    char *table = NULL;
    char *cols = NULL;
    char *pos = NULL;
    char *width = NULL;
    
    TabularT tabular;

    if (!(code & ON)) {
        diagnostics(4, "Exiting CmdTabular");
        g_processing_tabular = FALSE;
        return;
    }

    g_processing_tabular = TRUE;
    width = NULL;
    pos = NULL;
    cols = NULL;
    true_code = code & ~(ON);
    switch (true_code) {
        case TABULAR:
            end = strdup("\\end{tabular}");
            begin = strdup("\\begin{tabular}");
            break;
        case TABULAR_STAR:
            end = strdup("\\end{tabular*}");
            begin = strdup("\\begin{tabular*}");
            width = getBraceParam();
            break;
        case TABULAR_LONG:
            end = strdup("\\end{longtable}");
            begin = strdup("\\begin{longtable}");
            break;
        case TABULAR_LONG_STAR:
            end = strdup("\\end{longtable*}");
            begin = strdup("\\begin{longtable*}");
            width = getBraceParam();
            break;
    }

    pos = getBracketParam();
    cols = getBraceParam();
    table = getTexUntil(end, FALSE);

	if (g_tabular_display_bitmap) {
		char pre[151];
		
		snprintf(pre,150, "%s{%s}",begin,cols);
		PrepareDisplayedBitmap("tabular");
		WriteLatexAsBitmap(pre, table, end);
		FinishDisplayedBitmap();
	}
	
	if (g_tabular_display_rtf)
	{
	/* need to make sure that we don't have nested environments */
		begins = strstr_count(table, begin);
		ends = strstr_count(table, end);
	
		while (begins > ends) {
			char *table2, *table3, *table4;
	
			if (begins > ends) {
				table2 = getTexUntil(end, FALSE);
				table3 = strdup_together(table, end);
				table4 = strdup_together(table3, table2);
				free(table);
				free(table2);
				free(table3);
				table = table4;
			}
			begins = strstr_count(table, begin);
			ends = strstr_count(table, end);
		}
	
		if (begins > 0) {
			char *p;
			int num = TexFontNumber("Typewriter");
	
			diagnostics(WARNING, "Nested tabular/tabbing environments not allowed");
			diagnostics(5, "table_table_table_table_table\n%stable_table_table_table_table", table);
			fprintRTF("\\pard\\ql\\b0\\i0\\scaps0\\f%d ", num);
			p = begin;
			while (*p)
				fprintRTF("%c",*p++);
			p = table;
			while (*p)
				fprintRTF("%c",*p++);
			p = end;
			while (*p)
				fprintRTF("%c",*p++);
	
		} else {
	
			diagnostics(4, "Entering CmdTabular() options [%s], format {%s}", (pos) ? pos : "", cols);
			tabular = TabularPreamble(table, width, pos, cols);
			diagnostics(5, "table_table_table_table_table\n%stable_table_table_table_table", table);
	
			row_start = table;
			TabularGetRow(row_start, &this_row, &next_row_start, &this_height);
	
			/* scan entire table to get estimates for column widths */
			first_row = TRUE;
			while (this_row) {
				row_start = next_row_start;
				TabularGetRow(row_start, &next_row, &next_row_start, &next_height);
				TabularMeasureRow(tabular, this_row, next_row, this_height, first_row);
				free(this_row);
				this_row = next_row;
				this_height = next_height;
				first_row = FALSE;
			}
	
			row_start = table;
			TabularGetRow(row_start, &this_row, &next_row_start, &this_height);
			first_row = TRUE;
			while (this_row) {
				row_start = next_row_start;
				TabularGetRow(row_start, &next_row, &next_row_start, &next_height);
				TabularWriteRow(tabular, this_row, next_row, this_height, first_row);
				free(this_row);
				this_row = next_row;
				this_height = next_height;
				first_row = FALSE;
			}
	
			free(tabular.cline);
			free(tabular.align);
			free(tabular.vert);
			free(tabular.width);
			if (cols)
				free(cols);
			if (pos)
				free(pos);
			if (width)
				free(width);
		}
	}

	ConvertString(end);
	
	
    free(table);
    free(end);
    free(begin);
}

static int TabbingColumnPosition(int n, int total)

/******************************************************************************
 purpose:  return position of nth column 
 ******************************************************************************/
{
    int colWidth = getLength("textwidth") / total;

    return colWidth * (n + 1);
}

/******************************************************************************
 purpose:  simple minded way to skip \verb#contents$# in cell
 ******************************************************************************/
static char *skip_verb(char *s)
{
    char endchar;
    diagnostics(6,"before verb <<%s>>",s);
    if (s && strncmp(s,"verb",4)==0){
        s+=4;
        endchar = *s;
        s++;
        while (*s && *s!= endchar) s++;
        if (*s) s++;
    }
    diagnostics(6,"after  verb <<%s>>",s);
    return s;
}

static void TabbingNextCellEnd(char *t, char **cell_end, char **next_cell)

/******************************************************************************
 purpose:  find the end of this tabbing cell
 ******************************************************************************/
{
    char *s;

    s = t;

    while (s) {

        if (*s == '\0') {
            *cell_end = s;
            *next_cell = s;
            return;
        }

        if (*s == '\\') {
            s++;
            s = skip_verb(s);
            if (*s == '=' || *s == '>' || *s == '<' || *s == '\'' || *s == '`') {
                *cell_end = s - 1;
                *next_cell = s + 1;
                return;
            }
        }
        
        if (*s == '\0') {
            *cell_end = s;
            *next_cell = s;
            return;
        }
        s++;
    }
}

static char *TabbingNextCell(char *cell_start, char **cell_end)

/******************************************************************************
 purpose:  scan and duplicate contents of the next cell
 ******************************************************************************/
{
    char *end, *dup, *dup2, *next_cell, *p;

    TabbingNextCellEnd(cell_start, &end, &next_cell);

    if (end < cell_start)
        end = cell_start;
    dup = my_strndup(cell_start, (size_t) (end - cell_start));

    if (*next_cell == '\0')
        *cell_end = NULL;
    else
        *cell_end = next_cell;

    /* remove \- and \+ from string */
    while (((p = strstr(dup, "\\+")) != NULL) || ((p = strstr(dup, "\\-")) != NULL)) {
        *p = ' ';
        p++;
        *p = ' ';
    }

    dup2 = strdup_noendblanks(dup);
    free(dup);
    diagnostics(5,"next cell = [[%s]]", dup);
    return dup2;
}

static void TabbingBeginRow(int n, int n_total, char *align)

/******************************************************************************
 purpose:  emit RTF to start one row of a tabbing environment           
 ******************************************************************************/
{
    int i;

    if (n == 0)
        return;

    fprintRTF("\\trowd");

    for (i = 0; i < n; i++)
        fprintRTF("\\cellx%d", TabbingColumnPosition(i, n_total));

    fprintRTF("\n");
    for (i = 0; i < g_tabbing_left_position; i++) {
        BeginCellRtf(align[i]);
        EndCellRtf();
    }

}

static void TabbingWriteRow(char *this_row, int n, int n_total, char *align)
{
    char *start, *end, *cell;
    int i;

    diagnostics(5, "TabbingWriteRow n=%d <%s> [%s]", n, align, this_row);

    if (this_row == NULL || n == 0)
        return;

    if (strstr(this_row, "\\kill"))
        return;

    TabbingBeginRow(n, n_total, align);

    start = this_row;
    end = this_row + strlen(this_row);

    for (i = g_tabbing_left_position; i < n; i++) {
        BeginCellRtf(align[i]);
        cell = TabbingNextCell(start, &end);
        if (cell) {
            diagnostics(5, "cell=<%s>", cell);
            ConvertString(cell);
            free(cell);
        }
        EndCellRtf();
        start = end;
    }
    fprintRTF("\\row\n");
}

static void TabbingGetRow(char *table, char **row, char **next_row)

/******************************************************************************
 purpose:  scan and duplicate the next row from the tabbing environment
  ******************************************************************************/
{
    char *s, *arow;
    size_t row_chars = 0;
    bool slash;

    s = table;
    *row = NULL;
    *next_row = NULL;

    if (!s)
        return;

    /* the end of a row is caused by one of three things 
            1) the buffer ends 
            2) the line ends \\ 
            3) \kill is encountered 
     */
     
    slash = FALSE;
    while (!(*s == '\0') &&
           !(*s == '\\' && slash) && 
           !((row_chars > 6) && strncmp(s - 5, "\\kill", 5) == 0 && !isalpha((int) *s))
          ) {
        row_chars++;
        slash = (*s == '\\') ? 1 : 0;
        s++;
    }

    if (*s == '\\' && slash) {  /* line ends with \\ */
        row_chars--;
        
        if (*(s+1)=='[') {  /* skip optional height parameter */
        	s++;
        	while (*s != '\0' && *s != ']') s++;
        }
    }

    if (*s != '\0')
        *next_row = s + 1;

    arow = (char *) malloc((row_chars + 1) * sizeof(char));
    strncpy(arow, table, row_chars);
    arow[row_chars] = '\0';

    diagnostics(5, "TabbingGetRow obtained=<%s> remaining[%s]", arow, *next_row);

    *row = strdup_noendblanks(arow);
    free(arow);
}

static void TabbingGetColumnAlignments(char *row, char *align, int *n, int *next_left)

/******************************************************************************
 purpose:   scan one row of tabbing environment to obtain column alignments
 ******************************************************************************/
{
    int i;

    *next_left = g_tabbing_left_position;
    *n = *next_left + 1;

    for (i = 0; i < *n; i++)
        align[i] = 'l';

    while (row && *row) {

        if (*row != '\\') {
            row++;
            continue;
        }

        row++;
        row = skip_verb(row);
        
        switch (*row) {

            case '=':
                align[*n] = 'l';
                (*n)++;
                break;

            case '>':
                align[*n] = 'l';
                (*n)++;
                break;

            case '<':
                break;

            case '\'':
                align[*n - 2] = 'r';
                align[*n - 1] = 'l';
                break;

            case '`':
                align[*n] = 'r';
                (*n)++;
                break;

            case '+':
                (*next_left)++;
                break;

            case '-':
                (*next_left)--;
                break;

            default:
                break;
        }
    }
    align[*n] = '\0';

}


void CmdTabbing(int code)

/******************************************************************************
 purpose:   \begin{tabbing} ... \end{tabbing}
 ******************************************************************************/
{
    int n, n_total, next_left;
    char *end, *this_row, *next_row_start, *row_start;
    char *table = NULL;
    char align[31];

    if (!(code & ON)) {
        diagnostics(4, "Exiting CmdTabbing");
        g_processing_tabbing = FALSE;
        return;
    }

    g_processing_tabbing = TRUE;
    n = 0;
    g_tabbing_left_position = 0;
    strcpy(align, "l");

    end = strdup("\\end{tabbing}");
    table = getTexUntil(end, FALSE);
    diagnostics(4, "Entering CmdTabbing()");

	if (g_tabular_display_bitmap) {		
		PrepareDisplayedBitmap("tabbing");
		WriteLatexAsBitmap("\\begin{tabbing}", table, end);
		FinishDisplayedBitmap();
	}
	
	if (g_tabular_display_rtf) {
		row_start = table;
		TabbingGetRow(row_start, &this_row, &next_row_start);
	
		diagnostics(5, "tabbing_tabbing_tabbing\n%s\ntabbing_tabbing_tabbing", table);
	
		if (getTexMode() != MODE_HORIZONTAL) {
			CmdIndent(INDENT_NONE);
			startParagraph("tabbing", FIRST_PARAGRAPH);
		}
	
		fprintRTF("\\par\n");
	
		n_total = 0;
		while (this_row && strlen(this_row) >= 0) {
			show_string(5, this_row, "row");
	
			TabbingGetColumnAlignments(this_row, align, &n, &next_left);
			if (n > n_total)
				n_total = n;
	
			diagnostics(4, "this row n=%d <%s> left_tab=%d", n, align, g_tabbing_left_position);
	
			TabbingWriteRow(this_row, n, n_total, align);
	
			g_tabbing_left_position = next_left;
			row_start = next_row_start;
	
			free(this_row);
			TabbingGetRow(row_start, &this_row, &next_row_start);
		}
	}
    ConvertString(end);
    

    free(table);
    free(end);
}

void CmdTable(int code)

/******************************************************************************
 purpose: handles the \begin{table} ... \end{table} codes.  The primary use is
          to properly handle the spacing before and after the table.  It is 
          slightly more complicated because the endfloat package suppresses 
          inserting a table in the text, but does it later when the file.ttt 
          is read.  g_endfloat_tables is set to true during this phase --- 
          otherwise it is always false (most notably when the endfloat package
          is not being used.)
 ******************************************************************************/
{
    char *location, *table_contents;
    static char     oldalignment;
    int true_code = code & ~ON;
		
    if (code & ON) {
        location = getBracketParam();
        if (location) free(location);

        CmdEndParagraph(0);
        oldalignment = getAlignment();
        setAlignment(JUSTIFIED);

        CmdVspace(VSPACE_BIG_SKIP);
        CmdIndent(INDENT_NONE);

        g_processing_table = TRUE;
        
        if (true_code == TABLE)
        	table_contents = getTexUntil("\\end{table}", TRUE);
        else
        	table_contents = getTexUntil("\\end{table*}", TRUE);
        	
        g_table_label = ExtractLabelTag(table_contents);
        if (g_endfloat_tables) {
            if (g_endfloat_markers) {
                setAlignment(CENTERED);
                startParagraph("endfloat", GENERIC_PARAGRAPH);
                incrementCounter("endfloattable");  /* two separate counters */
                fprintRTF("[");                     /* one for tables and one for */
                ConvertBabelName("TABLENAME");      /* endfloat tables */
                fprintRTF(" ");
                if (g_document_type != FORMAT_ARTICLE &&
                    g_document_type != FORMAT_APA)
                    fprintRTF("%d.", getCounter("chapter"));
                fprintRTF("%d about here]", getCounter("endfloattable"));
            }
        } else {
            startParagraph("table", GENERIC_PARAGRAPH);
            ConvertString(table_contents);
        }
        free(table_contents);
        
        if (true_code == TABLE)
        	ConvertString("\\end{table}");
        else
        	ConvertString("\\end{table*}");
    } else {
        g_processing_table = FALSE;
        if (getTexMode() != MODE_VERTICAL)
            CmdEndParagraph(0);
        if (g_table_label)
            free(g_table_label);
        setAlignment(oldalignment);
        CmdEndParagraph(0);
        CmdVspace(VSPACE_BIG_SKIP);
    }
}

void CmdMultiCol(int code)

/******************************************************************************
 purpose: handles \multicolumn{n}{format}{content}
 ******************************************************************************/
{
    long numCol, i;
    char *num, *format, *content, *bcontent;

    num = getBraceParam();
    format = getBraceParam();
    bcontent = getBraceParam();

    content = strdup_noendblanks(bcontent);
    free(bcontent);

    diagnostics(4, "CmdMultiCol cols=%s format=<%s> content=<%s>", num, format, content);
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

void CmdHline(int code)
{

/* cline and hline are already handled by tabular code 
   here we just properly skip the commands             */
    char *s;

    if (code == 1) {
        s = getBraceParam();
        free(s);
        skipSpaces();
    }
}
