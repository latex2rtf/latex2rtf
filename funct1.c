
/* funct1.c - interpret various LaTeX commands and produce RTF

Copyright (C) 1995-2002 The Free Software Foundation

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
 
Authors:
	1995      Fernando Dorner, Andreas Granzer, Freidrich Polzer, Gerhard Trisko
    1995-1997 Ralf Schlatterbeck
    1998-2000 Georg Lehner
    2001-2002 Scott Prahl
*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "convert.h"
#include "funct1.h"
#include "commands.h"
#include "stack.h"
#include "fonts.h"
#include "cfg.h"
#include "ignore.h"
#include "utils.h"
#include "encodings.h"
#include "parser.h"
#include "counters.h"
#include "lengths.h"
#include "definitions.h"
#include "preamble.h"
#include "xrefs.h"
#include "equations.h"
#include "direct.h"
#include "styles.h"
#include "graphics.h"
#include "vertical.h"

#define ARABIC_NUMBERING 0
#define ALPHA_NUMBERING  1
#define ROMAN_NUMBERING  2

extern bool twocolumn;          /* true if twocolumn-mode is enabled */

void CmdPagestyle( /* @unused@ */ int code);
void CmdHeader(int code);
char *roman_item(int n, bool upper);

static int g_chapter_numbering = ARABIC_NUMBERING;
static bool g_appendix;

bool g_processing_list_environment = FALSE;

void CmdNewDef(int code)

/******************************************************************************
     purpose : handles \def \newcommand \renewcommand
 ******************************************************************************/
{
    char *name=NULL, *opt_param=NULL, *def=NULL, cThis;
    char *params = NULL;
    int param = 0;

    if (code == DEF_DEF) {

        name = getSimpleCommand();
        if (name == NULL) {
            diagnostics(WARNING, "Definition does not start with '\\' skipping");
            return;
        }

        /* handle simple parameters (discard delimiters) e.g., #1#2#3 */
        while ((cThis = getTexChar()) && cThis != '{') {
            if (isdigit((int) cThis))
                param++;
        }
        ungetTexChar('{');

        opt_param = NULL;
        def = getBraceParam();
        UpdateLineNumber(def);
        newDefinition(name + 1, opt_param, def, param);
    }

    if (code == DEF_NEW || code == DEF_RENEW) {
        name = getBraceParam();
        params = getBracketParam();
        opt_param = getBracketParam();
        def = getBraceParam();
        UpdateLineNumber(name);
        UpdateLineNumber(params);
        if (opt_param)
            UpdateLineNumber(opt_param);
        UpdateLineNumber(def);
        param = 0;
        if (params) {
            if ('0' <= *params && *params <= '9')
                param = *params - '0';
            else
                diagnostics(ERROR, "non-numeric number of parameters in newcommand");
        }


        if (code == DEF_NEW)
            newDefinition(name + 1, opt_param, def, param);
        else
            renewDefinition(name + 1, opt_param, def, param);

    }

    diagnostics(4, "CmdNewDef name=<%s> param=%d opt_param=<%s> def=<%s>", 
                   (name) ? name : "",
                   param, 
                   (opt_param) ? opt_param : "",
                   (def) ? def : "");
    free(name);
    free(def);
    if (params)
        free(params);
    if (opt_param)
        free(opt_param);
}

void CmdNewEnvironment(int code)
{
    char *name, *opt_param, *begdef, *enddef, *params;
    int param;

    name = getBraceParam();
    params = getBracketParam();
    opt_param = getBracketParam();
    begdef = getBraceParam();
    enddef = getBraceParam();
    UpdateLineNumber(name);
    UpdateLineNumber(params);
    if (opt_param)
        UpdateLineNumber(opt_param);
    UpdateLineNumber(begdef);
    UpdateLineNumber(enddef);
    param = 0;
    if (params) {
        if ('0' <= *params && *params <= '9')
            param = *params - '0';
        else
            diagnostics(ERROR, "non-numeric number of parameters in newcommand");
    }

    diagnostics(2, "CmdNewEnvironment name=<%s> param=%d", name, param);
    diagnostics(2, "CmdNewEnvironment begdef=<%s>", begdef);
    diagnostics(2, "CmdNewEnvironment enddef=<%s>", enddef);

    if (code == DEF_NEW)
        newEnvironment(name, opt_param, begdef, enddef, param);
    else
        renewEnvironment(name, opt_param, begdef, enddef, param);

    if (opt_param)
        free(opt_param);
    free(name);
    free(begdef);
    free(enddef);
    if (params)
        free(params);

}

void CmdNewTheorem(int code)
{
    char *name, *caption, *numbered_like, *within;

    name = getBraceParam();
    numbered_like = getBracketParam();
    caption = getBraceParam();
    within = getBracketParam();

    UpdateLineNumber(name);
    UpdateLineNumber(numbered_like);
    UpdateLineNumber(caption);
    UpdateLineNumber(within);

    diagnostics(2, "CmdNewTheorem name=<%s>", name);
    diagnostics(2, "CmdNewTheorem caption=<%s>", caption);
    diagnostics(2, "CmdNewTheorem like=<%s>", (numbered_like) ? numbered_like : "");
    newTheorem(name, caption, numbered_like, within);

    free(name);
    free(caption);
    if (numbered_like)
        free(numbered_like);
    if (within)
        free(within);
}

void CmdSlashSlash(int code)

/***************************************************************************
 purpose: handle \\, \\[1pt], \\*[1pt] 
 ***************************************************************************/
{
    char cThis, *vertical_space;

    if (g_processing_arrays) {  /* array */
        cThis = getNonBlank();
        ungetTexChar(cThis);
        fprintRTF("%c", g_field_separator);
        return;
    }

    cThis = getTexChar();
    if (cThis != '*')
        ungetTexChar(cThis);

    vertical_space = getBracketParam();
    if (vertical_space)         /* ignore for now */
        free(vertical_space);

    if (g_processing_eqnarray) {    /* eqnarray */
        if (g_processing_fields)
            fprintRTF("}}{\\fldrslt }}");
        if (g_show_equation_number && !g_suppress_equation_number) {
            char number[20];

            for (; g_equation_column < 3; g_equation_column++)
                fprintRTF("\\tab ");
            incrementCounter("equation");

            fprintRTF("\\tab{\\b0 (");
            snprintf(number, 20, "%d", getCounter("equation"));
            InsertBookmark(g_equation_label, number);
            if (g_equation_label) {
                free(g_equation_label);
                g_equation_label = NULL;
            }
            fprintRTF(")}");
        }

        fprintRTF("\\par\n\\tab ");
        if (g_processing_fields)
            fprintRTF("{\\field{\\*\\fldinst{ EQ ");

        g_suppress_equation_number = FALSE;
        g_equation_column = 1;
        return;
    }

/* this should only happen for an array environment */
    if (g_processing_tabular) { /* tabular or array environment */
        if (getTexMode() == MODE_MATH || getTexMode() == MODE_DISPLAYMATH) {    /* array */
            fprintRTF("\\par\n\\tab ");
            return;
        }
    	fprintRTF("\\row\n");
/*        for (; actCol < colCount; actCol++) {
            fprintRTF("\\cell\\pard\\intbl");
        }
        actCol = 0;
        fprintRTF("\\row\n\\pard\\intbl\\q%c ", colFmt[actCol]);
*/
        return;
    }

/* I don't think this should happen anymore either! */
    if (g_processing_tabbing) {
        PopBrace();
        PushBrace();
    }

    /* simple end of line ... */
    CmdEndParagraph(0);
    CmdIndent(INDENT_INHIBIT);

    tabcounter = 0;
}

