/***************************************************************************
purpose : Handles LaTeX commands specific to the letter format
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "parser.h"
#include "letterformat.h"
#include "cfg.h"
#include "util.h"
#include "commands.h"
#include "funct1.h"
#include "convert.h"

static bool g_letterOpened = FALSE;   /* true after \opening       */

static char * g_letterToAddress    = NULL;
static char * g_letterReturnAddress= NULL;
static char * g_letterSignature    = NULL;

void
CmdSignature(int code)
/******************************************************************************
 purpose: saves the signature in the letter document style
          Use in the preamble or between \begin{letter} and \opening
 ******************************************************************************/
{
	if (g_letterSignature) 
		free(g_letterSignature);
	g_letterSignature = getParam();
}

void 
CmdAddress(int code)
/******************************************************************************
 purpose: saves the address in the letter document style
          Use in the preamble or between \begin{letter} and \opening
 ******************************************************************************/
{
	if (g_letterReturnAddress) 
		free(g_letterReturnAddress);
	g_letterReturnAddress = getParam();
}

void 
CmdLetter(int code)
/******************************************************************************
  purpose: pushes all necessary letter-commands on a stack
parameter: code: on/off-option for environment
 ******************************************************************************/
{	
	if (code & ON) {	/* on switch */
		code &= ~(ON);	/* mask MSB */
		PushEnvironment(code);
		if (g_letterToAddress) 
			free(g_letterToAddress);
		g_letterToAddress = getParam();
	} else {		/* off switch */
		PopEnvironment();
	}
}

void 
CmdOpening(int code)
/******************************************************************************
 purpose: /opening{Dear...} prints return address, Date, to address and opening
 ******************************************************************************/
{
	char     oldalignment;
	char    *s;

/* put return address and date at the top right */
	g_letterOpened = TRUE;
	alignment = RIGHT;
	fprintRTF("\n\\par\\pard\\q%c ", alignment);
	diagnostics(5, "Entering ConvertString() from CmdAddress");
	ConvertString(g_letterReturnAddress);
	diagnostics(5, "Exiting ConvertString() from CmdAddress");

/* put the date on the right */
	fprintRTF("\\par\\chdate ");
	
/* put addressee on the left */
	alignment = LEFT;
	fprintRTF("\n\\par\\pard\\q%c ", alignment);
	diagnostics(4, "Entering Convert() from CmdOpening");
	ConvertString(g_letterToAddress);	
	diagnostics(4, "Exiting Convert() from CmdOpening");

/*  finally print the opening*/
	s = getParam();
	ConvertString(s);	
	free(s);
	
	alignment = oldalignment;
	fprintRTF("\n\\par\\pard\\q%c ", alignment);
}

void 
CmdClosing( /* @unused@ */ int code)
/******************************************************************************
 purpose: special command in the LaTex-letter-environment will be converted to a
	  similar Rtf-style
 globals: alignment
 ******************************************************************************/
{
	char           oldalignment;
	char           *s;
	
	oldalignment = alignment;

/* print closing on the right */
	alignment = RIGHT;
	fprintRTF("\n\\par\\pard\\q%c ", alignment);	
	diagnostics(5, "Entering ConvertString() from CmdClosing");
	s = getParam();
	ConvertString(s);
	free(s);
	diagnostics(5, "Exiting ConvertString() from CmdClosing");

/* print signature a couple of lines down */
	fprintRTF("\n\\par\\par\\par ");

	diagnostics(5, "Entering ConvertString() from CmdSignature");
	ConvertString(g_letterSignature);
	diagnostics(5, "Exiting ConvertString() from CmdSignature");

	g_letterOpened = FALSE;
	alignment = oldalignment;
	fprintRTF("\n\\par\\pard\\q%c ", alignment);
}

void 
CmdPs(int code)
/******************************************************************************
 purpose: translate encl and cc into appropriate language
 ******************************************************************************/
{
	char * s = getParam();
	
	if (code == LETTER_ENCL)
		ConvertBabelName("ENCLNAME");
	else if (code == LETTER_CC)
		ConvertBabelName("CCNAME");
	
	ConvertString(s);
	free(s);
}

