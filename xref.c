/* xref.c - commands for LaTeX cross references

Copyright (C) 2001-2002 The Free Software Foundation

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
    2001-2002 Scott Prahl
*/

#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "util.h"
#include "convert.h"
#include "funct1.h"
#include "commands.h"
#include "cfg.h"
#include "xref.h"
#include "parser.h"
#include "preamble.h"
#include "lengths.h"
#include "l2r_fonts.h"

char * g_figure_label = NULL;
char * g_table_label = NULL;
char * g_equation_label = NULL;
char * g_section_label = NULL;

#define MAX_LABELS 200
char * g_label_list[MAX_LABELS];
int g_label_list_number=-1;

char * ScanAux(char *token, char * reference, int code);

void 
CmdFootNote(int code)
/******************************************************************************
 purpose: converts footnotes from LaTeX to Rtf
 params : code specifies whether it is a footnote or a thanks-mark
 ******************************************************************************/
{
	char           *number, *text;
	static int      thankno = 1;
	int             text_ref_upsize, foot_ref_upsize;
	int				DefFont = DefaultFontFamily();
	
	diagnostics(4,"Entering ConvertFootNote");
	number = getBracketParam();	/* ignored by automatic footnumber generation */
	text = getBraceParam();

	if (number) free(number);
	text_ref_upsize = (int) (0.8 * CurrentFontSize());
	foot_ref_upsize = (int) (0.8 * CurrentFontSize());

	switch (code) {
		case FOOTNOTE_THANKS:
			thankno++;
			fprintRTF("{\\up%d %d}\n", text_ref_upsize, thankno);
			fprintRTF("{\\*\\footnote \\pard\\plain\\s246\\f%d",DefFont);
			fprintRTF("\\fs%d {\\up%d %d}", CurrentFontSize(), foot_ref_upsize, thankno);
			break;
	
		case FOOTNOTE:
			fprintRTF("{\\up%d\\chftn}\n", text_ref_upsize);
			fprintRTF("{\\*\\footnote \\pard\\plain\\s246\\f%d",DefFont);
			fprintRTF("\\fs%d {\\up%d\\chftn}", CurrentFontSize(), foot_ref_upsize);
			break;
	
		case FOOTNOTE_TEXT:
			fprintRTF("{\\*\\footnote \\pard\\plain\\s246\\f%d",DefFont);
			fprintRTF("\\fs%d ", CurrentFontSize());
			break;
	}

	ConvertString(text);
	fprintRTF("}\n ");
	diagnostics(4,"Exiting CmdFootNote");
	free(text);
}

void 
CmdNoCite(int code)
/******************************************************************************
 purpose: handle the \nocite{tag} 
 ******************************************************************************/
{
	free(getBraceParam());	/* just skip the parameter */
}

void
CmdBibliographyStyle(int code)
{
	char * s = getBraceParam();   /*throw away widest_label */
	free(s);
}

void 
CmdBibliography(int code)
{
	int err;
	char *s;
	
	s = getBraceParam();   /*throw away bibliography name */
	free(s);
	
	err=PushSource(g_bbl_name, NULL);
	
	if (!err) {
		diagnostics(4, "CmdBibliography ... begin Convert()");
		Convert();
		diagnostics(4, "CmdBibliography ... done Convert()");
		PopSource();
	} else
		diagnostics(WARNING, "Cannot open bibliography file.  Create %s using BibTeX", g_bbl_name);
}

void 
CmdThebibliography(int code)
{
	int amount = 450;
	
	if (code & ON) {
		char * s = getBraceParam();   /*throw away widest_label */
		free(s);
		
		CmdEndParagraph(0);
		CmdIndent(INDENT_NONE);
		fprintRTF("{");
		CmdStartParagraph(0);
		fprintRTF("\\fs28\\b ");
		if (g_document_type == FORMAT_ARTICLE)
			ConvertBabelName("REFNAME");
		else
			ConvertBabelName("BIBNAME");
		CmdEndParagraph(0);
		fprintRTF("}");

		PushEnvironment(GENERIC_ENV);
		setLength("parindent", -amount);
		indent += amount;
	} else {
		CmdEndParagraph(0);
		CmdVspace(1);
		PopEnvironment();
	}

	CmdIndent(INDENT_USUAL);
}

