/* $Id: funct1.c,v 1.30 2001/10/12 05:45:07 prahl Exp $ 
 
This file contains routines that interpret various LaTeX commands and produce RTF

Authors:  Dorner, Granzer, Polzer, Trisko, Schlatterbeck, Lehner, Prahl
*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "convert.h"
#include "funct1.h"
#include "commands.h"
#include "stack.h"
#include "l2r_fonts.h"
#include "cfg.h"
#include "ignore.h"
#include "util.h"
#include "encode.h"
#include "parser.h"
#include "counters.h"
#include "lengths.h"
#include "preamble.h"

extern bool     twocolumn;	/* true if twocolumn-mode is enabled */
extern int      indent;		/* includes the left margin e.g. for itemize-commands */
extern enum     TexCharSetKind TexCharSet;
int 			indent_right;

static void     CmdLabel1_4(int code, char *text);
static void     CmdLabelOld(int code, char *text);
void            CmdPagestyle( /* @unused@ */ int code);
void            CmdHeader(int code);

static bool  g_paragraph_no_indent = FALSE;
static bool  g_paragraph_inhibit_indent = FALSE;

void
CmdStartParagraph(int code)
/******************************************************************************
     purpose : RTF codes to create a new paragraph.  If the paragraph should
               not be indented then emit \fi0 otherwise use the current value
               of \parindent as the indentation of the first line.
 ******************************************************************************/
{
	int parindent;
	
	diagnostics(4,"CmdStartParagraph mode = %d", GetTexMode());
	diagnostics(4,"Noindent is %d", (int) g_paragraph_no_indent);
	diagnostics(4,"Inhibit  is %d", (int) g_paragraph_inhibit_indent);
	diagnostics(4,"parindent  is %d", getLength("parindent"));

	fprintRTF("\\q%c\\li%d ", alignment, indent);
	if (indent!=0)
		fprintRTF("\\li%d ", indent);
	if (indent_right!=0)
		fprintRTF("\\ri%d ", indent_right);

	if (g_paragraph_no_indent || g_paragraph_inhibit_indent) 
		parindent = 0;
	else
		parindent = getLength("parindent");
	
	fprintRTF("\\fi%d ", parindent);
	
	SetTexMode(MODE_HORIZONTAL);
	g_paragraph_no_indent = FALSE;
	g_paragraph_inhibit_indent = FALSE;
}

void
CmdEndParagraph(int code)
/******************************************************************************
     purpose : ends the current paragraph and return to MODE_VERTICAL.
 ******************************************************************************/
{
	int mode = GetTexMode();

	diagnostics(4,"CmdEndParagraph mode = %d", GetTexMode());
	if (mode != MODE_VERTICAL) {
		fprintRTF("\\par\n");
		SetTexMode(MODE_VERTICAL);
	}
	g_paragraph_inhibit_indent = FALSE;
}

void
CmdVspace(int code)
/******************************************************************************
     purpose : vspace, vspace*, and vskip
     		   code ==  0 if vspace or vspace*
     		   code == -1 if vskip
     		   code ==  1 if \smallskip
     		   code ==  2 if \medskip
     		   code ==  3 if \bigskip
 ******************************************************************************/
{
	int vspace;
	char c;
	int mode = GetTexMode();
	
	switch (code) {
		case -1 :
			vspace = getDimension();
			break;
			
		case 0 :
			while ((c = getTexChar()) && c != '{');
			vspace = getDimension();
			parseBrace();
			break;
			
		case 1:
			vspace = getLength("smallskipamount");
			break;
			
		case 2:
			vspace = getLength("medskipamount");
			break;
	
		case 3:
			vspace = getLength("bigskipamount");
			break;
	}

	diagnostics(4,"CmdVspace mode = %d, vspace=%d", GetTexMode(), vspace);

	if (vspace<0) vspace =0;
	fprintRTF("\\sa%d ", vspace);
	if (mode == MODE_VERTICAL) 			
		fprintRTF("\\par ");		/* forces \sa to take effect */
	else {
		CmdEndParagraph(0);
		CmdIndent(INDENT_INHIBIT);
	}
	fprintRTF("\\sa0");

}

void
CmdIndent(int code)
/******************************************************************************
 purpose : set flags so that CmdStartParagraph() does the right thing
     
     	   INDENT_INHIBIT allows the next paragraph to be indented if
     	   a paragraph break occurs before CmdStartParagraph() is called
     			     		
           INDENT_NONE tells CmdStartParagraph() to not indent the next paragraph
           
           INDENT_USUAL has CmdStartParagraph() uses the value of \parindent
 ******************************************************************************/
{
	diagnostics(4,"CmdIndent mode = %d", GetTexMode());
	if (code == INDENT_NONE)
		g_paragraph_no_indent = TRUE;
	
	else if (code == INDENT_INHIBIT) 
		g_paragraph_inhibit_indent = TRUE;

	else if (code == INDENT_USUAL) {
		g_paragraph_no_indent = FALSE;
		g_paragraph_inhibit_indent = FALSE;
	}
	diagnostics(4,"Noindent is %d", (int) g_paragraph_no_indent);
	diagnostics(4,"Inhibit  is %d", (int) g_paragraph_inhibit_indent);
}

void 
CmdBeginEnd(int code)
/***************************************************************************
 purpose: reads the parameter after the \begin or \end-command; ( see also getParam )
	      after reading the parameter the CallParamFunc-function calls the
	      handling-routine for that special environment
 parameter: code: CMD_BEGIN: start of environment
		          CMD_END:   end of environment
 ***************************************************************************/
{
/*	char            c;*/
	char           *s = getParam();

	switch (code) {
	case CMD_BEGIN:
		(void) CallParamFunc(s, ON);
		break;
	case CMD_END:
		(void) CallParamFunc(s, OFF);
		CmdIndent(INDENT_INHIBIT);
		break;
	default:
		assert(0);
	}
	free(s);
}

