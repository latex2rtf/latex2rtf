
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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

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

char *roman_item(int n, int upper);

static int g_chapter_numbering = ARABIC_NUMBERING;
static int g_section_numbering = ARABIC_NUMBERING;
static int g_appendix = 0;

int g_processing_list_environment = FALSE;

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
        CmdEndParagraph(0);
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
            startParagraph("theorem", PARAGRAPH_FIRST);
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
        diagnostics(5, "\\begin{%s}", s);
        (void) CallParamFunc(s, ON);
    } else {
        diagnostics(5, "\\end{%s}", s);
        (void) CallParamFunc(s, OFF);
    	if (strcmp(s, "setspace") != 0 && strcmp(s, "doublespace") != 0) 
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
    return FormatNumber(g_section_numbering, getCounter("section"));
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
    char *s = NULL;

    if (g_document_type == FORMAT_APA) { /* no numbers in apa at all! */
        return strdup("");
    }
    
    label[0] = '\0';
    if (strcmp(name, "part") == 0) 
        return roman_item(getCounter(name), TRUE);

    if (strcmp(name, "chapter") == 0) 
        return FormatChapter();

    if (strcmp(name, "section") == 0) {
        
        if (g_document_type == FORMAT_ARTICLE) {
            return FormatSection();
        } else {
            s = FormatChapter();
            snprintf(label, 20, "%s.%d", s, getCounter("section"));
        }
    }

    else if (strcmp(name, "subsection") == 0) {
        if (g_document_type == FORMAT_ARTICLE) {
            s = FormatSection();
            snprintf(label, 20, "%s.%d", s, getCounter("subsection"));
        } else {
            s = FormatChapter();
            snprintf(label, 20, "%s.%d.%d", s, getCounter("section"), getCounter("subsection"));
        }
    }

    else if (strcmp(name, "subsubsection") == 0) {
        if (g_document_type == FORMAT_ARTICLE) {
            s = FormatSection();
            snprintf(label, 20, "%s.%d.%d", s, getCounter("subsection"), getCounter("subsubsection"));
        } else {
            s = FormatChapter();
            snprintf(label, 20, "%s.%d.%d.%d", s, getCounter("section"), getCounter("subsection"), 
                                 getCounter("subsubsection"));
        }
    }

    else if (strcmp(name, "paragraph") == 0) {
        if (g_document_type == FORMAT_ARTICLE) {
            s = FormatSection();
            snprintf(label, 20, "%s.%d.%d.%d", s,
              getCounter("subsection"), getCounter("subsubsection"), getCounter("paragraph"));
        } else {
            s = FormatChapter();
            snprintf(label, 20, "%s.%d.%d.%d.%d", s, 
              getCounter("section"), getCounter("subsection"), getCounter("subsubsection"), getCounter("paragraph"));
        }
    }

    else if (strcmp(name, "subparagraph") == 0) {
        if (g_document_type == FORMAT_ARTICLE) {
            s = FormatSection();
            snprintf(label, 20, "%s.%d.%d.%d.%d",s,
              getCounter("subsection"), getCounter("subsubsection"),
              getCounter("paragraph"), getCounter("subparagraph"));
        } else {
            s = FormatChapter();
            snprintf(label, 20, "%s.%d.%d.%d.%d.%d", s,
              getCounter("section"), getCounter("subsection"),
              getCounter("subsubsection"), getCounter("paragraph"), getCounter("subparagraph"));
        }
    }

    if (s) free(s);
    return strdup(label);
}

void CmdAppendix(int code)