void 
CmdBibitem(int code)
{
	char *label, *key, *signet, *s;
	
	CmdEndParagraph(0);
	CmdIndent(INDENT_USUAL);
	CmdStartParagraph(0);
	
	label = getBracketParam();
	key = getBraceParam();
	signet = strdup_nobadchars(key);
	s=ScanAux("bibcite", key, 0);

	if (label && !s) {		/* happens when file needs to be latex'ed again */
		diagnostics(WARNING,"file needs to be latexed again for references");
		fprintRTF("[");
		ConvertString(label);
		fprintRTF("]");
	} else {
		diagnostics(4,"CmdBibitem <%s>",s);
		fprintRTF("[");
		fprintRTF("{\\*\\bkmkstart BIB_%s}",signet);
		ConvertString(s);
		fprintRTF("{\\*\\bkmkend BIB_%s}",signet);
		fprintRTF("]");
	}

	if (s) free(s);
	if (label) free(label);
	free(signet);
	free(key);
	
	fprintRTF("\\tab ");
	skipSpaces();
}

void 
CmdNewblock(int code)
{
	/* if openbib chosen then start a paragraph with 1.5em indent 
	   otherwise do nothing */
}

void
CmdIndex(int code)
/******************************************************************************
purpose: convert \index{classe!article@\textit{article}!section}
              to {\xe\v "classe:{\i article}:section"}
******************************************************************************/
{
	char cThis, *text, *r, *s, *t;
	
	cThis = getNonBlank();
	text = getDelimitedText('{', '}', TRUE);
	diagnostics(4, "CmdIndex \\index{%s}", text);
	fprintRTF("{\\xe{\\v ");
	
	t = text;
	while (t) {
		s = t;
		t = strchr(s,'!');
		if (t) *t = '\0';
		r = strchr(s,'@');
		if (r) s=r+1;
		ConvertString(s);
/*		while (*s && *s != '@') putRtfChar(*s++);*/
		if (t) {
			fprintRTF("\\:");
			t++;
		}
	}
			
	fprintRTF("}}");
	diagnostics(4, "leaving CmdIndex");
	free(text);
}

void
CmdPrintIndex(int code)
{
	CmdEndParagraph(0);
	fprintRTF("\\page ");
	fprintRTF("{\\field{\\*\\fldinst{INDEX \\\\c 2}}{\\fldrslt{}}}");
}

static int
ExistsBookmark(char *s)
{
	int i;
	if (!s) return FALSE;
	for (i=0; i<=g_label_list_number; i++) {
		if (strcmp(s,g_label_list[i])==0) 
			return TRUE;
	}
	return FALSE;
}

static void
RecordBookmark(char *s)
{
	if (!s) return;
	if (g_label_list_number>=MAX_LABELS)
		diagnostics(WARNING, "Too many labels...some cross-references will fail");
	else {
		g_label_list_number++;
		g_label_list[g_label_list_number] = strdup(s);
	}
}

void
InsertBookmark(char *name, char *text)
{
	char *signet;
	if (!name) {
		fprintRTF("%s",text);
		return;
	}

	signet = strdup_nobadchars(name);

	if (ExistsBookmark(signet)) {
		diagnostics(3,"bookmark %s already exists",signet);

	} else {
		diagnostics(3,"bookmark %s being inserted around <%s>",signet,text);
		RecordBookmark(signet);
		fprintRTF("{\\*\\bkmkstart BM%s}%s{\\*\\bkmkend BM%s}",signet,text,signet);
	}
	
	free(signet);
}

