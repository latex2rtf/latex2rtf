/*
 * $Id: funct2.c,v 1.10 2001/08/12 19:32:24 prahl Exp $
 * History:
 * $Log: funct2.c,v $
 * Revision 1.10  2001/08/12 19:32:24  prahl
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
 * Revision 1.10  1998/11/05 13:21:49  glehner
 * *** empty log message ***
 *
 * Revision 1.9  1998/10/28 06:04:51  glehner
 * (CmdIgnoreParameter) now put in Frank Barnes parser.c
 * Factored out WriteRefList into Open BblFile and MakeBiblio
 * CmdConvertBiblio added: makes bibliography from
 * \thebibliograpy environment.
 * Removed #include <malloc.h>
 * Internationalized Title in (CmdAbstract).
 * Changed all output eol-codes to \n
 *
 * Revision 1.8  1998/07/03 07:00:13  glehner
 * added hyperlatex-support, CmdColsep
 *
 * Revision 1.7  1997/02/15 20:59:16  ralf
 * Corrected core-dump bug in tabular environment (gcc only)
 * Some lclint changes
 *
 * Revision 1.6  1995/05/24  17:11:43  ralf
 * Corrected bug with variable input being NULL
 *
 * Revision 1.5  1995/05/24  14:48:00  ralf
 * Added checks for malloc failures
 * Corrected parsing of bbl
 * Corrected searching for citations in .aux
 *
 * Revision 1.4  1995/05/24  11:51:47  ralf
 * Removed 2 off-by-one malloc errors
 *
 * Revision 1.3  1995/03/23  15:58:08  ralf
 * Reworked version by Friedrich Polzer and Gerhard Trisko
 *
 *
 * Revision 1.2  1994/06/21  08:14:11  ralf
 * Corrected Bug in keyword search
 *
 * Revision 1.1  1994/06/17  11:26:29  ralf
 * Initial revision
 * 
 *
 ***************************************************************************
   name : funct2.c
 author : DORNER Fernando, GRANZER Andreas
          POLZER Friedrich,TRISKO Gerhard
 * CmdTabular & CommandTable changed
 * added CmdCite and WriteRefTable for creating correct citations
 purpose : includes besides funct1.c all functions which are called from the programm commands.c;
 ****************************************************************************/

/********************************* includes *********************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include "main.h"
#include "l2r_fonts.h"
#include "commands.h"
#include "funct1.h"
#include "funct2.h"
#include "stack.h"
#include "cfg.h"
#include "util.h"
#include "parser.h"
/******************************************************************************/

/***************************** global variables ********************************/
static int tabstoparray[100];
static int number_of_tabstops=0;
/*****************************************************************************/

/***************************** function prototypes ***************************/

static void Convert_Tabbing_with_kill(void);
FILE* OpenBblFile(void);
void MakeBiblio(FILE* fBbl);


/*----------------------------Tabbing Environment ------------------------*/
/******************************************************************************/
void Tabbing(int code)
/******************************************************************************
  purpose: pushes all tabbing-commands on a stack
parameter: code : on/off at begin/end-environment
  globals: tabbing_on: true if tabbing-mode is on (only in this environment)
	   tabbing_return, tabbing_itself: true if environmend ends
 ******************************************************************************/
{
  if (code & ON)  /* on switch */
  {
    code &= ~(ON);  /* mask MSB */
    if (code == TABBING)
    {
      tabbing_on = TRUE;
      /*tabbing_on_itself = FALSE; */

      PushEnvironment(code);

      fprintf(fRtf,"\\par\\line ");
      if(fgetpos(fRtf,&pos_begin_kill) != 0)
	diagnostics(ERROR,"Failed fgetpos; funct2.c (Tabbing): errno %d", errno);
      /* Test ConvertTabbing(); */
    }
  }
  else /* off switch */
  {
    /* tabbing_return = TRUE;
    tabbing_on_itself = TRUE; */
    tabbing_on = FALSE;
    PopEnvironment();

    fprintf(fRtf,"\\par\\pard\\line\\q%c ",alignment);
  }
}

/******************************************************************************/
void CmdTabset()
/******************************************************************************
 purpose: sets an tabstop
globals:  tabcounter: specifies the tabstop-position
 ******************************************************************************/
{int tabstop;
    tabstop = (tabcounter/6)*567;
    tabstoparray[number_of_tabstops] = tabstop;
    number_of_tabstops++;
    fprintf(fRtf,"\\tx%d ",tabstop); /* Tab at tabstop/567 centimeters */
}

/******************************************************************************/
void CmdTabjump()
/******************************************************************************
 purpose: jumps to an tabstop
 ******************************************************************************/
{
    fprintf(fRtf,"\\tab ");
}

/******************************************************************************/
void CmdTabkill(/*@unused@*/ int code)
/******************************************************************************
 purpose: a line in the tabbing-Environment which ends with an kill-command won't be
	 written to the rtf-FILE
 ******************************************************************************/
{
  int i;

  if(fsetpos(fRtf,&pos_begin_kill) != 0)
    diagnostics(ERROR,"Failed fsetpos; funct2.c (CmdTabkill): errno %d", errno);

  for(i=0;i<number_of_tabstops;i++)
    {
    fprintf(fRtf,"\\tx%d ",tabstoparray[i]); /* Tab at tabstop/567 centimeters */
    }

  number_of_tabstops = 0;
}
/*-------------------- End of Tabbing Environment -------------------------*/



