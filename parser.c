/*
 * $Id: parser.c,v 1.6 2001/08/12 19:00:04 prahl Exp $
 * History:
 * $Log: parser.c,v $
 * Revision 1.6  2001/08/12 19:00:04  prahl
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
/* 24th May  2001 - Now includes getparam, getbrackeparam, getbraceparam    */
/*                - which really should be rewritten -sap                   */
/****************************************************************************/

/*------------------- includes ----------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "cfg.h"
#include "stack.h"
#include "parser.h"


#define POSSTACKSIZE   256  /* Size of stack to save positions              */

/*------------------- globals ----------------------*/
char currentChar;      /* Global current character                          */

long posStack[POSSTACKSIZE];
int  stackIndex = 0;
/*------------------- externals -----------------------*/
extern FILE *fTex;
/*------------------- prototypes ----------------------*/
static char getTexChar();     /* Get next none - comment character                 */
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
/* function: getTexChar()                                                        */
/*                                                                          */
/* Description: get the next character from the input stream                */
/****************************************************************************/
char getTexChar()
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
  while((getTexChar() == ' ')||(currentChar == '\n'));
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


/*********************************************************
 * LEG210698
 * rewind the filepointer in the LaTeX-file by one
 * globals: fTex, LaTeX-filepointer
 *********************************************************/
void rewind_one(void)
{
  if(fseek(fTex,-1L,SEEK_CUR) != 0)
    diagnostics(ERROR, "Seek failed in LaTeX-file");
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

/******************************************************************************/
bool GetBracketParam(char *string, int size)
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

  *string = '\0';

  while (c == getTexChar())  /* skip initial spaces */
  {
  	if ((c != ' ') && (c != '\n')) break;
  }

  if ( c != '[' )		/* does not start with a brace, abort */
  {
    rewind_one();
    return FALSE;
  }

  while (c == getTexChar())
  {
    
    if ((c == ']') && (bracketlevel == 0)) break;
  		 
    if (c == '%')
    {
       IgnoreTo('\n');
       continue;
    }
    
    if (c == '[') bracketlevel++;

  	if (c == ']') bracketlevel--;
    
    if (i < size-1)				/* throw away excess */
      string[i++] = c;
  }
  string[i] = '\0';
  return TRUE;
/* fprintf(stderr, "\nthe bracketed string is %s\n", string); */
}

/******************************************************************************/
void GetBraceParam(char *string, int size)
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
bool read_one = FALSE;

  *string = '\0';

  while (c = getTexChar())  /* skip initial spaces */
  {
  	read_one = TRUE;
  	if ((c != ' ') && (c != '\n')) break;
  }

  if ( c != '{' )		/* does not start with a brace, abort */
  {
    if (read_one) rewind_one();
    return;
  }

  while (c = getTexChar())
  {
    
    if ((c == '}') && (bracelevel == 0)) break;
  		 
    if (c == '%')
{
       IgnoreTo('\n');
       continue;
    }
    
    if (c == '{') bracelevel++;

  	if (c == '}') bracelevel--;
    
    if (i < size-1)				/* throw away excess */
      string[i++] = c;
  }
  string[i] = '\0';

/* fprintf(stderr, "\nthe braced string is %s\n", string); */
}

/******************       helping function for CmdBegin               ***/
/**************************************************************************/
/*@only@*/ char *GetParam(void)
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
  char cThis;
  int i,PopLevel,PopBrack;
  int bracket=0;
  int size = 0;
  long oldpos;
  char *string;

  oldpos = ftell (fTex);
  /* read Param to get needed length, allocate it and position
     the file-pointer back to the beginning of the param in the TeX-file */
  if ( (fread(&cThis,1,1,fTex) < 1))
    numerror(ERR_EOF_INPUT);
  if ( cThis != '{' )
  {
    string = malloc (2*sizeof(char));
    if (string == NULL)
       error(" malloc error -> out of memory!\n");
    string[0] = cThis;
    string[1] = '\0';
    return string;
  }
  else
  {
    bracket++;
    ++BracketLevel;
    (void)Push(RecursLevel,BracketLevel);
  }
  /* Varibale Bracket is 1 here */
  for (i = 0; ;i++)   /* get param from input stream */
  {
    if (fread(&cThis,1,1,fTex) < 1)
       numerror(ERR_EOF_INPUT);
    size++;
    if (cThis == '}')
    {
      bracket--;
      if (bracket == 0)
      {
	--BracketLevel;
	(void)Pop(&PopLevel,&PopBrack);
       break;
      }
    }
    if (cThis == '{')
    {
      bracket++;
    }

    if (cThis == '%')
	{
	IgnoreTo('\n');
	i--;
	continue;
	}

  } /* for */
  string = malloc ( (size+1) * sizeof(char) );
  if (string == NULL)
     error(" malloc error -> out of memory!\n");

  if(fseek(fTex, oldpos, SEEK_SET) != 0)
    diagnostics(ERROR, "Could not seek in LaTeX file");
  /*LEG210698*** this is a very poor error message */




  if ( (fTexRead(&cThis,1,1,fTex) < 1))
    numerror(ERR_EOF_INPUT);
  else
  {
    bracket++;
    ++BracketLevel;
    (void)Push(RecursLevel,BracketLevel);
  }
  /* Varibale Bracket is 1 here */
  for (i = 0; i<=size; i++)   /* get param from input stream */
  {
    if (fTexRead(&cThis,1,1,fTex) < 1)
       numerror(ERR_EOF_INPUT);
    if (cThis == '}')
    {
      bracket--;
      if (bracket == 0)
      {
        --BracketLevel;
        (void)Pop(&PopLevel,&PopBrack);
        break;
      }
    }
    if (cThis == '{')
    {
      bracket++;
    }

# ifdef no_longer_needed
    /* \and-command handling routine */
    if (cThis == '\\')
    {
       /* command is overread ! */
        for(;;)
        {
           if (fread(&cThis,1,1,fTex) < 1)
              numerror(ERR_EOF_INPUT);
           if (!isalpha(cThis))
              break;
        }
       fseek(fTex,-1L,SEEK_CUR); /* reread last character */
       continue;
    }
# endif /*  no_longer_needed  */

    if (cThis == '%')
    {
        IgnoreTo('\n');
        i--;
        continue;
    }

    string[i] = cThis;
  }

  string[i] = '\0';
  return string;
}


