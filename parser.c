/*
 * $Id: parser.c,v 1.7 2001/08/12 19:32:24 prahl Exp $
 * History:
 * $Log: parser.c,v $
 * Revision 1.7  2001/08/12 19:32:24  prahl
 * 1.9f
 * 	Reformatted all source files ---
 * 	    previous hodge-podge replaced by standard GNU style
 * 	Compiles cleanly using -Wall under gcc
 *
 * 	added better translation of \frac, \sqrt, and \int
 * 	forced all access to the LaTeX file to use getTexChar() or ungetTexChar()
 * 	    allows better handling of %
 * 	    simplified and improved error checking
 * 	    eliminates the need for WriteTemp
 * 	    potentially allows elimination of getLineNumber()
 *
 * 	added new verbosity level -v5 for more detail
 * 	fixed bug with in handling documentclass options
 * 	consolidated package and documentclass options
 * 	fixed several memory leaks
 * 	enabled the use of the babel package *needs testing*
 * 	fixed bug in font used in header and footers
 * 	minuscule better support for french
 * 	Added testing file for % comment support
 * 	Enhanced frac.tex to include \sqrt and \int tests also
 * 	Fixed bugs associated with containing font changes in
 * 	    equations, tabbing, and quote environments
 * 	Added essential.tex to the testing suite --- pretty comprehensive test.
 * 	Perhaps fix missing .bbl crashing bug
 * 	Fixed ?` and !`
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
/* 24th May  2001 - Now includes getParam, getbrackeparam, getbraceparam    */
/*                - which really should be rewritten -sap                   */
/****************************************************************************/

/*------------------- includes ----------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "cfg.h"
#include "stack.h"
#include "util.h"
#include "parser.h"


#define POSSTACKSIZE   256  /* Size of stack to save positions              */

/*------------------- globals ----------------------*/
static char currentChar;      /* Global current character                          */
static char lastChar;
static char penultimateChar;

long posStack[POSSTACKSIZE];
int  stackIndex = 0;


/*------------------- externals -----------------------*/
extern FILE *fTex;
/*------------------- prototypes ----------------------*/
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



void skipToEOL(void)
/****************************************************************************
purpose: ignores anything from inputfile until the end of line.  Uses the fact
         that linenumber changes in getTexChar when a '\n' is read.
 ****************************************************************************/
{
  long line = linenumber;
  while (line==linenumber)
     (void) getTexChar();
}

char getNonBlank(void)
/***************************************************************************
 Description: get the next non-blank character from the input stream     
****************************************************************************/
{
  char currentChar;
  
  do
  {
    currentChar = getTexChar();
  } while (currentChar == ' ' || currentChar =='\n');

  return currentChar;
}

char getNonSpace(void)
/***************************************************************************
 Description: get the next non-space character from the input stream     
****************************************************************************/
{
  char currentChar;
  
  do
  {
    currentChar = getTexChar();
  } while (currentChar == ' ');

  return currentChar;
}

