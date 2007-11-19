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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

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

char TexModeName[7][25] = { "bad", "internal vertical", "horizontal",
    "restricted horizontal", "math", "displaymath", "vertical"
};

char ParOptionName[4][10] = { "bad", "FIRST", "GENERIC", "SECTION"};

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
void setLineSpacing(int spacing)
{
	g_line_spacing = spacing;
}

int getLineSpacing(void)
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
correctly, as well as vertical and horizontal space.
 ******************************************************************************/
void setTexMode(int mode)
{
    diagnostics(5, "TeX mode setting from [%s] -> [%s]", TexModeName[g_TeX_mode], TexModeName[mode]);
    g_TeX_mode = mode;
}

int getTexMode(void)
{
    return g_TeX_mode;
}

void changeTexMode(int mode)
{
    diagnostics(5, "TeX mode changing from [%s] -> [%s]", TexModeName[g_TeX_mode], TexModeName[mode]);

    if (g_TeX_mode == MODE_VERTICAL && mode == MODE_HORIZONTAL)
        startParagraph("body", GENERIC_PARAGRAPH);

    if (g_TeX_mode == MODE_HORIZONTAL && mode == MODE_VERTICAL)
        CmdEndParagraph(0);

    g_TeX_mode = mode;
}

/******************************************************************************
	RTF codes to create a new paragraph.  If the paragraph should
	not be indented then emit \fi0 otherwise use the current value
	of \parindent as the indentation of the first line.
	
	style describes the type of paragraph ... 
	  "body"
	  "caption"
	  "author"
	  "bibitem"
	  "section"
	  etc.
	  
	indenting describes how this paragraph and (perhaps) the following
	paragraph should be indented
	
	  SECTION_TITLE_PARAGRAPH  (do not indent this paragraph or the next)
	  FIRST_PARAGRAPH  (do not indent this paragraph but indent the next)
	  GENERIC_PARAGRAPH    (indent as needed)
	
	Sometimes it is necessary to know what the next paragraph will
	be before it has been parsed.  For example, a section command
	should create a paragraph for the section title and then the
	next paragraph encountered should be handled like as a first 
	paragraph.  
	
	For FIRST_PARAGRAPH, then it is the first paragraph in a section.
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
    int parindent;
	static int status = 0;
	
    parindent = getLength("parindent");

    if (indenting == SECTION_TITLE_PARAGRAPH) {      /* titles are never indented */
        parindent = 0;
        status = 1;
    	diagnostics(5, "SECTION_TITLE_PARAGRAPH");
    }
    else if (indenting == FIRST_PARAGRAPH) { /* French indents the first paragraph */
    	diagnostics(5, "FIRST_PARAGRAPH");
    	status = 1;
    	if (!FrenchMode && !g_processing_list_environment)
        	parindent = 0;
	} else {                              /* Worry about not indenting */
    	diagnostics(5, "GENERIC_PARAGRAPH");
	    if (g_paragraph_no_indent || g_paragraph_inhibit_indent)
        	parindent = 0;
        else if (status > 0) 
        	parindent = 0;
        status--;
	}
	
    diagnostics(5, "Paragraph mode    %s", TexModeName[getTexMode()]);
    diagnostics(5, "Paragraph option  %s", ParOptionName[indenting]);
    diagnostics(5, "Noindent is       %s", (g_paragraph_no_indent) ? "TRUE" : "FALSE");
    diagnostics(5, "Inhibit is        %s", (g_paragraph_inhibit_indent) ? "TRUE" : "FALSE");
    diagnostics(5, "left indent is    %d", g_left_margin_indent);
    diagnostics(5, "right indent is   %d", g_right_margin_indent);
    diagnostics(5, "current parindent %d", getLength("parindent"));
    diagnostics(5, "this parindent    %d", parindent);

    if (g_page_new) {
        fprintRTF("\\page{}");   /* causes new page */
        g_page_new = FALSE;
        g_column_new = FALSE;
    }

    if (g_column_new) {
        fprintRTF("\\column "); /* causes new page */
        g_column_new = FALSE;
    }

    fprintRTF("\\pard\\q%c",      getAlignment());
    fprintRTF("\\sl%i\\slmult1 ", getLineSpacing());

    if (getVspace() > 0)
        fprintRTF("\\sb%d ", getVspace());
    setVspace(0);

    if (g_left_margin_indent != 0)
        fprintRTF("\\li%d", g_left_margin_indent);

    if (g_right_margin_indent != 0)
        fprintRTF("\\ri%d", g_right_margin_indent);

    fprintRTF("\\fi%d ", parindent);

    setTexMode(MODE_HORIZONTAL); 

    if (!g_processing_list_environment) {
        g_paragraph_no_indent = FALSE;
        if (indenting == SECTION_TITLE_PARAGRAPH)
        	g_paragraph_inhibit_indent = TRUE;
        else
        	g_paragraph_inhibit_indent = FALSE;
    }
}

void CmdEndParagraph(int code)

/******************************************************************************
     purpose : ends the current paragraph and return to MODE_VERTICAL.
 ******************************************************************************/
{
    int mode = getTexMode();

    diagnostics(5, "CmdEndParagraph mode = %s", TexModeName[mode]);
    if (mode != MODE_VERTICAL  && g_processing_fields == 0) {
        fprintRTF("\\par\n");
        setTexMode(MODE_VERTICAL);
    }

    g_paragraph_inhibit_indent = FALSE;
}

void CmdVspace(int code)

/******************************************************************************
     purpose : vspace, vspace*, and vskip
     
     note that \vskip3mm will end a paragraph, but \vspace{1cm} will not.
 ******************************************************************************/
{
    int vspace;
    char *s;

    switch (code) {
        case VSPACE_VSPACE:
        	s = getBraceParam();
        	vspace = getStringDimension(s);
            free(s);
            break;

        case VSPACE_VSKIP:
            vspace = getDimension();
			CmdEndParagraph(0);
            break;

        case VSPACE_SMALL_SKIP:
            vspace = getLength("smallskipamount");
            break;

        case VSPACE_MEDIUM_SKIP:
            vspace = getLength("medskipamount");
            break;

        case VSPACE_BIG_SKIP:
            vspace = getLength("bigskipamount");
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
    diagnostics(5, "CmdIndent mode = %d", getTexMode());
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
  purpose: support for \doublespacing
 ******************************************************************************/
void CmdDoubleSpacing(int code)
{
	g_line_spacing = 480;
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
        s = getBraceParam();
        free(s);
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

/*		fprintRTF("{"); */
            diagnostics(4, "Entering Convert from CmdAlign (centerline)");
            Convert();
            diagnostics(4, "Exiting Convert from CmdAlign (centerline)");
            setAlignment(old_alignment_before_centerline);
            CmdEndParagraph(0);

/*		fprintRTF("}");*/
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
