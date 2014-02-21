/* vertical.c - routines that handle vertical space issues

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

This file is available from http://sourceforge.net/projects/latex2rtf/

**************************************************************************

The tricky part is that latex2rtf is a one-pass converter.  The right thing
must be done with each character.  Consider the sequence 

           \vspace{1cm}\noindent New paragraph.

When latex2rtf reaches the 'N', then a new paragraph should be started.  We
know it is a new paragraph because \vspace should have put the converter 
into a MODE_VERTICAL and characters are only emitted in MODE_HORIZONTAL.  

RTF needs to set up the entire paragraph at one time and the 

    * text alignment
    * line spacing
    * vertical space above
    * left margin
    * right margin
    * paragraph indentation 
    
must all be emitted at this time.  This file contains routines
that affect these quantities
******************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include "main.h"
#include "funct1.h"
#include "cfg.h"
#include "utils.h"
#include "parser.h"
#include "lengths.h"
#include "vertical.h"
#include "convert.h"
#include "commands.h"
#include "styles.h"
#include "fonts.h"
#include "stack.h"
#include "xrefs.h"
#include "counters.h"
#include "fields.h"
#include "acronyms.h"

static int g_TeX_mode = MODE_VERTICAL;
static int g_line_spacing = 240;
static int g_paragraph_no_indent = FALSE;
static int g_paragraph_inhibit_indent = FALSE;
static int g_vertical_space_to_add = 0;
static int g_right_margin_indent;
static int g_left_margin_indent;
static int g_page_new = FALSE;
static int g_column_new = FALSE;
static int g_alignment = JUSTIFIED;
static int g_par_brace = 0;

char TexModeName[7][25] = { "bad", "internal vertical", "horizontal",
    "restricted horizontal", "math", "displaymath", "vertical"
};

char ParOptionName[8][13] = { "bad", "FIRST", "GENERIC", "SECTION", 
                              "EQUATION", "SLASHSLASH", "LIST", "ENVIRONMENT"};

/******************************************************************************
     left and right margin accessor functions
 ******************************************************************************/
void setLeftMarginIndent(int indent)
{
    g_left_margin_indent = indent;
}

void setRightMarginIndent(int indent)
{
    g_right_margin_indent = indent;
}

int getLeftMarginIndent(void)
{
    return g_left_margin_indent;
}

int getRightMarginIndent(void)
{
    return g_right_margin_indent;
}

/******************************************************************************
     paragraph alignment accessor functions
 ******************************************************************************/
void setAlignment(int align)
{
    g_alignment = align;
}

int getAlignment(void)
{
    return g_alignment;
}

/******************************************************************************
     vertical space between paragraph accessor functions
 ******************************************************************************/
void setVspace(int space)
{
    g_vertical_space_to_add = space;
}

int getVspace(void)
{
    return g_vertical_space_to_add;
}

/******************************************************************************
     line spacing accessor functions
 ******************************************************************************/
static void setLineSpacing(int spacing)
{
    g_line_spacing = spacing;
}

static int getLineSpacing(void)
{
    return g_line_spacing;
}


/******************************************************************************
TeX has six modes:
    
    MODE_VERTICAL              Building the main vertical list, from which the 
                               pages of output are derived
                  
    MODE_INTERNAL_VERTICAL     Building a vertical list from a vbox
    
    MODE_HORIZONTAL            Building a horizontal list for a paragraph
    
    MODE_RESTICTED_HORIZONTAL  Building a horizontal list for an hbox
    
    MODE_MATH                  Building a mathematical formula to be placed in a 
                               horizontal list
                               
    MODE_DISPLAYMATH           Building a mathematical formula to be placed on a
                               line by itself, temporarily interrupting the current paragraph
                               
LaTeX has three modes: paragraph mode, math mode, or left-to-right mode.
This is not a particularly useful, since paragraph mode is a combination of
vertical and horizontal modes. 
                         
Why bother keeping track of modes?  Mostly so that paragraph indentation gets handled
correctly, as well as vertical and horizontal space.  The mode is also the primary
way that the end of a paragraph is signalled.
 ******************************************************************************/
void setTexMode(int mode)
{
    diagnostics(6, "TeX mode setting from [%s] -> [%s]", TexModeName[g_TeX_mode], TexModeName[mode]);
    g_TeX_mode = mode;
}

int getTexMode(void)
{
    return g_TeX_mode;
}

