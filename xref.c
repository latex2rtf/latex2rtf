/* $Id: xref.c,v 1.4 2001/10/28 04:02:44 prahl Exp $ 
 
This file contains routines to handle cross references :
	\label{key}, \ref{key},   \pageref{key}, \bibitem{key},
	\cite{key},  \index{str}, \glossary{key}
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
char * ScanAux(char *token, char * reference, int code);

void 
CmdFootNote(int code)
/******************************************************************************
 purpose: converts footnotes from LaTeX to Rtf
 params : code specifies whether it is a footnote or a thanks-mark
 ******************************************************************************/
{
	char           *number;
	static int      thankno = 1;
	int             text_ref_upsize, foot_ref_upsize;
	int				DefFont = DefaultFontFamily();
	
	diagnostics(4,"Entering ConvertFootNote");
	number = getBracketParam();	/* ignored by automatic footnumber generation */

	if (number) free(number);
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
CmdNoCite(int code)
/******************************************************************************
 purpose: handle the \nocite{tag} 
 ******************************************************************************/
{
	free(getBraceParam());	/* just skip the parameter */
}

void 
CmdCite(int code)
/******************************************************************************
 purpose: opens existing aux-file and reads the citation number
parameter: if FALSE (0) work as normal
           if HYPERLATEX get reference string from remembered \link parameter
 ******************************************************************************/
{
	char            *reference, *str1, *str2, *nonblank;

	fprintRTF("[");

	if (code == HYPERLATEX) {
		if (hyperref == NULL) {
			diagnostics(WARNING, "WARNING: \\Cite called before \\link\n");
			fprintRTF("?]");
			return;
		}
		reference = strdup(hyperref);
	} else
		reference = getBraceParam();

	nonblank = strdup_noblanks(reference);
	diagnostics(4,"before <%s>, after <%s>", reference, nonblank);
	str1 = nonblank;
	while ((str2 = strchr(str1, ',')) != NULL) {
		*str2 = '\0';	/* replace ',' with '\0' */
		ScanAux("bibcite", str1, 0);
		fprintRTF(",");
		str1 = str2 + 1;
	}

	ScanAux("bibcite", str1, 0);
	fprintRTF("]");
	free(nonblank);
	free(reference);
}

void 
CmdBibliography(int code)
{
	if (PushSource(BblName, NULL)) {
		diagnostics(4, "CmdBibliography ... begin Convert()");
		Convert();
		diagnostics(4, "CmdBibliography ... done Convert()");
		PopSource();
	} else
		diagnostics(WARNING, "Cannot open bibliography file.  Create %s using BibTeX", BblName);
}

void 
CmdThebibliography(int code)
{
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
	}
	
}

void 
CmdBibitem(int code)
{
	char *label, *key, *signet, *s;
	int  old_indent;	
	
	/* new paragraph for bib entry */
	CmdEndParagraph(0);
	old_indent = getLength("parindent");
	setLength("parindent", -450);
	CmdStartParagraph(0);
	fprintRTF("\\li450 ");
	
	label = getBracketParam();
	if (label) {
		fprintRTF("[%s]", label);
		free(label);
	}
	
	key = getBraceParam();
	signet = strdup_nobadchars(key);
	s=ScanAux("bibcite", key, 0);
	
	fprintRTF("[");
	fprintRTF("{\\*\\bkmkstart BIB_%s}",signet);
	fprintRTF("%s",((s)?s:""));
	fprintRTF("{\\*\\bkmkend BIB_%s}",signet);
	fprintRTF("]");

	if (s) free(s);
	free(signet);
	free(key);
	
	fprintRTF("\\tab ");
	setLength("parindent", old_indent);
	skipSpaces();
}

void 
CmdNewblock(int code)
{
	/* if openbib chosen then start a paragraph with 1.5em indent 
	   otherwise do nothing */
}

void CmdLabel(int code)
/******************************************************************************
purpose: handles \label \ref \pageref \cite
******************************************************************************/
{
	char *text, *signet;
	char *str1, *comma, *s, *str;
	
	text = getBraceParam();
	if (strlen(text)==0) {
		free(text);
		return;
	}
	
	switch (code) {
		case LABEL_LABEL:
			if (g_processing_figure || g_processing_table) break;
			signet = strdup_nobadchars(text);
			fprintRTF("{\\*\\bkmkstart LBL_%s}",signet);
			fprintRTF("{\\*\\bkmkend LBL_%s}",signet);
			free(signet);
			break;
		
		case LABEL_HYPERREF:
		case LABEL_REF:
			signet = strdup_nobadchars(text);
			s = ScanAux("newlabel", text, 1);
			fprintRTF("{\\field{\\*\\fldinst{\\lang1024 REF LBL_%s}}",signet);
			fprintRTF("{\\fldrslt{%s}}}", ((s)?s:"?"));
			free(signet);
			if (s) free(s);
			break;
		
		case LABEL_CITE:
			fprintRTF("[");
			str = strdup_noblanks(text);
			str1 = str;
			do {
				comma = strchr(str1, ',');
				if (comma) *comma = '\0';	/* replace ',' with '\0' */
				s = ScanAux("bibcite", str1, 0);
				signet = strdup_nobadchars(str1);
				fprintRTF("{\\field{\\*\\fldinst{\\lang1024 REF BIB_%s}}",signet);
				fprintRTF("{\\fldrslt{%s}}}", ((s)?s:"?"));
				if (comma) fprintRTF(",");
				str1 = comma + 1;
				if (s) free(s);
				free(signet);
			} while (comma != NULL);

			fprintRTF("]");
			free(str);
			break;
		
		case LABEL_HYPERPAGEREF:
		case LABEL_PAGEREF:
			signet = strdup_nobadchars(text);
			fprintRTF("{\\field{\\*\\fldinst{\\lang1024 PAGEREF LBL_%s}}",signet);
			fprintRTF("{\\fldrslt{}}}",signet);
			free(signet);
			break;
	}
	
	free(text);
}

char * 
ScanAux(char *token, char * reference, int code)
/*************************************************************************
purpose: obtains a reference from .aux file

code=0 means \\token{reference}{number}       -> "number"
code=1 means \\token{reference}{{sect}{line}} -> "sect"
 ************************************************************************/
{
	static FILE    *fAux = NULL;
	char            AuxLine[1024];
	char            target[256];
	char           *s,*t;

	if (g_aux_file_missing || strlen(token) == 0) {
		return NULL;
	}

	sprintf(target, "\\%s{%s}", token, reference);
	
	if (fAux == NULL && (fAux = fopen(AuxName, "r")) == NULL) {
		diagnostics(WARNING, "No .aux file.  Run LaTeX to create %s\n", AuxName);
		g_aux_file_missing = TRUE;
		return NULL;
	}
	
	rewind(fAux);
	
	while (fgets(AuxLine, 1023, fAux) != NULL) {

		s = strstr(AuxLine, target);
		if (s) {
			s += strlen(target);

			s = strchr(s, '{');
			if (!s) return NULL;
			s++;

			if (code) {
				s = strchr(s, '{');
				if (!s) return NULL;
				s++;		
			}
			
			t = strchr(s, '}');		/* find end of string */
			if (t) *t = '\0';		/* replace '}'        */
			
			return strdup(s);
		}
	}
	return NULL;
}