/******************************************************************************
  purpose: handles \appendix
 ******************************************************************************/
{
    g_chapter_numbering = ALPHA_NUMBERING;
    if (g_document_type == FORMAT_ARTICLE)
    	g_section_numbering = ALPHA_NUMBERING;
    g_appendix=1;
    setCounter("chapter",0);
    setCounter("section", 0);
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
    
    /*protect current font settings from abuse by titles */

    switch (code) {
        case SECT_PART:
        case SECT_PART_STAR:
            if (getCounter("part") > 0) CmdNewPage(NewPage);
            startParagraph("part", PARAGRAPH_SECTION_TITLE);
            ConvertBabelName("PARTNAME");
            if (code == SECT_PART) {
                incrementCounter("part");
                unit_label = FormatUnitNumber("part");
                fprintRTF(" %s\\par ", unit_label);
                free(unit_label);
            }
            ConvertString(heading);
            CmdEndParagraph(0);
            CmdNewPage(NewPage);
            break;

        case SECT_CHAPTER:
        case SECT_CHAPTER_STAR:
            unit_label = NULL;
            if (getCounter("chapter") > 0) CmdNewPage(NewPage);
            startParagraph("chapter0", PARAGRAPH_SECTION_TITLE);
            if (g_appendix)
            	chapter_name=GetBabelName("APPENDIXNAME");
            else
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
            startParagraph("chapter", PARAGRAPH_SECTION_TITLE);
            ConvertString(heading);
            CmdEndParagraph(0);
/*            InsertContentMark('c', chapter_name, " ", unit_label, " ", heading);*/
            CmdVspace(VSPACE_BIG_SKIP);
            if (unit_label) free(unit_label);
            break;

        case SECT_NORM:
        case SECT_NORM_STAR:
            CmdVspace(VSPACE_BIG_SKIP);
            if (g_document_type == FORMAT_APA) {
                startParagraph("section", PARAGRAPH_SECTION_TITLE);
            } else {            
                startParagraph("section", PARAGRAPH_SECTION_TITLE);
            
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
            CmdEndParagraph(0);
            CmdVspace(VSPACE_SMALL_SKIP);
            break;

        case SECT_SUB:
        case SECT_SUB_STAR:
            CmdVspace(VSPACE_MEDIUM_SKIP);
            if (g_document_type == FORMAT_APA) {
                startParagraph("subsection", PARAGRAPH_SECTION_TITLE);
            } else {            
                startParagraph("subsection", PARAGRAPH_SECTION_TITLE);
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
            CmdVspace(VSPACE_SMALL_SKIP);
            break;

        case SECT_SUBSUB:
        case SECT_SUBSUB_STAR:
            CmdVspace(VSPACE_MEDIUM_SKIP);
            if (g_document_type == FORMAT_APA) {
                startParagraph("subsubsection", PARAGRAPH_GENERIC);
                fprintRTF("{\\i ");
                ConvertString(heading);
                fprintRTF(".} ");
            } else {            
                startParagraph("subsubsection", PARAGRAPH_SECTION_TITLE);
                
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
                ConvertString(heading);
                CmdEndParagraph(0);
                CmdVspace(VSPACE_SMALL_SKIP);
            }
            break;

        case SECT_SUBSUBSUB:
        case SECT_SUBSUBSUB_STAR:
            CmdVspace(VSPACE_MEDIUM_SKIP);
            startParagraph("paragraph", PARAGRAPH_SECTION_TITLE);
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
            CmdVspace(VSPACE_SMALL_SKIP);
            break;

        case SECT_SUBSUBSUBSUB:
        case SECT_SUBSUBSUBSUB_STAR:
            CmdVspace(VSPACE_MEDIUM_SKIP);
            startParagraph("subparagraph", PARAGRAPH_SECTION_TITLE);
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
    startParagraph("caption", PARAGRAPH_FIRST);
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

    } else if (code == COUNTER_NEW) {
        setCounter(s, 0);
	} else if (code == COUNTER_INCR) {
		incrementCounter(s);
	}

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

        getDimension();   /* discard for now */
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
        	setLength("parindent", 15 * DefaultFontSize());
        }
        else {
            PushEnvironment(QUOTATION_MODE);
            diagnostics(4, "Entering \\begin{quote}");
            CmdVspace(VSPACE_SMALL_SKIP);
            setLeftMarginIndent(getLeftMarginIndent()+512);
            setRightMarginIndent(getRightMarginIndent()+512);
        	setLength("parindent", 15 * DefaultFontSize());
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
            CmdIndent(INDENT_INHIBIT);    /* need to reset INDENT_NONE from CmdItem */
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
    static int item_number[5];
    int vspace;

    if (code == RESET_ITEM_COUNTER) {
        if (g_enumerate_depth < 4)
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
        startParagraph("item", PARAGRAPH_LIST);
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
            fprintRTF("\\tab\n");
    }

    switch (code) {
        case ITEMIZE_MODE:
            if (!itemlabel) {
                if (FrenchMode)
                    fprintRTF("\\endash\\tab\n");
                else
                    fprintRTF("\\bullet\\tab\n");
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
                
                default:
                    break;
            }
            if (code != INPARAENUM_MODE)
                fprintRTF("\\tab\n");
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
        CmdIndent(INDENT_INHIBIT);
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
    char BoxName[7][10] = { "hbox", "vbox", "mbox", "fbox", "parbox", "makebox", "framebox" };
    int mode = getTexMode();

    diagnostics(4, "Entering CmdBox() [%s]", BoxName[code - 1]);
/*    if (g_processing_fields)
        g_processing_fields++;   hack to stop fields within fields */

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

    if (code == BOX_MAKEBOX || code == BOX_FRAMEBOX) {
        char *position, *width;

        position = getBracketParam();
        width = getBracketParam();
        if (position)
            free(position);
        if (width) 
            free(width);
    }

    diagnostics(4, "Entering Convert() from CmdBox");
    Convert();
    diagnostics(4, "Exiting Convert() from CmdBox");

/*    if (g_processing_fields)
        g_processing_fields--;*/

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
    VERBATIM_1   for \begin{verbatim} ... \end{verbatim}
    VERBATIM_2   for \begin{Verbatim} ... \end{Verbatim}
******************************************************************************/
{
    char *verbatim_text, *endtag=NULL;
/*  int num;  */
    int true_code = code & ~ON;

    if (code & ON) {

        diagnostics(4, "Entering CmdVerbatim");

        if (true_code != VERBATIM_4) {

            PushEnvironment(VERBATIM_MODE);
            CmdEndParagraph(0);
            CmdIndent(INDENT_NONE);
            startParagraph("verbatim", PARAGRAPH_GENERIC);
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
            fprintRTF("\\'ab");
            break;
        case '>':
            fprintRTF("\\'bb");
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

typedef struct iftag {
    char *if_name;            /* LaTeX newif name without \newif\if */
    int is_true;              /* if this name is set to true */
    int did_push_env;         /* keep track which of 'if' or 'else' is true */
} IfName;

/* ifCommands maintains all the \newif CONDitions */
static int iIfNameCount = 0;   /* number of if condition names */
static IfName ifCommands[100] = {
    {NULL,0, 0}
};

/* ifEnvs/iIfDepth is used to handle nested conditions. */
static IfName ifEnvs[100];
static int iIfDepth = 0;   /* number of nested if conditions */

void CmdNewif( /* @unused@ */ int code)

/******************************************************************************
     purpose : initiate handing of \newif\ifsomething
    : \newif\ifSOMETHING
    :   => create a new ENTRY with SOMETHING with false
    : \SOMETHINGfalse
    :   => set SOMETHING to false
    : \SOMETHINGtrue
    :   => set SOMETHING to true
    : \ifSOMETHING
    :   => if SOMETHING is true, process it.
    : \else
    :   => if SOMETHING is NOT true, process it.
    : \fi
 ******************************************************************************/
{
    char *s;
    s = getSimpleCommand();
    diagnostics(4,"discarding %s",s);
    if(strncmp(s, "\\if", 3) == 0)
    {
        int i;
        for(i = 0; i < iIfNameCount; i++)
        {
            if(strcmp(ifCommands[i].if_name, &s[3]) == 0)
                break;
        }
        if(i < iIfNameCount)
                diagnostics(WARNING, "Duplicated \\newif command '%s'", s); 
        else
        {
            ifCommands[iIfNameCount].if_name = strdup(&s[3]);
            ifCommands[iIfNameCount].is_true = FALSE;
            ifCommands[iIfNameCount].did_push_env = FALSE;
            iIfNameCount++;
        }
    }
    else
        diagnostics(WARNING, "Mystery \\newif command '%s'", s); 
    if (s) free(s);
}

void CmdElse( /* @unused@ */ int code)
{
    iIfDepth--;
    if(ifEnvs[iIfDepth].did_push_env) /* if-closure is true, so else is false */
    {
        fprintRTF("\" }");
        ifEnvs[iIfDepth].did_push_env = FALSE;
    }
    else /* if-closure is false, so else is true */
    {
        ifEnvs[iIfDepth].did_push_env = TRUE;
        fprintRTF("{\\v \"");
    }
    iIfDepth++;
}

void CmdFi( /* @unused@ */ int code)
{
    iIfDepth--;
    if(ifEnvs[iIfDepth].did_push_env)
    {
        fprintRTF("\" }");
    }
}

int TryConditionSet(char *command)
{
    int i;
    if(strncmp(command, "if", 2) == 0)
    {
        for(i = 0; i < iIfNameCount; i++)
        {
            if(strcmp(&command[2], ifCommands[i].if_name) == 0)
        {
                ifEnvs[iIfDepth] = ifCommands[i];
                if(ifCommands[i].is_true)
                {
                    /* no-op */;
                }
                else
                {
                    ifEnvs[iIfDepth].did_push_env = TRUE;
                    fprintRTF("{\\v \"");
                }
                iIfDepth++;
            return TRUE;
        }
    }
    }
    for(i = 0; i < iIfNameCount; i++)
    {
        char *s = ifCommands[i].if_name;
        char *t = strdup_together(s, "true");
        char *f = strdup_together(s, "false");
    if(strcmp(command, t) == 0)
    {
        ifCommands[i].is_true = TRUE;
        free(t); free(f);
        return TRUE;
    }
    else if(strcmp(command, f) == 0)
    {
        ifCommands[iIfNameCount].is_true = FALSE;
        free(t); free(f);
        return TRUE;
    }
    free(t); free(f);
    }
    return FALSE;
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
    int size = (int) (CurrentFontSize() * kk);

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
    char *figure_contents, *lines, *position, *width;
    char endfigure[50];
    static char     oldalignment;
    int real_code = code & ~ON;
    
    switch (real_code) {
        case FIGURE:
            strcpy(endfigure, "\\end{figure}");
            break;
            
        case FIGURE_1:
            strcpy(endfigure, "\\end{figure*}");
            break;
    
        case WRAP_FIGURE:
            strcpy(endfigure, "\\end{wrapfigure}");
            break;
    }
    
    if (code & ON) {
        setCounter("subfigure", 0);
        CmdEndParagraph(0);
        oldalignment = getAlignment();
        setAlignment(JUSTIFIED);

        CmdVspace(VSPACE_BIG_SKIP);
        
        if (real_code == WRAP_FIGURE) {
            lines    = getBracketParam();
            position = getBraceParam();
            width    = getBraceParam();
            if (lines)    free(lines);
            if (position) free(position);
            if (width)    free(width);
        } else {
            char * loc = getBracketParam();
            diagnostics(4, "entering CmdFigure [%s]", (loc) ? loc : "");
            if (loc) free(loc);
        }
        
        g_processing_figure = TRUE;
        figure_contents = getTexUntil(endfigure, TRUE);
        g_figure_label = ExtractLabelTag(figure_contents);
        if (g_endfloat_figures) {
            if (g_endfloat_markers) {
                setAlignment(CENTERED);
                startParagraph("endfloat", PARAGRAPH_GENERIC);
                incrementCounter("endfloatfigure");  /* two separate counters */
                fprintRTF("[");                      /* one for figures and one for */
                ConvertBabelName("FIGURENAME");      /* endfloat figures */
                fprintRTF(" ");
                if (g_document_type != FORMAT_ARTICLE)
                    fprintRTF("%d.", getCounter("chapter"));
                fprintRTF("%d about here]", getCounter("endfloatfigure"));
            }
        } else if (g_latex_figures) {
            char *caption, *new_caption, *label;

            caption = ExtractAndRemoveTag("\\caption", figure_contents);
            label = ExtractAndRemoveTag("\\label", figure_contents);

            PrepareDisplayedBitmap("figure");
            WriteLatexAsBitmapOrEPS("\\begin{figure}", figure_contents, "\\end{figure}", BITMAP);
            FinishDisplayedBitmap();
            new_caption = strdup_together3("\\caption{", caption, "}");
            ConvertString(new_caption);
            safe_free(label);
            safe_free(caption);
            safe_free(new_caption);
        } else {
            startParagraph("figure", PARAGRAPH_GENERIC);
            ConvertString(figure_contents);
        }
        free(figure_contents);
        ConvertString(endfigure);
    } else {
        safe_free(g_figure_label);
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
    
    startParagraph("subfigure", PARAGRAPH_FIRST);
    
    /* insert the figure */
    ConvertString(contents);
    free(contents);
    CmdEndParagraph(0);

    /* now the caption */
    if (caption) {
        int n;
        startParagraph("subfigure", PARAGRAPH_FIRST);
        n = getCounter("subfigure");
        fprintRTF("(%c) ", (char) (n + (int) 'a'));
        ConvertString(caption);
        CmdEndParagraph(0);
        free(caption);
    }       
    incrementCounter("subfigure");
}

static void FixTildes(char *s)
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
            fprintRTF("\\page\n\\colsx709\\endnhere ");  /* new page & one column */
            twocolumn = FALSE;
            break;
        case Two_Column:
            fprintRTF("\\page\n\\cols2\\colsx709\\endnhere ");   /* new page & two * columns */
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
                
    if (code == ABSTRACT_PRELUDE_BEGIN || 
        code == ABSTRACT_SIMPLE        || 
        code == (ABSTRACT_BEGIN_END | ON) ) {
        CmdEndParagraph(0);
        oldalignment = getAlignment();
        if (g_document_type == FORMAT_REPORT || titlepage)
            CmdNewPage(NewPage);

        startParagraph("abstract_title", PARAGRAPH_GENERIC);        
        ConvertBabelName("ABSTRACTNAME");

        setLeftMarginIndent(getLeftMarginIndent() + 1024);
        setRightMarginIndent(getRightMarginIndent() + 1024);
        setAlignment(JUSTIFIED);
        CmdEndParagraph(0);

        CmdIndent(INDENT_USUAL);
        startParagraph("Normal", PARAGRAPH_GENERIC);        
    } 
    
    if (code == ABSTRACT_SIMPLE) {
        char *s = getBraceParam();
        ConvertString(s);
        free(s);
    }
    
    if (code == ABSTRACT_PRELUDE_END || 
        code == (ABSTRACT_BEGIN_END | OFF) ) {
        CmdIndent(INDENT_USUAL);
        CmdEndParagraph(0);
        setLeftMarginIndent(getLeftMarginIndent() - 1024);
        setRightMarginIndent(getRightMarginIndent() - 1024);
        setAlignment(oldalignment);
        CmdVspace(VSPACE_MEDIUM_SKIP);  /* put \medskip after abstract */
    }
}

/*
 * Keywords
 */
void CmdKeywords(int code)
{
    char *keywords = getBraceParam();
    if (NULL != keywords) {
	CmdEndParagraph(0);
	CmdVspace(VSPACE_SMALL_SKIP);
	startParagraph("Normal", PARAGRAPH_FIRST);
	fprintRTF("{{\\b ");
	ConvertBabelName("KEYWORDS");
	fprintRTF("}\\tab\n");
	fprintRTF("%s",keywords);
	fprintRTF("\n}\n");
	CmdEndParagraph(0);
	CmdVspace(VSPACE_SMALL_SKIP);

	free(keywords);
    }
}

void
CmdAcknowledgments(int code)
{
    static char     oldalignment;

    CmdEndParagraph(0);
    
    if (code == ON) {
        
        CmdVspace(VSPACE_BIG_SKIP);
        startParagraph("acknowledgments", PARAGRAPH_GENERIC);
        fprintRTF("\n{\\b ");
        fprintRTF("Acknowledgments"); /* should be in cfg file, but it is not */
        fprintRTF("}\n");
        CmdEndParagraph(0);
        oldalignment = getAlignment();
        setAlignment(JUSTIFIED);

    } else {
        setAlignment(oldalignment);
        CmdVspace(VSPACE_BIG_SKIP);             /* put \medskip after acknowledgments */
    }
}


void 
CmdTitlepage(int code)
/******************************************************************************
  purpose: \begin{titlepage} ... \end{titlepage}
           add pagebreaks before and after this environment
           need to add code to supress page numbering and display
 ******************************************************************************/
{
    CmdNewPage(NewPage);
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

    if (getTexMode() == MODE_DISPLAYMATH || getTexMode() == MODE_MATH) { /* in an eqnarray or array environment */
        fprintRTF("\\tab\n");
    } else {
        fprintRTF("\\cell}{\\pard\\intbl ");
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

char *roman_item(int n, int upper)
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
    safe_free(s);
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
    
    n = dim / CurrentFontSize()/5;
    
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

static int NumForColor(char * color)
{
    int n = -1;
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
    else if (strcmp(color, "Almond") == 0)
        n = 17;
    else if (strcmp(color, "AntiqueBrass") == 0)
        n = 18;
    else if (strcmp(color, "Apricot") == 0)
        n = 19;
    else if (strcmp(color, "Aquamarine") == 0)
        n = 20;
    else if (strcmp(color, "Asparagus") == 0)
        n = 21;
    else if (strcmp(color, "AtomicTangerine") == 0)
        n = 22;
    else if (strcmp(color, "BananaMania") == 0)
        n = 23;
    else if (strcmp(color, "Beaver") == 0)
        n = 24;
    else if (strcmp(color, "Bittersweet") == 0)
        n = 25;
    else if (strcmp(color, "Black") == 0)
        n = 26;
    else if (strcmp(color, "Blue") == 0)
        n = 27;
    else if (strcmp(color, "BlueBell") == 0)
        n = 28;
    else if (strcmp(color, "BlueGreen") == 0)
        n = 29;
    else if (strcmp(color, "BlueViolet") == 0)
        n = 30;
    else if (strcmp(color, "Blush") == 0)
        n = 31;
    else if (strcmp(color, "BrickRed") == 0)
        n = 32;
    else if (strcmp(color, "Brown") == 0)
        n = 33;
    else if (strcmp(color, "BurntOrange") == 0)
        n = 34;
    else if (strcmp(color, "BurntSienna") == 0)
        n = 35;
    else if (strcmp(color, "CadetBlue") == 0)
        n = 36;
    else if (strcmp(color, "Canary") == 0)
        n = 37;
    else if (strcmp(color, "CaribbeanGreen") == 0)
        n = 38;
    else if (strcmp(color, "CarnationPink") == 0)
        n = 39;
    else if (strcmp(color, "Cerise") == 0)
        n = 40;
    else if (strcmp(color, "Cerulean") == 0)
        n = 41;
    else if (strcmp(color, "Chestnut") == 0)
        n = 42;
    else if (strcmp(color, "Copper") == 0)
        n = 43;
    else if (strcmp(color, "Cornflower") == 0)
        n = 44;
    else if (strcmp(color, "CottonCandy") == 0)
        n = 45;
    else if (strcmp(color, "Dandelion") == 0)
        n = 46;
    else if (strcmp(color, "Denim") == 0)
        n = 47;
    else if (strcmp(color, "DesertSand") == 0)
        n = 48;
    else if (strcmp(color, "Eggplant") == 0)
        n = 49;
    else if (strcmp(color, "ElectricLime") == 0)
        n = 50;
    else if (strcmp(color, "Fern") == 0)
        n = 51;
    else if (strcmp(color, "ForestGreen") == 0)
        n = 52;
    else if (strcmp(color, "Fuchsia") == 0)
        n = 53;
    else if (strcmp(color, "FuzzyWuzzyBrown") == 0)
        n = 54;
    else if (strcmp(color, "Gold") == 0)
        n = 55;
    else if (strcmp(color, "Goldenrod") == 0)
        n = 56;
    else if (strcmp(color, "GrannySmithApple") == 0)
        n = 57;
    else if (strcmp(color, "Gray") == 0)
        n = 58;
    else if (strcmp(color, "Green") == 0)
        n = 59;
    else if (strcmp(color, "GreenYellow") == 0)
        n = 60;
    else if (strcmp(color, "HotMagenta") == 0)
        n = 61;
    else if (strcmp(color, "InchWorm") == 0)
        n = 62;
    else if (strcmp(color, "Indigo") == 0)
        n = 63;
    else if (strcmp(color, "JazzberryJam") == 0)
        n = 64;
    else if (strcmp(color, "JungleGreen") == 0)
        n = 65;
    else if (strcmp(color, "LaserLemon") == 0)
        n = 66;
    else if (strcmp(color, "Lavender") == 0)
        n = 67;
    else if (strcmp(color, "MacaroniandCheese") == 0)
        n = 68;
    else if (strcmp(color, "Magenta") == 0)
        n = 69;
    else if (strcmp(color, "Mahogany") == 0)
        n = 70;
    else if (strcmp(color, "Manatee") == 0)
        n = 71;
    else if (strcmp(color, "MangoTango") == 0)
        n = 72;
    else if (strcmp(color, "Maroon") == 0)
        n = 73;
    else if (strcmp(color, "Mauvelous") == 0)
        n = 74;
    else if (strcmp(color, "Melon") == 0)
        n = 75;
    else if (strcmp(color, "MidnightBlue") == 0)
        n = 76;
    else if (strcmp(color, "MountainMeadow") == 0)
        n = 77;
    else if (strcmp(color, "NavyBlue") == 0)
        n = 78;
    else if (strcmp(color, "NeonCarrot") == 0)
        n = 79;
    else if (strcmp(color, "OliveGreen") == 0)
        n = 80;
    else if (strcmp(color, "Orange") == 0)
        n = 81;
    else if (strcmp(color, "Orchid") == 0)
        n = 82;
    else if (strcmp(color, "OuterSpace") == 0)
        n = 83;
    else if (strcmp(color, "OutrageousOrange") == 0)
        n = 84;
    else if (strcmp(color, "PacificBlue") == 0)
        n = 85;
    else if (strcmp(color, "Peach") == 0)
        n = 86;
    else if (strcmp(color, "Periwinkle") == 0)
        n = 87;
    else if (strcmp(color, "PiggyPink") == 0)
        n = 88;
    else if (strcmp(color, "PineGreen") == 0)
        n = 89;
    else if (strcmp(color, "PinkFlamingo") == 0)
        n = 90;
    else if (strcmp(color, "PinkSherbet") == 0)
        n = 91;
    else if (strcmp(color, "Plum") == 0)
        n = 92;
    else if (strcmp(color, "PurpleHeart") == 0)
        n = 93;
    else if (strcmp(color, "PurpleMountainMajesty") == 0)
        n = 94;
    else if (strcmp(color, "PurplePizzazz") == 0)
        n = 95;
    else if (strcmp(color, "RadicalRed") == 0)
        n = 96;
    else if (strcmp(color, "RawSienna") == 0)
        n = 97;
    else if (strcmp(color, "RazzleDazzleRose") == 0)
        n = 98;
    else if (strcmp(color, "Razzmatazz") == 0)
        n = 99;
    else if (strcmp(color, "Red") == 0)
        n = 100;
    else if (strcmp(color, "RedOrange") == 0)
        n = 101;
    else if (strcmp(color, "RedViolet") == 0)
        n = 102;
    else if (strcmp(color, "RobinEggBlue") == 0)
        n = 103;
    else if (strcmp(color, "RoyalPurple") == 0)
        n = 104;
    else if (strcmp(color, "Salmon") == 0)
        n = 105;
    else if (strcmp(color, "Scarlet") == 0)
        n = 106;
    else if (strcmp(color, "ScreaminGreen") == 0)
        n = 107;
    else if (strcmp(color, "SeaGreen") == 0)
        n = 108;
    else if (strcmp(color, "Sepia") == 0)
        n = 109;
    else if (strcmp(color, "Shadow") == 0)
        n = 110;
    else if (strcmp(color, "Shamrock") == 0)
        n = 111;
    else if (strcmp(color, "ShockingPink") == 0)
        n = 112;
    else if (strcmp(color, "Silver") == 0)
        n = 113;
    else if (strcmp(color, "SkyBlue") == 0)
        n = 114;
    else if (strcmp(color, "SpringGreen") == 0)
        n = 115;
    else if (strcmp(color, "Sunglow") == 0)
        n = 116;
    else if (strcmp(color, "SunsetOrange") == 0)
        n = 117;
    else if (strcmp(color, "Tan") == 0)
        n = 118;
    else if (strcmp(color, "TickleMePink") == 0)
        n = 119;
    else if (strcmp(color, "Timberwolf") == 0)
        n = 120;
    else if (strcmp(color, "TropicalRainForest") == 0)
        n = 121;
    else if (strcmp(color, "Tumbleweed") == 0)
        n = 122;
    else if (strcmp(color, "TurquoiseBlue") == 0)
        n = 123;
    else if (strcmp(color, "UnmellowYellow") == 0)
        n = 124;
    else if (strcmp(color, "Violet(Purple)") == 0)
        n = 125;
    else if (strcmp(color, "VioletRed") == 0)
        n = 126;
    else if (strcmp(color, "VividTangerine") == 0)
        n = 127;
    else if (strcmp(color, "VividViolet") == 0)
        n = 128;
    else if (strcmp(color, "White") == 0)
        n = 129;
    else if (strcmp(color, "WildBlueYonder") == 0)
        n = 130;
    else if (strcmp(color, "WildStrawberry") == 0)
        n = 131;
    else if (strcmp(color, "WildWatermelon") == 0)
        n = 132;
    else if (strcmp(color, "Wisteria") == 0)
        n = 133;
    else if (strcmp(color, "Yellow") == 0)
        n = 134;
    else if (strcmp(color, "YellowGreen") == 0)
        n = 135;
    else if (strcmp(color, "YellowOrange") == 0)
        n = 136;
    return n;
}

/******************************************************************************
  purpose: support for \color{thecolor}  and \textcolor{color}{words to be in color}
  horrible implementation ... but who uses color anyhow??
******************************************************************************/

void CmdTextColor(int code)
{
    char *color, *text, *color1, *text1;
    int n;

    diagnostics(4, "Entering CmdTextColor");
    color1 = getBraceParam();
    color = strdup_noendblanks(color1);
    n = NumForColor(color);
    free(color1);
    free(color);
    
    if (code) {  /* non-zero code indicates \textcolor */
        text1 = getBraceParam();
        text = strdup_noendblanks(text1);
        fprintRTF("{");
        if (n > 0) fprintRTF("\\cf%d ", n);
        ConvertString(text);
        fprintRTF("}");
        free(text);
        free(text1);
    } else {
        if (n > 0) fprintRTF("\\cf%d ", n); 
    }
    
}

/******************************************************************************
  purpose: parse \rlap{text} or \llap{text} , but ignore spacing changes
******************************************************************************/
void CmdLap(int code)
{
    char *s = getBraceParam();
    ConvertString(s);
    free(s);
}