void CmdBeginEnd(int code)

/***************************************************************************
 purpose: reads the parameter after the \begin or \end-command; ( see also getBraceParam )
	      after reading the parameter the CallParamFunc-function calls the
	      handling-routine for that special environment
 parameter: code: CMD_BEGIN: start of environment
		          CMD_END:   end of environment
 ***************************************************************************/
{
    int i;
    char *str, *option;
    char *s = getBraceParam();

    if (code == CMD_BEGIN)
        diagnostics(5, "\\begin{%s}", s);
    else
        diagnostics(5, "\\end{%s}", s);

    if (strcmp(s, "document") == 0) {
        free(s);
        EndSource();            /* done! */
        return;
    }

/* hack to avoid problems with multicols */
    if (strcmp(s, "multicols") == 0) {
        free(s);
        return;
    }

/* user defined environments */
    i = existsEnvironment(s);
    if (i > -1) {
    	diagnostics(WARNING, "starting user-defined environment '%s'", s);
        str = expandEnvironment(i, code);
        ConvertString(str);
        free(str);
        free(s);
        return;
    }

/* theorem environment */
    i = existsTheorem(s);
    if (i > -1) {
        if (code == CMD_BEGIN) {
            option = getBracketParam();
            str = expandTheorem(i, option);
            CmdEndParagraph(0);
            CmdVspace(VSPACE_SMALL_SKIP);
            startParagraph("theorem", FIRST_PARAGRAPH);
            fprintRTF("{\\b %s} {\\i ", str);
            PushBrace();
            if (option)
                free(option);
            free(str);
        } else {
            PopBrace();
            fprintRTF("}");
            CmdEndParagraph(0);
            CmdVspace(VSPACE_SMALL_SKIP);
            CmdIndent(INDENT_INHIBIT);
        }
        free(s);
        return;
    }

/* usual environments */
    if (code == CMD_BEGIN) {
        diagnostics(4, "\\begin{%s}", s);
        (void) CallParamFunc(s, ON);
    } else {
        diagnostics(4, "\\end{%s}", s);
        (void) CallParamFunc(s, OFF);
        CmdIndent(INDENT_INHIBIT);
    }
    free(s);
}

void CmdToday(int code)

/******************************************************************************
    purpose: converts LaTeX \today into RTF \chdate
 ******************************************************************************/
{
    if (getTexMode() == MODE_VERTICAL)
    	changeTexMode(MODE_HORIZONTAL);
    fprintRTF("\\chdate ");
}


void CmdIgnore(int code)

/******************************************************************************
 purpose: allows handling of constructs that do not require changes to RTF
 ******************************************************************************/
{
}

void Environment(int code)

/******************************************************************************
  purpose: pushes/pops the new environment-commands on/from the stack
parameter: code includes the type of the environment
 ******************************************************************************/
{
    if (code & ON) {
        code &= ~(ON);          /* mask MSB */
        diagnostics(4, "Entering Environment (%d)", code);
        PushEnvironment(GENERIC_MODE);
    } else {                    /* off switch */
        CmdEndParagraph(0);
        diagnostics(4, "Exiting  Environment (%d)", code);
        PopEnvironment();
    }
}

static char *FormatNumber(int formatting, int n)
{
    char label[20];
    switch (formatting) {
    
		case ALPHA_NUMBERING:
			snprintf(label, 20, "%c", (char) (n + (int) 'A' - 1) );
			break;
	
		case ARABIC_NUMBERING:
			snprintf(label, 20, "%d", n);
			break;
			
		case ROMAN_NUMBERING:
			return roman_item(n, TRUE);
    }
    
    return strdup(label);
}

static char *FormatSection(void) 
{
    if (g_appendix)
    	return FormatNumber(ALPHA_NUMBERING, getCounter("section"));
    else
    	return FormatNumber(ARABIC_NUMBERING, getCounter("section"));
}

static char *FormatChapter(void) 
{
    return FormatNumber(g_chapter_numbering, getCounter("chapter"));
}

char *FormatUnitNumber(char *name)

/******************************************************************************
  purpose: returns the x.x.x number for the specified sectional unit.
 ******************************************************************************/
{
    char label[20];

    if (g_document_type == FORMAT_APA) { /* no numbers in apa at all! */
    	return strdup("");
    }
    
    label[0] = '\0';
    if (strcmp(name, "part") == 0) {
        char *s = roman_item(getCounter(name), TRUE);

        snprintf(label, 20, "%s", s);
        free(s);
    }

    else if (strcmp(name, "chapter") == 0) 
    	
    	return FormatChapter();

    else if (strcmp(name, "section") == 0) {
        if (g_document_type == FORMAT_ARTICLE)
            snprintf(label, 20, "%s", FormatSection());
        else
            snprintf(label, 20, "%s.%d", FormatChapter(), getCounter("section"));
    }

    else if (strcmp(name, "subsection") == 0) {
        if (g_document_type == FORMAT_ARTICLE)
            snprintf(label, 20, "%s.%d", FormatSection(), getCounter("subsection"));
        else
            snprintf(label, 20, "%s.%d.%d", FormatChapter(), getCounter("section"), getCounter("subsection"));
    }

    else if (strcmp(name, "subsubsection") == 0) {
        if (g_document_type == FORMAT_ARTICLE)
            snprintf(label, 20, "%s.%d.%d", FormatSection(),
              getCounter("subsection"), getCounter("subsubsection"));
        else
            snprintf(label, 20, "%s.%d.%d.%d", FormatChapter(), 
             getCounter("section"), getCounter("subsection"), getCounter("subsubsection"));
    }

    else if (strcmp(name, "paragraph") == 0) {
        if (g_document_type == FORMAT_ARTICLE)
            snprintf(label, 20, "%s.%d.%d.%d", FormatSection(),
              getCounter("subsection"), getCounter("subsubsection"), getCounter("paragraph"));
        else
            snprintf(label, 20, "%s.%d.%d.%d.%d", FormatChapter(), 
              getCounter("section"), getCounter("subsection"), getCounter("subsubsection"), getCounter("paragraph"));
    }

    else if (strcmp(name, "subparagraph") == 0) {
        if (g_document_type == FORMAT_ARTICLE)
            snprintf(label, 20, "%s.%d.%d.%d.%d",FormatSection(),
              getCounter("subsection"), getCounter("subsubsection"),
              getCounter("paragraph"), getCounter("subparagraph"));
        else
            snprintf(label, 20, "%s.%d.%d.%d.%d.%d", FormatChapter(),
              getCounter("section"), getCounter("subsection"),
              getCounter("subsubsection"), getCounter("paragraph"), getCounter("subparagraph"));
    }

    return strdup(label);
}

void CmdAppendix(int code)

/******************************************************************************
  purpose: handles \appendix
 ******************************************************************************/
{
	g_chapter_numbering = ALPHA_NUMBERING;
	g_appendix=0;
	setCounter("chapter",0);
}

void CmdSection(int code)

