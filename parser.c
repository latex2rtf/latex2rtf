/*
 * $Id: parser.c,v 1.2 2001/08/12 17:50:50 prahl Exp $
 * History:
 * $Log: parser.c,v $
 * Revision 1.2  2001/08/12 17:50:50  prahl
 * latex2rtf version 1.9b by Scott Prahl
 * 1.9b
 * 	Improved enumerate environment so that it may be nested and
 * 	    fixed labels in nested enumerate environments
 * 	Improved handling of description and itemize environments
 * 	Improved eqnarray environment
 * 	Improved array environment
 * 	Improved \verb handling
 * 	Improved handling of \mbox and \hbox in math mode
 * 	Improved handling of \begin{array} environment
 * 	Improved handling of some math characters on the mac
 * 	Fixed handling of \( \) and \begin{math} \end{math} environments
 * 	Fixed bugs in equation numbering
 * 	Made extensive changes to character translation so that the RTF
 * 	     documents work under Word 5.1 and Word 98 on the Mac
 *
 *
 * 1.9a
 * 	Fixed bug with 'p{width}' in tabular environment
 * 		not fully implemented, but no longer creates bad RTF code
 *
 * 1.9
 * 	Fixed numbering of equations
 * 	Improved/added support for all types of equations
 * 	Now includes PICT files in RTF
 * 	Fixed \include to work (at least a single level of includes)
 *
 * 1.8
 * 	Fixed problems with \\[1mm]
 * 	Fixed handling of tabular environments
 * 	Fixed $x^\alpha$ and $x_\alpha$
 *
 * Revision 1.2  1998/10/27 04:51:38  glehner
 * Changed function prototype of parseBrace & parseBracket to
 * void parse...  (from char parse...)
 *
 * Revision 1.1  1998/10/27 04:46:43  glehner
 * Initial revision
 *
 *
 * LEG 070798 adapted Frank Barnes contribution to r2l coding conventions
 *
 */
/****************************************************************************/
/* file: parser.h                                                           */
/*                                                                          */
/* Description:                                                             */
/*    Contains declarations for a generic recursive parser the              */
/*    LaTex2RTF code.                                                       */
/*                                                                          */
/* Revision history                                                         */
/* ================                                                         */
/* 26th June 1998 - Created initial version - fb                            */
/****************************************************************************/
/*------------------- includes ----------------------*/
#include <stdio.h>
#include "main.h"
#include "parser.h"


#define POSSTACKSIZE   256  /* Size of stack to save positions              */

/*------------------- globals ----------------------*/
char currentChar;      /* Global current character                          */

long posStack[POSSTACKSIZE];
int  stackIndex = 0;
/*------------------- externals -----------------------*/
extern FILE *fTex;
/*------------------- prototypes ----------------------*/
static char getChar();        /* Get next none - comment character                 */
static char getNoBlank();     /* get next none - whitespace character              */
static void parseBrace();     /* parse an open/close brace sequence                */
static void parseBracket();   /* parse an open/close bracket sequence              */

static void pushPos();        /* Push current file position                        */
static void popPos();         /* Pop (lose) previously pushed position             */
static void resetPos();       /* Pop and restore previously pushed position        */

/****************************************************************************/
/* End of file parser.h                                                     */
/****************************************************************************/
/****************************************************************************/
/* file: parser.c                                                           */
/*                                                                          */
/* Description:                                                             */
/*    Contains functions for a generic recursive parser the                 */
/*    LaTex2RTF code.                                                       */
/*                                                                          */
/* Revision history                                                         */
/* ================                                                         */
/* 26th June 1998 - Created initial version - fb                            */
/****************************************************************************/