void 
CmdAlign(int code)
/*****************************************************************************
    purpose : sets the alignment for a paragraph
  parameter : code: alignment centered, justified, left or right
     globals: alignment: alignment of paragraphs
 ********************************************************************************/
{
	char * s;
	static char     old_alignment_before_center = JUSTIFIED;
	static char     old_alignment_before_right = JUSTIFIED;
	static char     old_alignment_before_left = JUSTIFIED;
	static char     old_alignment_before_centerline = JUSTIFIED;

	if (code == PAR_VCENTER){
		s = getParam();
		free(s);
		return;
	}
	
	CmdEndParagraph(0);
	switch (code) {
	case (PAR_CENTERLINE):
		old_alignment_before_centerline = alignment;
		alignment = CENTERED;
		fprintRTF("{");
		diagnostics(4,"Entering Convert from CmdAlign (centerline)");
		Convert();
		diagnostics(4,"Exiting Convert from CmdAlign (centerline)");
		alignment = old_alignment_before_centerline;
		CmdEndParagraph(0);
		fprintRTF("}");
		break;


	case (PAR_CENTER | ON):
		CmdIndent(INDENT_NONE);
		old_alignment_before_center = alignment;
		alignment = CENTERED;
		break;
	case (PAR_CENTER | OFF):
		alignment = old_alignment_before_center;
		CmdEndParagraph(0);
		CmdIndent(INDENT_INHIBIT);
		break;

	case (PAR_RIGHT | ON):
		old_alignment_before_right = alignment;
		alignment = RIGHT;
		CmdIndent(INDENT_NONE);
		break;
	case (PAR_RIGHT | OFF):
		alignment = old_alignment_before_right;
		CmdIndent(INDENT_INHIBIT);
		break;

	case (PAR_LEFT | ON):
		old_alignment_before_left = alignment;
		alignment = LEFT;
		CmdIndent(INDENT_NONE);
		break;
	case (PAR_LEFT | OFF):
		alignment = old_alignment_before_left;
		CmdIndent(INDENT_INHIBIT);
		break;
	}
}

void 
CmdToday( /* @unused@ */ int code)
/******************************************************************************
    purpose: converts the LaTeX-date-command into a Rtf-chdate-command which
	     prints the current date into an document
 ******************************************************************************/
{
	fprintRTF("\\chdate ");
}


void 
CmdIgnore( /* @unused@ */ int code)
/******************************************************************************
 purpose: LaTeX-commands which can't be converted in Rtf-commands are overread
	  as such
 ******************************************************************************/
{
}

void 
CmdLdots( /* @unused@ */ int code)
/******************************************************************************
 purpose: converts the LaTeX-\ldots-command into "..." in Rtf
 globals : fRtf
 ******************************************************************************/
{
	fprintRTF("...");
}

void 
Environment(int code)
/******************************************************************************
  purpose: pushes/pops the new environment-commands on/from the stack
parameter: code includes the type of the environment
 ******************************************************************************/
{
	if (code & ON) {
		code &= ~(ON);	/* mask MSB */
		diagnostics(4,"Entering Environment (%d)", code);		
		PushEnvironment(code);
	} else {		/* off switch */
		CmdEndParagraph(0);
		diagnostics(4,"Exiting  Environment (%d)", code);
		PopEnvironment();
	}
}

void 
CmdSection(int code)
/******************************************************************************
  purpose: converts the LaTeX-section-commands into similar Rtf-styles
parameter: code: type of section-recursion-level
 ******************************************************************************/
{
	char            optparam[100] = "";
	char            *heading;
/*	char			c;
	int				DefFont = DefaultFontFamily();
*/	
	getBracketParam(optparam, 99);
	heading = getParam();
	diagnostics(4,"entering CmdSection [%s]{%s}",optparam,heading);
	
	CmdEndParagraph(0);
	CmdIndent(INDENT_NONE);
	CmdStartParagraph(0);
	
	switch (code) {
	case SECT_PART:
		incrementCounter("part");
		fprintRTF("\\page");
		fprintRTF("{\\qc\\b\\fs40 ");
		ConvertBabelName("PARTNAME");
		fprintRTF("%d\\par ", getCounter("part"));
		break;

	case SECT_CHAPTER:
		incrementCounter("chapter");
		setCounter("section",0);
		setCounter("subsection",0);
		setCounter("subsubsection",0);
/*		fprintRTF("\\pard\\page\\pard{\\pntext\\pard\\plain\\b\\fs32\\kerning28 ");
		fprintRTF("%d\\par \\par }\\pard\\plain\n", getCounter("chapter"));
		fprintRTF("%s\\f%d%s{", HEADER11, DefFont, HEADER12);
*/
		fprintRTF("\\page{\\plain\\b\\fs32\\kerning28 ");
		ConvertBabelName("CHAPTERNAME");
		fprintRTF(" %d\\par}", getCounter("chapter"));
		fprintRTF("{\\fi0\\plain\\b\\fs40\\kerning28 ");
		break;

	case SECT_NORM:
		incrementCounter("section");
		setCounter("subsection",0);
		setCounter("subsubsection",0);
/*
		fprintRTF("{\\pard\\pntext\\plain\\b");
		if (g_document_type == FORMAT_ARTICLE) {
			fprintRTF("\\fs32\\kerning28 %d\\tab}\\pard\\plain\n", getCounter("section"));
			fprintRTF("%s%s{", HEADER11, HEADER12);
		} else {
			fprintRTF("\\fs24 %d.%d\\tab}\\pard\\plain\n", getCounter("chapter"),getCounter("section"));
			fprintRTF("%s%s{", HEADER21, HEADER22);
		}
*/
		fprintRTF("{\\plain\\b");
		if (g_document_type == FORMAT_ARTICLE) {
			fprintRTF("\\fs32 %d.  ", getCounter("section"));
		} else {
			fprintRTF("\\fs24 %d.%d  ", getCounter("chapter"),getCounter("section"));
		}

		break;

	case SECT_SUB:
		incrementCounter("subsection");
		setCounter("subsubsection",0);
/*		fprintRTF("{\\par\\pard\\pntext\\pard\\plain\\b");
		if (g_document_type == FORMAT_ARTICLE) {
			fprintRTF("\\fs24 %d.%d\\tab}\\pard\\plain\n", getCounter("section"), getCounter("subsection"));
			fprintRTF("%s\\f%d%s{", HEADER21, DefFont, HEADER22);
		} else {
			fprintRTF("\\fs24 %d.%d.%d\\tab}\\pard\\plain\n", getCounter("chapter"), getCounter("section"), getCounter("subsection"));
			fprintRTF("%s\\f%d%s{", HEADER31, DefFont, HEADER32);
		}
*/
		fprintRTF("{\\plain\\b\\fs24 ");
		if (g_document_type != FORMAT_ARTICLE) 
			fprintRTF("%d.", getCounter("chapter"));
		fprintRTF("%d.%d  ", getCounter("section"), getCounter("subsection"));
		break;

	case SECT_SUBSUB:
		incrementCounter("subsubsection");
/*		fprintRTF("{\\par\\pard\\pntext\\pard\\plain\\b");
		if (g_document_type == FORMAT_ARTICLE) {
			fprintRTF("\\fs24 %d.%d.%d\\tab}\\pard\\plain\n", getCounter("section"), getCounter("subsection"), getCounter("subsubsection"));
			fprintRTF("%s\\f%d%s{", HEADER31, DefFont, HEADER32);
		} else {
			fprintRTF("\\fs24 %d.%d.%d.%d\\tab}\\pard\\plain\n", getCounter("chapter"), getCounter("section"), getCounter("subsection"), getCounter("subsubsection"));
			fprintRTF("%s\\f%d%s{", HEADER41, DefFont, HEADER42);
		}
*/
		fprintRTF("{\\plain\\b\\fs24 ");
		if (g_document_type != FORMAT_ARTICLE) 
			fprintRTF("%d.", getCounter("chapter"));
		fprintRTF("%d.%d", getCounter("section"), getCounter("subsection"));
		fprintRTF(".%d  ", getCounter("subsubsection"));
		break;
	}
	
	ConvertString(heading);
	CmdEndParagraph(0);
	fprintRTF("}");
	if (code == SECT_PART)
		fprintRTF("\\page");

	if (heading) free(heading);
	CmdIndent(INDENT_NONE);
}