/******************************************************************************
  purpose: converts the LaTeX-section-commands into similar Rtf-styles
parameter: code: type of section-recursion-level
 ******************************************************************************/
{
    char *toc_entry;
    char *heading;
    char *unit_label;
    char *chapter_name=NULL;

    toc_entry = getBracketParam();
    heading = getBraceParam();

    if (toc_entry) {
        diagnostics(4, "entering CmdSection [%s]{%s}", toc_entry, heading);
        free(toc_entry);
    } else
        diagnostics(4, "entering CmdSection {%s}", heading);

    CmdEndParagraph(0);

    switch (code) {
        case SECT_PART:
        case SECT_PART_STAR:
            if (getCounter("part") > 0) CmdNewPage(NewPage);
            startParagraph("part", SECTION_TITLE_PARAGRAPH);
            fprintRTF("{");
            InsertStyle("part");
            fprintRTF(" ");
            ConvertBabelName("PARTNAME");
            if (code == SECT_PART) {
                incrementCounter("part");
                unit_label = FormatUnitNumber("part");
                fprintRTF(" %s\\par ", unit_label);
                free(unit_label);
            }
            ConvertString(heading);
            CmdEndParagraph(0);
            fprintRTF("}\n");
            CmdNewPage(NewPage);
            break;

        case SECT_CHAPTER:
        case SECT_CHAPTER_STAR:
            unit_label = NULL;
            if (getCounter("chapter") > 0) CmdNewPage(NewPage);
            startParagraph("chapter", SECTION_TITLE_PARAGRAPH);
            fprintRTF("{");
            InsertStyle("chapter");
            fprintRTF(" ");
            chapter_name=GetBabelName("CHAPTERNAME");
            ConvertString(chapter_name);
            if (code == SECT_CHAPTER && getCounter("secnumdepth") >= -1) {
                incrementCounter("chapter");
                setCounter("section", 0);
        		setCounter("figure",0);
       			setCounter("table",0);
       			setCounter("equation",0);
                resetTheoremCounter("chapter");
                unit_label = FormatUnitNumber("chapter");
                fprintRTF(" ");
                InsertBookmark(g_section_label, unit_label);
            }
            CmdEndParagraph(0);
            CmdVspace(VSPACE_BIG_SKIP);
            startParagraph("chapter", SECTION_TITLE_PARAGRAPH);
            ConvertString(heading);
            CmdEndParagraph(0);
            fprintRTF("\\par\\par}");
/*            InsertContentMark('c', chapter_name, " ", unit_label, " ", heading);*/
            CmdVspace(VSPACE_SMALL_SKIP);
            if (unit_label) free(unit_label);
            break;

        case SECT_NORM:
        case SECT_NORM_STAR:
            CmdVspace(VSPACE_BIG_SKIP);
            if (g_document_type == FORMAT_APA) {
            	ConvertString("\\begin{center}\\bf");
			} else {        	
            	startParagraph("section", SECTION_TITLE_PARAGRAPH);
            	fprintRTF("{");
				InsertStyle("section");
				fprintRTF(" ");
            
				if (code == SECT_NORM && getCounter("secnumdepth") >= 0) {
					incrementCounter("section");
					setCounter("subsection", 0);
					resetTheoremCounter("section");
					unit_label = FormatUnitNumber("section");
					InsertBookmark(g_section_label, unit_label);
					fprintRTF("  ");
					free(unit_label);
				}
            }
            ConvertString(heading);
            if (g_document_type == FORMAT_APA) {
            	ConvertString("\\end{center}");
            	CmdEndParagraph(0);
            	startParagraph("apa_section", GENERIC_PARAGRAPH);
            } else {
            	CmdEndParagraph(0);
	            fprintRTF("}");
            }
            CmdVspace(VSPACE_SMALL_SKIP);
            break;

        case SECT_SUB:
        case SECT_SUB_STAR:
            CmdVspace(VSPACE_MEDIUM_SKIP);
            if (g_document_type == FORMAT_APA) {
            	ConvertString("\\noindent");
				startParagraph("apa_subsection", FIRST_PARAGRAPH);
				fprintRTF("{\\i ");
			} else {        	
				startParagraph("subsection", SECTION_TITLE_PARAGRAPH);
				fprintRTF("{");
				InsertStyle("subsection");
				fprintRTF(" ");          
				if (code == SECT_SUB && getCounter("secnumdepth") >= 1) {
					incrementCounter("subsection");
					setCounter("subsubsection", 0);
					resetTheoremCounter("subsection");
					unit_label = FormatUnitNumber("subsection");
					InsertBookmark(g_section_label, unit_label);
					fprintRTF("  ");
					free(unit_label);
				}
            }
            ConvertString(heading);
            CmdEndParagraph(0);
            fprintRTF("}");
            CmdVspace(VSPACE_SMALL_SKIP);
            break;

        case SECT_SUBSUB:
        case SECT_SUBSUB_STAR:
            CmdVspace(VSPACE_MEDIUM_SKIP);
            if (g_document_type == FORMAT_APA) {
				startParagraph("apa_subsubsection", GENERIC_PARAGRAPH);
				fprintRTF("{\\i ");
			} else {        	
				startParagraph("subsubsection", SECTION_TITLE_PARAGRAPH);
				fprintRTF("{");
				InsertStyle("subsubsection");
				fprintRTF(" ");
				
				if (code == SECT_SUBSUB && (getCounter("secnumdepth") > 2 ||
					(g_document_type == FORMAT_ARTICLE && getCounter("secnumdepth") == 2))) {
					incrementCounter("subsubsection");
					setCounter("paragraph", 0);
					setCounter("subparagraph", 0);
					resetTheoremCounter("subsubsection");
					unit_label = FormatUnitNumber("subsubsection");
					InsertBookmark(g_section_label, unit_label);
					fprintRTF("  ");
					free(unit_label);
				}
            }
            ConvertString(heading);
            if (g_document_type == FORMAT_APA) {
            	fprintRTF(".} ");
            } else {
            	CmdEndParagraph(0);
            	fprintRTF("}");
            	CmdVspace(VSPACE_SMALL_SKIP);
            }
            break;

        case SECT_SUBSUBSUB:
        case SECT_SUBSUBSUB_STAR:
            CmdVspace(VSPACE_MEDIUM_SKIP);
            startParagraph("paragraph", SECTION_TITLE_PARAGRAPH);
            fprintRTF("{");
            InsertStyle("paragraph");
            fprintRTF(" ");
            if (code == SECT_SUBSUBSUB && getCounter("secnumdepth") >= 3) {
                incrementCounter("paragraph");
                resetTheoremCounter("paragraph");
                unit_label = FormatUnitNumber("paragraph");
                setCounter("subparagraph", 0);
                InsertBookmark(g_section_label, unit_label);
                fprintRTF("  ");
                free(unit_label);
            }
            ConvertString(heading);
            CmdEndParagraph(0);
            fprintRTF("} ");
            CmdVspace(VSPACE_SMALL_SKIP);
            break;

        case SECT_SUBSUBSUBSUB:
        case SECT_SUBSUBSUBSUB_STAR:
            CmdVspace(VSPACE_MEDIUM_SKIP);
            startParagraph("subparagraph", SECTION_TITLE_PARAGRAPH);
            fprintRTF("{");
            InsertStyle("subparagraph");
            fprintRTF(" ");
            if (code == SECT_SUBSUBSUBSUB && getCounter("secnumdepth") >= 4) {
                incrementCounter("subparagraph");
                resetTheoremCounter("subparagraph");
                unit_label = FormatUnitNumber("subparagraph");
                InsertBookmark(g_section_label, unit_label);
                fprintRTF("  ");
                free(unit_label);
            }
            ConvertString(heading);
            CmdEndParagraph(0);
            fprintRTF("} ");
            CmdVspace(VSPACE_SMALL_SKIP);
            break;
    }

    if (heading)
        free(heading);
    if (g_section_label) {
        free(g_section_label);
        g_section_label = NULL;
    }
}

