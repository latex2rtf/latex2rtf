/*
 * $Id: parser.c,v 1.3 2001/08/12 18:25:13 prahl Exp $
 * History:
 * $Log: parser.c,v $
 * Revision 1.3  2001/08/12 18:25:13  prahl
 * latex2rtf version 1.9c
 *
 * 	Added support for \frac
 * 	Complete support for all characters in the symbol font now
 * 	Better support for unusual ansi characters (e.g., \dag and \ddag)
 * 	Gave direct.cfg a spring cleaning
 * 	Added support for \~n and \~N
 * 	New file oddchars.tex for testing many of these translations.
 * 	New file frac.tex to test \frac and \over
 * 	Removed a lot of annoying warning messages that weren't very helpful
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



