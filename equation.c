#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "convert.h"
#include "commands.h"
#include "stack.h"
#include "l2r_fonts.h"
#include "cfg.h"
#include "ignore.h"
#include "util.h"
#include "parser.h"
#include "equation.h"
#include "counters.h"
#include "funct1.h"
#include "lengths.h"

extern bool     g_processing_equation;	/* true at a formula-convertion */

void
CmdFormula(int code)
/******************************************************************************
 purpose   : sets the Math-Formula-Mode depending on the code-parameter
 parameter : type of braces which include the formula
 globals   : g_processing_equation
 ******************************************************************************/
{
	int true_code = code & ~ON;
	
	switch (true_code) {
	
		case EQN_MATH:
			if (code & ON) {
				fprintRTF("\\i ");
				g_processing_equation = TRUE;
				diagnostics(4, "Switching g_processing_equation on with \\begin{math}");
			} else {
				g_processing_equation = FALSE;
				diagnostics(4, "Switching g_processing_equation off with \\end{math}");
			}
			break;
	
		case EQN_NO_NUMBER:
			if (g_processing_eqnarray)
				g_suppress_equation_number = TRUE;
			break;
		
		case EQN_DOLLAR:
			if (g_processing_equation) {
				fprintRTF("}");
				g_processing_equation = FALSE;
				diagnostics(4, "Switching g_processing_equation off with $");
			} else {
				fprintRTF("{\\i ");
				g_processing_equation = TRUE;
				diagnostics(4, "Switching g_processing_equation on with $");
			}
			break;
	
		case EQN_RND_OPEN:	/* \( */
			fprintRTF("{\\i ");
			g_processing_equation = TRUE;
			diagnostics(4, "Switching g_processing_equation on with \\(");
			break;
	
		case EQN_RND_CLOSE:	/* \) */
			fprintRTF("}");
			g_processing_equation = FALSE;
			diagnostics(4, "Switching g_processing_equation off with \\)");
			break;
	}
}

void 
CmdFormula2(int code)
/******************************************************************************
 purpose: creates a displayed equation
          \begin{equation} gets a right justified equation number
          \begin{displaymath} gets no equation number
          \[ gets no equation number
          $$ gets no equation number
 ******************************************************************************/
{
	int width, mid, true_code,a,b,c;
	width = getLength("textwidth");
	mid = width/2;
	true_code = code & ~ON;
	
	if (true_code == EQN_DOLLAR_DOLLAR) {
		if (!g_processing_equation) {
			g_processing_equation = TRUE;
			g_show_equation_number = FALSE;
			fprintRTF("\\par\\par\n{\\pard\\i\\tqc\\tx%d\n\\tab ", mid);
			diagnostics(4,"Entering CmdFormula2 -- $$");
		} else {
			g_processing_equation = FALSE;
			fprintRTF("\\par\\par\n}");
			diagnostics(4,"Exiting CmdFormula2 -- $$");
		}
		return;
	}
	
	if (true_code == EQN_BRACKET_OPEN) {
		g_processing_equation = TRUE;
		g_show_equation_number = TRUE;
		fprintRTF("\\par\\par\n{\\pard\\i\\tqc\\tx%d\\tqr\\tx%d\n\\tab ", mid, width);
		diagnostics(4,"Entering CmdFormula2 -- \\[");
		return;
	}

	if (true_code == EQN_BRACKET_CLOSE) {
		g_processing_equation = FALSE;
		fprintRTF("\\par\\par\n}");
		diagnostics(4,"Exiting CmdFormula2 -- \\]");
		return;
	}

	if (code & ON) {  /* \begin{equation}, etc. */

		g_processing_equation = TRUE;
		g_suppress_equation_number = FALSE;
		
		a = 0.25 *width;
		b = 0.3 * width;
		c = 0.35 * width;
		fprintRTF("\\par\\par\n\\pard\\i");
		switch (true_code) {
		case EQN_DISPLAYMATH:
			g_show_equation_number = FALSE;
			fprintRTF("\\tqc\\tx%d", mid);
			diagnostics(4,"Entering CmdFormula2 -- displaymath");
			break;

		case EQN_EQUATION_STAR:
			g_show_equation_number = FALSE;
			fprintRTF("\\tqc\\tx%d", mid);
			diagnostics(4,"Entering CmdFormula2 -- equation*");
			break;

		case EQN_EQUATION:
			actCol = 5;							/* avoid adding \tabs */
			g_show_equation_number = TRUE;
			fprintRTF("\\tqc\\tx%d\\tqr\\tx%d", mid, width);
			diagnostics(4,"Entering CmdFormula2 -- equation");
			break;

		case EQN_ARRAY_STAR:
			g_show_equation_number = FALSE;
			g_processing_eqnarray = TRUE;
			g_processing_tabular = TRUE;
			actCol = 1;
			diagnostics(4,"Entering CmdFormula2 -- eqnarray* ");
			fprintRTF("\\tqr\\tx%d\\tqc\\tx%d\\tql\\tx%d", a, b, c);
			break;

		case EQN_ARRAY:
			g_show_equation_number = TRUE;
			g_processing_eqnarray = TRUE;
			g_processing_tabular = TRUE;
			actCol = 1;
		    diagnostics(4,"Entering CmdFormula2 --- eqnarray");
			fprintRTF("\\tqr\\tx%d\\tqc\\tx%d\\tql\\tx%d\\tqr\\tx%d ", a, b, c, width);
			break;
		}
		fprintRTF("\n\\tab ");
				
		
	} else {
	
		diagnostics(4,"Exiting CmdFormula2");
		g_processing_equation = FALSE;
		
		if (g_show_equation_number && !g_suppress_equation_number) {
			incrementCounter("equation");
			for (; actCol < 3; actCol++)
					fprintRTF("\\tab ");
			fprintRTF("\\tab{\\i0 (%d)}", getCounter("equation"));
		}
		fprintRTF("\\par\\par\n");

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
	denominator = getParam();

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