/******************************************************************************
 purpose: converts \caption from LaTeX to Rtf
 ******************************************************************************/
void CmdCaption(int code)
{
    char *thecaption;
    char *lst_entry;
    int n, vspace;
    char old_align;
    char number[20];
    char c;

    old_align = getAlignment();
    setAlignment(CENTERED);

    lst_entry = getBracketParam();
    thecaption = getBraceParam();

    if (lst_entry) {
        diagnostics(4, "entering \\caption[%s]{}", lst_entry);
        show_string(4, thecaption, "caption");
        free(lst_entry);
    } else {
        diagnostics(4, "entering \\caption{}");
        show_string(4, thecaption, "caption");
	}
	
    if (getTexMode() != MODE_VERTICAL)
        CmdEndParagraph(0);
    vspace = getLength("abovecaptionskip");
    setVspace(vspace);
    startParagraph("caption", FIRST_PARAGRAPH);
    fprintRTF("{");

    if (g_processing_figure) {
        incrementCounter("figure");
        ConvertBabelName("FIGURENAME");
	    fprintRTF(" ");
        n = getCounter("figure");
        c = 'f';
    } else {
        incrementCounter("table");
        ConvertBabelName("TABLENAME");
        fprintRTF(" ");
        n = getCounter("table");
        c = 't';
    }

    if (g_document_type != FORMAT_ARTICLE &&
        g_document_type != FORMAT_APA)
        snprintf(number, 20, "%d.%d", getCounter("chapter"), n);
    else
        snprintf(number, 20, "%d", n);

    if (g_processing_figure && g_figure_label)
        InsertBookmark(g_figure_label, number);

    else if (g_processing_table && g_table_label)
        InsertBookmark(g_table_label, number);

    else
        fprintRTF("%s", number);

	fprintRTF(": ");
    ConvertString(thecaption);
    fprintRTF("}");

	InsertContentMark(c, number, "  ", thecaption);

    free(thecaption);

    CmdEndParagraph(0);
    vspace = getLength("belowcaptionskip") + getLength("textfloatsep");
    setVspace(vspace);
    setAlignment(old_align);
    diagnostics(4, "exiting CmdCaption");
}

void CmdCounter(int code)

/******************************************************************************
 purpose: handles \newcounter, \setcounter, \addtocounter, \value
 ******************************************************************************/
{
    char *s, *s2, *s3, *s4;
    int num;

    s = getBraceParam();

    diagnostics(4, "Entering CmdCounter(), <%s>", s);

    if (code == COUNTER_ADD || code == COUNTER_SET) {

        s2 = getBraceParam();

        if ((s3 = strchr(s2, '{')) && (s4 = strchr(s2, '}'))) {
            s3++;
            *s4 = '\0';
            diagnostics(4, "CmdCounter() processing \\value{%s}", s3);
            num = getCounter(s3);

        } else if (sscanf(s2, "%d", &num) != 1) {

            diagnostics(WARNING, "\nBad parameter in set/addcounter{%s}{%s}", s, s2);
            free(s2);
            free(s);
            return;
        }

        free(s2);
        if (code == COUNTER_ADD)
            setCounter(s, getCounter(s) + num);
        else
            setCounter(s, num);

    } else if (code == COUNTER_NEW)
        setCounter(s, 0);

    free(s);
}

void CmdLength(int code)

/******************************************************************************
 purpose: handles \newlength, \setlength, \addtolength
 purpose: handle \textwidth=0.8in or \moveright0.1\textwidth
 ******************************************************************************/
{
    char *s, *s1;
    char cThis;
    int num;


    if (code == LENGTH_ADD || code == LENGTH_SET || code == LENGTH_NEW) {
        s = getBraceParam();
        if (strlen(s) <= 1) {
            free(s);
            diagnostics(WARNING, "missing argument in \\newlength \\addtolength or \\setlength");
            return;
        }
        s1 = s + 1;             /* skip initial '\\' */
        diagnostics(4, "Entering CmdLength(), <%s>", s1);
        if (code == LENGTH_ADD || code == LENGTH_SET) {

            cThis = getNonSpace();

            if (cThis == '{') {
                num = getDimension();

                diagnostics(4, "size is <%d> twips", num);
                cThis = getTexChar();

                while (cThis && cThis != '}')
                    cThis = getTexChar();

                if (code == LENGTH_ADD)
                    setLength(s1, getLength(s1) + num);
                else
                    setLength(s1, num);
            } else
                diagnostics(WARNING, "bad parameter to \\addtolength or \\setlength");

        } else
            setLength(s1, 0);

        free(s);

    } else {
        skipSpaces();
        cThis = getTexChar();

        if (cThis == '=')       /* optional '=' */
            skipSpaces();
        else
            ungetTexChar(cThis);

        num = getDimension();   /* discard for now */
    }
}

void CmdQuote(int code)

/******************************************************************************
  purpose: handles \begin{quote} ... \end{quote} 
                   \begin{quotation} ... \end{quotation}
  globals:   indent which is the left-indent-position
 ******************************************************************************/
{
    int true_code = code & ~ON;
    CmdEndParagraph(0);

    if (code & ON) {

        if (true_code == QUOTATION) {
            PushEnvironment(QUOTE_MODE);
            diagnostics(4, "Entering \\begin{quotation}");
            CmdVspace(VSPACE_SMALL_SKIP);
            setLeftMarginIndent(getLeftMarginIndent()+512);
            setRightMarginIndent(getRightMarginIndent()+512);
			startParagraph("quote", INDENT_USUAL);			
        }
        else {
            PushEnvironment(QUOTATION_MODE);
            diagnostics(4, "Entering \\begin{quote}");
            CmdVspace(VSPACE_SMALL_SKIP);
            setLeftMarginIndent(getLeftMarginIndent()+512);
            setRightMarginIndent(getRightMarginIndent()+512);
            setLength("parindent", 0);
			startParagraph("quote", INDENT_USUAL);			
		}
	}
	else {
		PopEnvironment();
        diagnostics(4, "Exiting \\end{quote} or \\end{quotation}");
        CmdIndent(INDENT_INHIBIT);
        CmdVspace(VSPACE_SMALL_SKIP);
    }
}

void CmdList(int code)