void CmdLabel(int code)
/******************************************************************************
purpose: handles \label \ref \pageref \cite
******************************************************************************/
{
	char *text, *signet;
	char *str1, *comma, *s, *str;
	char *option = NULL;
	int mode=GetTexMode();
	
	option = getBracketParam();
	text = getBraceParam();
	if (strlen(text)==0) {
		free(text);
		return;
	}
	
	switch (code) {
		case LABEL_LABEL:
			if (g_processing_figure || g_processing_table) break;
			if (mode == MODE_DISPLAYMATH) {
				g_equation_label = strdup_nobadchars(text);
				diagnostics(3,"equation label is <%s>",text);
			} else 
				InsertBookmark(text,"");
			break;
		
		case LABEL_HYPERREF:
		case LABEL_REF:
			signet = strdup_nobadchars(text);
			s = ScanAux("newlabel", text, 1);
			fprintRTF("{\\field{\\*\\fldinst{\\lang1024 REF BM%s \\\\* MERGEFORMAT }}",signet);
			fprintRTF("{\\fldrslt{");
			if (s)
				ConvertString(s);
			else
				fprintRTF("?");
			fprintRTF("}}}");
				
			free(signet);
			if (s) free(s);
			break;
		
/* {\field{\*\fldinst{\lang1024 REF section31 \\* MERGEFORMAT }}{\fldrslt{?}}} */
		case LABEL_CITE:
			fprintRTF("\n[");
			str = strdup_noblanks(text);
			str1 = str;
			do {
				comma = strchr(str1, ',');
				if (comma) *comma = '\0';	/* replace ',' with '\0' */
				s = ScanAux("bibcite", str1, 0);
				signet = strdup_nobadchars(str1);
				fprintRTF("{\\field{\\*\\fldinst{\\lang1024 REF BIB_%s \\\\* MERGEFORMAT }}",signet);
				fprintRTF("{\\fldrslt{");
				if (s)
					fprintRTF(s);
				else
					fprintRTF("?");
				fprintRTF("}}}");
				if (comma) fprintRTF(",");
				str1 = comma + 1;
				if (s) free(s);
				free(signet);
			} while (comma != NULL);

			if (option) {
				fprintRTF(", ");
				ConvertString(option);
			}
			fprintRTF("]");
			free(str);
			break;
		
		case LABEL_HYPERPAGEREF:
		case LABEL_PAGEREF:
			signet = strdup_nobadchars(text);
			fprintRTF("{\\field{\\*\\fldinst{\\lang1024 PAGEREF BM%s \\\\* MERGEFORMAT }}",signet);
			fprintRTF("{\\fldrslt{}}}",signet);
			free(signet);
			break;
	}
	
	free(text);
	if (option) free(option);
}

char * 
ScanAux(char *token, char * reference, int code)
/*************************************************************************
purpose: obtains a reference from .aux file

code=0 means \token{reference}{number}       -> "number"
code=1 means \token{reference}{{sect}{line}} -> "sect"
 ************************************************************************/
{
	static FILE    *fAux = NULL;
	char            AuxLine[1024];
	char            target[256];
	char           *s,*t;
	int				braces;

	if (g_aux_file_missing || strlen(token) == 0) {
		return NULL;
	}

	sprintf(target, "\\%s{%s}", token, reference);
	
	if (fAux == NULL && (fAux = my_fopen(g_aux_name, "r")) == NULL) {
		diagnostics(WARNING, "No .aux file.  Run LaTeX to create %s\n", g_aux_name);
		g_aux_file_missing = TRUE;
		return NULL;
	}
	
	rewind(fAux);
	
	while (fgets(AuxLine, 1023, fAux) != NULL) {

		s = strstr(AuxLine, target);
		if (s) {
		
			s += strlen(target);		/* move to \token{reference}{ */			
			if (code==1) s++;			/* move to \token{reference}{{ */

			t = s;					
			braces = 1;
			while ( braces >= 1) {		/* skip matched braces */
				t++;
				if (*t == '{') braces++;
				if (*t == '}') braces--;
				if (*t == '\0') return NULL;
			}
			
			*t = '\0';
			return strdup(s+1);
		}
	}
	return NULL;
}

void
CmdHtml(int code)
/******************************************************************************
purpose: handles \htmladdnormallink{text}{link}
******************************************************************************/
{
	char * text, *ref, *s;
	
	if (code == LABEL_HTMLADDNORMALREF) {
		text = getBraceParam();
		ref = getBraceParam();

		while ((s = strstr(text,"\\~{}")) != NULL) {
			*s = '~';
			strcpy(s+1,s+4);
		}
		while ((s = strstr(ref,"\\~{}")) != NULL) {
			*s = '~';
			strcpy(s+1,s+4);
		}
			
		fprintRTF("{\\field{\\*\\fldinst{ HYPERLINK \"%s\" }{{}}}",ref);
		fprintRTF("{\\fldrslt{\\ul %s}}}", text);
		free(text);
		free(ref);
	} else if (code == LABEL_HTMLREF) {
		text = getBraceParam();
		ref = getBraceParam();
		ConvertString(text);
		free(text);
		free(ref);
	}
}