/******************************************************************************/
void CmdIgnoreFigure(int code)
/******************************************************************************
  purpose: function, which overreads the Figure,Picture,Bibliography and Minipage
	   Environment
parameter: code: which environment to ignore
 ******************************************************************************/
{
 char endfigure[30];
 char thechar;
 bool found = FALSE;
 int i, endstring=0;

    endfigure[0] = '\0';
    switch (code & ~(ON))  /* mask MSB */
    {
	 case FIGURE : {
			   strcpy(endfigure,"end{figure}");
			   endstring = strlen(endfigure) -1;
			   break;
		       }
	 case FIGURE_1 : {
			   strcpy(endfigure,"end{figure*}");
			   endstring = strlen(endfigure) -1;
			   break;
		       }
	 case PICTURE : {
			   strcpy(endfigure,"end{picture}");
			   endstring = strlen(endfigure) -1;
			   break;
			}
	 case MINIPAGE : {
			   strcpy(endfigure,"end{minipage}");
			   endstring = strlen(endfigure) -1;
			   break;
			}
	 case THEBIBLIOGRAPHY : {
			   strcpy(endfigure,"end{thebibliography}");
			   endstring = strlen(endfigure) -1;
			   break;
			}
	 default: assert(0);

    } /* end switch */

    while (thechar = getTexChar())
    {
       if (thechar == '\\')
       {
	  bool found_space = FALSE;
	  for (i=0; i<=endstring; i++)
	    {
	    bool is_nl = FALSE;
	    thechar = getTexChar();
            while(thechar == ' ' || thechar == '\n' && !is_nl)
	    {
		if(thechar == '\n')
                {
		    is_nl = TRUE;
                    linenumber++;
                }
                thechar = getTexChar();
                found_space = TRUE;
	    }
	    if(found_space && thechar != '{') /* for vi } */
		break;
	    found_space = FALSE;
	    if (thechar != endfigure[i])
		break;
	    if (i == endstring)                 /* end-figure-found */
		found = TRUE;
	    } /* for */
       } /* if */
       if (thechar == '%')
	  {
               fprintf(stderr," percent encountered in IgnoreFigure alert\n");
         skipToEOL();
         }

       if (found)
	   return;
    } /* while */
    numerror(ERR_EOF_INPUT);
}

/******************************************************************************
CmdLink:

  purpose: hyperlatex support. function, which translates the first parameter
           to the rtf-file and ignores the second, the proposed optional
	   parameter is also (still) ignored.
  parameter: not (yet?) used.

  The second parameter should be remembered for the \Cite (\Ref \Pageref)
  command.
  globals: hyperref, set to second Parameter

The first parameter of a \link{anchor}[ltx]{label} is converted to the
rtf-output. Label is stored to hyperref for later use, the optional
parameter is ignored.
[ltx] should be processed as Otfried recommends it, to use for
exclusive latex output.e.g:

	\link{readhere}[~\Ref]{explaining:chapter}.

Since {explaining:chapter} is yet read by latex and hyperlatex when
[...] is evaluated it produces the correct reference. We are only
strolling from left to right through the text and can't remember what
we will see in the future.

 ******************************************************************************/
void
CmdLink(int code)
{
  char *param2;
  char optparam[255] = "";

  diagnostics(4, "> hyperlatex \\link command");
     Convert(); /* convert routine is called again for evaluating the
		   contents of the first parameter */
     diagnostics(4, "  Converted first parameter");

     getBracketParam(optparam, 255);
     /**/ /*LEG190498 now should come processing of the optional parameter */
     diagnostics(4, "  Converted optional parameter");
     
     param2 = getParam();
     diagnostics(4, "  Converted second parameter");

     if(hyperref != NULL)
       free(hyperref);

      hyperref = (char*) malloc((strlen(param2)+1));
      if (hyperref == NULL)
	error(" malloc error -> out of memory!\n");

      strcpy(hyperref,param2);
      free(param2);
      /*LEG210698*** better? hyperref = param2 */      
}

/******************************************************************************/
void Ignore_Environment(char *searchstring)
/******************************************************************************
  purpose: function, which ignores an unconvertable environment in LaTex
           and writes text unchanged into the Rtf-file.
parameter: searchstring : includes the string to search for
	   example: \begin{unknown} ... \end{unknown}
		    searchstring="end{unknown}"
 ******************************************************************************/
{
 char thechar;
 bool found = FALSE;
 int i, j, endstring;
    endstring = strlen(searchstring) - 1;
    while (thechar = getTexChar())
    {
       if (thechar == '\\')
       {
	  for (i=0; i<=endstring; i++)
	  {
            thechar = getTexChar();

	    if (thechar != searchstring[i])
		break;
	    if (i == endstring)                 /* end-environment-found */
		found = TRUE;
	  } /* for */

          if (!found)
          {
             fprintf (fRtf, "\\\\"); 
             for (j = 0; j < i; j++)
                 switch (searchstring[j])
                 {
                    case '\n':
                         fprintf (fRtf, "\\par \n"); 
                         break;
                    case '\\':
                    case '{':
                    case '}':
                         fprintf (fRtf, "\\"); 
		    /*@fallthrough@*/
                    default:
                         fprintf (fRtf, "%c", searchstring[j]); 
                         break;
                 }
          }
         
	} /* if */

  
       if ((thechar != '%') && !found)
          switch (thechar)
          {
             case '\n':
                  fprintf (fRtf, "\\par \n"); 
                  break;
             case '\\':
             case '{':
             case '}':
                  fprintf (fRtf, "\\"); 
	     /*@fallthrough@*/
             default:
                  fprintf (fRtf, "%c", thechar); 
                  break;
          }

       if (thechar == '%')
	  {
               fprintf(stderr," percent encountered in IgnoreEnvironment alert\n");
         skipToEOL();
         }

       if (found)
	   return;

    } /* while */
    numerror(ERR_EOF_INPUT);
}