/******************************************************************************
  purpose : set indentation and counters for itemize, description and enumerate
 globals  : indent
 ******************************************************************************/
{
    int vspace;
    int amount = 300;

    vspace = getLength("topsep") + getLength("parskip");

	if (code != (INPARAENUM_MODE | ON) && code != (INPARAENUM_MODE | OFF) ) {
		if (getTexMode() == MODE_VERTICAL)
			vspace += getLength("partopsep");
		else
			CmdEndParagraph(0);
	}
	
    switch (code) {
        case (ITEMIZE_MODE | ON):
            setVspace(vspace);
            PushEnvironment(ITEMIZE_MODE);
            setLength("parindent", -amount);
            setLeftMarginIndent(getLeftMarginIndent() + 2 * amount);
            CmdIndent(INDENT_USUAL);
            break;

        case (ENUMERATE_MODE | ON):
            setVspace(vspace);
            PushEnvironment(ENUMERATE_MODE);
            g_enumerate_depth++;
            CmdItem(RESET_ITEM_COUNTER);
            setLength("parindent", -amount);
            setLeftMarginIndent(getLeftMarginIndent() + 2 * amount);
            CmdIndent(INDENT_USUAL);
            break;

        case (DESCRIPTION_MODE | ON):
            setVspace(vspace);
            PushEnvironment(DESCRIPTION_MODE);
            setLength("parindent", -amount);
            setLeftMarginIndent(getLeftMarginIndent() + 2 * amount);
            CmdIndent(INDENT_USUAL);
            break;

        case (INPARAENUM_MODE | ON):
            PushEnvironment(INPARAENUM_MODE);
            g_enumerate_depth++;
            CmdItem(RESET_ITEM_COUNTER);
            break;

        case (INPARAENUM_MODE | OFF):
            g_enumerate_depth--;
            PopEnvironment();
            g_processing_list_environment = FALSE;
            break;

        case (ENUMERATE_MODE | OFF):
            g_enumerate_depth--;    /* fall through */
        case (ITEMIZE_MODE | OFF):
        case (DESCRIPTION_MODE | OFF):
            PopEnvironment();
            CmdIndent(INDENT_USUAL);    /* need to reset INDENT_NONE from CmdItem */
            g_processing_list_environment = FALSE;
            setVspace(vspace);
            break;
    }
}

void CmdItem(int code)

/******************************************************************************
 purpose : handles \item command.  Since the \item command is delimited by 
           a later \item command or the ending of an environment (\end{itemize})
           this routine will get called recursively.
 ******************************************************************************/
{
    char *itemlabel, thechar;
    static int item_number[4];
    int vspace;

    if (code == RESET_ITEM_COUNTER) {
        item_number[g_enumerate_depth] = 1;
        return;
    }

    diagnostics(4, " CmdItem depth=%d #=%d (%s)", 
       g_enumerate_depth, 
       item_number[g_enumerate_depth],
       (code==ENUMERATE_MODE)?"enumerate":
       (code==ITEMIZE_MODE)?"itemize":
       (code==DESCRIPTION_MODE)?"description":
       (code==INPARAENUM_MODE)?"inparaenum":"not enum");

    g_processing_list_environment = TRUE;

	/* suppress vertical space for inparaenum */
	if (code != INPARAENUM_MODE) {
    	CmdEndParagraph(0);
    	vspace = getLength("itemsep") + getLength("parsep");
    	setVspace(vspace);

    	CmdIndent(INDENT_USUAL);
    	startParagraph("item", FIRST_PARAGRAPH);
    }

    itemlabel = getBracketParam();
    if (itemlabel) {            /* \item[label] */
        fprintRTF("{");
        if (code == DESCRIPTION_MODE)
            fprintRTF("\\b ");
        diagnostics(5, "Entering ConvertString from CmdItem");
        ConvertString(itemlabel);
        diagnostics(5, "Exiting ConvertString from CmdItem");
        fprintRTF("}");
        if (code != DESCRIPTION_MODE && code != INPARAENUM_MODE)
            fprintRTF("\\tab ");
    }

    switch (code) {
        case ITEMIZE_MODE:
            if (!itemlabel) {
                if (FrenchMode)
                    fprintRTF("\\endash\\tab ");
                else
                    fprintRTF("\\bullet\\tab ");
            }
            break;

        case INPARAENUM_MODE:
        case ENUMERATE_MODE:
            if (itemlabel) 
            	break;
			switch (g_enumerate_depth) {
				case 1:
					fprintRTF("%d.", item_number[g_enumerate_depth]);
					break;

				case 2:
					fprintRTF("(%c)", 'a' + item_number[g_enumerate_depth] - 1);
					break;

				case 3:
					fprintRTF("%s.", roman_item(item_number[g_enumerate_depth], FALSE));
					break;

				case 4:
					fprintRTF("%c.", 'A' + item_number[g_enumerate_depth] - 1);
					break;
			}
			if (code != INPARAENUM_MODE)
				fprintRTF("\\tab ");
			else
				fprintRTF(" ");
			
            item_number[g_enumerate_depth]++;
            break;

        case DESCRIPTION_MODE:
            fprintRTF(" ");
            break;
    }

    if (itemlabel)
        free(itemlabel);
    thechar = getNonBlank();
    ungetTexChar(thechar);
    
    if (code != INPARAENUM_MODE)
    	CmdIndent(INDENT_NONE);
}

void CmdResizeBox(int code)
{
	char *size, *options, *content;
	size = getBraceParam();
	options = getBraceParam();
	content = getBraceParam();
	free(size);
	free(options);
	ConvertString(content);
	free(content);
}

void CmdBox(int code)

/******************************************************************************
  purpose: converts the LaTeX \box-commands into  an similar Rtf-style
 ******************************************************************************/
{
    char BoxName[5][10] = { "hbox", "vbox", "mbox", "fbox", "parbox" };
    int mode = getTexMode();

    diagnostics(4, "Entering CmdBox() [%s]", BoxName[code - 1]);
    if (g_processing_fields)
        g_processing_fields++;  /* hack to stop fields within fields */

    if (code == BOX_HBOX || code == BOX_MBOX)
        changeTexMode(MODE_RESTRICTED_HORIZONTAL);

    if (code == BOX_PARBOX) {
        char *position, *width;

        position = getBracketParam();
        width = getBraceParam();
        if (position)
            free(position);
        free(width);
    }

    diagnostics(4, "Entering Convert() from CmdBox");
    Convert();
    diagnostics(4, "Exiting Convert() from CmdBox");

    if (g_processing_fields)
        g_processing_fields--;

    if (code == BOX_VBOX) {
        CmdEndParagraph(0);
        CmdIndent(INDENT_INHIBIT);

    } else {
        changeTexMode(mode);
    }

    diagnostics(4, "Exited CmdBox() [%s]", BoxName[code - 1]);
}

void CmdVerb(int code)

/******************************************************************************
 purpose: converts the LaTeX-verb-environment to a similar Rtf-style
 ******************************************************************************/
{
    char cThis;
    char markingchar='#';
    int num;
	
    if (getTexMode() == MODE_VERTICAL)
    	changeTexMode(MODE_HORIZONTAL);
    num = TexFontNumber("Typewriter");
    fprintRTF("{\\b0\\i0\\scaps0\\f%d ", num);

    if (code == VERB_STAR || code == VERB_VERB) {

        while ((cThis = getRawTexChar())) {
            if ((cThis != ' ') && (cThis != '*') && !isalpha((int) cThis)) {
                markingchar = cThis;
                break;
            }
        }
    }


    while ((cThis = getRawTexChar()) && cThis != markingchar)
        putRtfCharEscaped(cThis);

    fprintRTF("}");
}

void CmdVerbatim(int code)

