#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "convert.h"
#include "commands.h"
#include "stack.h"
#include "l2r_fonts.h"
#include "cfg.h"
#include "ignore.h"
#include "parser.h"
#include "equation.h"
#include "counters.h"
#include "funct1.h"
#include "lengths.h"

void
CmdSuperscript(int code)
/******************************************************************************
 purpose   : Handles superscripts ^\alpha, ^a, ^{a} and \textsuperscript{a}
 ******************************************************************************/
{
	char           *s = NULL;
	int  size, newsize, upsize;

	if ((s = getMathParam())) {
		size = CurrentFontSize();
		newsize = size / 1.2;
		upsize = size / 3;
		fprintRTF("{\\up%d\\fs%d ",upsize,newsize);
		ConvertString(s);
		fprintRTF("}");
		free(s);
	}
}

void
CmdSubscript(int code)
/******************************************************************************
 purpose   : Handles superscripts ^\alpha, ^a, ^{a}
 ******************************************************************************/
{
	char           *s = NULL;
	int  size, newsize, upsize;

	if ((s = getMathParam())) {
		size = CurrentFontSize();
		newsize = size / 1.2;
		upsize = size / 3;
		fprintRTF("{\\dn%d\\fs%d ",upsize,newsize);
		ConvertString(s);
		fprintRTF("}");
		free(s);
	}
}

void
CmdNonumber(int code)
/******************************************************************************
 purpose   : Handles \nonumber to suppress numbering in equations
 ******************************************************************************/
{	
	if (g_processing_eqnarray || !g_processing_tabular)
		g_suppress_equation_number = TRUE;
}

void
CmdMath(int code)
/******************************************************************************
 purpose   : sets the TeX mode to math or horizontal as appropriate
             for $...$ \( ... \) and \begin{math} ... \end{math}
 ******************************************************************************/
{
	int true_code = code & ~ON;
	
	switch (true_code) {
	
		case EQN_MATH:
			if (code & ON) {
				diagnostics(4, "CmdMath() ... \\begin{math}");
				fprintRTF("\\i ");
				SetTexMode(MODE_MATH);
			} else {
				diagnostics(4, "CmdMath() ... \\end{math}");
				SetTexMode(MODE_HORIZONTAL);
			}
			break;
	
		case EQN_DOLLAR:
			if (GetTexMode() != MODE_MATH) {
				diagnostics(4, "Exiting CmdMath() ... $");
				fprintRTF("{\\i ");
				SetTexMode(MODE_MATH);
			} else {
				diagnostics(4, "Entering CmdMath() ... $");
				fprintRTF("}");
				SetTexMode(MODE_HORIZONTAL);
			}
			break;
	
		case EQN_RND_OPEN:	/* \( */
			diagnostics(4, "CmdMath() ... \\(");
			fprintRTF("{\\i ");
			SetTexMode(MODE_MATH);
			break;
	
		case EQN_RND_CLOSE:	/* \) */
			diagnostics(4, "CmdMath() ... \\)");
			fprintRTF("}");
			SetTexMode(MODE_HORIZONTAL);
			break;
	}
}

