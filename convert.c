/* $Id: convert.c,v 1.5 2001/09/16 05:11:19 prahl Exp $ 
	purpose: routines to */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include "main.h"
#include "convert.h"
#include "commands.h"
#include "chars.h"
#include "funct1.h"
#include "l2r_fonts.h"
#include "stack.h"
#include "tables.h"
#include "equation.h"
#include "direct.h"
#include "ignore.h"
#include "cfg.h"
#include "encode.h"
#include "util.h"
#include "parser.h"
#include "lengths.h"
#include "counters.h"
#include "preamble.h"

static bool     TranslateCommand();	/* converts commands */

extern enum   TexCharSetKind TexCharSet;
static int      ret = 0;
static int      ConvertFlag;

void 
ConvertString(char *string)
/******************************************************************************
     purpose : converts string in TeX-format to Rtf-format
   parameter : string to be converted
     globals : linenumber, fTex
 ******************************************************************************/
{
	FILE           *fp, *LatexFile;
	long            oldlinenumber;
	int             test;
	char            temp[51];
	
	if ((fp = tmpfile()) == NULL) {
		fprintf(stderr, "%s: Fatal Error: cannot create temporary file\n", progname);
		exit(EXIT_FAILURE);
	}
	
	test = fwrite(string, strlen(string), 1, fp);
	if (test != 1)
		diagnostics(WARNING,
			    "(ConvertString): "
			    "Could not write `%s' to tempfile %s, "
			    "fwrite returns %d, should be 1",
			    string, "tmpfile()", test);
	if (fseek(fp, 0L, SEEK_SET) != 0)
		diagnostics(ERROR, "Could not position to 0 in tempfile (ConvertString)");

	LatexFile = fTex;
	fTex = fp;
	diagnostics(5, "changing fTex file pointer in ConvertString");
	oldlinenumber = linenumber;

	strncpy(temp,string,50);
	diagnostics(5, "Entering Convert() from StringConvert() <%s>",temp);
	while (!feof(fTex))
		Convert();
	diagnostics(5, "Exiting Convert() from StringConvert()");

	fTex = LatexFile;
	diagnostics(5, "resetting fTex file pointer in ConvertString");
	linenumber = oldlinenumber;
	if (fclose(fp) != 0)
		diagnostics(ERROR, "Could not close tempfile, (ConvertString).");
}