void skipSpaces(void)
/***************************************************************************
 Description: get the next non-space character from the input stream     
****************************************************************************/
{
  char c;
  while ((c = getTexChar()) && c == ' ');
  rewind_one(c);
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
  while (getTexChar() != '}')
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
  while (getTexChar() != ']')
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
      while(!((getTexChar() == '{')||(currentChar == '[')));
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
      if (getNonBlank() == '[')
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

/******************************************************************************/
bool getBracketParam(char *string, int size)
/******************************************************************************
  purpose: function to get an optional parameter
parameter: string: returnvalue of optional parameter
	   size: max. size of returnvalue
	   returns true if a brackets are found
	   allows us to figure out if \item[] is found for example 
 ******************************************************************************/
{
char c;
int i=0;
int bracketlevel=0;

  diagnostics(5,"Entering getBracketParam()");
  c = getNonBlank();

  if ( c != '[' )		/* does not start with a bracket, abort */
  {
    rewind_one(c);
    string[0] = '\0';
    return FALSE;
  }

  for(i=0; ; i++)
  {
    c = getTexChar();
    
    if ((c == ']') && (bracketlevel == 0)) break;

    if (c == '[') bracketlevel++;

    if (c == ']') bracketlevel--;
    
    if (i < size-1)				/* throw away excess */
      string[i] = c;
  }

  if (bracketlevel > 0)
    {
        fprintf(stderr,"**Error - Bracketed string is longer than %d characters\n", size-1);
        i = size -1;
    }

  string[i] = '\0';
  diagnostics(5,"Leaving getBracketParam() [%s]",string);
  return TRUE;
}

/******************************************************************************/
void getBraceParam(char *string, int size)
/******************************************************************************
  purpose: function to get a parameter between {}
parameter: string: returnvalue of optional parameter
	   size: max. size of returnvalue

If it a {} expression does not follow, then return an empty expression
with fTex pointing to the first non-space character
 ******************************************************************************/
{
char c;
int i=0;
int bracelevel=0;

  *string = '\0';

  c = getNonBlank();

  if ( c != '{' )		/* does not start with a brace, abort */
  {
    rewind_one(c);
    return;
  }

  while (c = getTexChar())
  {
    
    if ((c == '}') && (bracelevel == 0)) break;
  		 
    if (c == '{') bracelevel++;

    if (c == '}') bracelevel--;
    
    if (i < size-1)				/* throw away excess */
      string[i++] = c;
  }
  string[i] = '\0';

/* fprintf(stderr, "\nthe string in braces is %s\n", string); */
}

/**************************************************************************
     purpose: returns a simple command.  
     
     For example \alpha\beta will return "alpha"
 **************************************************************************/
static char * getSimpleCommand(void)
{
  char buffer[128];
  int size;

  buffer[0] = getTexChar();
  
  if ( buffer[0] != '\\' )
    return NULL;
    
    for(size=1; size < 127; size++)
    {
        buffer[size] = getTexChar();
        
        if (!isalpha(buffer[size]))
        {
        rewind_one(buffer[size]);
        break;
        }
    }
  
  buffer[size] = '\0';
  if (size == 127) 
     error(" Misplaced brace in command.  Scanned 127 chars looking for end\n");
      
  diagnostics(5,"getSimpleCommand result <%s>", buffer);
  return strdup(buffer);
}

char *getParam(void)
/**************************************************************************
     purpose: returns the parameter after the \begin-command
	      for instance: \begin{environment}
		    return: -> string = "environment"
   parameter: string: pointer to string, which returns the parameter
     globals: BracketLevel: counts open braces
              fTex	  : Tex-file-pointer
     returns: success: string
	      miss : string = ""
 **************************************************************************/
{
  char cThis, buffer[512];
  int PopLevel,PopBrack,bracket,size;

  if ( (cThis=getTexChar()) != '{' )
  {
    buffer[0]=cThis;
    buffer[1]='\0';
    return strdup(buffer);
  }

  ++BracketLevel;
  (void)Push(RecursLevel,BracketLevel);
  
  size=0;
  bracket=1;
  while (bracket > 0 && size < 511)   
  {
    buffer[size] = getTexChar();
    
    if (buffer[size] == '}')
    {
      bracket--;
      if (bracket == 0)
      {
	--BracketLevel;
	(void)Pop(&PopLevel,&PopBrack);
       break;
      }
    }

    if (buffer[size] == '{')
      bracket++;

    size++;
  }
  
  buffer[size] = '\0';
  if (size == 511) 
     error(" Misplaced brace in command.  Scanned 511 chars looking for end\n");
      
  diagnostics(5,"getParam result <%s>", buffer);

  return strdup(buffer);
}

char *getMathParam(void)
/**************************************************************************
     purpose: returns the parameter after ^ or _
     example  ^\alpha and ^{\alpha} both return \alpha 
              ^2      and ^{2} both return 2
**************************************************************************/
{
  char buffer[2];
  
  buffer[0]=getTexChar();
  
  if (buffer[0] == '{')
  {
    rewind_one(buffer[0]);
    return getParam();
  }
  else if (buffer[0] == '\\')
  {
    if (buffer[0]!= ' ')         /* TeX eats the space after control words */
      rewind_one(buffer[0]);
    return getSimpleCommand();
  }
  else 
  {
    buffer[1] = '\0';
    return strdup(buffer);
  }
    
}

/***************************************************************************
 function: getTexChar()                                                   
 Description: get the next character from the input stream  
              This should be the only place that characters are read from!              
****************************************************************************/
char getTexChar()
{
    int thechar;
    
    thechar = getc(fTex);
    if (thechar == EOF  && !feof(fTex))
    {
        error("Unknown error reading latex file\n");
    }
    
    if (thechar == '\r')
    {
        thechar = getc(fTex);
        if (thechar != '\n' && !feof(fTex))
            ungetc(thechar, fTex);
        thechar = '\n';
    }
    else if (thechar == '\t')
        thechar = ' ';
        
    currentChar = (char) thechar;    

    if (currentChar=='\n') 
       linenumber++;
    
    if (currentChar=='%' && lastChar != '\\')
    {
       skipToEOL();
       currentChar=getTexChar();
    }
    
    penultimateChar = lastChar;
    lastChar = currentChar;
    return currentChar;
}

/*********************************************************
 * rewind the filepointer in the LaTeX-file by one
 * globals: fTex
*********************************************************/
void rewind_one(char c)
{
  ungetc(c,fTex);
  
  if (c=='\n') linenumber--;
    
  lastChar = penultimateChar;
  penultimateChar = '\0';         /* no longer know what that it was */
}