void
CmdCaption(int code)
/******************************************************************************
 purpose: converts \caption from LaTeX to Rtf
 ******************************************************************************/
{
	char           *thecaption;
	char            lst_entry[300];
	int				n;
	char   			old_align;
	
	old_align = alignment;
	alignment = CENTERED;

	getBracketParam(lst_entry,299);  /* discard entry for list of tables or figures */
	
	diagnostics(4, "entering CmdCaption [%s]", lst_entry);

	CmdEndParagraph(0);
	CmdIndent(INDENT_NONE);
	CmdStartParagraph(0);
	fprintRTF("{");

	if (g_processing_figure) {
		incrementCounter("figure");
		ConvertBabelName("FIGURENAME");
		n = getCounter("figure");
	} else {
		incrementCounter("table");
		ConvertBabelName("TABLENAME");
		n = getCounter("table");
	}

	fprintRTF(" ");
	if (g_document_type != FORMAT_ARTICLE) 
		fprintRTF("%d.", getCounter("chapter"));
	fprintRTF("%d:  ", n);

	thecaption = getParam();
	diagnostics(4, "in CmdCaption [%s]", thecaption);
	ConvertString(thecaption);
	free(thecaption);
	fprintRTF("}");
	CmdEndParagraph(0);
	alignment = old_align;
	diagnostics(4, "exiting CmdCaption");
}

void
CmdCounter(int code)
/******************************************************************************
 purpose: handles \newcounter, \setcounter, \addtocounter, \value
 ******************************************************************************/
{
	char            *s, *s2, *s3, *s4;
	int              num;
	
	s = getParam();
		
	diagnostics(4,"Entering CmdCounter(), <%s>", s);
	
	if (code == COUNTER_ADD || code == COUNTER_SET) {
	
		s2 = getParam();

		if ((s3 = strchr(s2,'{')) && (s4 = strchr(s2,'}')) ) {
			s3++;
			*s4 = '\0';
			diagnostics(4,"CmdCounter() processing \\value{%s}", s3);
			num = getCounter(s3);

		} else if (sscanf(s2, "%d", &num) != 1) {

			fprintf(stderr, "\nBad parameter in set/addcounter{%s}{%s}\n", s,s2);
			free(s2);
			free(s);
			return;
		}
		
		free(s2);
		if (code == COUNTER_ADD)
			setCounter(s, getCounter(s)+num);
		else
			setCounter(s, num);
			
	} else if (code == COUNTER_NEW)
		setCounter(s,0);
		
	free(s);
}

void
CmdLength(int code)
/******************************************************************************
 purpose: handles \newlength, \setlength, \addtolength
 purpose: handle \textwidth=0.8in or \moveright0.1\textwidth
 ******************************************************************************/
{
	char            *s, *s1;
	char             cThis;
	int              num;
	

	if (code == LENGTH_ADD || code == LENGTH_SET || code == LENGTH_NEW) {
		s = getParam();
		if (strlen(s)<=1) {
			free(s);
			fprintf(stderr,"missing argument in \\newlength \\addtolength or \\setlength");
			return;
		}
		s1 = s + 1; /* skip initial '//' */
		diagnostics(4,"Entering CmdLength(), <%s>", s1);
		if (code == LENGTH_ADD || code == LENGTH_SET) {
		
			cThis = getNonSpace();
			
			if (cThis=='{') {
				num = getDimension();
				while ((cThis=getTexChar()) != '}');
						
				if (code == LENGTH_ADD)
					setLength(s1, getLength(s1)+num);
				else
					setLength(s1, num);
			} else 
				fprintf(stderr,"bad parameter to \\addtolength or \\setlength");
				
		} else
			setLength(s1,0);
			
		free(s);
	
	} else {
		skipSpaces();
		cThis = getTexChar();
		
		if (cThis == '=')			/* optional '=' */
			skipSpaces();
		else
			ungetTexChar(cThis);
			
		num = getDimension();      /* discard for now */
	}
}