/******************************************************************************/
void CmdIgnoreEnvironment(int code)
/******************************************************************************
  purpose: overreads an ignoreable environment
parameter: code: type of environment & ON/OFF
 ******************************************************************************/
{
     switch (code & ~(ON))   /* ON/OFF-parameter exclude */
     {
	case BIBLIOGRAPHY :
			  Ignore_Environment("end{thebibliography}");
			  break;                
	case LETTER : Ignore_Environment("end{letter}");
			  break;
	case TABLE : Ignore_Environment("end{table}");
			  break;
	case TABLE_1 : Ignore_Environment("end{table*}");
			  break;
	default : numerror(ERR_WRONG_COMMAND);
     } /* switch */
}

/******************************************************************************/
void CmdColumn (int code)
/******************************************************************************
  purpose: chooses beetween one/two-columns
parameter: number of columns
 globals: twocolumn: true if twocolumn-mode is set
 ******************************************************************************/
{
   switch (code)
   {
       case One_Column : fprintf(fRtf,"\\page \\colsx709\\endnhere "); /* new page & one column */
			 twocolumn = FALSE;
			 break;
       case Two_Column : fprintf(fRtf,"\\page \\cols2\\colsx709\\endnhere "); /* new page & two columns */
			 twocolumn = TRUE;
			 break;
   } /* switch */
}

/******************************************************************************/
void CmdNewPage(int code)
/******************************************************************************
  purpose: starts a new page
parameter: code: newpage or newcolumn-option
 globals: twocolumn: true if twocolumn-mode is set
 ******************************************************************************/
{
   switch (code)
   {
      case NewPage :  fprintf(fRtf,"\\page "); /* causes new page */
		      break;
      case NewColumn : if (twocolumn)
			  fprintf(fRtf,"\\column "); /* new column */
		       else
			  fprintf(fRtf,"\\page ");  /* causes new page */
		       break;
   } /* switch */
}

/******************************************************************************/
void Cmd_OptParam_Without_braces(/*@unused@*/ int code)

/******************************************************************************
 purpose: gets an optional parameter which isn't surrounded by braces but by spaces
 ******************************************************************************/
{char cNext=' ';
 char cLast=' ';

    do
    {
       cLast = cNext;
       cNext = getTexChar();
    } while ((cNext != ' ') &&
	     (cNext != '\\') &&
	     (cNext != '{') &&
	     (cNext != '\n') &&
	     (cNext != ',') &&
	     ((cNext != '.')  || (isdigit((unsigned char) cLast))) &&
	     /* . doesn't mean the end of an command inside an number of the type real */
	     (cNext != '}') &&
	     (cNext != '\"') &&
	     (cNext != '[') &&
	     (cNext != '$'));

    rewind_one(cNext);
}


/******************************************************************************/
void GetInputParam(char *string, int size)
/******************************************************************************
  purpose: gets the parameter followed the \include or \input -command
parameter: string: returnvalue of the input/include-parameter
	   size: max. size of the returnvalue
		 must be determined by the calling-function
 ******************************************************************************/
{
  char cThis;
  int i,PopLevel,PopBrack;
  bool readuntilnewline = FALSE;

  cThis = getTexChar();
  
  if ( cThis == '{' )
  {
    ++BracketLevel;
    (void)Push(RecursLevel,BracketLevel);
  }
  else
  {
    readuntilnewline = TRUE;
    rewind_one(cThis); /* reread last character */
  }
  
  for (i = 0; ;i++)   /* get param from input stream */
  {
    cThis = getTexChar();
    if (cThis == '}')
    {
      --BracketLevel;
      (void)Pop(&PopLevel,&PopBrack);
      break;
    }

    if ((readuntilnewline) && ((cThis == ' ') || (cThis == '\n')))
	{
/*	if (cThis == '\n')
	    linenumber++; */
	break;
	}

    if (size-- > 0)
      string[i] = cThis;
  }
  string[i] = '\0';
}

/******************************************************************************/
void ConvertTabbing(void)
/******************************************************************************
 purpose: routine which converts the tabbing-commands from LaTex to Rtf
 ******************************************************************************/
{ int read_end = 1024;
  char cCommand[MAXCOMMANDLEN];
  int i;
  long j=0;
  char cThis;
  bool getcommand;
  bool command_end_line_found;
  bool command_kill_found;

while (tabbing_on)
{
command_end_line_found = FALSE;
command_kill_found = FALSE;

while (command_end_line_found)
  {
  for (;;) /* do forever */
  {
    getcommand=FALSE;
    cThis = getTexChar();
    j++; 

    if (cThis == '\\')
       {
       getcommand=TRUE;
       strcpy(cCommand,"");

       for (i = 0; ;i++)   /* get command from input stream */
	   {
           cThis = getTexChar();
	   j++;

	   if (i == 0) /* test for special characters */
	      {
	      switch(cThis)
		  {
		  case '\\':    command_end_line_found=TRUE;
				break;
		    } /* switch */
	      }  /* if */

	   if (!isalpha((unsigned char) cThis))
	       {
	       while (cThis == ' ')   /* all spaces after commands are ignored */
	       {
	         cThis = getTexChar();
	         j++;
	       }

	       rewind_one(cThis); /* position of next character after command
					    except space */
	       j--;
	       break; /* for */
	       }
	   cCommand[i] = cThis;
	   }  /* for */

	   cCommand[i] = '\0';  /* mark end of string with zero */
    }  /* if \\ */

    if ((getcommand) &&
	((command_end_line_found) ||
	 (strcmp(cCommand,"kill") == 0) ||
	 (strcmp(cCommand,"end") == 0)))
	{
	command_end_line_found = TRUE;
	if (strcmp(cCommand,"kill") == 0)
	    command_kill_found = TRUE;
	break;
	}

    if (j >= read_end)
	{
	command_end_line_found = TRUE;
	break;
	}
  } /* for */
  } /* while command_end_line_found */

  rewind_one(cThis); /* re_read line  PROBABLY WRONG!*/
  if (command_kill_found)
    Convert_Tabbing_with_kill();
  else
    Convert();
} /* while Tabbing_ON */

tabbing_on = FALSE;
} /* ConvertTabbing */