void changeTexMode(int mode)
{
    diagnostics(6, "TeX mode changing from [%s] -> [%s]", TexModeName[g_TeX_mode], TexModeName[mode]);

    if (g_TeX_mode == MODE_VERTICAL && mode == MODE_HORIZONTAL)
        startParagraph("Normal", PARAGRAPH_GENERIC);

    if (g_TeX_mode == MODE_HORIZONTAL && mode == MODE_VERTICAL)
        CmdEndParagraph(0);

    g_TeX_mode = mode;
}

/******************************************************************************
    RTF codes to create a new paragraph.  If the paragraph should
    not be indented then emit \fi0 otherwise use the current value
    of \parindent as the indentation of the first line.
    
    style describes the type of paragraph ... 
      "Normal"
      "caption"
      "author"
      "bibitem"
      "section"
      etc.
      
    indenting describes how this paragraph and (perhaps) the following
    paragraph should be indented
    
      PARAGRAPH_SECTION_TITLE  (do not indent this paragraph or the next)
      PARAGRAPH_FIRST  (do not indent this paragraph but indent the next)
      PARAGRAPH_GENERIC        (indent as needed)
      PARAGRAPH_LIST           (the first paragraph of a list item)
      PARAGRAPH_SLASHSLASH     
      PARAGRAPH_ENVIRONMENT    (include topsep before and after final --- not impl.)   
    
    Sometimes it is necessary to influence the next paragraph will
    be before it has been parsed.  For example, a section command
    should create a paragraph for the section title and then the
    next paragraph encountered should be handled like as a first 
    paragraph.  
    
    The problem arises because "\n\n" means different things in different
    contexts.  After \section{aaa} "\n\n" does not indicate that the next
    paragraph should be indented.  However after \end{itemize} "\n\n"
    means that the next paragraph should be indented.  Now CmdEndParagraph()
    will set g_paragraph_inhibit_indent to FALSE so that the common case
    of starting new paragraphs is handled appropriately.
    
    For PARAGRAPH_FIRST, then it is the first paragraph in a section.
    Usually the first paragraph is not indented.  However, when the
    document is being typeset in french it should have normal indentation.
    Another special case occurs when the paragraph being typeset is
    in a list environment.  In this case, we need to indent according
    to the current parindent to obtain the proper hanging indentation
    
    The default is to indent according to
    the current parindent.  However, if the g_paragraph_inhibit_indent
    flag or the g_paragraph_no_indent flag is TRUE, then do not indent
    the next line.  Typically these flags are set just after a figure
    or equation or table.
 ******************************************************************************/