void 
CmdDisplayMath(int code)
/******************************************************************************
 purpose: creates a displayed equation
          \begin{equation} gets a right justified equation number
          \begin{displaymath} gets no equation number
          \[ gets no equation number
          $$ gets no equation number
 ******************************************************************************/
{
	int width, mid, mode, true_code,a,b,c;
	width = getLength("textwidth");
	mid = width/2;
	mode = GetTexMode();
	true_code = code & ~ON;
	
	if (true_code == EQN_DOLLAR_DOLLAR) {
		if (mode != MODE_DISPLAYMATH) {
			diagnostics(4,"Entering CmdDisplayMath -- $$");
			CmdEndParagraph(0);
			SetTexMode(MODE_DISPLAYMATH);
			g_show_equation_number = FALSE;
			fprintRTF("{\\pard\\i\\tqc\\tx%d\\tab ", mid);
		} else {
			diagnostics(4,"Exiting CmdDisplayMath -- $$");
			CmdEndParagraph(0);
			CmdIndent(INDENT_INHIBIT);
			fprintRTF("}");
		}
		return;
	}
	
	if (true_code == EQN_BRACKET_OPEN) {
		diagnostics(4,"Entering CmdDisplayMath -- \\[");
		SetTexMode(MODE_DISPLAYMATH);
		g_show_equation_number = TRUE;
		fprintRTF("\\par\\par\n{\\pard\\i\\tqc\\tx%d\\tqr\\tx%d\n\\tab ", mid, width);
		return;
	}

	if (true_code == EQN_BRACKET_CLOSE) {
		diagnostics(4,"Exiting CmdDisplayMath -- \\]");
		SetTexMode(MODE_VERTICAL);
		fprintRTF("\\par\\par\n}");
		return;
	}

	if (code & ON) {  /* \begin{equation}, etc. */

		g_suppress_equation_number = FALSE;
		
		a = 0.25 *width;
		b = 0.3 * width;
		c = 0.35 * width;
		fprintRTF("\\par\\par\n\\pard\\i");
		switch (true_code) {
		case EQN_DISPLAYMATH:
			diagnostics(4,"Entering CmdDisplayMath -- displaymath");
			g_show_equation_number = FALSE;
			fprintRTF("\\tqc\\tx%d", mid);
			break;

		case EQN_EQUATION_STAR:
			diagnostics(4,"Entering CmdDisplayMath -- equation*");
			g_show_equation_number = FALSE;
			fprintRTF("\\tqc\\tx%d", mid);
			break;

		case EQN_EQUATION:
			diagnostics(4,"Entering CmdDisplayMath -- equation");
			actCol = 5;							/* avoid adding \tabs when finishing */
			g_show_equation_number = TRUE;
			fprintRTF("\\tqc\\tx%d\\tqr\\tx%d", mid, width);
			break;

		case EQN_ARRAY_STAR:
			diagnostics(4,"Entering CmdDisplayMath -- eqnarray* ");
			g_show_equation_number = FALSE;
			g_processing_eqnarray = TRUE;
			g_processing_tabular = TRUE;
			actCol = 1;
			fprintRTF("\\tqr\\tx%d\\tqc\\tx%d\\tql\\tx%d", a, b, c);
			break;

		case EQN_ARRAY:
		    diagnostics(4,"Entering CmdDisplayMath --- eqnarray");
			g_show_equation_number = TRUE;
			g_processing_eqnarray = TRUE;
			g_processing_tabular = TRUE;
			actCol = 1;
			fprintRTF("\\tqr\\tx%d\\tqc\\tx%d\\tql\\tx%d\\tqr\\tx%d ", a, b, c, width);
			break;
		}
		fprintRTF("\n\\tab ");
		SetTexMode(MODE_DISPLAYMATH);
		
	} else {
	
		diagnostics(4,"Exiting CmdDisplayMath");
		
		if (g_show_equation_number && !g_suppress_equation_number) {
			incrementCounter("equation");
			for (; actCol < 3; actCol++)
					fprintRTF("\\tab ");
			fprintRTF("\\tab{\\i0 (%d)}", getCounter("equation"));
		}

		CmdEndParagraph(0);
		CmdIndent(INDENT_INHIBIT);

		if (true_code == EQN_ARRAY || true_code == EQN_ARRAY_STAR) {
			g_processing_tabular = FALSE;
			g_processing_eqnarray = FALSE;
		}
	}
}

void 
CmdRoot(int code)
/******************************************************************************
 purpose: converts \sqrt{x} or \root[\alpha]{x+y}
******************************************************************************/
{
	char           *root;
	char           power[50];

	getBracketParam(power, 49);
	root = getParam();
	fprintRTF("{\\field{\\*\\fldinst  EQ \\\\R(");
	if (strlen(power)>0)
		ConvertString(power);
	fprintRTF("%c", FORMULASEP);
	ConvertString(root);
	fprintRTF(")}{\\fldrslt }}");
	free(root);
}

void 
CmdFraction(int code)
/******************************************************************************
 purpose: converts \frac{x}{y} (following Taupin's implementation in ltx2rtf)
******************************************************************************/
{
	char           *denominator, *numerator;

	numerator = getParam();
	skipSpaces();
	denominator = getParam();

	diagnostics(4,"CmdFraction -- numerator   = <%s>", numerator);
	diagnostics(4,"CmdFraction -- denominator = <%s>", denominator);

	fprintRTF("{\\field{\\*\\fldinst  EQ \\\\F(");
	ConvertString(numerator);
	fprintRTF("%c", FORMULASEP);
	ConvertString(denominator);
	fprintRTF(")}{\\fldrslt }}");

	free(numerator);
	free(denominator);
}

void 
CmdIntegral(int code)
/******************************************************************************
 purpose: converts integral symbol + the "exponent" and "subscript" fields
parameter: type of operand
 ******************************************************************************/
{
	char           *upper_limit = NULL;
	char           *lower_limit = NULL;
	char            cThis;

	/* is there an exponent/subscript ? */
	cThis = getNonBlank();

	if (cThis == '_')
		lower_limit = getMathParam();
	else if (cThis == '^')
		upper_limit = getMathParam();
	else
		ungetTexChar(cThis);

	if (upper_limit || lower_limit) {
		cThis = getNonBlank();
		if (cThis == '_')
			lower_limit = getMathParam();
		else if (cThis == '^')
			upper_limit = getMathParam();
		else
			ungetTexChar(cThis);
	}

	fprintRTF("{\\field{\\*\\fldinst  EQ \\\\I");
	  switch(code)
	  {
		case 0 : fprintRTF("\\\\in("); break;	
		case 1 : fprintRTF("\\\\su("); break;
		case 2 : fprintRTF("\\\\pr("); break;
		default: error("Illegal code to CmdIntegral");
	  }

	if (lower_limit)
		ConvertString(lower_limit);
	fprintRTF("%c", FORMULASEP);
	if (upper_limit)
		ConvertString(upper_limit);
	fprintRTF("%c)}{\\fldrslt }}", FORMULASEP);

	if (lower_limit)
		free(lower_limit);
	if (upper_limit)
		free(upper_limit);
}