/******************************************************************************/
void Convert_Tabbing_with_kill(void)
/******************************************************************************
 purpose: routine which converts the tabbing-kill-option from LaTex to Rtf
 globals: tabcounter:
 ******************************************************************************/
{ int i=0;
  bool command_kill_found=FALSE;
  char cThis;
  char cCommand[MAXCOMMANDLEN];

tabcounter=0;

while (command_kill_found)
   {
    cThis = getTexChar();

    strcpy(cCommand,"");

    if (cThis == '\\')
       {

       for (i = 0; ;i++)   /* get command from input stream */
	   {
	   cThis=getTexChar();
           
	   if (i == 0) /* test for special characters */
	      {
	      switch(cThis)
		  {
		  case '=': CmdTabset();
			    break;
		  default : if(!isalpha((unsigned char) cThis))
				 numerror(ERR_WRONG_COMMAND_IN_TABBING);
		    } /* switch */
	      }  /* if */

	   if (!isalpha((unsigned char) cThis))
	       {
	       while (cThis == ' ')   /* all spaces after commands are ignored */
                cThis = getTexChar();

	       rewind_one(cThis); /* position of next character after command
					    except space */
	       break; /* for */
	       }
	   cCommand[i] = cThis;
	   }  /* for */

	   cCommand[i] = '\0';  /* mark end of string with zero */
    }  /* if \\ */
    else
      tabcounter++;

      if (strcmp(cCommand,"kill") == 0)
	{
	command_kill_found = TRUE;
	tabcounter = 0;
	break;
	}
  } /* while command_kill_found */
} /* Convert_Tabbing_with_kill */


/******************************************************************************/
void CmdBottom(/*@unused@*/ int code)
/******************************************************************************/
{
  /* it's conventional for the height of the text to be the same on all full pages */
}

/******************************************************************************
  purpose: converts the LaTex-abstract-command to an similar Rtf-style
parameter: code: on/off-option
 globals : article and titlepage from the documentstyle
 ******************************************************************************/
void
CmdAbstract(int code)
{ static char oldalignment = JUSTIFIED;


  switch (code)
     {
     case  ON:
	 if ((article) && (titlepage))
	    {
	    fprintf(fRtf,"\n\\par\n\\par\\pard ");
	    fprintf(fRtf,"\\pard\\qj ");  /* blocked */
	    fprintf (fRtf,
		     "{\\b\\fs%d %s}\\par ",
		     CurrentFontSize(), TranslateName("ABSTRACT"));
	    }
	 else
	    {
	    fprintf(fRtf,"\n\\par\n\\par\\pard \\page ");
	    fprintf(fRtf,"\\pard\\qj ");   /* blocked */
	    fprintf (fRtf,
		     "{\\b\\fs%d %s}\\par ",
		     CurrentFontSize(), TranslateName("ABSTRACT"));
	    }
	  oldalignment = alignment;
	  alignment = JUSTIFIED;
	  break;
    case  OFF:
	  fprintf(fRtf,"\\pard ");
	  alignment = oldalignment;
	  fprintf(fRtf,"\n\\par\\q%c ",alignment);
	  break;
     } /* switch */
}



/******************************************************************************/
void CmdTitlepage(int code)
/******************************************************************************
  purpose: converts the LaTex-Titlepage-command to an similar Rtf-style
parameter: on/off option
 globals : alignment: is used for the default-alignment-setting after this environment
 ******************************************************************************/
{
  switch (code)
     {
     case  ON:
	    fprintf(fRtf,"\n\\par\\pard \\page ");  /* new page */
	    fprintf(fRtf,"\n\\par\\q%c ",alignment);
	    break;
    case  OFF:
	  fprintf(fRtf,"\\pard ");
	  fprintf(fRtf,"\n\\par\\q%c \\page ",alignment);
	 break;
     } /* switch */
}

/******************************************************************************/
void CmdHyphenation(/*@unused@*/ int code)
/******************************************************************************
 purpose: the parameter surrrounded by braces after the hyphenation-command
	  won't be seperated at a line-end.
 ******************************************************************************/
{
    char *hyphenparameter;

    hyphenparameter = getParam();

/* In a future version we may correctly hyphenate all occurencies of
 * hyphenation-words
 */
# ifdef notdef
    for (i=0; i<(strlen(hyphenparameter)-1); i++)
	{
	if (hyphenparameter[i] != '-')
	    fprintf(fRtf,"%c",hyphenparameter[i]);
	else
	    fprintf(fRtf,"\\-");
	} /* for */
# endif /* notdef */
  
  free (hyphenparameter);
}