void startParagraph(const char *style, int indenting)
{
    int width, a, b, c;
    int parindent,parskip;
    static char last_style[50] = "Normal";
    static char the_style[50] = "Normal";
    static int last_indent = 0;
    static int next_paragraph_after_section = TRUE;
    
    int orig_font_family = CurrentFontFamily();
    int orig_font_size = CurrentFontSize();
    int orig_font_series = CurrentFontSeries();
    int orig_font_shape = CurrentFontShape();
    
    /* special style "last" will just repeat previous */
    if (strcmp(style,"last")==0) {
        diagnostics(4,"using last style = '%s'",last_style);
        if (indenting != PARAGRAPH_SLASHSLASH)
        	indenting = last_indent;
        strcpy(the_style,last_style);
    } else {
        diagnostics(4,"using style = '%s'",style);
        last_indent = indenting;
        strcpy(last_style,style);
        strcpy(the_style,style);
    }
        
    parindent = getLength("parindent");
    parskip   = getLength("parskip");

    if (g_par_brace !=0 )
        diagnostics(5,"******************* starting %s paragraph with braces = %d", style, g_par_brace);        
    
    if (g_par_brace == 1)
        CmdEndParagraph(0);

    width = getLength("textwidth");
    a = (int) (0.45 * width);
    b = (int) (0.50 * width);
    c = (int) (0.55 * width);

    switch(indenting) {
    
        case PARAGRAPH_SECTION_TITLE:         /* titles are never indented */
            diagnostics(5, "PARAGRAPH_SECTION_TITLE");
            parindent = 0;
            break;
    
        case PARAGRAPH_FIRST:                 /* French indents first paragraph */
            diagnostics(5, "PARAGRAPH_FIRST");
            if (!FrenchMode)
                parindent = 0;
            break;
            
        case PARAGRAPH_EQUATION:              /* typically centered with no indent */
            diagnostics(5, "PARAGRAPH_EQUATION");
            parindent = 0;
            break;
            
        case PARAGRAPH_SLASHSLASH:            /* \\ is a line break, don't indent */
            diagnostics(5, "PARAGRAPH_SLASHSLASH");
            parindent = 0;
            break;

        case PARAGRAPH_LIST:                  /* first paragraph in a list, don't monkey */
            diagnostics(5, "PARAGRAPH_LIST"); /* with indenting */
            break;

        default:                              /* Worry about not indenting */
            diagnostics(5, "PARAGRAPH_GENERIC");
            if (next_paragraph_after_section || g_paragraph_no_indent || 
                g_paragraph_inhibit_indent   || g_processing_list_environment)
                parindent = 0;
            break;
    }
    
    if (g_processing_preamble) {
        diagnostics(5,"Encountered StartParagraph() in preamble");    
        return;
    }
    
    if (g_par_brace != 0)
        diagnostics(5,"starting paragraph with braces = %d", g_par_brace);      
    g_par_brace++;
    
    diagnostics(5, "Paragraph mode    %s", TexModeName[getTexMode()]);
    diagnostics(5, "Paragraph option  %s", ParOptionName[indenting]);
    diagnostics(5, "Noindent is       %s", (g_paragraph_no_indent) ? "TRUE" : "FALSE");
    diagnostics(5, "Inhibit is        %s", (g_paragraph_inhibit_indent) ? "TRUE" : "FALSE");
    diagnostics(5, "left indent is    %d", g_left_margin_indent);
    diagnostics(5, "right indent is   %d", g_right_margin_indent);
    diagnostics(5, "current parindent %d", getLength("parindent"));
    diagnostics(5, "this parindent    %d", parindent);
    diagnostics(5, "current style is    %s", the_style);
    diagnostics(6, "current family      %d", CurrentFontFamily());
    diagnostics(6, "current font size   %d", CurrentFontSize());
    diagnostics(6, "current font series %d", CurrentFontSeries());
    diagnostics(6, "current font shape  %d", CurrentFontShape());

    if (g_page_new) {
        fprintRTF("\\page\n");   /* causes new page */
        g_page_new = FALSE;
        g_column_new = FALSE;
    }

    if (g_column_new) {
        fprintRTF("\\column\n"); /* causes new column */
        g_column_new = FALSE;
    }

    fprintRTF("\\pard\\plain");
    InsertStyle(the_style);
    if (strcmp(the_style,"equation")==0)
        fprintRTF("\\tqc\\tx%d", b);
    if (strcmp(the_style,"equationNum")==0)
        fprintRTF("\\tqc\\tx%d\\tqr\\tx%d", b, width);
    if (strcmp(the_style,"equationAlign")==0)
        fprintRTF("\\tqr\\tx%d\\tql\\tx%d", a, b);
    if (strcmp(the_style,"equationAlignNum")==0)
        fprintRTF("\\tqr\\tx%d\\tql\\tx%d\\tqr\\tx%d", a, b, width);
    if (strcmp(the_style,"equationArray")==0)
        fprintRTF("\\tqr\\tx%d\\tqc\\tx%d\\tql\\tx%d", a, b, c);
    if (strcmp(the_style,"equationArrayNum")==0)
        fprintRTF("\\tqr\\tx%d\\tqc\\tx%d\\tql\\tx%d\\tqr\\tx%d", a, b, c, width);

    if (strcmp(the_style,"bitmapCenter")==0)
        fprintRTF("\\tqc\\tx%d\\tqr\\tx%d", b, width);

    /* TODO change width/6 with hint */
    if (strcmp(the_style,"acronym")==0)
        fprintRTF("\\tx%d\\tqr\\tldot\\tx%d", acronymHint(width), width);
        
    fprintRTF("\\sl%i\\slmult1 ", getLineSpacing());

    if (getVspace() > 0)
        fprintRTF("\\sb%d ", getVspace());
    setVspace(parskip);

    if (g_left_margin_indent != 0)
        fprintRTF("\\li%d", g_left_margin_indent);

    if (g_right_margin_indent != 0)
        fprintRTF("\\ri%d", g_right_margin_indent);

    fprintRTF("\\fi%d ", parindent);
    
    /* these are strstr because might end in 0 */
    if (strstr("part",the_style)    == NULL && 
        strstr("title",the_style)   == NULL &&
        strstr("chapter",the_style) == NULL &&
        strstr("section",the_style) == NULL ) {
        
        if (CurrentFontFamily() != orig_font_family)
            fprintRTF("\\f%d ", orig_font_family);
    
        if (CurrentFontSize() != orig_font_size)
            fprintRTF("\\fs%d ", orig_font_size);

        if (CurrentFontSeries() != orig_font_series) 
            CmdFontSeries(orig_font_series);
            
        if (CurrentFontShape() != orig_font_shape)
            CmdFontShape(orig_font_shape);      
    }
    
    setTexMode(MODE_HORIZONTAL); 

    if (!g_processing_list_environment) {
        g_paragraph_no_indent = FALSE;
        if (indenting == PARAGRAPH_SECTION_TITLE)
            g_paragraph_inhibit_indent = TRUE;
        else
            g_paragraph_inhibit_indent = FALSE;
    }
     
    if (indenting == PARAGRAPH_SECTION_TITLE && !FrenchMode)
    	next_paragraph_after_section = TRUE;
    else
    	next_paragraph_after_section = FALSE;

}

