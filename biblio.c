/* $Id: biblio.c,v 1.2 2001/09/16 05:11:19 prahl Exp $ 
 
This file contains routines to handle bibliographic and cite commands
*/

#include <stdio.h>
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
#include "biblio.h"
#include "parser.h"

static char           *
EmptyString(void)
{
	char           *s = malloc(1);
	if (s == NULL) {
		error(" malloc error -> out of memory!\n");
	}
	*s = '\0';
	return s;
}

void 
CmdNoCite( /* @unused@ */ int code)
/******************************************************************************
 purpose: produce reference-list at the end
  globals: bCite: is set to true if a \nocite appeared in text,
                  necessary to produce reference-list at the end of the
                  article
 ******************************************************************************/
{
	bCite = TRUE;
	free(getParam());	/* just skip the parameter */
}

void 
CmdCite(int code)
/******************************************************************************
 purpose: opens existing aux-file and reads the citation number
LEG190498
parameter: if FALSE (0) work as normal
           if HYPERLATEX get reference string from remembered \link parameter
  globals: input  (name of LaTeX-Inputfile)
           bCite: is set to true if a \cite appeared in text,
                  necessary to produce reference-list at the end of the
                  article
                  is set to false if the .aux file cannot be opened or
                  it is not up to date
	   LEG190498
	   hyperref: NULL, or the last used reference by \link command.
 ******************************************************************************/
{
	char            *reference;
	char           *str1, *str2;

	fprintRTF("[");

	if (code == HYPERLATEX) {
		if (hyperref == NULL) {
			fprintf(stderr, "ERROR: \\Cite called before \\link\n");
			fprintRTF("?]");
			return;
		}
		reference=strdup(hyperref);
	} else
		reference = getParam();

	str1 = reference;
	while ((str2 = strchr(str1, ',')) != NULL) {
		*str2 = '\0';	/* replace ',' with '\0' */
		if (ScanAux("bibcite", str1, 0))
			bCite = TRUE;
		fprintRTF(",");
		str1 = str2 + 1;
	}

	if (ScanAux("bibcite", str1, 0))
		bCite = TRUE;

	fprintRTF("]");
	free(reference);
}

FILE           *
OpenBblFile(void)
/***********************************************************************
 * purpose: opens either the "input".bbl file, or the .bbl file named by
 *          the -b command line option.
 * globals: BblName,
 **********************************************************************/
{
	static FILE    *fBbl = NULL;

	if (BblName == NULL) {
		char           *s;
		if (input != NULL) {
			if ((BblName = malloc(strlen(input) + 5)) == NULL)
				error(" malloc error -> out of memory!\n");
			strcpy(BblName, input);
			if ((s = strrchr(BblName, '.')) == NULL || strcmp(s, ".tex") != 0)
				strcat(BblName, ".bbl");
			else
				strcpy(s, ".bbl");
		} else
			BblName = EmptyString();
	}
	
	if ((fBbl = fopen(BblName, "r")) == NULL) {
		fprintf(stderr, "Cannot open bibliography file %s - run bibtex", BblName);
	}
	return fBbl;
}

void
MakeBiblio(FILE * fBbl)
/*****************************************************************
 * Converts a bibliography environment
 *****************************************************************/
{
	char            BblLine[255];
	char           *allBblLine = NULL;
	int             refcount = 0;
	char           *str;

	fprintRTF("\\par\\par\\pard{\\fs28 \\b ");
	if (article)
		ConvertBabelName("REFNAME");
	else
		ConvertBabelName("BIBNAME");
	fprintRTF("}\n");

	while (fgets(BblLine, 255, fBbl) != NULL) {
		if (strstr(BblLine, "\\begin{thebibliography}"))
			continue;
		if (strstr(BblLine, "\\end{thebibliography}")) {
			if (allBblLine != NULL) {
				ConvertString(allBblLine);
				fprintRTF(" ");
				free(allBblLine);
				allBblLine = NULL;
			}
			break;
		}
		if (strstr(BblLine, "\\bibitem")) {

			char           *label;
			if (allBblLine != NULL) {
				ConvertString(allBblLine);
				fprintRTF(" ");
				free(allBblLine);
				allBblLine = NULL;
			}
			fprintRTF("\\par \\par \\pard ");
			if ((label = strchr(BblLine, '[')) != NULL) {
				for (; *label != '\0' && *label != ']'; label++)
					if (fputc(*label, fRtf) != (int) *label)
						diagnostics(ERROR, "WriteRefList: Failed fputc('%c')", *label);
				if (*label != '\0') {
					if (fputc(*label, fRtf) != (int) *label)
						diagnostics(ERROR, "WriteRefList: Failed fputc('%c')", *label);
				}
				if (fputc(' ', fRtf) != (int) ' ')
					diagnostics(ERROR, "WriteRefList: Failed fputc(' ')");
			} else
				fprintRTF("[%d] ", ++refcount);
			continue;
		} else if ((str = strstr(BblLine, "\\newblock")) != NULL) {
			str += strlen("\\newblock");
			if (allBblLine != NULL) {
				ConvertString(allBblLine);
				fprintRTF(" ");
				free(allBblLine);
				allBblLine = NULL;
			}
			if ((allBblLine = malloc(strlen(str) + 1)) == NULL)
				error(" malloc error -> out of memory!\n");
			strcpy(allBblLine, str);
		} else {
			if (BblLine[0] != '\n') {
				if (allBblLine != NULL) {
					if ((allBblLine = (char *) realloc(allBblLine,
									   strlen(allBblLine) + strlen(BblLine) + 1)) == NULL)
						error(" realloc error -> out of memory!\n");
					strcat(allBblLine, BblLine);
				} else {
					if ((allBblLine = malloc(strlen(BblLine) + 1)) == NULL)
						error(" malloc error -> out of memory!\n");
					strcpy(allBblLine, BblLine);
				}
			}
		}
	}
	if (ferror(fBbl) != 0)
		error("Error reading BBL-File!\n");

	if (allBblLine != NULL) {
		free(allBblLine);
	}
	(void) fclose(fBbl);
}

void 
CmdConvertBiblio(int code)
/*******************************************************************
 * converts a bibliography environment
 *****************************************************************/
{
	MakeBiblio(fTex);
	bCite = FALSE;
}


void
WriteRefList(void)
/********************************************************************
 * Converts a .bbl File to rtf output
 *******************************************************************/
{
	FILE * f;
	
	if ((f=OpenBblFile()))
		MakeBiblio(f);
}