/******************************************************************************/
void CmdFormula2(int code)
/******************************************************************************
 purpose: creates a displayed equation
          \begin{equation} gets a right justified equation number
          \begin{displaymath} gets no equation number
          \[ gets no equation number
          $$ gets no equation number
          
 ******************************************************************************/
{
  if ((code & ON) || ((code == FORM_DOLLAR) && !g_processing_equation))  /* on switch */
  {
     code &= ~(ON);  /* mask MSB */
     g_processing_equation = TRUE;
	 g_suppress_equation_number = FALSE;
     BracketLevel++;
     fprintf(fRtf,"\n\\par\n\\par\\pard");
     switch (code)
     {
     	case FORM_DOLLAR:							/* $$ or displaymath */
     	case EQUATION_1:							/* equation* */
		    g_show_equation_number = FALSE;
	        fprintf(fRtf,"\\tqc\\tx4320\n");
	        break;
        
		case EQUATION:								/* equation */
	    	g_show_equation_number = TRUE;
        	fprintf(fRtf,"\\tqc\\tx4320\\tqr\\tx8640\n");
        	break;
        
     	case EQNARRAY_1:							/* eqnarray* */
			g_show_equation_number = FALSE;
			g_processing_eqnarray = TRUE;
	     	g_processing_tabular = TRUE;
			actCol = 1;
	        fprintf(fRtf,"\\tqr\\tx2880\\tqc\\tx3240\\tql\\tx3600\n");
	     	break;

     	case EQNARRAY:								/* eqnarray */
			g_show_equation_number = TRUE;
			g_processing_eqnarray = TRUE;
	     	g_processing_tabular = TRUE;
			actCol = 1;
	        fprintf(fRtf,"\\tqr\\tx2880\\tqc\\tx3240\\tql\\tx3600\\tqr\\tx8640\n");		
	     	break;

        default: ;
     }
     fprintf(fRtf,"\\tab {\\i ");
  }
  else /* off switch */
  {
     code &= ~(OFF);  /* mask MSB */
     BracketLevel--;
     g_processing_equation = FALSE;
     fprintf(fRtf,"}");
     if (g_show_equation_number && !g_suppress_equation_number)
     {
     	g_equation_number++;
        fprintf(fRtf,"\\tab (%d)",g_equation_number);
     }
     fprintf(fRtf,"\n\\par\n\\par");

     if (code == EQNARRAY || code == EQNARRAY_1)
     {
     	g_processing_tabular = FALSE;
		g_processing_eqnarray = FALSE;
     }
  }
}

/******************************************************************************/
void CmdLetter(int code)
/******************************************************************************
  purpose: pushes all necessary letter-commands on a stack
parameter: code: on/off-option for environment
 ******************************************************************************/
{
  if (code & ON)  /* on switch */
  {
    code &= ~(ON);  /* mask MSB */
    if (code == LETTER)
    {
      PushEnvironment(code);
    }
  }
  else /* off switch */
  {
    PopEnvironment();
  }
}

/******************************************************************************/
void CmdAddress(/*@unused@*/ int code)
/******************************************************************************
 purpose: prints the address in a letter
 globals: alignment
 ******************************************************************************/
{ static char oldalignment = JUSTIFIED;

     oldalignment = alignment;
     alignment = RIGHT;
     fprintf(fRtf,"\n\\par\\pard\\q%c ",alignment);   /* address will be printed on the right top */

     Convert(); /* convert routine is called again for evaluating the contens
		 hold in braces after the \address-command */

     alignment = oldalignment;
     fprintf(fRtf,"\\par\\chdate "); /* additional to the address the actual date is printed */

     fprintf(fRtf,"\n\\par\\pard\\q%c ",alignment);
}


/******************************************************************************/
void CmdSignature(/*@unused@*/ int code)
/******************************************************************************
 purpose: prints the signature in a letter
 globals: alignment
 ******************************************************************************/
{ static char oldalignment = JUSTIFIED;

     oldalignment = alignment;
     alignment = RIGHT;
     fprintf(fRtf,"\n\\par\\pard\\q%c ",alignment);   /* signature will be printed on the right top */

     Convert(); /* convert routine is called again for evaluating the contens
		 hold in braces after the \signature-command */

     alignment = oldalignment;
     fprintf(fRtf,"\n\\par\\pard\\q%c ",alignment);
}

/******************************************************************************/
void CmdOpening(/*@unused@*/ int code)
/******************************************************************************
 purpose: special command in the LaTex-letter-environment will be converted to a
	  similar Rtf-style
 globals: alignment
 ******************************************************************************/
{ static char oldalignment;

     oldalignment = alignment;
     alignment = LEFT;
     fprintf(fRtf,"\n\\par\\pard\\q%c ",alignment);   /* opening will be printed on the right top */

     Convert(); /* convert routine is called again for evaluating the contens
		 hold in braces after the \opening-command */

     alignment = oldalignment;
     fprintf(fRtf,"\n\\par\\pard\\q%c ",alignment);
}

/******************************************************************************/
void CmdClosing(/*@unused@*/ int code)
/******************************************************************************
 purpose: special command in the LaTex-letter-environment will be converted to a
	  similar Rtf-style
 globals: alignment
 ******************************************************************************/
{ static char oldalignment;

     oldalignment = alignment;
     alignment = LEFT;
     fprintf(fRtf,"\n\\par\\pard\\q%c ",alignment);   /* closing will be printed on the right top */

     Convert(); /* convert routine is called again for evaluating the contens
		 hold in braces after the \closing-command */

     alignment = oldalignment;
     fprintf(fRtf,"\n\\par\\pard\\q%c ",alignment);
}

void CmdPs(/*@unused@*/ int code)
{
    /* additional text to the \ps-command will be converted by the basic convert-routine */
    /* but you'll have to type the 'P.S.:'-text yourself */
}

/******************************************************************************/
void CmdMultiCol (/*@unused@*/ int code)
/******************************************************************************
 purpose: converts the LaTex-Multicolumn to a similar Rtf-style
	  this converting is only partially
	  so the user has to convert some part of the Table-environment by hand
parameter: unused
 ******************************************************************************/
{
  char inchar[10];
  char numColStr[100];
  long numCol, i, toBeInserted;
  char colFmtChar = 'u';
  char *eptr;         /* for srtol   */
  static bool bWarningDisplayed = FALSE;

  if (!bWarningDisplayed)
  {
     fprintf (stderr, "\nWARNING - Multicolumn: Cells must be merged by hand!\n");
     bWarningDisplayed = TRUE;
  }


  i = 0;
  do
  {
     inchar[0] = getTexChar();
     if (isdigit((unsigned char) inchar[0]))
        numColStr[i++] = inchar[0];     
  }
  while (inchar[0] != '}');
  numColStr[i] = '\0';
  numCol = strtol (numColStr, &eptr, 10);
  if (eptr == numColStr)
     error (" multicolumn: argument num invalid\n");
  

  do
  {
     inchar[0] = getTexChar();
     switch(inchar[0])
     {
          case 'c':
          case 'r':
          case 'l':
               if (colFmtChar == 'u') 
                  colFmtChar = inchar[0];
               break;
          default:
               break;
       }
  }
  while (inchar[0] != '}');
  if (colFmtChar == 'u') 
     colFmtChar = 'l';

  switch (colFmtChar)
  {
     case 'r':
          toBeInserted = numCol;
          break;
     case 'c':
          toBeInserted = (numCol+1) / 2;
          break;
     default:
          toBeInserted = 1;
          break;
  }

  for (i=1; i<toBeInserted; i++, actCol++)
  {
     fprintf (fRtf, " \\cell \\pard \\intbl ");
  }
  fprintf (fRtf, "\\q%c ", colFmtChar);
  
  Convert();
  
  for (i=toBeInserted; (i<numCol) && (actCol<colCount) ; i++, actCol++)
  {
     fprintf (fRtf, " \\cell \\pard \\intbl ");
  }


}