void CmdEndParagraph(int code)

/******************************************************************************
     purpose : ends the current paragraph and return to MODE_VERTICAL.
 ******************************************************************************/
{
    int mode = getTexMode();
    diagnostics(5, "CmdEndParagraph mode = %s, braces=%d", TexModeName[mode], g_par_brace);
        
    if (g_par_brace == 1) {
        endAllFields();
        fprintRTF("\\par\n");
        setTexMode(MODE_VERTICAL);
        g_par_brace=0;
        g_paragraph_inhibit_indent = FALSE;
    } else {
        if (getTexMode() != MODE_VERTICAL)
            diagnostics(5,"*********************** ending paragraph with braces = %d", g_par_brace);
        g_paragraph_inhibit_indent = FALSE;
    }

}

void CmdHfill(int code)

/******************************************************************************
     purpose : should do something about the paragraph style but
     for now, just make sure that we are in horizontal mode.
 ******************************************************************************/
{
	if (getTexMode()==MODE_VERTICAL) 
		changeTexMode(MODE_HORIZONTAL);
}

void CmdVspace(int code)

/******************************************************************************
     purpose : vspace, vspace*, and vskip
     
     note that \vskip3mm will end a paragraph, but \vspace{1cm} will not.
 ******************************************************************************/
{
    int vspace=0;
    char *s;

    switch (code) {
        case VSPACE_VSPACE:
            s = getBraceParam();
            vspace = getStringDimension(s);
            free(s);
            break;

        case VSPACE_VSKIP:
            vspace = getDimension();
    		if (getTexMode() != MODE_VERTICAL) {
            	CmdEndParagraph(0);
            	CmdIndent(INDENT_INHIBIT);
            }    		
            break;

        case VSPACE_SMALL_SKIP:
            vspace = getLength("smallskipamount");
    		if (getTexMode() != MODE_VERTICAL) {
            	CmdEndParagraph(0);
            	CmdIndent(INDENT_INHIBIT);
            }    		
            break;

        case VSPACE_MEDIUM_SKIP:
            vspace = getLength("medskipamount");
    		if (getTexMode() != MODE_VERTICAL) {
            	CmdEndParagraph(0);
            	CmdIndent(INDENT_INHIBIT);
            }
            break;

        case VSPACE_BIG_SKIP:
            vspace = getLength("bigskipamount");
    		if (getTexMode() != MODE_VERTICAL) {
            	CmdEndParagraph(0);
            	CmdIndent(INDENT_INHIBIT);
            }
            break;
    }

    if (getTexMode() == MODE_VERTICAL)
        setVspace(getVspace()+vspace);
}

void CmdIndent(int code)

/******************************************************************************
 purpose : set flags so that startParagraph() does the right thing
     
           INDENT_INHIBIT allows the next paragraph to be indented if
           a paragraph break occurs before startParagraph() is called
                            
           INDENT_NONE tells startParagraph() to not indent the next paragraph
           
           INDENT_USUAL has startParagraph() use the value of \parindent
 ******************************************************************************/
{
    diagnostics(5, "CmdIndent TeX Mode = %s", TexModeName[getTexMode()]);
    if (code == INDENT_NONE)
        g_paragraph_no_indent = TRUE;

    else if (code == INDENT_INHIBIT)
        g_paragraph_inhibit_indent = TRUE;

    else if (code == INDENT_USUAL) {
        g_paragraph_no_indent = FALSE;
        g_paragraph_inhibit_indent = FALSE;
    }
    diagnostics(5, "Noindent is %d", (int) g_paragraph_no_indent);
    diagnostics(5, "Inhibit  is %d", (int) g_paragraph_inhibit_indent);
}

