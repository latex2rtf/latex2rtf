/* $Id: convert.c,v 1.10 2001/10/07 05:42:18 prahl Exp $ 
	purpose: ConvertString(), Convert(), TranslateCommand() 
	
TeX has six modes according to the TeX Book:
	
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

*/

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
static int    ret = 0;
static int g_TeX_mode = MODE_VERTICAL;

char TexModeName[6][25] = {"vertical", "internal vertical", "horizontal", 
                        "restricted horizontal", "math", "displaymath"};

void SetTexMode(int mode)
{
	if (g_TeX_mode != mode) 
		diagnostics(4, "TeX mode now %s", TexModeName[mode]);
		
	if (g_TeX_mode == MODE_VERTICAL && mode == MODE_HORIZONTAL) {
		g_TeX_mode = mode;  /* prevent recursion */
		CmdStartParagraph(0);
	}
	
	g_TeX_mode = mode;
}

int GetTexMode(void)
{
	return g_TeX_mode;
}

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
	char            cNext;
	int				mode, count;

	RecursionLevel++;
	PushLevels();

	while ((cThis = getTexChar()) && cThis != '\0') {
	
		if (cThis == '\n')
			diagnostics(5, "Current character is '\\n' mode = %d", GetTexMode());
		else
			diagnostics(5, "Current character is '%c' mode = %d", cThis, GetTexMode());

		mode = GetTexMode();
		
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
			
			
		case '{':
			PushBrace();
			fprintRTF("{");
			break;
			
		case '}':
			ret = RecursionLevel - PopBrace();
			fprintRTF("}");
			if (ret > 0) {
				ret--;
				RecursionLevel--;
				return;
			}
			break;

		case ' ':
			if (mode==MODE_VERTICAL || mode==MODE_MATH || mode==MODE_DISPLAYMATH) 
			    	cThis = cLast;
			    						
			else if ( cLast != ' ' && cLast != '\n' ) { 

				if (GetTexMode()==MODE_RESTRICTED_HORIZONTAL)
					fprintRTF("\\~");
				else
					fprintRTF(" ");
			}
			
			break;
			
		case '\n': 
			tabcounter = 0;
			
			if (mode==MODE_MATH || mode==MODE_DISPLAYMATH) {
			
				cNext = getNonBlank();  	
				ungetTexChar(cNext);
			
			} else {
				cNext = getNonSpace(); 
				
				if (cNext == '\n') {			/* new paragraph ... skip all ' ' and '\n' */
					CmdEndParagraph(0);
					cNext = getNonBlank();  	
					ungetTexChar(cNext);
					
				} else {						/* add a space if needed */
					ungetTexChar(cNext);
					if (mode != MODE_VERTICAL && cLast != ' ')			
						fprintRTF(" ");  
				}               
			}
			break;


		case '$':
			cNext = getTexChar();
			diagnostics(5,"Processing $, next char <%c>",cNext);

			if (cNext == '$')
				CmdDisplayMath(EQN_DOLLAR_DOLLAR);
			else {
				ungetTexChar(cNext);
				CmdMath(EQN_DOLLAR);
			}

			/* 
			   Formulas need to close all Convert() operations when they end 
			   This works for \begin{equation} but not $$ since the BraceLevel
			   and environments don't get pushed properly.  We do it explicitly here.
			*/
			if (GetTexMode() == MODE_MATH || GetTexMode() == MODE_DISPLAYMATH)
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
			if (GetTexMode() == MODE_MATH || GetTexMode() == MODE_DISPLAYMATH) {	/* in eqnarray */
				fprintRTF("\\tab ");
				actCol++;
				break;
			}
			
			if (g_processing_tabular) {	/* in tabular */
				fprintRTF(" \\cell \\pard \\intbl ");
				fprintRTF("\\q%c ", colFmt[actCol]);
				actCol++;
				break;
			}
			fprintRTF("&");
			break;

		case '~':
			fprintRTF("\\~");
			break;			
			
		case '^':		
			CmdSuperscript(0);
			break;

		case '_':		
			CmdSubscript(0);
			break;

		case '-':
			if (mode == MODE_MATH || mode == MODE_DISPLAYMATH) 
				fprintRTF("-");
			else {
				SetTexMode(MODE_HORIZONTAL);
				
				count = getSameChar('-')+1;
				
				if (count == 1) 
					fprintRTF("-");
				else if (count == 2)
					fprintRTF("\\endash ");
				else if (count == 3)
					fprintRTF("\\emdash ");
				else 
					while (count--) fprintRTF("-");
			}
			break;
						
		case '\'':
			if (mode == MODE_MATH || mode == MODE_DISPLAYMATH)
					fprintRTF("'");	
			else {
				SetTexMode(MODE_HORIZONTAL);
				count = getSameChar('\'')+1;
				if (count == 2) 
					fprintRTF("\\rdblquote ");
				else
					while (count--) fprintRTF("\\rquote ");
			}
			break;
			
		case '`':
			SetTexMode(MODE_HORIZONTAL);
			count = getSameChar('`')+1;
			if (count == 2) 
				fprintRTF("\\ldblquote ");
			else
				while (count--) fprintRTF("\\lquote ");
			break;
			
		case '\"':
			SetTexMode(MODE_HORIZONTAL);
			if (GermanMode)
				TranslateGerman();
			else
				fprintRTF("\"");
			break;

		case '!':
			if (mode == MODE_MATH || mode == MODE_DISPLAYMATH)
				fprintRTF("!");	
			else {
				SetTexMode(MODE_HORIZONTAL);
				if ((cNext = getTexChar()) && cNext == '`') {
					fprintRTF("\\'a1 ");
				} else {
					fprintRTF("! ");
					ungetTexChar(cNext);
				}
			}
			break;	
			
		case '?':
			SetTexMode(MODE_HORIZONTAL);
			if ((cNext = getTexChar()) && cNext == '`') {
				fprintRTF("\\'bf ");
			} else {
				fprintRTF("? ");
				ungetTexChar(cNext);
			}
			break;
			
		case ':':
			if (mode == MODE_MATH || mode == MODE_DISPLAYMATH)
				fprintRTF(":");	
			else {
				SetTexMode(MODE_HORIZONTAL);
				fprintRTF(": ");
			}
			break;	

		case '.':
			if (mode == MODE_MATH || mode == MODE_DISPLAYMATH)
				fprintRTF(".");	
			else {
				SetTexMode(MODE_HORIZONTAL);
				fprintRTF(".");
	
				/* try to simulate double spaces after sentences */
				cNext = getTexChar();
				if (cNext == ' ' && (isalpha(cLast) && !isupper(cLast)))
					fprintRTF(" ");
				ungetTexChar(cNext);
			}
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '(':
		case ')':
		case '[':
		case ']':
			if (mode == MODE_MATH || mode == MODE_DISPLAYMATH)
				fprintRTF("{\\i0 %c}", cThis);	
			else {
				SetTexMode(MODE_HORIZONTAL);
				fprintRTF("%c", cThis);
			}
			break;
			
		case '\t':
			diagnostics(WARNING, "This should not happen, ignoring \\t");
			cThis = ' ';
			break;
			
		case '\r':
			diagnostics(WARNING, "This should not happen, ignoring \\r");
			cThis = ' ';
			break;
			
		case '%':
			diagnostics(WARNING, "This should not happen, ignoring %%");
			cThis = ' ';
			break;

		default:
			if (mode != MODE_MATH && mode != MODE_DISPLAYMATH)
				SetTexMode(MODE_HORIZONTAL);
				
			if ((int) cThis < 0)
				WriteEightBitChar(cThis);
			else
				fprintRTF("%c", cThis);
			break;
		}

		tabcounter++;
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
	int             i,mode;
	int            cThis;
	char            option_string[100];

	diagnostics(5, "Beginning TranslateCommand()");

	cThis = getTexChar();
	mode = GetTexMode();
	
	switch (cThis) {
	case '}':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		fprintRTF("\\}");
		return TRUE;
	case '{':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		fprintRTF("\\{");
		return TRUE;
	case '#':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		fprintRTF("#");
		return TRUE;
	case '$':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		fprintRTF("$");
		return TRUE;
	case '&':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		fprintRTF("&");
		return TRUE;
	case '%':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		fprintRTF("%%");
		return TRUE;
	case '_':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		fprintRTF("_");
		return TRUE;
		
	case '\\':		/* \\[1mm] or \\*[1mm] possible */

		cThis = getTexChar();
		if (cThis != '*')
			ungetTexChar(cThis);

		getBracketParam(option_string, 99);	

		if (g_processing_eqnarray) {	/* eqnarray */

			if (g_show_equation_number && !g_suppress_equation_number) {
				for (; actCol < 3; actCol++)
					fprintRTF("\\tab ");
				incrementCounter("equation");
				
				fprintRTF("\\tab {\\i0 (%d)}", getCounter("equation"));
			}
			fprintRTF("\\par\n\\tab ");
			g_suppress_equation_number = FALSE;
			actCol = 1;
			return TRUE;
		}
		
		if (g_processing_tabular) {	/* tabular or array environment */
			if (GetTexMode() == MODE_MATH || GetTexMode() == MODE_DISPLAYMATH) {	/* array */
				fprintRTF("\\par\n\\tab ");
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
			PopBrace();
			PushBrace();
		}

		/* simple end of line ... */
		CmdEndParagraph(0);
		CmdIndent(INDENT_INHIBIT);
		
		tabcounter = 0;
		if (tabbing_on && (fgetpos(fRtf, &pos_begin_kill) != 0))
				diagnostics(ERROR, "File access problem in tabbing environment");
		return TRUE;

	case ' ':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		fprintRTF(" ");	/* ordinary interword space */
		skipSpaces();
		return TRUE;

/* \= \> \< \+ \- \' \` all have different meanings in a tabbing environment */

	case '-':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		if (tabbing_on){
			(void) PopBrace();
			PushBrace();
		} else
			fprintRTF("\\-");
		return TRUE;

	case '+':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		if (tabbing_on){
			(void) PopBrace();
			PushBrace();
		}
		return TRUE;
		
	case '<':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		if (tabbing_on){
			(void) PopBrace();
			PushBrace();
		}
		return TRUE;

	case '>':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		if (tabbing_on){
			(void) PopBrace();
			CmdTabjump();
			PushBrace();
		} else 
			CmdSpace(0.50);  /* medium space */
		return TRUE;
		
	case '`':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		if (tabbing_on){
			(void) PopBrace();
			PushBrace();
		} else
			CmdLApostrophChar(0);
		return TRUE;
		
	case '\'':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		if (tabbing_on){
			(void) PopBrace();
			PushBrace();
			return TRUE;
		} else
			CmdRApostrophChar(0);	/* char ' =?= \' */
		return TRUE;

	case '=':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		if (tabbing_on){
			(void) PopBrace();
			CmdTabset();
			PushBrace();
		}
		else
			CmdMacronChar(0);
		return TRUE;
		
	case '~':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		CmdTildeChar(0);
		return TRUE;
	case '^':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		CmdHatChar(0);
		return TRUE;
	case '.':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		CmdDotChar(0);
		return TRUE;
	case '\"':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		CmdUmlauteChar(0);
		return TRUE;
	case '(':
		CmdMath(EQN_RND_OPEN);
		PushBrace();
		return TRUE;
	case '[':
		CmdDisplayMath(EQN_BRACKET_OPEN);
		PushBrace();
		return TRUE;
	case ')':
		CmdMath(EQN_RND_CLOSE);
		ret = RecursionLevel - PopBrace();
		return TRUE;
	case ']':
		CmdDisplayMath(EQN_BRACKET_CLOSE);
		ret = RecursionLevel - PopBrace();
		return TRUE;
	case '/':
		CmdIgnore(0);		/* italic correction */
		return TRUE;
	case '!':
		CmdIgnore(0);		/* \! negative thin space */
		return TRUE;
	case ',':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		CmdSpace(0.33);	/* \, produces a small space */
		return TRUE;
	case ';':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
		CmdSpace(0.75);	/* \; produces a thick space */
		return TRUE;
	case '@':
		CmdIgnore(0);	/* \@ produces an "end of sentence" space */
		return TRUE;
	case '3':
		if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
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