void 
Convert()
/****************************************************************************
purpose: converts inputfile and writes result to outputfile
globals: fTex, fRtf and all global flags for convert (see above)
 ****************************************************************************/
{
	char            cThis = '\n';
	char            cLast = '\0';
	char            cLast2 = '\0';
	char            cNext;
	int             count = 0;
	int             i;
	bool            babelMode = FALSE;

	RecursionLevel++;
	PushLevels();
	++ConvertFlag;
/*	if (!strstr(g_babel_language,"english"))
		babelMode = TRUE;
*/		
	while ((cThis = getTexChar()) && cThis != '\0') {
	
		if (cThis == '\n')
			diagnostics(5, "Current character in Convert() is '\\n'");
		else if (cThis == '\0')
			diagnostics(5, "Current character in Convert() is NULL");
		else
			diagnostics(5, "Current character in Convert() is '%c'", cThis);

/*		if (babelMode) {
			cThis = babelConvert(cThis, g_babel_language);
			if (cThis == '\0') break;
		}
*/		
		switch (cThis) {

		case '\\':
			PushLevels();
			
			(void) TranslateCommand();

			CleanStack();

			if (ret > 0) {
				ret--;
				RecursionLevel--;
				return;
			}
			break;
			
			
		case '%':
			diagnostics(WARNING, "Ignoring %% in %s at line %ld\n", latexname, getLinenumber());
			cThis = ' ';
			break;
			
		case '{':
			bBlankLine = FALSE;
			PushBrace();
			fprintRTF("{");
			break;
			
		case '}':
			bBlankLine = FALSE;
			ret = RecursionLevel - PopBrace();
			fprintRTF("}");
			if (ret > 0) {
				ret--;
				RecursionLevel--;
				return;
			}
			break;

		case ' ':
			if (g_processing_preamble) 
				break;
			if ( (cLast != ' ') && (cLast != '\n') ) { 
				if (!mbox)
					/* if (bNewPar == FALSE) */
					fprintRTF(" ");
				else
					fprintRTF("\\~");
			}
			break;
			
		case '~':
			bBlankLine = FALSE;
			fprintRTF("\\~");
			break;
			
		case '\r':
			diagnostics(WARNING, "Ignoring \\r in %s at line %ld", latexname, getLinenumber());
			cThis = ' ';
			break;
			
		case '\n':
			if (g_processing_preamble) 
				break;
			tabcounter = 0;

			while ((cNext = getTexChar()) == ' ');	/* blank line with
								 * spaces */
			ungetTexChar(cNext);

			if (cLast != '\n') {
				if (bNewPar) {
					bNewPar = FALSE;
					cThis = ' ';
					break;
				}
				if (cLast != ' ')
					fprintRTF(" ");	/* treat as 1 space */
				else if (bBlankLine)
					fprintRTF("\n\\par\\fi0\\li%d ", indent);
			} else {
				if (cLast2 != '\n') {
					fprintRTF("\n\\par\\fi0\\li%d ", indent);
				}
			}
			bBlankLine = TRUE;
			break;

		case '^':
			bBlankLine = FALSE;

			{
				char           *s = NULL;
				if ((s = getMathParam())) {
					fprintRTF("{\\up6 \\fs20 ");
					ConvertString(s);
					fprintRTF("}");
					free(s);
				}
			}
			break;

		case '_':
			bBlankLine = FALSE;
			{
				char           *s = NULL;
				if ((s = getMathParam())) {
					diagnostics(5, "subscript parameter is <%s>",s);
					fprintRTF("{\\dn6 \\fs20 ");
					ConvertString(s);
					fprintRTF("}");
					free(s);
				}
			}
			break;

		case '$':
			bBlankLine = FALSE;
			cNext = getTexChar();
			diagnostics(5,"Processing $, next char <%c>",cNext);

			if (cNext == '$')	/* check for $$ */
				CmdFormula2(FORM_DOLLAR);
			else {
				ungetTexChar(cNext);
				CmdFormula(FORM_DOLLAR);
			}

			/* 
			   Formulas need to close all Convert() operations when they end 
			   This works for \begin{equation} but not $$ since the BraceLevel
			   and environments don't get pushed properly.  We do it explicitly here.
			*/
			if (g_processing_equation)
				PushBrace();
			else {
				ret = RecursionLevel - PopBrace();
				if (ret > 0) {
					ret--;
					RecursionLevel--;
					return;
				}
			}
			
			break;

		case '&':
			if (g_processing_tabular && g_processing_equation) {	/* in an eqnarray */
				fprintRTF("\\tab ");
				break;
			}
			if (g_processing_tabular) {	/* in tabular */
				fprintRTF(" \\cell \\pard \\intbl ");
				actCol++;
				fprintRTF("\\q%c ", colFmt[actCol]);
				break;
			}
			fprintRTF("&");
			break;

		case '-':
			bBlankLine = FALSE;
			count++;
			while ((cNext = getTexChar()) && cNext == '-')
				count++;
			ungetTexChar(cNext);	/* reread last character */
			switch (count) {
			case 1:
				fprintRTF("-");
				break;
			case 2:
				fprintRTF("\\endash ");
				break;
			case 3:
				fprintRTF("\\emdash ");
				break;
			default:
				{
					for (i = count - 1; i >= 0; i--)
						fprintRTF("-");
				}
			}
			count = 0;
			break;
			
		case '\'':
			bBlankLine = FALSE;
			if (g_processing_equation)
					fprintRTF("'");
			else {
				count++;
				while ((cNext = getTexChar()) && cNext == '\'')
					count++;
				ungetTexChar(cNext);
				if (count != 2) {
					for (i = count - 1; i >= 0; i--)
						fprintRTF("\\rquote ");
				} else
					fprintRTF("\\rdblquote ");
				count = 0;
			}
			break;
			
		case '`':
			bBlankLine = FALSE;
			count++;
			while ((cNext = getTexChar()) && cNext == '`')
				count++;
			ungetTexChar(cNext);
			if (count != 2) {
				for (i = count - 1; i >= 0; i--)
					fprintRTF("\\lquote ");
			} else
				fprintRTF("\\ldblquote ");
			count = 0;
			break;
			
		case '\"':
			bBlankLine = FALSE;
			if (GermanMode)
				TranslateGerman();
			else
				fprintRTF("\"");
			break;

		case '?':
		case '!':
			{
				char            ch;
				bBlankLine = FALSE;
				if ((ch = getTexChar()) && ch == '`') {
					if (cThis == '?')
						fprintRTF("{\\'bf}");
					else
						fprintRTF("{\\'a1}");
				} else {
						fprintRTF("%c", cThis);
						ungetTexChar(ch);
				}
			}
			break;
			
		default:
			bBlankLine = FALSE;

			if (isupper((int)cThis) && ((cLast == '.') || (cLast == '!') || (cLast == '?') || (cLast == ':')))
				fprintRTF(" ");

			if (TexCharSet == ISO_8859_1)
				Write_ISO_8859_1(cThis);
			else
				Write_Default_Charset(cThis);

			bNewPar = FALSE;
			break;
		}

		tabcounter++;
		cLast2 = cLast;
		cLast = cThis;
	}
	RecursionLevel--;
}