void 
CmdFootNote(int code)
/******************************************************************************
 purpose: converts footnotes from LaTeX to Rtf
 params : code specifies whether it is a footnote or a thanks-mark
 ******************************************************************************/
{
	char            number[255];
	static int      thankno = 1;
	int             text_ref_upsize, foot_ref_upsize;
	int				DefFont = DefaultFontFamily();
	
	diagnostics(4,"Entering ConvertFootNote");
	getBracketParam(number, 254);	/* is ignored because of the
					 * automatic footnumber-generation */

	text_ref_upsize = (6 * CurrentFontSize()) / 20;
	foot_ref_upsize = (6 * CurrentFontSize()) / 20;

	if (code == THANKS) {
		thankno++;
		fprintRTF("{\\up%d %d}\n", text_ref_upsize, thankno);
		fprintRTF("{\\*\\footnote \\pard\\plain\\s246\\f%d",DefFont);
		fprintRTF("\\fs%d {\\up%d %d}", CurrentFontSize(), foot_ref_upsize, thankno);
	} else {
		fprintRTF("{\\up%d\\chftn}\n", text_ref_upsize);
		fprintRTF("{\\*\\footnote \\pard\\plain\\s246\\f%d",DefFont);
		fprintRTF("\\fs%d {\\up%d\\chftn}", CurrentFontSize(), foot_ref_upsize);
	}

	Convert();
	diagnostics(4,"Exiting CmdFootNote");
	fprintRTF("}\n ");
}

void 
CmdQuote(int code)
/******************************************************************************
  purpose: handles \begin{quote} ... \end{quote} 
                   \begin{quotation} ... \end{quotation}
  globals:   indent which is the left-indent-position
 ******************************************************************************/
{
	CmdEndParagraph(0);

	switch (code) {
	case (QUOTATION | ON):
		case (QUOTE | ON):
		diagnostics(4,"Entering CmdQuote");
		indent += 512;
		CmdIndent(INDENT_NONE);
		break;
		
	case (QUOTATION | OFF):
	case (QUOTE | OFF):
		diagnostics(4,"Exiting CmdQuote");
		CmdIndent(INDENT_INHIBIT);
		indent -= 512;
	}
}

void 
CmdList(int code)
/******************************************************************************
  purpose : set indentation and counters for itemize, description and enumerate
 globals  : indent
 ******************************************************************************/
{
	CmdEndParagraph(0);
	CmdIndent(INDENT_INHIBIT);

	switch (code) {
		case (ITEMIZE | ON):
		PushEnvironment(ITEMIZE);
		indent += 512;
		break;
	case (ITEMIZE | OFF):
		PopEnvironment();
		indent -= 512;
		break;

	case (ENUMERATE | ON):
		PushEnvironment(ENUMERATE);
		g_enumerate_depth++;
		CmdItem(RESET_ITEM_COUNTER);
		indent += 512;
		break;
	case (ENUMERATE | OFF):
		PopEnvironment();
		g_enumerate_depth--;
		indent -= 512;
		break;

	case (DESCRIPTION | ON):
		PushEnvironment(DESCRIPTION);
		indent += 512;
		break;
	case (DESCRIPTION | OFF):
		PopEnvironment();
		indent -= 512;
		break;
	}
}

void 
CmdItem(int code)
/******************************************************************************
 purpose : handles \item command.  Since the \item command is delimited by 
           a later \item command or the ending of an environment (\end{itemize})
           this routine will get called recursively.
 ******************************************************************************/
{
	char            itemlabel[100];
	static int      item_number[4];

	if (code == RESET_ITEM_COUNTER) {
		item_number[g_enumerate_depth] = 1;
		return;
	}

	diagnostics(4, "Entering CmdItem depth=%d item=%d",g_enumerate_depth,item_number[g_enumerate_depth]);

	CmdEndParagraph(0);
	CmdIndent(INDENT_NONE);
	CmdStartParagraph(0);
	
	if (getBracketParam(itemlabel, 99)) {	/* \item[label] */

		fprintRTF("{\\b ");	/* bold on */
		diagnostics(5,"Entering ConvertString from CmdItem");
		ConvertString(itemlabel);
		diagnostics(5,"Exiting ConvertString from CmdItem");
		fprintRTF("}\\tab ");	/* bold off */
		
	}
	
	switch (code) {
	case ITEMIZE:
		fprintRTF("\\bullet\\tab ");
		break;

	case ENUMERATE:
		switch (g_enumerate_depth) {
		case 1:
			fprintRTF("%d.", item_number[g_enumerate_depth]);
			break;

		case 2:
			fprintRTF("(%c)", 'a' + item_number[g_enumerate_depth] - 1);
			break;

		case 3:
			roman_item(item_number[g_enumerate_depth], itemlabel);
			fprintRTF("%s.", itemlabel);
			break;

		case 4:
			fprintRTF("%c.", 'A' + item_number[g_enumerate_depth] - 1);
			break;
		}
		fprintRTF("\\tab ");
		item_number[g_enumerate_depth]++;
		break;

	case DESCRIPTION:
		fprintRTF("\\tab ");	/* indent */
		break;
	}
	
	Convert();
	CmdEndParagraph(0);
	CmdIndent(INDENT_NONE);
	diagnostics(4, "Exiting Convert() from CmdItem");
}