void CmdNewPage(int code)

/******************************************************************************
  purpose: starts a new page
parameter: code: newpage or newcolumn-option
 ******************************************************************************/
{
    diagnostics(5, "CmdNewPage mode = %d", getTexMode());
    switch (code) {
        case NewPage:
            g_page_new = TRUE;
            break;

        case NewColumn:
            g_column_new = TRUE;
            break;
    }
}

/******************************************************************************
  purpose: support for \singlespacing, \onehalfspacing, and \doublespacing
 ******************************************************************************/
void CmdLineSpacing(int code)
{
    setLineSpacing(code);
}

/******************************************************************************
  purpose: support \begin{spacing}{xx} ... \end{spacing}
 ******************************************************************************/
void CmdSpacingEnviron(int code)
{
	char *sizeParam;
	static int originalSpacing=240;
	float spacing;
    int true_code = code & ~ON;

    if (code & ON) {
		originalSpacing = getLineSpacing();
		if (true_code==2)
			setLineSpacing(480);
		else {
			sizeParam = getBraceParam();
			if (*sizeParam) {     	
				sscanf(sizeParam, "%f", &spacing);
				setLineSpacing((int)240*spacing);
				free(sizeParam);       	
			}
		}
		PushEnvironment(SPACING_MODE);
		return;
	}
	
	CmdEndParagraph(0);
	PopEnvironment();
	setLineSpacing(originalSpacing);
}

void CmdAlign(int code)

/*****************************************************************************
    purpose : sets the alignment for a paragraph
  parameter : code: alignment centered, justified, left or right
 ********************************************************************************/
{
    char *s;
    static char old_alignment_before_center = JUSTIFIED;
    static char old_alignment_before_right = JUSTIFIED;
    static char old_alignment_before_left = JUSTIFIED;
    static char old_alignment_before_centerline = JUSTIFIED;

    if (code == PAR_VCENTER) {
        int restart_field=0;
        
        if (EQ_field_active()) {
            diagnostics(4,"ending field due to \\vcenter");
            restart_field = 1;
            endCurrentField();
        }
        
        s = getBraceParam();     
        ConvertString(s);
        free(s);

        if (restart_field) 
            startField(FIELD_EQ);
        return;
    }
    
    CmdEndParagraph(0);
    switch (code) {
        case (PAR_CENTERLINE):
            old_alignment_before_centerline = getAlignment();
            setAlignment(CENTERED);
            fprintRTF("{");
            diagnostics(4, "Entering Convert from CmdAlign (centerline)");
            Convert();
            diagnostics(4, "Exiting Convert from CmdAlign (centerline)");
            setAlignment(old_alignment_before_centerline);
            CmdEndParagraph(0);
            fprintRTF("}");
            break;

        case (PAR_RAGGEDRIGHT):
            old_alignment_before_centerline = getAlignment();
            setAlignment(LEFT);

            diagnostics(4, "Entering Convert from CmdAlign (raggedright)");
            Convert();
            diagnostics(4, "Exiting Convert from CmdAlign (raggedright)");
            setAlignment(old_alignment_before_centerline);
            CmdEndParagraph(0);

            break;

        case (PAR_CENTER | ON):
            CmdIndent(INDENT_NONE);
            old_alignment_before_center = getAlignment();
            setAlignment(CENTERED);
            break;
        case (PAR_CENTER | OFF):
            setAlignment(old_alignment_before_center);
            CmdEndParagraph(0);
            CmdIndent(INDENT_INHIBIT);
            break;

        case (PAR_RIGHT | ON):
            old_alignment_before_right = getAlignment();
            setAlignment(RIGHT);
            CmdIndent(INDENT_NONE);
            break;
        case (PAR_RIGHT | OFF):
            setAlignment(old_alignment_before_right);
            CmdIndent(INDENT_INHIBIT);
            break;

        case (PAR_LEFT | ON):
            old_alignment_before_left = getAlignment();
            setAlignment(LEFT);
            CmdIndent(INDENT_NONE);
            break;
        case (PAR_LEFT | OFF):
            setAlignment(old_alignment_before_left);
            CmdIndent(INDENT_INHIBIT);
            break;
        case (PAR_CENTERING):
            CmdIndent(INDENT_NONE);
            old_alignment_before_center = getAlignment();
            setAlignment(CENTERED);
            break;
    }
}