bool 
TranslateCommand()
/****************************************************************************
purpose: The function is called on a backslash in input file and
	 tries to call the command-function for the following command.
returns: sucess or not
globals: fTex, fRtf, command-functions have side effects or recursive calls;
         global flags for convert
 ****************************************************************************/
{
	char            cCommand[MAXCOMMANDLEN];
	int             i;
	int            cThis;
	char            option_string[100];

	diagnostics(5, "Beginning TranslateCommand()");

	cThis = getTexChar();

	switch (cThis) {
	case '}':
		fprintRTF("\\}");
		return TRUE;
	case '{':
		fprintRTF("\\{");
		return TRUE;
	case '#':
		fprintRTF("#");
		return TRUE;
	case '$':
		fprintRTF("$");
		return TRUE;
	case '&':
		fprintRTF("&");
		return TRUE;
	case '%':
		fprintRTF("%%");
		return TRUE;
	case '_':
		fprintRTF("_");
		return TRUE;
		
	case '\\':		/* \\[1mm] or \\*[1mm] possible */

		if ((cThis = getTexChar()) != '*')
			ungetTexChar(cThis);

		getBracketParam(option_string, 99);	

		if (g_processing_eqnarray) {	/* eqnarray */
			fprintRTF("}");	/* close italics */
			if (g_show_equation_number && !g_suppress_equation_number) {
				incrementCounter("equation");
				fprintRTF("\\tab (%d)", getCounter("equation"));
			}
			fprintRTF("\n\\par\\tab {\\i ");
			g_suppress_equation_number = FALSE;
			actCol = 1;
			return TRUE;
		}
		
		if (g_processing_tabular) {	/* tabular or array environment */
			if (g_processing_equation) {	/* array */
				fprintRTF("\n\\par\\tab ");
				return TRUE;
			}
			for (; actCol < colCount; actCol++) {
				fprintRTF("\\cell\\pard\\intbl");
			}
			actCol = 1;
			fprintRTF("\\cell\\pard\\intbl\\row\n\\pard\\intbl\\q%c ", colFmt[1]);
			return TRUE;
		}

		if (tabbing_on){
			(void) PopBrace();
			PushBrace();
		}

		/* simple end of line ... */
		fprintRTF("\\par ");
		bNewPar = TRUE;
		tabcounter = 0;
		if (tabbing_on && (fgetpos(fRtf, &pos_begin_kill) != 0))
				diagnostics(ERROR, "File access problem in tabbing environment");
		return TRUE;

	case ' ':
		fprintRTF(" ");	/* ordinary interword space */
		skipSpaces();
		return TRUE;

/* \= \> \< \+ \- \' \` all have different meanings in a tabbing environment */

	case '-':
		if (tabbing_on){
			(void) PopBrace();
			PushBrace();
		} else
			fprintRTF("\\-");
		return TRUE;

	case '+':
		if (tabbing_on){
			(void) PopBrace();
			PushBrace();
		}
		return TRUE;
		
	case '<':
		if (tabbing_on){
			(void) PopBrace();
			PushBrace();
		}
		return TRUE;

	case '>':
		if (tabbing_on){
			(void) PopBrace();
			CmdTabjump();
			PushBrace();
		} else 
			CmdSpace(0.50);  /* medium space */
		return TRUE;
		
	case '`':
		if (tabbing_on){
			(void) PopBrace();
			PushBrace();
		} else
			CmdLApostrophChar(0);
		return TRUE;
		
	case '\'':
		if (tabbing_on){
			(void) PopBrace();
			PushBrace();
			return TRUE;
		} else
			CmdRApostrophChar(0);	/* char ' =?= \' */
		return TRUE;

	case '=':
		if (tabbing_on){
			(void) PopBrace();
			CmdTabset();
			PushBrace();
		}
		else
			CmdMacronChar(0);
		return TRUE;
		
	case '~':
		CmdTildeChar(0);
		return TRUE;
	case '^':
		CmdHatChar(0);
		return TRUE;
	case '.':
		CmdDotChar(0);
		return TRUE;
	case '\"':
		CmdUmlauteChar(0);
		return TRUE;
	case '(':
		CmdFormula(FORM_RND_OPEN);
		PushBrace();
		return TRUE;
	case '[':
		CmdFormula2(FORM_DOLLAR);
		PushBrace();
		return TRUE;
	case ')':
		CmdFormula(FORM_RND_CLOSE);
		ret = RecursionLevel - PopBrace();
		return TRUE;
	case ']':
		CmdFormula2(FORM_DOLLAR);
		ret = RecursionLevel - PopBrace();
		return TRUE;
	case '/':
		CmdIgnore(0);
		return TRUE;
	case ',':
		CmdSpace(0.33);	/* \, produces a small space */
		return TRUE;
	case ';':
		CmdSpace(0.75);	/* \; produces a thick space */
		return TRUE;
	case '@':
		CmdIgnore(0);	/* \@ produces an "end of sentence" space */
		return TRUE;
	case '3':
		fprintRTF("{\\'df}");	/* german symbol 'á' */
		return TRUE;
	}


	/* LEG180498 Commands consist of letters and can have an optional * at the end */
	for (i = 0; i < MAXCOMMANDLEN; i++) {
		if (!isalpha(cThis) && (cThis != '*')) {
			bool            found_nl = FALSE;

			/* all spaces after commands are ignored, a single \n may occur */
			while (cThis == ' ' || (cThis == '\n' && !found_nl)) {
				if (cThis == '\n')
					found_nl = TRUE;
				cThis = getTexChar();
			}

			ungetTexChar(cThis);	/* put back first non-space char after command */
			break;					/* done skipping spaces */
		} else
			cCommand[i] = cThis;

		cThis = getTexChar();
	}

	cCommand[i] = '\0';	/* mark end of string with zero */
	diagnostics(5, "TranslateCommand() <%s>", cCommand);

	if (i == 0)
		return FALSE;
		
	if (strcmp(cCommand,"begin")==0){
		fprintRTF("{");
		PushBrace();
	}
		
	if (CallCommandFunc(cCommand)){	/* call handling function for command */
		if (strcmp(cCommand,"end")==0) {
			ret = RecursionLevel - PopBrace();
			fprintRTF("}");
		}
		return TRUE;
	}
	if (TryDirectConvert(cCommand, fRtf))
		return TRUE;
	if (TryVariableIgnore(cCommand))
		return TRUE;
	diagnostics(WARNING, "Command \\%s not found - ignored", cCommand);
	return FALSE;
}
