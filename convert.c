/* $Id: convert.c,v 1.23 2001/11/13 05:43:56 prahl Exp $ 
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

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
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
 ******************************************************************************/
{
	char            temp[51];
	
	strncpy(temp,string,50);

	if (PushSource(NULL, string)) {
		diagnostics(5, "Entering Convert() from StringConvert() <%s>",temp);

		while (StillSource())
			Convert();
			
		PopSource();
		diagnostics(5, "Exiting Convert() from StringConvert()");
	}
}

void 
ConvertAllttString(char *s)
/******************************************************************************
     purpose : converts string in TeX-format to Rtf-format
			   according to the alltt environment, which is like
			   verbatim environment except that \, {, and } have
			   their usual meanings
******************************************************************************/
{	
	char cThis;
	diagnostics(4, "Entering Convert() from StringAllttConvert()");

	if (PushSource(NULL, s)) {

		while (StillSource()) {

			cThis = getRawTexChar();   /* it is a verbatim like environment */
			switch (cThis) {
			
				case '\\':
					PushLevels();	
					TranslateCommand();
					CleanStack();
					break;
					
				case '{':
					PushBrace();
					fprintRTF("{");
					break;
				
				case '}':			
					ret = RecursionLevel - PopBrace();
					fprintRTF("}");
					break;
				
				default:
					putRtfChar(cThis);
					break;
			}
		}
		PopSource();
	}

	diagnostics(4, "Exiting Convert() from StringAllttConvert()");
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

	diagnostics(3, "Entering Convert ret = %d", ret);
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
			
			TranslateCommand();

			CleanStack();

			if (ret > 0) {
				ret--;
				RecursionLevel--;
				diagnostics(5, "Exiting Convert via TranslateCommand ret = %d", ret);
				return;
			}
			break;
			
			
		case '{':
			CleanStack();
			PushBrace();
			fprintRTF("{");
			break;
			
		case '}':
			CleanStack();
			ret = RecursionLevel - PopBrace();
			fprintRTF("}");
			if (ret > 0) {
				ret--;
				RecursionLevel--;
				diagnostics(5, "Exiting Convert via '}' ret = %d", ret);
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

			if (cNext == '$' && GetTexMode() != MODE_MATH)
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
					diagnostics(5, "Exiting Convert via Math ret = %d", ret);
					return;
				}
			}
			
			break;

		case '&':
			if (g_processing_arrays) {
				fprintRTF("%c",FORMULASEP);
				break;
			}

			if (GetTexMode() == MODE_MATH || GetTexMode() == MODE_DISPLAYMATH) {	/* in eqnarray */
				fprintRTF("\\tab ");
				g_equation_column++;
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

		case '<':
			if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
			if (GetTexMode() == MODE_HORIZONTAL && FrenchMode ){
				cNext = getTexChar();
				if (cNext == '<')
					fprintRTF("\\'ab");
				else {
					ungetTexChar(cNext);
					fprintRTF("<");
				}
			}
			break;

		case '>':
			if (mode == MODE_VERTICAL) SetTexMode(MODE_HORIZONTAL);
			if (GetTexMode() == MODE_HORIZONTAL && FrenchMode ){
				cNext = getTexChar();
				if (cNext == '>')
					fprintRTF("\\'bb");
				else {
					ungetTexChar(cNext);
					fprintRTF(">");
				}
			}
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


#ifdef SEMICOLONSEP
		case ';':
			if (g_processing_fields)
				fprintRTF("\\\\;");
			else
				fprintRTF(";");
		break;
#else
		case ',':
			if (g_processing_fields)
				fprintRTF("\\\\,");
			else
				fprintRTF(",");
			break;
#endif
			
		default:
			if (mode == MODE_MATH || mode == MODE_DISPLAYMATH) {
				if (('a' <= cThis && cThis <= 'z') || ('A' <= cThis && cThis <= 'Z'))
					fprintRTF("{\\i %c}", cThis);	
				else
					fprintRTF("%c", cThis);	

			} else {
			
				SetTexMode(MODE_HORIZONTAL);
				fprintRTF("%c", cThis);
			}
			break;
		}

		tabcounter++;
		cLast = cThis;
	}
	RecursionLevel--;
	diagnostics(5, "Exiting Convert via exhaustion ret = %d", ret);
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


	cThis = getTexChar();
	mode = GetTexMode();
	
	diagnostics(5, "Beginning TranslateCommand() \\%c", cThis);

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
		CmdSlashSlash(0);
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

			if (cThis == '%'){			/* put the % back and get the next char */
				ungetTexChar('%');
				cThis=getTexChar();
			}

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

		cThis = getRawTexChar();	/* Necessary because % ends a command */
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
