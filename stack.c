/*
 * $Id: stack.c,v 1.11 2001/08/12 19:40:25 prahl Exp $
 * History:
 * $Log: stack.c,v $
 * Revision 1.11  2001/08/12 19:40:25  prahl
 * 1.9g
 *         Added commands to read and set TeX lengths
 *         Added commands to read and set TeX counters
 *         Fixed bug in handling of \item[text]
 *         Eliminated comparison of fpos_t variables
 *         Revised getLinenumber ... this is not perfect
 *         Fixed bug in getTexChar() routine
 *         Clearly separate preamble from the document in hopes that
 *           one day more appropriate values for page size, margins,
 *           paragraph spacing etc, will be used in the RTF header
 *         I have added initial support for page sizes still needs testing
 *         added two more test files misc3.tex and misc4.tex
 *         misc4.tex produces a bad rtf file currently
 *         separated all letter commands into letterformat.c
 *         cleaned up warning calls throughout code
 *         added \neq \leq \geq \mid commands to direct.cfg
 *         collected and added commands to write RTF header in preamble.c
 *         broke isolatin1 and hyperlatex support, these will be fixed next version
 *
 * Revision 1.5  1998/07/03 07:03:16  glehner
 * lclint cleaning
 *
 * Revision 1.4  1997/02/15 20:29:45  ralf
 * Did some corrections for lclint checking
 *
 * Revision 1.3  1995/05/10 06:37:43  ralf
 * Added own includefile (for consistency checking of decls)
 *
 * Revision 1.2  1995/03/23  15:58:08  ralf
 * Reworked version by Friedrich Polzer and Gerhard Trisko
 *
 * Revision 1.1  1994/06/17  11:26:29  ralf
 * Initial revision
 *
 */
/***************************************************************************
     name : stack.c
    autor : DORNER Fernando, GRANZER Andreas
  purpose : this is an stack-model to handle braces and recursive calls
	        created by environments, and open and closing-braces
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "stack.h"

#define STACKSIZE 300

static int      stack[STACKSIZE];
static int      top = 0;

int 
Push(int lev, int brack)
/******************************************************************************
  purpose: pushes the parameter lev and brack on the stack
parameter: lev...level
	   brack...brackets
  globals: progname
 return: top of stack
 ******************************************************************************/
{
	diagnostics(5,"pushing rec=%d and bra=%d on  stack",lev,brack);
	++top;
	stack[top] = lev;
	++top;
	stack[top] = brack;

	if (top >= STACKSIZE) {
		fprintf(stderr, "\n%s: ERROR: too deep nesting -> internal stack-overflow", progname);
		fprintf(stderr, "\nprogram aborted\n");
		exit(EXIT_FAILURE);
	}
	return top;
}

int 
Pop(int *lev, int *brack)
/******************************************************************************
  purpose: pops the parameter lev and brack from the stack
parameter: lev...level
	   brack...brackets
  globals: progname
           latexname
           linenumber
 return: top of stack
 ******************************************************************************/
{
	*brack = stack[top];
	--top;
	*lev = stack[top];
	--top;


	if (top < 0) {
		fprintf(stderr, "\n%s: ERROR: error in LaTeX-File: %s  ", progname, latexname);
		fprintf(stderr, "line:%ld", getLinenumber());
		fprintf(stderr, "\n-> internal stack-underflow");
		fprintf(stderr, "\nprogram aborted\n");
		exit(EXIT_FAILURE);
	}

	diagnostics(5,"popped rec=%d and bra=%d off stack",*lev,*brack);
	return top;
}

int
getStackRecursionLevel(void)
/******************************************************************************
  purpose: returns the recursion level for the current BracketLevel
 ******************************************************************************/
{
int             PopLevel, PopBrack, PPopLevel, PPopBrack, size;

		PPopLevel = RecursLevel;
		PPopBrack = BracketLevel;
		size = Pop(&PopLevel, &PopBrack);
		while ((size = Pop(&PopLevel, &PopBrack)) >= 0) {
			if (PopBrack < BracketLevel) {
				break;
			}
			PPopLevel = PopLevel;
			PPopBrack = PopBrack;
		}	/* while */
		(void) Push(PopLevel, PopBrack);	/* push back */
		(void) Push(PPopLevel, BracketLevel);
		return PPopLevel;
}

void
CleanStack(void)
/******************************************************************************
  purpose: removes multiple identical copies on stack
 ******************************************************************************/
{
int             PopLevel = 0, PopBrack, PPopLevel, PPopBrack, size;

	for (;;) {
		if ((size = Pop(&PPopLevel, &PPopBrack)) <= 0) {
			(void) Push(PPopLevel, PPopBrack);
			break;
		}
		if ((size = Pop(&PopLevel, &PopBrack)) <= 0) {
			(void) Push(PopLevel, PopBrack);
			break;
		}
		if ((PPopLevel == PopLevel) && (PPopBrack == PopBrack)) {
			(void) Push(PopLevel, PopBrack);
		} else {
			(void) Push(PopLevel, PopBrack);
			(void) Push(PPopLevel, PPopBrack);
			break;
		}
	}
}


/* The use of stack 

The stack keeps track of the RecursLevel and BracketLevel for each command.

Each stack element consists of two integers RecursLevel and BracketLevel. 

   RecursLevel is the number of recursive calls of Convert()
   BracketLevel is the number of open curly braces. 
   
The value on top of stack represents the current value of the these two
global variables (RecursLevel and BracketLevel).

Before every command and before an opening curly brace the current settings are
written to the stack.  On appearance of a closing curly brace the
corresponding RecursLevel is found by searching the stack. 

It is the lowest
RecursLevel with the same BracketLevel as now (after subtract of the 1
closing brace found). The initial value RecLev 1, BracketLev 0 remains
always on the stack. The begin document command Pushes 1,1

For example:

{ Text {\em Text} Text }
1      2 3	4      5

1 Push 12
2 Push 13
3 Push 23
4 Bracket 3->2 Pop 23 Pop 13 Pop 12 Pop 11 -found- Push back 11
  return to level 1
5 Bracket 2->1
  return to level 1 = current -> no return

\mbox{\em Text}
1    2 3      4
1 Push 11  RecursLevel+1
2 Push 22
3 Push 32
4 Bracket 2->1 Pop 32 Pop 22 Pop 11 -found-
  return to level 1 from level 3 -> double return from convert

The necessary Push before every command increases the stack size. If the
commands don't include a recursive call the stack is not cleaned up.
After every TranslateCommand-function the stack is cleaned

For example:

\ldots \LaTeX \today \TeX
 1	2      3      4
1 Push 11
2 Push 11
3 Push 11
4 Push 11
The clean-up loop pops till the values are not identical and pushes back the last
Therefore 11 will only occur once on the stack.
*/