/******************************************************************************/
void CmdTabular(int code)
/******************************************************************************
 purpose: converts the LaTex-Tabular to a similar Rtf-style
          the size of the table should be adjusted by hand
parameter: type of array-environment
 globals: fTex: Tex-file-pointer
          fRtf: Rtf-file-pointer
          g_processing_tabular: TRUE if EnvironmenTabular is converted
          colFmt: contains alignment of the columns in the tabular
          colCount: number of columns in tabular is set
          actCol: actual treated column

   Does not handle \begin{tabular*}{width}[h]{ccc} 
   but it does do \begin{tabular*}[h]{ccc}
              and \begin{tabular*}{ccc}

 ******************************************************************************/
{
  char dummy[51];
  int i, n;
  static bool bWarningDisplayed = FALSE;
  int openBracesInParam = 1;

  if (code & ON)  /* on switch */
  {
    code &= ~(ON);  /* mask MSB */
    
    if (g_processing_tabular) error(" Nested tabular and array environments not supported! Giving up! \n");
    g_processing_tabular = TRUE;
    
    getBracketParam(dummy, 50);	  /* throw it away */
/*	fprintf(stderr, "the bracket string is '%s'\n",dummy);*/
    getBraceParam(dummy, 50);	  /* dummy should now have all the column instructions */
    
/*	fprintf(stderr, "the brace string is '%s'\n",dummy);*/
	
    if (!bWarningDisplayed)
    {
       fprintf (stderr, "\nWARNING - tabular or array environment: May need resizing.\n");
       bWarningDisplayed = TRUE;
    }

    assert(colFmt == NULL);
    colFmt = (char*) malloc (sizeof(char) * 20);
    if (colFmt == NULL)
      error(" malloc error -> out of memory!\n");
    n = 0;
    colFmt[n++] = ' ';     /* colFmt[0] unused */
    i = 0;
	while (dummy[i])
	{
/*       fprintf(stderr,"char='%c'\n",dummy[i]); */
       switch(dummy[i++])
       {
          case 'c':
               colFmt[n++] = 'c';
               break;
          case 'r':
               colFmt[n++] = 'r';
               break;
          case 'l':
               colFmt[n++] = 'l';
               break;
          case '{':
               openBracesInParam++;
               break;
          case '}':
               openBracesInParam--;
               break;
          case 'p':
       			fprintf (stderr, "\nWARNING - 'p{width}' not fully supported.\n");
               colFmt[n++] = 'l';
               break;
          case '*':
       			fprintf (stderr, "\nWARNING - '*{num}{cols}' not supported.\n");
               break;
          case '@':
       			fprintf (stderr, "\nWARNING - '@{text}' not supported.\n");
               break;
          default:
               break;
       }
    }

    colFmt[n] = '\0';
    colCount = n-1;
    actCol = 1;

	if (code == TABULAR_2)
	{
	     fprintf(fRtf,"\n\\par\\pard");
         fprintf(fRtf,"\\tqc\\tx1000\\tx2000\\tx3000\\tx4000\\tx5000\\tx6000\\tx7000\n\\tab");
		  return;
	}
		
    fprintf(fRtf, "\\par\\trowd\\trqc\\trrh0");
    for (i=1; i<= colCount; i++)
    { 
       fprintf (fRtf, "\\cellx%d ", (7236 / colCount) * i);  /* 7236 twips in A4 page */
    }
    fprintf (fRtf, "\n\\pard\\intbl\\q%c ", colFmt[1]);
  }
  else /* off switch */
  {
    code &= ~(OFF);  /* mask MSB */
    g_processing_tabular = FALSE;

    assert(colFmt != NULL);
    free (colFmt);
    colFmt = NULL;

	if (code == TABULAR_2)
		return;

    for (; actCol< colCount; actCol++)
    {
       fprintf (fRtf, "\\cell\\pard\\intbl ");
    }
    fprintf(fRtf,"\\cell\\pard\\intbl\\row\n"); 
    fprintf(fRtf,"\n\\pard\\par\\pard\\q%c\n",alignment); 

  }
}



/***************************************************************************
 * LEG190498
 *
 * purpose: hyperlatex support, makes the same as '&' in the convert
 * routine in main.c parameter: not used
 ***************************************************************************/
void CmdColsep(int code)
{
	if (!g_processing_tabular)
	{
		fprintf(fRtf,"{\\ansi\\'a7}");
		return;
	}
	
	actCol++;

	if (g_processing_equation)   /* means that we are in an eqnarray or array */
	{
		fprintf(fRtf,"\\tab ");
	}
 	else
 	{
		fprintf(fRtf," \\cell \\pard \\intbl ");
		if(colFmt == NULL)
			diagnostics(WARNING, "Fatal, Fatal! CmdColsep called whith colFmt == NULL.");
  		else
			fprintf (fRtf, "\\q%c ", colFmt[actCol]);
	}
}