void 
CmdBox(int code)
/******************************************************************************
  purpose: converts the LaTeX \box-commands into  an similar Rtf-style
 ******************************************************************************/
{
	int mode = GetTexMode();
	
	diagnostics(4, "Entering CmdBox()");
	if (mode == MODE_MATH || mode == MODE_DISPLAYMATH)
		fprintRTF("{\\i0 ");	/* brace level without math italics */
	
	SetTexMode(MODE_RESTRICTED_HORIZONTAL);
	diagnostics(4, "Entering Convert() from CmdBox");
	Convert();
	diagnostics(4, "Exiting Convert() from CmdBox");
	
	if (mode == MODE_MATH || mode == MODE_DISPLAYMATH)
		fprintRTF("}");	/* brace level without math italics */

	SetTexMode(mode);
	diagnostics(4, "Exited CmdBox()");
}

void
CmdInclude(int code)
/******************************************************************************
 purpose: reads an extern-LaTeX-File from the into the actual document and converts it to
	  an similar Rtf-style
 globals: GermanMode: is set if germanstyles are included
          fTex
          progname
 ******************************************************************************/
{
	char            *fname;
	FILE *fp;
	FILE *LatexFile;
	char           *olatexname;
	long            oldlinenumber;

	strcpy(fname, "");

	fname = getParam();
	if (strstr(fname, "german.sty") != NULL) {
		GermanMode = TRUE;
		PushEnvironment(GERMAN_MODE);
		return;
	}

	if (strstr(fname, "french.sty") != NULL) {
		PushEnvironment(FRENCH_MODE);
		return;
	}
	assert(fname != NULL);

	if (strcmp(fname, "") == 0) {
		diagnostics(WARNING, "Empty or invalid filename in \\include{filename}");
		return;
	}
	/* extension .tex is appended automatically */
	if (strchr(fname, '.') == NULL)
		strcat(fname, ".tex");

/* 
   this is needed in classic MacOS because of the weird way that directories
   are handled under DropUnix
*/
#ifdef __MWERKS__
	{
	char            fullpath[1024];
		char           *dp;
		strcpy(fullpath, latexname);
		dp = strrchr(fullpath, ':');
		if (dp != NULL) {
			dp++;
			*dp = '\0';
		} else
			strcpy(fullpath, "");
		strcat(fullpath, fname);
		strcpy(fname, fullpath);
	}
#endif

	if (g_processing_include) {
		diagnostics(WARNING, "Cannot process nested \\include{%s}", latexname);
		return;
	}
	g_processing_include = TRUE;

	if ((fp = (fopen(fname, "rb"))) == NULL) {
		diagnostics(WARNING, "Cannot open \\include file: %s", latexname);
		return;
	}
	fprintf(stderr, "\nProcessing include file: %s\n", latexname);

	LatexFile = fTex;	/* Save current file pointer */
	diagnostics(5, "changing fTex file pointer in CmdInclude");
	fTex = fp;
	oldlinenumber = linenumber;
	linenumber = 1;
	olatexname = latexname;
	latexname = fname;

	diagnostics(4, "Entering Convert() from CmdInclude");
	Convert();
	diagnostics(4, "Exiting Convert() from CmdInclude");

	if (fclose(fp) != 0)
		diagnostics(ERROR, "Could not close include file.");

	diagnostics(5, "resetting fTex file pointer in CmdInclude");
	fTex = LatexFile;
	latexname = olatexname;
	linenumber = oldlinenumber;
	g_processing_include = FALSE;
	free(fname);
}

void 
CmdVerb(int code)
/******************************************************************************
 purpose: converts the LaTeX-verb-environment to a similar Rtf-style
 ******************************************************************************/
{
	char            cThis;
	char            markingchar = '\177';
	int             num;

	while ((cThis = getRawTexChar())) {
		if ((cThis != ' ') && (cThis != '*') && !isalpha((int)cThis)) {
			markingchar = cThis;
			break;
		}
	}

	SetTexMode(MODE_HORIZONTAL);
	num = TexFontNumber("Typewriter");
	fprintRTF("{\\b0\\i0\\scaps0\\f%d ", num);

	while ((cThis = getRawTexChar()) && cThis != markingchar) 
		putRtfChar(cThis);
	
	fprintRTF("}");
}

void 
CmdVerbatim(int code)
/******************************************************************************
	convert characters 1:1 until \end{verbatim} or \end{Verbatim} is reached
	VERBATIM_1	 for \begin{verbatim} ... \end{verbatim}
	VERBATIM_2   for \begin{Verbatim} ... \end{Verbatim}
******************************************************************************/
{
	char			*verbatim_text, *vptr;
	int				num;
	int true_code = code & ~ON;
	
	if (code & ON) {
		
		diagnostics(4, "Entering CmdVerbatim");
		CmdEndParagraph(0);
		CmdIndent(INDENT_NONE);
		CmdStartParagraph(0);
		num = TexFontNumber("Typewriter");
		fprintRTF("\\b0\\i0\\scaps0\\f%d ", num);
		
		if (true_code == VERBATIM_2) 
			verbatim_text = getTexUntil("\\end{Verbatim}", 1);
		else
			verbatim_text = getTexUntil("\\end{verbatim}", 1);

		vptr = verbatim_text;
		
		while (*vptr) 
			putRtfChar(*vptr++);
		
		free(verbatim_text);

	} else {
		diagnostics(4, "Exiting CmdVerbatim");
		CmdEndParagraph(0);
	}
		
}

void 
CmdVerse(int code)
/******************************************************************************
  purpose: converts the LaTeX-Verse-environment to a similar Rtf-style
 ******************************************************************************/
{
	static int previous_indentation;
	static int previous_parindent;
	
	CmdEndParagraph(0);
	switch (code) {
		case ON:
		CmdIndent(INDENT_USUAL);
		previous_indentation = indent;
		previous_parindent = getLength("parindent");
		indent = 1134;
		setLength("parindent", -567);
		CmdStartParagraph(0);
		fprintRTF("\\ri1134\\keep ");
		break;
	case OFF:
		setLength("parindent", previous_parindent);
		indent = previous_indentation;
		CmdIndent(INDENT_INHIBIT);
		break;
	}
}