/******************************************************************************
	convert characters 1:1 until \end{verbatim} or \end{Verbatim} is reached
	VERBATIM_1	 for \begin{verbatim} ... \end{verbatim}
	VERBATIM_2   for \begin{Verbatim} ... \end{Verbatim}
******************************************************************************/
{
    char *verbatim_text, *vptr, *endtag=NULL;
    int num;
    int true_code = code & ~ON;

    if (code & ON) {

        diagnostics(4, "Entering CmdVerbatim");

        if (true_code != VERBATIM_4) {

            PushEnvironment(VERBATIM_MODE);
            CmdEndParagraph(0);
            CmdIndent(INDENT_NONE);
            startParagraph("verbatim", GENERIC_PARAGRAPH);
            num = TexFontNumber("Typewriter");
            fprintRTF("\\pard\\ql\\b0\\i0\\scaps0\\f%d ", num);
        }

        switch (true_code) {
            case VERBATIM_1:
                endtag = strdup("\\end{verbatim}");
                break;
            case VERBATIM_2:
                endtag = strdup("\\end{Verbatim}");
                break;
            case VERBATIM_3:
                endtag = strdup("\\end{alltt}");
                break;
            case VERBATIM_4:
                endtag = strdup("\\end{comment}");
                break;
        }

        verbatim_text = getTexUntil(endtag, 1);
        UpdateLineNumber(verbatim_text);
        vptr = verbatim_text;

        if (true_code == VERBATIM_3)
            /* alltt environment */
            ConvertAllttString(verbatim_text);

        else if (true_code == VERBATIM_1 || true_code == VERBATIM_2) {

            show_string(5, verbatim_text, "verbatim");
            putRtfStrEscaped(verbatim_text);
        }

        free(verbatim_text);
        ConvertString(endtag);
        free(endtag);
        

    } else {
        diagnostics(4, "Exiting CmdVerbatim");

        if (true_code != VERBATIM_4) {
        	PopEnvironment();
            CmdEndParagraph(0);
        	CmdVspace(VSPACE_SMALL_SKIP);
        	skipWhiteSpace();
        }
            
        
    }

}

void CmdVerse(int code)

/******************************************************************************
  purpose: converts the LaTeX-Verse-environment to a similar Rtf-style
 ******************************************************************************/
{
    CmdEndParagraph(0);
    switch (code) {
        case ON:
            PushEnvironment(VERSE_MODE);
            CmdIndent(INDENT_USUAL);
            setLeftMarginIndent(getLeftMarginIndent() + 1134);
            setLength("parindent", 0);
            break;
        case OFF:
            PopEnvironment();
            diagnostics(4, "Exiting \\end{verse}");
            CmdIndent(INDENT_INHIBIT);
            CmdVspace(VSPACE_SMALL_SKIP);
            break;
    }
}


void CmdIgnoreDef( /* @unused@ */ int code)

/*****************************************************************************
 purpose: newenvironments or newcommands which are defined by the user aren't
	      converted into Rtf and so they must be ignored
 ******************************************************************************/
{
    char cThis;

    while ((cThis = getTexChar()) && cThis != '{') {
    }

    parseBrace();

}

void TranslateGerman(void)

/***************************************************************************
purpose: called on active german-mode and " character in input file to
	 handle " as an active (meta-)character.
 ***************************************************************************/
{
    char cThis;

    cThis = getTexChar();

    switch (cThis) {
        case 'a':
            fprintRTF("\\'e4");
            break;
        case 'o':
            fprintRTF("\\'f6");
            break;
        case 'u':
            fprintRTF("\\'fc");
            break;
        case 's':
            fprintRTF("\\'df");
            break;
        case 'A':
            fprintRTF("\\'c4");
            break;
        case 'O':
            fprintRTF("\\'d6");
            break;
        case 'U':
            fprintRTF("\\'dc");
            break;
        case '|':
            break;              /* ignore */
        case '-':
            break;              /* ignore */
        case '"':
            break;              /* ignore */
        case '\'':
            fprintRTF("\\ldblquote ");
            break;
        case '`':
            fprintRTF("{\\'84}");
            break;
        case '<':
            break;
        case '>':
            break;
        default:
            fprintRTF("%c", cThis);
    }
}

void GermanPrint(int code)
{
    switch (code) {
        case GP_CK:
            fprintRTF("ck");
            break;
        case GP_LDBL:
            fprintRTF("{\\'84}");
            break;
        case GP_L:
            fprintRTF(",");
            break;
        case GP_R:
            fprintRTF("\\lquote");
            break;
        case GP_RDBL:
            fprintRTF("\\ldblquote");
    }
}


void CmdIgnoreLet(int code)

/******************************************************************************
     purpose : ignore \let 
	   Format: \let\XXXXX = \YYYYYY or \let\XXXXX\YYYYYY
 ******************************************************************************/
{
    char t[100];
    char cThis;
    int i;
    char *s;

	s = getSimpleCommand();
	
	cThis = getNonBlank();
	if (cThis == '=') 
		cThis = getNonBlank();
	
	t[0] = cThis;
	for (i=1; i<100; i++) {
		t[i] = getTexChar();
		if (t[i]==' ' || t[i]=='\n') break;
	}
	t[i]='\0';
	
	newDefinition(s+1,NULL,t,0);
	free(s);	
}

void CmdNewif( /* @unused@ */ int code)

/******************************************************************************
     purpose : ignore \newif\ifsomething
 ******************************************************************************/
{
    char *s;
	s = getSimpleCommand();
	diagnostics(4,"discarding %s",s);
	if (s) free(s);
}

void CmdQuad(int kk)

/******************************************************************************
 purpose: inserts kk quad spaces
 ******************************************************************************/
{
    int z;

    fprintRTF("{");
    for (z = 0; z <= kk; z++)
        fprintRTF("  ");
    fprintRTF("}");
}

void CmdSpace(float kk)

/******************************************************************************
 purpose: inserts a space of width kk*space 
 ******************************************************************************/
{
    int size = CurrentFontSize() * kk;

    if (getTexMode() == MODE_VERTICAL)
    	changeTexMode(MODE_HORIZONTAL);
    fprintRTF("{\\fs%d  }", size);
}

/******************************************************************************
 purpose: handles \kern command
 ******************************************************************************/
void CmdKern(int code)
{
int size = getDimension()*2;  /* size is in quarter-points */

	fprintRTF("\\expnd%d\\expndtw%d ",size,size);
}

void CmdFigure(int code)

/******************************************************************************
  purpose: Process \begin{figure} ... \end{figure} environment
  		   This is only complicated because we need to know what to
  		   label the caption before the caption is processed.  So 
  		   we just slurp the figure environment, extract the tag, and
  		   then process the environment as usual.
 ******************************************************************************/
{
    char *loc, *figure_contents;
    char *endfigure = ((code & ~ON) == FIGURE) ? "\\end{figure}" : "\\end{figure*}";
	static char     oldalignment;
	
    if (code & ON) {
        setCounter("subfigure", 0);
		CmdEndParagraph(0);
		oldalignment = getAlignment();
		setAlignment(JUSTIFIED);

		CmdVspace(VSPACE_BIG_SKIP);
        loc = getBracketParam();
        diagnostics(4, "entering CmdFigure [%s]", (loc) ? loc : "");
        g_processing_figure = TRUE;
        if (loc)
            free(loc);
        figure_contents = getTexUntil(endfigure, TRUE);
        g_figure_label = ExtractLabelTag(figure_contents);
        if (g_endfloat_figures) {
			if (g_endfloat_markers) {
				setAlignment(CENTERED);
				startParagraph("endfloat", GENERIC_PARAGRAPH);
				incrementCounter("endfloatfigure");  /* two separate counters */
				fprintRTF("[");                      /* one for figures and one for */
				ConvertBabelName("FIGURENAME");      /* endfloat figures */
				fprintRTF(" ");
				if (g_document_type != FORMAT_ARTICLE)
					fprintRTF("%d.", getCounter("chapter"));
				fprintRTF("%d about here]", getCounter("endfloatfigure"));
			}
		} else if (g_latex_figures) {
            char *caption, *label;

            caption = ExtractAndRemoveTag("\\caption", figure_contents);
            label = ExtractAndRemoveTag("\\label", figure_contents);

            PrepareDisplayedBitmap("figure");
            WriteLatexAsBitmap("\\begin{figure}", figure_contents, "\\end{figure}");
            FinishDisplayedBitmap();
            ConvertString(caption);
            if (label)
                free(label);
            if (caption)
                free(caption);
        } else {
			startParagraph("figure", GENERIC_PARAGRAPH);
            ConvertString(figure_contents);
        }
        free(figure_contents);
        ConvertString(endfigure);
    } else {
        if (g_figure_label)
            free(g_figure_label);
        g_processing_figure = FALSE;
        diagnostics(4, "exiting CmdFigure");
		setAlignment(oldalignment);
		CmdEndParagraph(0);
		CmdVspace(VSPACE_BIG_SKIP);
    }
}