/******************************************************************************/
void CmdTable(int code)
/******************************************************************************
 purpose: converts the LaTex-Table to a similar Rtf-style
	  this converting is only partially
	  so the user has to convert some part of the Table-environment by hand
parameter: type of array-environment
 ******************************************************************************/
{
  char location[10];

  if (code & ON)  /* on switch */
  {
    code &= ~(ON);  /* mask MSB */

    if ((code == FIGURE) || (code == FIGURE_1))
    	g_processing_figure=TRUE;

    getBracketParam(location, 10);
  }
  else /* off switch */
  {
    code &= ~(OFF);  /* mask MSB */
    g_processing_figure = FALSE;   
  }
}

/******************************************************************************/
void CmdNoCite(/*@unused@*/ int code)
/******************************************************************************
 purpose: produce reference-list at the end
  globals: bCite: is set to true if a \nocite appeared in text,
                  necessary to produce reference-list at the end of the
                  article
 ******************************************************************************/
{
  bCite = TRUE;
  free(getParam ()); /* just skip the parameter */
}

void CmdGraphics(int code)
{
	char options[255];
	char fullpath[1023], filename[255];
	char *dp;
	int cc,i;
	short top,left,bottom,right;
	FILE *fp;
	
	/* could be \includegraphics*[0,0][5,5]{file.pict} */
	
	getBracketParam(options, 255);
	getBracketParam(options, 255);
	getBraceParam(filename, 255);
	
    if (strstr(filename,".pict")||strstr(filename,".PICT"))
    {
	/*SAP fixes for Mac Platform*/
	  	strcpy(fullpath,latexname);
	  	dp=strrchr(fullpath,':');
	  	if (dp!=NULL) {dp++;*dp='\0';} else strcpy(fullpath,"");
	  	strcat(fullpath,filename);
	/*SAP end fix*/
	
		fprintf(stderr,"processing picture %s\n",fullpath);
		fp = fopen(fullpath,"rb");

		if(fseek(fp, 514L, SEEK_CUR)) {fclose(fp); return;}
	    if(fread(&top,2,1,fp) <1) {fclose(fp); return;}
	    if(fread(&left,2,1,fp) <1) {fclose(fp); return;}
	    if(fread(&bottom,2,1,fp) <1) {fclose(fp); return;}
	    if(fread(&right,2,1,fp) <1) {fclose(fp); return;}
		if(fseek(fp, -10L, SEEK_CUR)){fclose(fp); return;}
	    
	    fprintf (fRtf, "\n{\\pict\\macpict\\picw%d\\pich%d\n",right-left,bottom-top);
	    
	    i=0;
		while ((cc=fgetc(fp)) != EOF) 
		{
			fprintf(fRtf, "%.2x", cc);
			if (++i>126) {i=0; fprintf(fRtf, "\n");}  /* keep lines 254 chars long */
		}
		
	    fclose(fp);
	    fprintf (fRtf, "}\n");
    }
}

#undef FORMULASEP
#define FORMULASEP ','

void CmdRoot(int code)
/******************************************************************************
 purpose: converts \sqrt{x}
******************************************************************************/
{
  char * root;
 
  root = getParam();
  fprintf(fRtf, "{\\field{\\*\\fldinst  EQ \\\\R(%c", FORMULASEP);
  ConvertString(root);
  fprintf(fRtf,")}{\\fldrslt }}");

  free(root);
}

void CmdFraction(int code)
/******************************************************************************
 purpose: converts \frac{x}{y} (following Taupin's implementation in ltx2rtf)
******************************************************************************/
{
  char * denominator, *numerator;
 
  numerator = getParam();
  denominator = getParam();
  
  fprintf(fRtf, "{\\field{\\*\\fldinst  EQ \\\\F(");
  ConvertString(numerator);
  fprintf(fRtf,"%c", FORMULASEP);
  ConvertString(denominator);
  fprintf(fRtf,")}{\\fldrslt }}");

  free(numerator);
  free(denominator);
}

void CmdIntegral(int code)
/******************************************************************************
 purpose: converts integral symbol + the "exponent" and "subscript" fields
parameter: type of operand
 ******************************************************************************/
{
  char *upper_limit = NULL;
  char *lower_limit = NULL;
  char cThis;

  /* is there an exponent/subscript ? */
  cThis = getNonBlank();

  if(cThis == '_')
    lower_limit = getMathParam();
  else if (cThis == '^')
    upper_limit = getMathParam();
  else 
    rewind_one(cThis);
    
  if (upper_limit || lower_limit)
  {
    cThis = getNonBlank();
    if(cThis == '_')
        lower_limit = getMathParam();
    else if (cThis == '^')
        upper_limit = getMathParam();
    else 
        rewind_one(cThis);
   }

  fprintf(fRtf, "{\\field{\\*\\fldinst  EQ \\\\I(");
  if (lower_limit) ConvertString(lower_limit);
  fprintf(fRtf,"%c", FORMULASEP);
  if (upper_limit) ConvertString(upper_limit);
  fprintf(fRtf,"%c)}{\\fldrslt }}", FORMULASEP);

  if (lower_limit) free(lower_limit);
  if (upper_limit) free(upper_limit);
}