void 
CmdIgnoreDef( /* @unused@ */ int code)
/*****************************************************************************
 purpose: newenvironments or newcommands which are defined by the user aren't
	      converted into Rtf and so they must be ignored
 ******************************************************************************/
{
	char            cThis;

	while ((cThis = getTexChar()) && cThis != '{');
    
    parseBrace();
    
}

void 
TranslateGerman(void)
/***************************************************************************
purpose: called on active german-mode and " character in input file to
	 handle " as an active (meta-)character.
 ***************************************************************************/
{
	char            cThis;

	cThis = getTexChar();

	switch (cThis) {
	case 'a':
		fprintRTF("{\\'e4}");
		break;
	case 'o':
		fprintRTF("{\\'f6}");
		break;
	case 'u':
		fprintRTF("{\\'fc}");
		break;
	case 's':
		fprintRTF("{\\'df}");
		break;
	case '|':
		break;		/* ignore */
	case '-':
		break;		/* ignore */
	case '"':
		break;		/* ignore */
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

void 
GermanPrint(int code)
{
	switch (code) {
		case GP_CK:fprintRTF("ck");
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


void 
CmdIgnoreLet( /* @unused@ */ int code)
/******************************************************************************
     purpose : ignore \let 
	   Format: \let\XXXXX = \YYYYYY or \let\XXXXX\YYYYYY
 ******************************************************************************/
{
	char            cThis;

	while ((cThis = getTexChar()) && cThis != ' '  && cThis != '\\');

	if (cThis == ' ')
	{
		skipSpaces();
		while ((cThis = getTexChar()) && cThis != ' ');
		skipSpaces();
	}
}


/*************************************************************************
 *LEG250498
 * purpose: opens existing aux-file and scans for token, with value
 * reference.  Then outputs the corresponding ref-information to the
 * rtf-file parameters: 
 * code 0 = get the first parameter {refnumber} -> refnumber)
 *      1 = get the first par. of the first {{{sectref}{line}} } -> sectref 
 * 
 * globals: input (name of LaTeX-Inputfile) fRtf rtf-Outputfile
 * returns 1 if target is found otherwise 0
 ************************************************************************/

int 
ScanAux(char *token, char *reference, int code)
{
	static FILE    *fAux = NULL;
	static char     openbrace = '{';
	static char     closebrace = '}';
	char            tokenref[255];	/* should really be allocated dynmically */
	char            AuxLine[1024];
	char           *s;

	if (g_aux_file_missing || strlen(reference) == 0) {
		fprintRTF("?");
		return 0;
	}
	
	if (fAux == NULL && (fAux = fopen(AuxName, "r")) == NULL) {	/* open .aux file if not
									 * opened before */
		diagnostics(WARNING, "No .aux file.  Run LaTeX to create %s\n", AuxName);
		g_aux_file_missing = TRUE;
		fprintRTF("?");
		return 0;
	}
	
	rewind(fAux);
	sprintf(tokenref, "\\%s{%s}", token, reference);

	while (fgets(AuxLine, 1023, fAux) != NULL) {

		if ((s = strstr(AuxLine, tokenref)) != NULL) {
			s += strlen(tokenref);

			for (; code >= 0; code--) {	/* Do once or twice depending on code */
				s = strchr(s, openbrace);
				if (s == NULL) {	/* no parameter found, print '?' and return */
					fprintRTF("?");
					return 0;
				}
				s++;
			}

			while (*s != closebrace && *s != '\0')	/* print the number and exit */
				fprintRTF("%c", *s++);

			return 1;
		}
	}

	diagnostics(WARNING, "\\%s{%s} not found in %s", token, reference, AuxName);
	fprintRTF("?");
	return 0;
}

void
CmdLabel(int code)
/******************************************************************************
  purpose : label, produce rtf-output in dependency of the rtfversion.
  parameter : code  kind of label, passed through  *** BROKEN by SAP ***
 ******************************************************************************/
{
	char           *label;
	char            cThis;

	label=getParam();
	
	switch (code) {
	
		case REF:	
			ScanAux("newlabel", label, 1);
			break;
		
		default:
			break;
	}
	
	free(label);
	return;
	
	if (code < HYPER) {
		label = getParam();
		ungetTexChar(label[strlen(label) - 1]);	/* somewhat screwy */
	} else {
		label = hyperref;
	}

	diagnostics(3, "Generating label/bookmark `%s'", label);

	if (rtf_restrict(1, 1))
		CmdLabelOld(code, label);
	if (rtf_restrict(1, 4))
		CmdLabel1_4(code, label);

	if (code >= HYPER)
		free(label);

	cThis = getTexChar();

	cThis = getNonSpace();

	cThis = getTexChar();

	if (cThis != '\n')
		ungetTexChar(cThis);

	/* LEG190498 */
}

void
CmdLabel1_4(int code, char *text)
/******************************************************************************
  purpose : label
  parameter : code  kind of label, text name of the label
 ******************************************************************************/
{
	switch (code) {
		case LABEL:
		/*
		 * Note that Hyperlabels do not exist, if they are
		 * encountered it's a severe bug
		 */

		fprintRTF("{\\*\\bkmkstart %s} {\\*\\bkmkend %s}", text, text);
		break;
	case REF:
	case HYPERREF:
		fprintRTF("{\\field\\fldlock{\\*\\fldinst REF %s  \\\\n}{\\fldrslt ", text);
		ScanAux("newlabel", text, 1);
		fprintRTF("}}");
		break;
	case PAGEREF:
	case HYPERPAGEREF:
		fprintRTF("{\\field{\\*\\fldinst PAGEREF %s}{\\fldrslt ?}}", text);
		break;
	default:
		diagnostics(ERROR, "Called CmdLabel with wrong Code %d", code);
	}
}


void
CmdLabelOld(int code, char *text)
/******************************************************************************
     purpose : label
   parameter : code  kind of label
 ******************************************************************************/
{
	switch (code) {
		case LABEL:
		fprintRTF("{\\v[LABEL: %s]}", text);
		break;
	case REF:
	case HYPERREF:
		fprintRTF("{\\v[REF: %s]}", text);
		break;
	case PAGEREF:
	case HYPERPAGEREF:
		fprintRTF("{\\v[PAGEREF: %s]}", text);
		break;
	default:
		diagnostics(ERROR, "Called CmdLabel with wrong Code %d", code);
	}
}

void CmdQuad(int kk)
/******************************************************************************
 purpose: inserts kk quad spaces (D. Taupin)
 ******************************************************************************/
{
	int z;	
	fprintRTF("{\\emspace ");
	for (z=0; z<kk; z++) fprintRTF(" ");
	fprintRTF("}");
}

void CmdSpace(float kk)
/******************************************************************************
 purpose: inserts a space of width kk*space 
 ******************************************************************************/
{
	int size = CurrentFontSize()*kk;
	fprintRTF("{\\fs%d  }", size);
}

void 
CmdFigure(int code)
/******************************************************************************
  purpose: Process \begin{figure} ... \end{figure} environment
 ******************************************************************************/
{
	char            loc[5];

	if (code & ON) {
		getBracketParam(loc,4);
		diagnostics(4, "entering CmdFigure [%s]", loc);
		g_processing_figure = TRUE;
	} else {
		g_processing_figure = FALSE;
		diagnostics(4, "exiting CmdFigure");
	}
}

void 
CmdIgnoreFigure(int code)
/******************************************************************************
  purpose: function to ignore Picture and Minipage Environment
 ******************************************************************************/
{
	if (code & ON) {
	
		switch (code & ~(ON)) {
			case PICTURE:
					getTexUntil("\\end{picture}",0);
					break;
			
			case MINIPAGE:
					getTexUntil("\\end{minipage}",0);
					break;
		}
	}
}

/******************************************************************************
CmdLink:

  purpose: hyperlatex support. function, which translates the first parameter
           to the rtf-file and ignores the second, the proposed optional
	   parameter is also (still) ignored.
  parameter: not (yet?) used.

  The second parameter should be remembered for the \Cite (\Ref \Pageref)
  command.
  globals: hyperref, set to second Parameter

The first parameter of a \link{anchor}[ltx]{label} is converted to the
rtf-output. Label is stored to hyperref for later use, the optional
parameter is ignored.
[ltx] should be processed as Otfried recommends it, to use for
exclusive latex output.e.g:

	\link{readhere}[~\Ref]{explaining:chapter}.

Since {explaining:chapter} is yet read by latex and hyperlatex when
[...] is evaluated it produces the correct reference. We are only
strolling from left to right through the text and can't remember what
we will see in the future.

 ******************************************************************************/
void
CmdLink(int code)
{
	char           *param2;
	char            optparam[255] = "";

	diagnostics(4, "Entering hyperlatex \\link command");
	Convert();		/* convert routine is called again for
				 * evaluating the contents of the first
				 * parameter */
	diagnostics(4, "  Converted first parameter");

	getBracketParam(optparam, 255);
	/* LEG190498 now should come processing of the optional parameter */
	diagnostics(4, "  Converted optional parameter");

	param2 = getParam();
	diagnostics(4, "  Converted second parameter");

	if (hyperref != NULL)
		free(hyperref);

	hyperref = (char *) malloc((strlen(param2) + 1));
	if (hyperref == NULL)
		error(" malloc error -> out of memory!\n");

	strcpy(hyperref, param2);
	free(param2);
	/* LEG210698*** better? hyperref = param2 */
}

void 
CmdColumn(int code)
/******************************************************************************
  purpose: chooses between one/two-columns
parameter: number of columns
 globals: twocolumn: true if twocolumn-mode is set
 ******************************************************************************/
{
	switch (code) {
		case One_Column:fprintRTF("\\page \\colsx709\\endnhere ");	/* new page & one column */
		twocolumn = FALSE;
		break;
	case Two_Column:
		fprintRTF("\\page \\cols2\\colsx709\\endnhere ");	/* new page & two
									 * columns */
		twocolumn = TRUE;
		break;
	}			/* switch */
}

void 
CmdNewPage(int code)
/******************************************************************************
  purpose: starts a new page
parameter: code: newpage or newcolumn-option
 globals: twocolumn: true if twocolumn-mode is set
 ******************************************************************************/
{
	CmdEndParagraph(0);
	switch (code) {
		case NewPage:
			fprintRTF("\\page ");	/* causes new page */
			break;
			
		case NewColumn:
			if (twocolumn)
				fprintRTF("\\column ");	/* new column */
			else
				fprintRTF("\\page ");	/* causes new page */
			break;
	}			/* switch */
}

void 
Cmd_OptParam_Without_braces( /* @unused@ */ int code)
/******************************************************************************
 purpose: gets an optional parameter which isn't surrounded by braces but by spaces
 ******************************************************************************/
{
	char            cNext = ' ';
	char            cLast = ' ';

	do {
		cLast = cNext;
		cNext = getTexChar();
	} while ((cNext != ' ') &&
		 (cNext != '\\') &&
		 (cNext != '{') &&
		 (cNext != '\n') &&
		 (cNext != ',') &&
		 ((cNext != '.') || (isdigit((unsigned char) cLast))) &&
	/*
	 * . doesn't mean the end of an command inside an number of the type
	 * real
	 */
		 (cNext != '}') &&
		 (cNext != '\"') &&
		 (cNext != '[') &&
		 (cNext != '$'));

	ungetTexChar(cNext);
}

void 
CmdBottom(int code)
/******************************************************************************
  purpose: ignore raggedbottom command
 ******************************************************************************/
{
}

/******************************************************************************
parameter: code: on/off-option
 globals : article and titlepage from the documentstyle
 ******************************************************************************/
void
CmdAbstract(int code)
{
	static char     oldalignment;

	CmdEndParagraph(0);
	if (code == ON) {
		if (g_document_type == FORMAT_REPORT || titlepage) 
			fprintRTF("\\page ");

		CmdStartParagraph(0);
		fprintRTF("\\qc{\\b ");
		ConvertBabelName("ABSTRACTNAME");
		fprintRTF("}");
		CmdEndParagraph(0);
		indent += 1024;
		indent_right +=1024;
		oldalignment = alignment;
		alignment = JUSTIFIED;
	} else {
		CmdEndParagraph(0);
		indent -= 1024;
		indent_right -=1024;
		alignment = oldalignment;
		CmdVspace(2);				/* put \medskip after abstract */
	}
}

void 
CmdTitlepage(int code)
/******************************************************************************
  purpose: \begin{titlepage} ... \end{titlepage}
           add pagebreaks before and after this environment
 ******************************************************************************/
{
	switch (code) {
		case ON:
		fprintRTF("\n\\par\\pard \\page ");	/* new page */
		fprintRTF("\n\\par\\q%c ", alignment);
		break;
	case OFF:
		fprintRTF("\\pard ");
		fprintRTF("\n\\par\\q%c \\page ", alignment);
		break;
	}			/* switch */
}

void 
CmdMultiCol( /* @unused@ */ int code)
/******************************************************************************
 purpose: converts the LaTex-Multicolumn to a similar Rtf-style
	  this converting is only partially
	  so the user has to convert some part of the Table-environment by hand
 ******************************************************************************/
{
	char            inchar[10];
	char            numColStr[100];
	long            numCol, i, toBeInserted;
	char            colFmtChar = 'u';
	char           *eptr;	/* for srtol   */
	static bool     bWarningDisplayed = FALSE;

	if (!bWarningDisplayed) {
		diagnostics(WARNING, "Multicolumn: Cells must be merged by hand!");
		bWarningDisplayed = TRUE;
	}
	i = 0;
	do {
		inchar[0] = getTexChar();
		if (isdigit((unsigned char) inchar[0]))
			numColStr[i++] = inchar[0];
	}
	while (inchar[0] != '}');
	numColStr[i] = '\0';
	numCol = strtol(numColStr, &eptr, 10);
	if (eptr == numColStr)
		error(" multicolumn: argument num invalid\n");


	do {
		inchar[0] = getTexChar();
		switch (inchar[0]) {
		case 'c':
		case 'r':
		case 'l':
			if (colFmtChar == 'u')
				colFmtChar = inchar[0];
			break;
		default:
			break;
		}
	}
	while (inchar[0] != '}');
	if (colFmtChar == 'u')
		colFmtChar = 'l';

	switch (colFmtChar) {
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

	for (i = 1; i < toBeInserted; i++, actCol++) {
		fprintRTF(" \\cell \\pard \\intbl ");
	}
	fprintRTF("\\q%c ", colFmtChar);

	diagnostics(4, "Entering Convert() from CmdMultiCol()");
	Convert();
	diagnostics(4, "Exiting Convert() from CmdMultiCol()");

	for (i = toBeInserted; (i < numCol) && (actCol < colCount); i++, actCol++) {
		fprintRTF(" \\cell \\pard \\intbl ");
	}


}

void 
CmdColsep(int code)
/***************************************************************************
 * purpose: hyperlatex support, handles '&' as in Convert() in convert.c
 ***************************************************************************/
{
	if (!g_processing_tabular) {
		fprintRTF("{\\'a7}");
		return;
	}
	actCol++;

	if (GetTexMode() == MODE_DISPLAYMATH) {	/* in an eqnarray or array environment */
		fprintRTF("\\tab ");
	} else {
		fprintRTF(" \\cell \\pard \\intbl ");
		if (colFmt == NULL)
			diagnostics(WARNING, "Fatal, Fatal! CmdColsep called whith colFmt == NULL.");
		else
			fprintRTF("\\q%c ", colFmt[actCol]);
	}
}

void 
CmdGraphics(int code)
{
	char            options[255];
	char            fullpath[1023];
	char           *filename;
	int             cc, i;
	short           top, left, bottom, right;
	FILE           *fp;

	/* could be \includegraphics*[0,0][5,5]{file.pict} */

	getBracketParam(options, 255);
	getBracketParam(options, 255);
	filename = getParam();

	if (strstr(filename, ".pict") || strstr(filename, ".PICT")) {
		/* SAP fixes for Mac Platform */
#ifdef __MWERKS__
		{
		char           *dp;
		strcpy(fullpath, latexname);
		dp = strrchr(fullpath, ':');
		if (dp != NULL) {
			dp++;
			*dp = '\0';
		} else
			strcpy(fullpath, "");
		strcat(fullpath, filename);
		}
#else
		strcpy(fullpath,filename);
#endif

		fprintf(stderr, "processing picture %s\n", fullpath);
		fp = fopen(fullpath, "rb");

		if (fseek(fp, 514L, SEEK_CUR) ||     /* skip 512 byte header + 2 bytes for version info */
		    (fread(&top, 2, 1, fp) < 1) ||    /* read the pict file dimensions in points */
		    (fread(&left, 2, 1, fp) < 1) || 
			(fread(&bottom, 2, 1, fp) < 1) || 
			(fread(&right, 2, 1, fp) < 1) || 
			fseek(fp, -10L, SEEK_CUR)) {    /* move back ten bytes so that entire file will be encoded */
				free(filename);
				fclose(fp);
				return;
			}
		fprintRTF("\n{\\pict\\macpict\\picw%d\\pich%d\n", right - left, bottom - top);

		i = 0;
		while ((cc = fgetc(fp)) != EOF) {
			fprintRTF("%.2x", cc);
			if (++i > 126) {
				i = 0;
				fprintRTF("\n");
			}	/* keep lines 254 chars long */
		}

		fprintRTF("}\n");
		fclose(fp);
		free(filename);
	}
}

void 
CmdVerbosityLevel(int code)
/***************************************************************************
 * purpose: insert \verbositylevel{5} in the tex file to set the verbosity 
            in the LaTeX file!
 ***************************************************************************/
{
	char * s = getParam();
	g_verbosity_level = atoi(s);
	free(s);

}