void CmdSubFigure(int code)
{
	char *caption, *contents;
	
	diagnostics(4,"CmdSubFigure");
	
	caption = getBracketParam();
	contents = getBraceParam();
	
    startParagraph("subfigure", FIRST_PARAGRAPH);
    
    /* insert the figure */
	ConvertString(contents);
    free(contents);
    CmdEndParagraph(0);

	/* now the caption */
	if (caption) {
    	int n;
    	startParagraph("subfigure", FIRST_PARAGRAPH);
        n = getCounter("subfigure");
	    fprintRTF("(%c) ", (char) (n + (int) 'a'));
		ConvertString(caption);
    	CmdEndParagraph(0);
		free(caption);
	}		
    incrementCounter("subfigure");
}

void FixTildes(char *s)
{
    char *p, *p3;

    while ((p = strstr(s, "\\~{}")) != NULL) {
        *p = '~';
        p++;
        p3 = p + 3;
        while (*p3) {
            *p++ = *p3++;
        }
        *p = '\0';
    }
}

void CmdTextColor(int code)

/******************************************************************************
  purpose: hyperlatex support for \textcolor{color}{words to be in color}
******************************************************************************/
{
    char *color, *text, *color1, *text1;
    int n;

    diagnostics(4, "Entering CmdTextColor");
    color1 = getBraceParam();
    text1 = getBraceParam();
    color = strdup_noendblanks(color1);
    text = strdup_noendblanks(text1);
    free(color1);
    free(text1);

    n = -1;
    if (strcmp(color, "black") == 0)
        n = 1;
    else if (strcmp(color, "blue") == 0)
        n = 2;
    else if (strcmp(color, "cyan") == 0)
        n = 3;
    else if (strcmp(color, "green") == 0)
        n = 4;
    else if (strcmp(color, "magenta") == 0)
        n = 5;
    else if (strcmp(color, "red") == 0)
        n = 6;
    else if (strcmp(color, "yellow") == 0)
        n = 7;
    else if (strcmp(color, "white") == 0)
        n = 8;
    else if (strcmp(color, "gray") == 0)
        n = 16;

    if (n > 0) {
        fprintRTF("{\\cf%d ", n);
        ConvertString(text);
        fprintRTF("}");
    }

    free(text);
    free(color);
}

void CmdLink(int code)

/******************************************************************************
  purpose: hyperlatex support for \link{anchor}[ltx]{label}
                              and \xlink{anchor}[printed reference]{URL}
******************************************************************************/
{
    char *anchor, *latex, *url;

    diagnostics(4, "Entering hyperlatex \\link command");
    anchor = getBraceParam();
    latex = getBracketParam();
    url = getBraceParam();

    FixTildes(url);
    fprintRTF("{\\field\\fldedit{\\*\\fldinst { HYPERLINK \"%s\" \\\\* MERGEFORMAT }}", url);
    fprintRTF("{\\fldrslt {\\cs15\\ul\\cf2 ");
    ConvertString(anchor);
    fprintRTF("}}}");

    if (latex)
        free(latex);
    free(anchor);
    free(url);
}

void CmdColumn(int code)

/******************************************************************************
  purpose: chooses between one/two-columns
parameter: number of columns
 globals: twocolumn: true if twocolumn-mode is set
 ******************************************************************************/
{
    switch (code) {
        case One_Column:
            fprintRTF("\\page \\colsx709\\endnhere ");  /* new page & one column */
            twocolumn = FALSE;
            break;
        case Two_Column:
            fprintRTF("\\page \\cols2\\colsx709\\endnhere ");   /* new page & two * columns */
            twocolumn = TRUE;
            break;
    }                           /* switch */
}


void Cmd_OptParam_Without_braces( /* @unused@ */ int code)

/******************************************************************************
 purpose: gets an optional parameter which isn't surrounded by braces but by spaces
 ******************************************************************************/
{
    char cNext = ' ';
    char cLast = ' ';

    do {
        cLast = cNext;
        cNext = getTexChar();
    } while ((cNext != ' ') &&
      (cNext != '\\') &&
      (cNext != '{') && (cNext != '\n') && (cNext != ',') && ((cNext != '.') || (isdigit((int) cLast))) &&
      /* 
       * . doesn't mean the end of an command inside an number of the type
       * real
       */
      (cNext != '}') && (cNext != '\"') && (cNext != '[') && (cNext != '$'));

    ungetTexChar(cNext);
}

void CmdBottom(int code)

/******************************************************************************
  purpose: ignore raggedbottom command
 ******************************************************************************/
{
}

/******************************************************************************
parameter: code: on/off-option
 globals : article and titlepage from the documentstyle
 ******************************************************************************/
void CmdAbstract(int code)
{
    static char oldalignment;
	    	    
    if (code == 3 || code == 2 || code == (1 | ON) ) {
	    CmdEndParagraph(0);
        oldalignment = getAlignment();
        if (g_document_type == FORMAT_REPORT || titlepage)
            CmdNewPage(NewPage);

        startParagraph("abstract", FIRST_PARAGRAPH);		
        fprintRTF("\\qc{\\b ");
        ConvertBabelName("ABSTRACTNAME");
        fprintRTF("}");
        CmdEndParagraph(0);
        setLeftMarginIndent(getLeftMarginIndent() + 1024);
        setRightMarginIndent(getRightMarginIndent() + 1024);
        setAlignment(JUSTIFIED);

    } 
    
    if (code == 2) {
    	char *s = getBraceParam();
    	ConvertString(s);
    	free(s);
    }
    
    if (code == 4 || code == 2 || code == (1 | OFF) ) {
        CmdIndent(INDENT_USUAL);
    	CmdEndParagraph(0);
        setLeftMarginIndent(getLeftMarginIndent() - 1024);
        setRightMarginIndent(getRightMarginIndent() - 1024);
        setAlignment(oldalignment);
        CmdVspace(VSPACE_MEDIUM_SKIP);  /* put \medskip after abstract */
    }
}

void
CmdAcknowledgments(int code)
{
	static char     oldalignment;

	CmdEndParagraph(0);
	
	if (code == ON) {
		
		CmdVspace(VSPACE_BIG_SKIP);
		startParagraph("acknowledgments", GENERIC_PARAGRAPH);
		fprintRTF("\n{\\b ");
		fprintRTF("Acknowledgments"); /* should be in cfg file, but it is not */
		fprintRTF("}\n");
		CmdEndParagraph(0);
		oldalignment = getAlignment();
		setAlignment(JUSTIFIED);

	} else {
		setAlignment(oldalignment);
		CmdVspace(VSPACE_BIG_SKIP);				/* put \medskip after acknowledgments */
	}
}


