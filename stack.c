/*
 * $Id: stack.c,v 1.9 2001/08/12 19:00:04 prahl Exp $
 * History:
 * $Log: stack.c,v $
 * Revision 1.9  2001/08/12 19:00:04  prahl
 * 1.9e
 *         Revised all the accented character code using ideas borrowed from ltx2rtf.
 *         Comparing ltx2rtf and latex2rtf indicates that Taupin and Lehner tended to work on
 *         different areas of the latex->rtf conversion process.  Adding
 *         accented characters is the first step in the merging process.
 *
 *         Added MacRoman font handling (primarily to get the breve accent)
 *         Now supports a wide variety of accented characters.
 *         (compound characters only work under more recent versions of word)
 *         Reworked the code to change font sizes.
 *         Added latex logo code from ltx2rtf
 *         Extracted character code into separate file chars.c
 *         Fixed bug with \sf reverting to roman
 *         Added two new testing files fontsize.tex and accentchars.tex
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
  purpose : this is an stack-model which handles the recursions-levels
	    occurred by environments, and open and closing-braces
 ******************************************************************************/

/********************************* includes **********************************/
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "stack.h"
/******************************************************************************/


/********************************* defines ***********************************/
#define STACKSIZE 300
/******************************************************************************/

/******************************** global variables *****************************/
static int stack[STACKSIZE];
static int top = 0;
/******************************************************************************/

/******************************************************************************/
int Push(int lev, int brack)
/******************************************************************************
  purpose: pushes the parameter lev and brack on the stack
parameter: lev...level
	   brack...brackets
  globals: progname
 return: top of stack
 ******************************************************************************/
{
  ++top;
  stack[top] = lev;
  ++top;
  stack[top] = brack;

  if (top >= STACKSIZE)
  {
    fprintf(stderr,"\n%s: ERROR: too deep nesting -> internal stack-overflow",progname);
    fprintf(stderr,"\nprogram aborted\n");
    exit(EXIT_FAILURE);
  }
  return top;
}

int Pop(int *lev, int *brack)
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

 
  if (top < 0)
  {
    fprintf(stderr,"\n%s: ERROR: error in LaTeX-File: %s at linenumber: %ld\n-> internal stack-underflow",progname,latexname,getLinenumber());
    fprintf(stderr,"\nprogram aborted\n");
    exit(EXIT_FAILURE);
  }
  return top;
}


/* The use of stack */

/*
each stack elem consist of 2 integers RecursLevel and BracketLevel. Recurs-
Level is the number of recursive calls of convert function, BracketLevel is
the number of open curly braces. The value on top of stack represents the
current value of the two global variables (RecursLevel and BracketLevel).
Before every command and on an opening curly brace the current settings are
written on the stack. On appearance of a closing curly brace the
corresponding RecursLevel is found by search on the stack. It is the lowest
RecursLevel with the same BracketLevel as now (after subtract of the 1
closing brace found). The initial value RecLev 1, BracketLev 0 remains
always on the stack. The begin document command Pushes 1,1
examples:
{ Text {\em Text} Text }
1      2 3	4      5
1 Push 12
2 Push 13
3 Push 23
4 Bracket 3->2 Pop 23  Pop 13 Pop 12 Pop 11 -found- Push back 11
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
commands don't include a recursiv call the stack is not cleaned up.
After every TranslateCommand-function the stack is cleaned
example
\ldots \LaTeX \today \TeX
 1	2      3      4
1 Push 11
2 Push 11
3 Push 11
4 Push 11
The cleanup loop pops till the values are not ident and pushes back the last
Therefore 11 is only 1 times on the stack.
*/
