/* $Id: biblio.c,v 1.9 2001/10/14 18:24:10 prahl Exp $ 
 
This file contains routines to handle bibliographic and cite commands
*/

#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "convert.h"
#include "funct1.h"
#include "commands.h"
#include "cfg.h"
#include "biblio.h"
#include "parser.h"
#include "preamble.h"
#include "lengths.h"

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
	char            *reference;
	char           *str1, *str2;

	fprintRTF("[");

	if (code == HYPERLATEX) {
		if (hyperref == NULL) {
			diagnostics(WARNING, "WARNING: \\Cite called before \\link\n");
			fprintRTF("?]");
			return;
		}
		reference=strdup(hyperref);
	} else
		reference = getBraceParam();

	str1 = reference;
	while ((str2 = strchr(str1, ',')) != NULL) {
		*str2 = '\0';	/* replace ',' with '\0' */
		ScanAux("bibcite", str1, 0);
		fprintRTF(",");
		str1 = str2 + 1;
	}

	ScanAux("bibcite", str1, 0);
	fprintRTF("]");
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
	char *label, *key;
	int  old_indent;	
	
	/* new paragraph for bib entry */
	CmdEndParagraph(0);
	old_indent = getLength("parindent");
	setLength("parindent", -450);
	CmdStartParagraph(0);
	fprintRTF("\\li350 ");
	
	label = getBracketParam();
	if (label) {
		fprintRTF("[%s]", label);
		free(label);
	} else {
		key = getBraceParam();
		fprintRTF("[");
		ScanAux("bibcite", key, 0);
		fprintRTF("]");
		free(key);
	}
	
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