/******************************************************************************/
/*LEG190498*/
void CmdCite(int code)
/******************************************************************************
 purpose: opens existing aux-file and reads the citing-number
LEG190498
parameter: if FALSE (0) work as normal
           if HYPERLATEX get reference string from remembered \link parameter
  globals: input  (name of LaTeX-Inputfile)
           bCite: is set to true if a \cite appeared in text,
                  necessary to produce reference-list at the end of the
                  article
                  is set to false if the .aux file cannot be opened or
                  it is not up to date
	   LEG190498
	   hyperref: NULL, or the last used reference by \link command.
 ******************************************************************************/
{
  char reference[255];
  char *str1, *str2;

  fprintf (fRtf,"["); 

  if(code == HYPERLATEX)
  {
     if (hyperref == NULL) 
       {
       fprintf(stderr,"ERROR: \\Cite called before \\link\n");
       fprintf(fRtf, "?]");
       return;
       }
     strcpy(reference,hyperref);
  }
  else
    getBraceParam(reference, 255);

  str1 = reference;
  while ( (str2 = strchr(str1, ',')) != NULL )
  { 
        *str2 = '\0';                /* replace ',' with '\0' */
        if (ScanAux("bibcite",str1,0)) bCite = TRUE;
        fprintf(fRtf, ",");
        str1 = str2 + 1;
  }
        
  if (ScanAux("bibcite", str1, 0)) bCite = TRUE;
    
  fprintf (fRtf,"]"); 
}

/***********************************************************************
 * OpenBblFile
 * purpose: opens either the "input".bbl file, or the .bll file named by
 *          the -b command line option.
 * globals: BblName,
 */
FILE*
OpenBblFile(void)
{
  static FILE* fBbl = NULL;

  if (BblName==NULL)
    {
      char *s;
      if(input != NULL)
	{
	  if((BblName = malloc(strlen(input) + 5)) == NULL)
	    error(" malloc error -> out of memory!\n");
	  strcpy (BblName, input);
	  if((s = strrchr(BblName, '.')) == NULL || strcmp(s, ".tex") != 0)
	    strcat(BblName, ".bbl");
	  else
	    strcpy (s, ".bbl");
	}
      else
	BblName = EmptyString();
    }
  
  if ((fBbl = fopen(BblName,"r")) == NULL)   /* open file */
    {
      fprintf(stderr,"Error opening BBL-file: %s - ", BblName);
      error("no reference-list will be created.\n");
/* maybe @exits@ here */ 
    }
  return fBbl;
}

/*****************************************************************
 * MakeBiblio
 * Converts a bibliography environment
 */
void
MakeBiblio(FILE* fBbl)
{
  char BblLine[255];
  char *allBblLine = NULL;
  int refcount = 0;
  char *str;
  
  if (article)
    fprintf (fRtf, "\\par \\par \\pard{\\fs28 \\b %s}",
	     TranslateName("REFARTICLE"));
  else
    fprintf (fRtf, "\\par \\par \\pard{\\fs28 \\b %s}",
	     TranslateName("REF"));

  while (fgets (BblLine, 255, fBbl) != NULL)
  {
     if(strstr(BblLine, "\\begin{thebibliography}"))
	continue;
     if(strstr(BblLine, "\\end{thebibliography}"))
     {
	if(allBblLine != NULL)
	{
	   ConvertString(allBblLine);
	   fprintf (fRtf," ");
	   free (allBblLine);
	   allBblLine = NULL;
	}
	break;
     }
     if (strstr (BblLine, "\\bibitem") )
     {

	char *label;
	if(allBblLine != NULL)
	{
	   ConvertString(allBblLine);
	   fprintf (fRtf," ");
	   free (allBblLine);
	   allBblLine = NULL;
	}
	fprintf (fRtf, "\\par \\par \\pard ");
	if((label = strchr(BblLine, '[')) != NULL)
	{
	    for(; *label != '\0' && *label != ']'; label++)
		if(fputc(*label, fRtf) != (int) *label)
		  diagnostics(ERROR, "Failed fputc; funct2.c (WriteRefList): %c.", *label);
	    if(*label != '\0')
	      {
		if(fputc(*label, fRtf) != (int) *label)
		  diagnostics(ERROR, "Failed fputc; funct2.c (WriteRefList): %c.", *label);
	      }
	    if(fputc(' ', fRtf) != (int) ' ')
	      diagnostics(ERROR, "Failed fputc(' '); funct2.c (WriteRefList).");
	}
	else
	    fprintf (fRtf, "[%d] ", ++refcount);
	continue;
     }
     else if ((str = strstr (BblLine, "\\newblock")) != NULL)
     {
	 str += strlen("\\newblock");
	 if(allBblLine != NULL)
	 {
	    ConvertString(allBblLine);
	    fprintf (fRtf," ");
	    free (allBblLine);
	    allBblLine = NULL;
	 }
	 if ((allBblLine = malloc (strlen(str) + 1)) == NULL)
	   error(" malloc error -> out of memory!\n");
	 strcpy(allBblLine, str);
     }
     else
     {
	if (BblLine[0] != '\n')
	{
	  if(allBblLine != NULL)
	  {
	      if((allBblLine = (char *)realloc(allBblLine,
		strlen(allBblLine) + strlen(BblLine) + 1)) == NULL)
		 error(" realloc error -> out of memory!\n");
	      strcat (allBblLine, BblLine);
	  }
	  else
	  {
	     if ((allBblLine = malloc (strlen(BblLine) + 1)) == NULL)
	       error(" malloc error -> out of memory!\n");
	     strcpy(allBblLine, BblLine);
	  }
	}
     }
  }
  if (ferror(fBbl) != 0)
     error ("Error reading BBL-File!\n");

  if(allBblLine != NULL)
  {
    free (allBblLine);
  }
  (void)fclose (fBbl);
} 

/*******************************************************************
 * CmdConvertBiblio
 * converts a bibliography environment
 */
void CmdConvertBiblio(int code)
{
  MakeBiblio(fTex);
  /* Suppose!!! we are done now with bibliography */
  bCite = FALSE;
}


/**********************************************************************
 * WriteRefList
 * Converts a .bbl File to rtf output
 */
void
WriteRefList(void)
{
  MakeBiblio(OpenBblFile());
}
