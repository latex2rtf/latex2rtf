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

extern bool     g_processing_equation;	/* true at a formula-convertion */

void
CmdFormula(int code)
/******************************************************************************
 purpose   : sets the Math-Formula-Mode depending on the code-parameter
 parameter : type of braces which include the formula
 globals   : g_processing_equation
 ******************************************************************************/
{

	if (code & ON) {	/* this is only true if starting \begin{math} */
		fprintRTF("{\\i ");
		g_processing_equation = TRUE;
		diagnostics(4, "Switching g_processing_equation on with \\begin{math}");
		return;
	}
	
	switch (code) {
	case FORM_NO_NUMBER:
		if (g_processing_eqnarray)
			g_suppress_equation_number = TRUE;
		break;

	case FORM_DOLLAR:
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

	case FORM_RND_OPEN:	/* \( */
		fprintRTF("{\\i ");
		g_processing_equation = TRUE;
		diagnostics(4, "Switching g_processing_equation on with \\(");
		break;

	case FORM_RND_CLOSE:	/* \) */
		fprintRTF("}");
		g_processing_equation = FALSE;
		diagnostics(4, "Switching g_processing_equation off with \\)");
		break;

	case FORM_ECK_OPEN:	/* \[ */
		fprintRTF("\n\\par{\\i  ");
		g_processing_equation = TRUE;
		diagnostics(4, "Switching g_processing_equation on with \\[");
		break;

	case FORM_ECK_CLOSE:	/* \] */
		fprintRTF("}\n\\par ");
		g_processing_equation = FALSE;
		bNewPar = TRUE;
		diagnostics(4, "Switching g_processing_equation off with \\]");
		break;

	case FORM_MATH:	/* will only be encountered for \end{math} */
		fprintRTF("}");
		g_processing_equation = FALSE;
		diagnostics(4, "Switching g_processing_equation off with \\end{math}");
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
	if ((code & ON) || ((code == FORM_DOLLAR) && !g_processing_equation)) {	/* on switch */
		code &= ~(ON);	/* mask MSB */
		g_processing_equation = TRUE;
		g_suppress_equation_number = FALSE;
		
		fprintRTF("\n\\par\n\\par\\pard");
		switch (code) {
		case FORM_DOLLAR:	/* $$ or displaymath */
		case EQUATION_1:	/* equation* */
			g_show_equation_number = FALSE;
			fprintRTF("\\tqc\\tx4320\n");
			diagnostics(4,"Entering CmdFormula2 -- displaymath");
			break;

		case EQUATION:	/* equation */
			g_show_equation_number = TRUE;
			fprintRTF("\\tqc\\tx4320\\tqr\\tx8640\n");
			diagnostics(4,"Entering CmdFormula2 -- equation");
			break;

		case EQNARRAY_1:	/* eqnarray* */
			g_show_equation_number = FALSE;
			g_processing_eqnarray = TRUE;
			g_processing_tabular = TRUE;
			actCol = 1;
			diagnostics(4,"Entering CmdFormula2 -- eqnarray* ");
			fprintRTF("\\tqr\\tx2880\\tqc\\tx3240\\tql\\tx3600\n");
			break;

		case EQNARRAY:	/* eqnarray */
			g_show_equation_number = TRUE;
			g_processing_eqnarray = TRUE;
			g_processing_tabular = TRUE;
			actCol = 1;
		    diagnostics(4,"Entering CmdFormula2 --- eqnarray ");
			fprintRTF("\\tqr\\tx2880\\tqc\\tx3240\\tql\\tx3600\\tqr\\tx8640\n");
			break;

		default:;
		}
		fprintRTF("\\tab {\\i ");
				
		
	} else {		/* off switch */
		diagnostics(4,"Exiting CmdFormula2");
		code &= ~(OFF);	/* mask MSB */
		g_processing_equation = FALSE;
		fprintRTF("}");
		
/* close the equation environment properly */
		if (g_show_equation_number && !g_suppress_equation_number) {
			incrementCounter("equation");
			fprintRTF("\\tab (%d)", getCounter("equation"));
		}
		fprintRTF("\n\\par\n\\par");

		if (code == EQNARRAY || code == EQNARRAY_1) {
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