void 
CmdTitlepage(int code)
/******************************************************************************
  purpose: \begin{titlepage} ... \end{titlepage}
           add pagebreaks before and after this environment
 ******************************************************************************/
{
    CmdNewPage(NewPage);
    switch (code && 0) {
        case ON:
            fprintRTF("\n\\par\\pard \\page "); /* new page */
            fprintRTF("\n\\par\\q%c ", getAlignment());
            break;
        case OFF:
            fprintRTF("\\pard ");
            fprintRTF("\n\\par\\q%c \\page ", getAlignment());
            break;
    }
}

void CmdMinipage(int code)

/******************************************************************************
  purpose: recognize and parse Minipage parameters
  		   currently this does nothing
 ******************************************************************************/
{
    char *v_align, *width;

    switch (code) {
        case ON:
            v_align = getBracketParam();
            width = getBraceParam();
            if (v_align)
                free(v_align);
            free(width);
            break;
        case OFF:
            break;
    }
}

void CmdColsep(int code)

/***************************************************************************
 * purpose: hyperlatex support, handles '&' as in Convert() in convert.c
 only called by \S
 ***************************************************************************/
{
    if (!g_processing_tabular) {
        fprintRTF("{\\'a7}");
        return;
    }

    diagnostics(0, "CmdColsep called");
  /*  actCol++;*/

    if (getTexMode() == MODE_DISPLAYMATH) { /* in an eqnarray or array environment */
        fprintRTF("\\tab ");
    } else {
        fprintRTF("\\cell\\pard\\intbl ");
        /*
        if (colFmt == NULL)
            diagnostics(WARNING, "Fatal, Fatal! CmdColsep called whith colFmt == NULL.");
        else
            fprintRTF("\\q%c ", colFmt[actCol]);
        */
    }
}

void CmdVerbosityLevel(int code)

/***************************************************************************
 * purpose: insert \verbositylevel{5} in the tex file to set the verbosity 
            in the LaTeX file!
 ***************************************************************************/
{
    char *s = getBraceParam();

    g_verbosity_level = atoi(s);
    free(s);

}


/* convert integer to roman number --- only works up correctly up to 39 */

char *roman_item(int n, bool upper)
{
    char s[50];
    int i = 0;

    while (n >= 10) {
        n -= 10;
        s[i] = (upper) ? 'X' : 'x';
        i++;
    }

    if (n == 9) {
        s[i] = (upper) ? 'I' : 'i';
        i++;
        s[i] = (upper) ? 'X' : 'x';
        i++;
        s[i] = '\0';
        return strdup(s);
    }
    if (n >= 5) {
        n -= 5;
        s[i] = (upper) ? 'V' : 'v';
        i++;
    }
    if (n == 4) {
        s[i] = (upper) ? 'I' : 'i';
        i++;
        s[i] = (upper) ? 'V' : 'v';
        i++;
        s[i] = '\0';
        return strdup(s);
    }
    while (n >= 1) {
        n -= 1;
        s[i] = (upper) ? 'I' : 'i';
        i++;
    }

    s[i] = '\0';
    return strdup(s);
}

void CmdNonBreakSpace(int code)
{
	char cThis;
/*    int size = CurrentFontSize() * ((float) code / 100.0);*/

    cThis = getNonSpace();
    ungetTexChar(cThis);

    if (getTexMode() == MODE_VERTICAL)
		changeTexMode(MODE_HORIZONTAL);
/*    fprintRTF("{\\fs%d\\~}", size);*/

	if (code == 100) 
    	fprintRTF("\\~ ");
	else
    	fprintRTF("{\\charscalex%d\\~}", code);
    
}


void CmdIf(int code)

/******************************************************************************
 purpose: handles \ifx by skippint to next \fi
 ******************************************************************************/
{
    char *s = getTexUntil("\\fi", FALSE);

    if (s)
        free(s);
}

/******************************************************************************
 purpose: handles \endinput
 ******************************************************************************/
void CmdEndInput(int code)
{
    PopSource();
}

/******************************************************************************
 purpose: handles \textfont
 ******************************************************************************/
void CmdTextFont(int code)
{

	char *s = getBraceParam();
	free(s);
}

/******************************************************************************
 purpose: ignores \the
 ******************************************************************************/
void CmdThe(int code)
{

}

/******************************************************************************
 purpose: reads \rule[raise-height]{width}{height} 

 The \rule command generates a rectangular "blob of ink."  
 It can be used to produce horizontal or vertical lines. The arguments are
  	raise-height specifies how high to raise the rule (optional)
 	width specifies the length of the rule (mandatory)
 	height specifies the height of the rule (mandatory)
 The default value for raise-height is zero; a negative value lowers the rule.

 The reference point of the rule box is the lower left-hand corner.
 ******************************************************************************/
void CmdRule(int code)
{
	char *raise, *width, *height;
	int dim,n,i;
	
	raise = getBracketParam();
	width = getBraceParam();
	height = getBraceParam();
	
	dim = getStringDimension(width);
	
	n = dim / CurrentFontSize();
	
	for (i=0; i<n; i++) 
		fprintRTF("_");
	
	if (raise) free(raise);
	free(width);
	free(height);
}

/******************************************************************************
  purpose: handles \begin{sloppypar} ... \end{sloppypar} 
                   \begin{landscape} ... \end{landscape} 
  
  This function is used to continue processing the contents of the environment,
  without changing anything.  This is useful when the latex markup has no real
  meaning for the RTF conversion, but the contents of the environment should still
  be processed. 
 ******************************************************************************/
void CmdTolerateEnviron(int code)
{
    if (code == ON) {
            diagnostics(4, "Entering CmdTolerateEnviron \\begin{environ}");
    } 
    
    if (code == OFF) {
            diagnostics(4, "Exiting CmdTolerateEnviron \\end{environ}");
    }
}

/******************************************************************************
  purpose: function to ignore \begin{environ} ... \end{environ}
 ******************************************************************************/
void CmdIgnoreEnviron(int code)
{
    char *endtag = NULL;
    char *s = NULL;

    if (code & ON) {

        switch (code & ~(ON)) {

            case IGNORE_MINIPAGE:
                endtag = strdup("\\end{minipage}");
                break;

            case IGNORE_HTMLONLY:
                endtag = strdup("\\end{htmlonly}");
                break;

            case IGNORE_RAWHTML:
                endtag = strdup("\\end{rawhtml}");
                break;
        }

        if (endtag) {
            s = getTexUntil(endtag, 0);
            ConvertString(endtag);
            if (s)
                free(s);
            free(endtag);
        }
    }
}

/******************************************************************************
  purpose: simple support for \iflatextortf
 ******************************************************************************/
void CmdIflatextortf(int code)
{
	char *entire_if, *else_clause;
	
	/* extract entire if statement, removing comments */
	entire_if = getTexUntil("\\fi", FALSE);
	
	/* look for else */
	else_clause = strstr(entire_if, "\\else");
	if (else_clause != NULL) *else_clause = '\0';
	
	ConvertString(entire_if);
	
	/* put the appropriate piece back for processing */
	diagnostics(2,"CmdIflatextortf clause is");
	diagnostics(2,"       <%s>", entire_if);	
	free(entire_if);
}