/****************************************************************************/
/* function: getChar                                                        */
/*                                                                          */
/* Description: get the next character from the input stream                */
/****************************************************************************/
char getChar()
{
    if ( (fTexRead(&currentChar,1,1,fTex) < 1))
    {	
    	currentChar ='\0';	 /* SAP - hack to allow include files to work */
    	numerror(ERR_EOF_INPUT); /* numerror will not exit if processing an include file */  
    }

    return currentChar;
}
/****************************************************************************/
/* function: getNoBlank                                                     */
/*                                                                          */
/* Description: get the next none blank character from the input stream     */
/****************************************************************************/
char getNoBlank()
{
  while((getChar() == ' ')||(currentChar == '\n'));
  return currentChar;
}
/****************************************************************************/
/* function: pushPos                                                        */
/*                                                                          */
/* Description: Push the current file position on the stack                 */
/****************************************************************************/
void pushPos()
{
  if (++stackIndex == POSSTACKSIZE)
    {
      fprintf(stderr,"**Error - Position stack overflow\n");
      return;
    }
  posStack[stackIndex] = ftell(fTex);
  return;
}
/****************************************************************************/
/* function: popPos                                                         */
/*                                                                          */
/* Description: pop the file position stack                                 */
/****************************************************************************/
void popPos()
{
  if (--stackIndex < 0)
    {
      fprintf(stderr,"**Error - Position stack underflow\n");
      stackIndex = 0;
    }
  return;
}
/****************************************************************************/
/* function: resetPos                                                       */
/*                                                                          */
/* Description: pop the file position stack                                 */
/****************************************************************************/
void resetPos()
{
  if (--stackIndex < 0)
    {
      fprintf(stderr,"**Error - Position stack underflow\n");
      stackIndex = 0;
      return;
    }
  fseek(fTex, posStack[stackIndex + 1], SEEK_SET);
  return;
}
/****************************************************************************/
/* function: parseBrace                                                     */
/*                                                                          */
/* Description: Skip text to balancing close brace                          */
/****************************************************************************/
void
parseBrace()
{
  while (getChar() != '}')
    {
      switch(currentChar)
	{
	case '{':

	  parseBrace();
	  break;

	case '[':

	  parseBracket();
	  break;

	default:/* Skip other characters */;
	}
    }
}
/****************************************************************************/
/* function: parseBracket                                                   */
/*                                                                          */
/* Description: Skip text to balancing close brakcet                        */
/****************************************************************************/
void
parseBracket()
{
  while (getChar() != ']')
    {
      switch(currentChar)
	{
	case '{':

	  parseBrace();
	  break;

	case '[':

	  parseBracket();
	  break;

	default:/* Skip other characters */;
	}
    }
}
/****************************************************************************/
/* function: CmdIgnoreParameter                                             */
/*                                                                          */
/* Description: redo of the function                                        */
/*              code is a decimal # of the form "op" where `o' is the       */
/*              # of optional parameters (0-9) and `p' is the # of required */
/*              parameters.                                                 */
/*              The specified number of parameters is ignored. Order of     */
/*              opt and req parameters doesn't matter.                      */
/****************************************************************************/
void CmdIgnoreParameter(int code)
{
  int optParmCount = code / 10;
  int regParmCount = code % 10;

  /*******************************************/
  /* Parse the required number of parameters */
  /*******************************************/
  while(regParmCount)
    {
      while(!((getChar() == '{')||(currentChar == '[')));
      switch(currentChar)
	{
	case '{':

	  regParmCount--;
	  parseBrace();
	  break;

	case '[':

	  optParmCount--;
	  parseBracket();
	  break;

	default:;
	}
    }
  /**********************************************************/
  /* Check for trailing optional parameter                  */
  /* (I dont think optionals come last - but just in case!) */
  /* Of course, optionals very often come last. e.g.: the   */
  /* \item[label] of item in a description] list.           */
  /**********************************************************/
  if (optParmCount > 0)
    {
      pushPos();
      if (getNoBlank() == '[')
	{
	  parseBracket();
	  popPos();
	}
      else
	{
	  resetPos();
	  return;
	}
    }
  return;
}



