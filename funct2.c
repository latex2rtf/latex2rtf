/*
 * $Id: funct2.c,v 1.2 2001/08/12 15:47:04 prahl Exp $
 * History:
 * $Log: funct2.c,v $
 * Revision 1.2  2001/08/12 15:47:04  prahl
 * latex2rtf version 1.1 by Ralf Schlatterbeck
 *
 * Revision 1.2  1994/06/21  08:14:11  ralf
 * Corrected Bug in keyword search
 *
 * Revision 1.1  1994/06/17  11:26:29  ralf
 * Initial revision
 *
 */
/***************************************************************************
   name : funct2.c
 author : DORNER Fernando, GRANZER Andreas
purpose : includes besides funct1.c all functions which are called from the programm commands.c;
 ****************************************************************************/

/********************************* includes *********************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <malloc.h>
#include "main.h"
#include "funct1.h"
#include "commands.h"
#include "funct2.h"
#include "stack.h"
/******************************************************************************/

/********************************   extern variables   *************************/
extern int tabcounter;
extern FILE *fRtf;
extern FILE *fTex;
extern int bPard;
extern BOOL bInDocument;
extern int BracketLevel;
extern int RecursLevel;
extern BOOL twocolumn;
extern BOOL TABBING_ON;
extern BOOL TABBING_ON_itself;
extern BOOL TABBING_RETURN;
extern BOOL article;
extern BOOL titlepage;
extern int fontsize;
extern char *progname;
extern BOOL MathMode;
extern char alignment;
extern long linenumber;
extern fpos_t pos_begin_kill;
/*****************************************************************************/

/***************************** global variables ********************************/
int tabstoparray[100];
int number_of_tabstops=0;
/*****************************************************************************/

/*----------------------------Tabbing Environment ------------------------*/
/******************************************************************************/
void Tabbing(int code)
/******************************************************************************
  purpose: pushes all tabbing-commands on a stack
parameter: code : on/off at begin/end-environment
  globals: TABBING_ON: true if tabbing-mode is on (only in this environment)
	   TABBING_RETURN, TABBING_ITSELF: true if environmend ends
 ******************************************************************************/
{
  if (code & ON)  /* on switch */
  {
    code &= ~(ON);  /* mask MSB */
    if (code == TABBING)
    {
      TABBING_ON = TRUE;
      /*TABBING_ON_itself = FALSE; */

      PushEnvironment(code);

      fprintf(fRtf,"\\par\\line ");
      fgetpos(fRtf,&pos_begin_kill);
      /* Test ConvertTabbing(); */
    }
  }
  else /* off switch */
  {
    /* TABBING_RETURN = TRUE;
    TABBING_ON_itself = TRUE; */
    TABBING_ON = FALSE;
    PopEnvironment();

    fprintf(fRtf,"\\par\\pard\\line\\q%c ",alignment);
  }
}

/******************************************************************************/
void CmdTabset(int code)
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
void CmdTabjump(int code)
/******************************************************************************
 purpose: jumps to an tabstop
 ******************************************************************************/
{
    fprintf(fRtf,"\\tab ");
}

/******************************************************************************/
void CmdTabkill(int code)
/******************************************************************************
 purpose: a line in the TABBING-Environment which ends with an kill-command won't be
	 written to the rtf-FILE
 ******************************************************************************/
{int i;

  fsetpos(fRtf,&pos_begin_kill);

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
  purpose: function, which overreads the Figure,Picture,Bibliopgraphy and Minipage
	   Environment
parameter: code: which environment to ignore
 ******************************************************************************/
{
 char endfigure[20];
 char zeichen;
 BOOL found = FALSE;
 int i, endstring=0;

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

    } /* end switch */

    while (fread(&zeichen,1,1,fTex) >=1)
    {
       if (zeichen == '\\')
       {
	  int found_space = FALSE;
	  for (i=0; i<=endstring; i++)
	    {
	    int nl = FALSE;
	    if (fread(&zeichen,1,1,fTex) < 1)
		numerror(ERR_EOF_INPUT);
	    if (zeichen == '\\')
		linenumber++;                   /* \\-in figure is a line !!! */
	    while(zeichen == ' ' || zeichen == '\t' || zeichen == '\n' && !nl)
	    {
		if(zeichen == '\n')
		    nl = TRUE;
		if(fread(&zeichen,1,1,fTex) !=1)
		    numerror(ERR_EOF_INPUT);
		found_space = TRUE;
	    }
	    if(found_space && zeichen != '{') /* for vi } */
		break;
	    found_space = FALSE;
	    if (zeichen != endfigure[i])
		break;
	    if (i == endstring)                 /* end-figure-found */
		found = TRUE;
	    } /* for */
       } /* if */
       if (zeichen == '%')
	  IgnoreTo('\n');
       if (zeichen == '\n')
	  linenumber++;                         /* count lines */
       if (found)
	   return;
    } /* while */
    numerror(ERR_EOF_INPUT);
}
/*------------------------------------------------------------------------*/


/******************************************************************************/
void CmdIgnoreParameter(int code)
/******************************************************************************
  purpose: function, which ignores both: optional and normal parameters
parameter: code: number of optional/normal parameters to ignore
 ******************************************************************************/
{int count_opt_param_close = 0;
 int count_norm_param = 0;
 int count_opt_param_open = 0;
 char cThis = ' ';
 char cNext = ' ';
 BOOL firstloop = TRUE;
 int bracket_open = 0;

    for(;;)  /* forever */
    {
       if ( (fread(&cThis,1,1,fTex) < 1))
	  numerror(ERR_EOF_INPUT);
       switch(cThis)
       {
	 case '{':
	  {
	   bracket_open++ ;
	  } break;
	 case '}':
	 {
	   bracket_open--;
	   if (bracket_open == 0)
	      count_norm_param++;
	   if ( (fread(&cNext,1,1,fTex) < 1))
		{
		     numerror(ERR_EOF_INPUT);
		}
	   fseek(fTex,-1L,SEEK_CUR); /* reread last character */
	   if (cNext == '[')         /* count open braces */
	       count_opt_param_open++;
	 } break;
       case ']':
	 {
	   count_opt_param_close++;
	   if ( (fread(&cNext,1,1,fTex) < 1))
	       numerror(ERR_EOF_INPUT);
	   fseek(fTex,-1L,SEEK_CUR); /* reread last character */
	   if (cNext == '[')         /* count open braces */
	       count_opt_param_open++;
	 } break;
       case '%':
	 {
	 IgnoreTo('\n');
	 } break;
       } /* end switch */
       if ((firstloop) && (cThis == '['))
	   count_opt_param_open++;
       switch (code)
       {
	  case Two_Opt_One_NormParam :
	  case One_Opt_One_NormParam :
	  case No_Opt_One_NormParam  : if ((count_opt_param_close ==
					    count_opt_param_open) &&
					   (count_norm_param == 1))
					   return;
				       break;
	  case Two_Opt_Two_NormParam :
	  case One_Opt_Two_NormParam :
	  case No_Opt_Two_NormParam  :if ((count_opt_param_close ==
					   count_opt_param_open) &&
					   (count_norm_param == 2))
					   return;
				      break;
	  case Two_Opt_Three_NormParam:
	  case One_Opt_Three_NormParam:
	  case No_Opt_Three_NormParam:if ((count_opt_param_close ==
					   count_opt_param_open) &&
					   (count_norm_param == 3))
					   return;
				      break;
	  case Two_Opt_No_NormParam  :
	  case One_Opt_No_NormParam  :if ((count_opt_param_close ==
					   count_opt_param_open) &&
					   (count_norm_param == 0))
					   {
					    if (firstloop == TRUE)
						fseek(fTex,-1L,SEEK_CUR); /* reread last character */
					    return;
					   }
				      break;
       } /* switch */
       firstloop = FALSE;
    } /* for */
}
/*------------------------------------------------------------------------*/


/******************************************************************************/
void Ignore_Environment(char *searchstring)
/******************************************************************************
  purpose: function, which ignores an unconvertable environment in LaTex
parameter: searchstring : includes the string to search for
	   example: \begin{unknown} ... \end{unknown}
		    searchstring="end{unknown}"
 ******************************************************************************/
{
 char zeichen;
 BOOL found = FALSE;
 int i, endstring;
    endstring = strlen(searchstring) - 1;
    while (fread(&zeichen,1,1,fTex) >=1)
    {
       if (zeichen == '\\')
       {
	  for (i=0; i<=endstring; i++)
	    {
	    if (fread(&zeichen,1,1,fTex) < 1)
		numerror(ERR_EOF_INPUT);
	    if (zeichen != searchstring[i])
		break;
	    if (i == endstring)                 /* end-environment-found */
		found = TRUE;
	    } /* for */
	} /* if */
       if (zeichen == '%')
	   IgnoreTo('\n');
       if (zeichen == '\n')
	   linenumber++;
       if (found)
	   return;
    } /* while */
    numerror(ERR_EOF_INPUT);
}
/*------------------------------------------------------------------------*/


/******************************************************************************/
void GetOptParam(char *string, int size)
/******************************************************************************
  purpose: function to get an optional parameter
parameter: string: returnvalue of optional parameter
	   size: max. size of returnvalue
 ******************************************************************************/
{
char cThis;
int i,PopLevel,PopBrack;

  if ( (fread(&cThis,1,1,fTex) < 1))
    numerror(ERR_EOF_INPUT);

  if ( cThis != '[' )
  {
    string = "";
    return;
  }
  else
  {
    ++BracketLevel;
    Push(RecursLevel,BracketLevel);
  }
  for (i = 0; ;i++)   /* get param from input stream */
  {
    if (fread(&cThis,1,1,fTex) < 1)
       numerror(ERR_EOF_INPUT);
    if (cThis == ']')
    {
      --BracketLevel;
      Pop(&PopLevel,&PopBrack);
      break;
    }
    if (cThis == '%')
      {
       IgnoreTo('\n');
       continue;
      }
    if (size-- > 0)
      string[i] = cThis;
  }
  string[i] = '\0';
}
/*------------------------------------------------------------------------*/

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
char *GetSubString(char *s, char terminatesymbol)
/******************************************************************************
purpose:
-----------------------------------------------------------------------------
 GetString is a function which returns the substrings of the string s
   seperated by the terminatesymbol 
 after calling this funtion the first substring to the terminatesymbol
   is cut off the string s 
-----------------------------------------------------------------------------*/
{char *substring;
 char *hilfstring;
 int i=0;

      substring =(char*) malloc((strlen(s)+1) * sizeof(char)); /* get heap-memory */
      if (substring == NULL)
	  error(" malloc error -> out of memory!\n");
      strcpy(substring,"");

      for(i=0; (int)i<=(int)strlen(s); i++)  /* if *s == "" -> for won't be executed */
      {                            /* <= is used for recognising the endmark of the string */
	if (s[i] == '\0')
	{
	   strcpy(s,"");
	   break;
	}
	if (s[i] == terminatesymbol)
	{
	   hilfstring = &s[i+1];
	   strcpy(s,hilfstring);    /* is also replaced in the calling-function */
	   break;
	}
	substring[i] = s[i];
      }  /* for */

      substring[i] = '\0'; /* end-mark */
      return substring;
}
/*-----------------------------------------------------------------------------*/

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
      case NewColumn : if (twocolumn == TRUE)
			  fprintf(fRtf,"\\column "); /* new column */
		       else
			  fprintf(fRtf,"\\page ");  /* causes new page */
		       break;
   } /* switch */
}

/******************************************************************************/
void Cmd_OptParam_Without_braces(int code)
/******************************************************************************
 purpose: gets an optional parameter which isn't surrounded by braces but by spaces
 ******************************************************************************/
{char cNext=' ';
 char cLast=' ';

    do
    {
       cLast = cNext;
       if (fread(&cNext,1,1,fTex) < 1)
	   numerror(ERR_EOF_INPUT);

    } while ((cNext != ' ') &&
	     (cNext != '\\') &&
	     (cNext != '{') &&
	     (cNext != '\n') &&
	     (cNext != ',') &&
	     ((cNext != '.')  || (isdigit(cLast))) && /* . doesn't mean the end of an command inside an number of the type real */
	     (cNext != '}') &&
	     (cNext != '\"') &&
	     (cNext != '[') &&
	     (cNext != '$'));

    fseek(fTex,-1L,SEEK_CUR);
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
  BOOL readuntilnewline = FALSE;

  if ( (fread(&cThis,1,1,fTex) < 1))
    numerror(ERR_EOF_INPUT);
  if ( cThis == '{' )
  {
    ++BracketLevel;
    Push(RecursLevel,BracketLevel);

  }
  else
  {
    readuntilnewline = TRUE;
    fseek(fTex,-1L,SEEK_CUR); /* reread last character */
  }
  for (i = 0; ;i++)   /* get param from input stream */
  {
    if (fread(&cThis,1,1,fTex) < 1)
       numerror(ERR_EOF_INPUT);
    if (cThis == '}')
    {
      --BracketLevel;
      Pop(&PopLevel,&PopBrack);
      break;
    }

    if ((readuntilnewline == TRUE) &&
	((cThis == ' ') || (cThis == '\n')))
	{
	if (cThis == '\n')
	    linenumber++;
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
  BOOL getcommand;
  BOOL command_end_line_found;
  BOOL command_kill_found;

while (TABBING_ON)
{
command_end_line_found = FALSE;
command_kill_found = FALSE;

while (command_end_line_found == FALSE)
  {
  for (;;) /* do forever */
  {
    getcommand=FALSE;
    if (fread(&cThis,1,1,fTex) < 1)
       numerror(ERR_EOF_INPUT);
    j++;

    if (cThis == '\\')
       {
       getcommand=TRUE;
       strcpy(cCommand,"");

       for (i = 0; ;i++)   /* get command from input stream */
	   {
	   if (fread(&cThis,1,1,fTex) < 1)
	      numerror(ERR_EOF_INPUT);
	   j++;

	   if (i == 0) /* test for special characters */
	      {
	      switch(cThis)
		  {
		  case '\\':    command_end_line_found=TRUE;
				break;
		    } /* switch */
	      }  /* if */

	   if (!isalpha(cThis))
	       {
	       while (cThis == ' ')   /* all spaces after commands are ignored */
	       {
	       if (fread(&cThis,1,1,fTex) < 1)
		   numerror(ERR_EOF_INPUT);
	       j++;
	       }

	       fseek(fTex,-1L,SEEK_CUR); /* position of next character after command
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

  fseek(fTex,-j,SEEK_CUR); /* re_read line */
  if (command_kill_found)
    Convert_Tabbing_with_kill();
  else
    Convert();
} /* while Tabbing_ON */

TABBING_ON = FALSE;
} /* ConvertTabbing */


/******************************************************************************/
void Convert_Tabbing_with_kill(void)
/******************************************************************************
 purpose: routine which converts the tabbing-kill-option from LaTex to Rtf
 globals: tabcounter:
 ******************************************************************************/
{ int i=0;
  BOOL command_kill_found=FALSE;
  char cThis;
  char cCommand[MAXCOMMANDLEN];

tabcounter=0;

while (command_kill_found == FALSE)
   {
    if (fread(&cThis,1,1,fTex) < 1)
       numerror(ERR_EOF_INPUT);

    strcpy(cCommand,"");

    if (cThis == '\\')
       {

       for (i = 0; ;i++)   /* get command from input stream */
	   {
	   if (fread(&cThis,1,1,fTex) < 1)
	      numerror(ERR_EOF_INPUT);

	   if (i == 0) /* test for special characters */
	      {
	      switch(cThis)
		  {
		  case '=': CmdTabset(0);
			    break;
		  default : if(!isalpha(cThis))
				 numerror(ERR_WRONG_COMMAND_IN_TABBING);
		    } /* switch */
	      }  /* if */

	   if (!isalpha(cThis))
	       {
	       while (cThis == ' ')   /* all spaces after commands are ignored */
	       {
	       if (fread(&cThis,1,1,fTex) < 1)
		   numerror(ERR_EOF_INPUT);
	       }

	       fseek(fTex,-1L,SEEK_CUR); /* position of next character after command
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
void CmdBottom(int code)
/******************************************************************************/
{
  /* it's conventional for the height of the text to be the same on all full pages */
}

/******************************************************************************/
void CmdAbstract(int code)
/******************************************************************************
  purpose: converts the LaTex-abstract-command to an similar Rtf-style
parameter: code: on/off-option
 globals : article and titlepage from the documentstyle
 ******************************************************************************/
{ static char oldalignment = JUSTIFIED;


  switch (code)
     {
     case  ON:
	 if ((article) && (titlepage))
	    {
	    fprintf(fRtf,"\n\r\\par\n\\par\\pard ");
	    fprintf(fRtf,"\\pard\\qj ");  /* blocked */
	    fprintf(fRtf,"{\\b\\fs%d Abstract}",fontsize);
	    }
	 else
	    {
	    fprintf(fRtf,"\n\r\\par\n\\par\\pard \\page ");
	    fprintf(fRtf,"\\pard\\qj ");   /* blocked */
	    fprintf(fRtf,"{\\b\\fs%d Abstract:}\\par ",fontsize);
	    }
	  oldalignment = alignment;
	  alignment = JUSTIFIED;
	  break;
    case  OFF:
	  fprintf(fRtf,"\\pard ");
	  alignment = oldalignment;
	  fprintf(fRtf,"\n\r\\par\\q%c ",alignment);
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
	    fprintf(fRtf,"\n\r\\par\\pard \\page ");  /* new page */
	    fprintf(fRtf,"\n\r\\par\\q%c ",alignment);
	    break;
    case  OFF:
	  fprintf(fRtf,"\\pard ");
	  fprintf(fRtf,"\n\r\\par\\q%c \\page ",alignment);
	 break;
     } /* switch */
}

/******************************************************************************/
void CmdHyphenation(int code)
/******************************************************************************
 purpose: the parameter surrrounded by braces after the hyphenation-command
	  won't be seperated at a line-end.
 ******************************************************************************/
{ char hyphenparameter[10240];
  unsigned int i;

    GetParam(hyphenparameter,10239);

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
}

/******************************************************************************/
void CmdFormula2(int code)
/******************************************************************************
 purpose: the same as the function CmdFormula: see above!
 ******************************************************************************/
{
  if (code & ON)  /* on switch */
  {
     MathMode = TRUE;
  }
  else /* off switch */
  {
     MathMode = FALSE;
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
void CmdAddress(int code)
/******************************************************************************
 purpose: prints the address in a letter
 globals: alignment
 ******************************************************************************/
{ static char oldalignment = JUSTIFIED;

     oldalignment = alignment;
     alignment = RIGHT;
     fprintf(fRtf,"\n\r\\par\\pard\\q%c ",alignment);   /* address will be printed on the right top */

     Convert(); /* convert routine is called again for evaluating the contens
		 hold in braces after the \address-command */

     alignment = oldalignment;
     fprintf(fRtf,"\\par\\chdate "); /* additional to the address the actual date is printed */

     fprintf(fRtf,"\n\r\\par\\pard\\q%c ",alignment);
}


/******************************************************************************/
void CmdSignature(int code)
/******************************************************************************
 purpose: prints the signature in a letter
 globals: alignment
 ******************************************************************************/
{ static char oldalignment = JUSTIFIED;

     oldalignment = alignment;
     alignment = RIGHT;
     fprintf(fRtf,"\n\r\\par\\pard\\q%c ",alignment);   /* signature will be printed on the right top */

     Convert(); /* convert routine is called again for evaluating the contens
		 hold in braces after the \signature-command */

     alignment = oldalignment;
     fprintf(fRtf,"\n\r\\par\\pard\\q%c ",alignment);
}

/******************************************************************************/
void CmdOpening(int code)
/******************************************************************************
 purpose: special command in the LaTex-letter-environment will be converted to a
	  similar Rtf-style
 globals: alignment
 ******************************************************************************/
{ static char oldalignment;

     oldalignment = alignment;
     alignment = LEFT;
     fprintf(fRtf,"\n\r\\par\\pard\\q%c ",alignment);   /* opening will be printed on the right top */

     Convert(); /* convert routine is called again for evaluating the contens
		 hold in braces after the \opening-command */

     alignment = oldalignment;
     fprintf(fRtf,"\n\r\\par\\pard\\q%c ",alignment);
}

/******************************************************************************/
void CmdClosing(int code)
/******************************************************************************
 purpose: special command in the LaTex-letter-environment will be converted to a
	  similar Rtf-style
 globals: alignment
 ******************************************************************************/
{ static char oldalignment;

     oldalignment = alignment;
     alignment = LEFT;
     fprintf(fRtf,"\n\r\\par\\pard\\q%c ",alignment);   /* closing will be printed on the right top */

     Convert(); /* convert routine is called again for evaluating the contens
		 hold in braces after the \closing-command */

     alignment = oldalignment;
     fprintf(fRtf,"\n\r\\par\\pard\\q%c ",alignment);
}

void CmdPs(int code)
{
    /* additional text to the \ps-command will be converted by the basic convert-routine */
    /* but you'll have to type the 'P.S.:'-text yourself */
}

/******************************************************************************/
void CmdArray(int code)
/******************************************************************************
 purpose: converts the LaTex-Array/eqnarray to a similar Rtf-style
	  this converting is only partially
	  so the user has to convert some part of the array/eqnarray-environment by hand
parameter: type of array-environment
 ******************************************************************************/
{
  if (code & ON)  /* on switch */
  {
    code &= ~(ON);  /* mask MSB */
    fprintf(stderr,"%s: WARNING: Following environment cannot be converted completely!\n",progname);
    fprintf(stderr,"Some parts of this environment have to be converted and corrected by hand\n");

    if (code == ARRAY)
      {
      fprintf(stderr,"Begin of environment: ARRAY\n");
      fprintf(fRtf,"\\par ***begin of environment: ARRAY ***\\par");
      }
    if (code == EQNARRAY)
      {
      fprintf(stderr,"Begin of environment: EQNARRAY\n");
      fprintf(fRtf,"\\par ***begin of environment: EQNARRAY ***\\par");
      }
    if (code == EQNARRAY_1)
      {
      fprintf(stderr,"Begin of environment: EQNARRAY* \n");
      fprintf(fRtf,"\\par ***begin of environment: EQNARRAY*    ***\\par");
      }
  }
  else /* off switch */
  {
    code &= ~(OFF);  /* mask MSB */
    if (code == ARRAY)
      {
      fprintf(stderr,"End of environment: ARRAY\n");
      fprintf(fRtf,"\\par ***end of environment: ARRAY ***\\par");
      }
    if (code == EQNARRAY)
      {
      fprintf(stderr,"End of environment: EQNARRAY\n");
      fprintf(fRtf,"\\par ***end of environment: EQNARRAY ***\\par");
      }
    if (code == EQNARRAY_1)
      {
      fprintf(stderr,"End of environment: EQNARRAY* \n");
      fprintf(fRtf,"\\par ***end of environment: EQNARRAY*    ***\\par");
      }
  }
}

/******************************************************************************/
void CmdTabular(int code)
/******************************************************************************
 purpose: converts the LaTex-Tabular to a similar Rtf-style
	  this converting is only partially
	  so the user has to convert some part of the Tabular-environment by hand
parameter: type of array-environment
 ******************************************************************************/
{
  if (code & ON)  /* on switch */
  {
    code &= ~(ON);  /* mask MSB */
    fprintf(stderr,"%s: WARNING: Following environment cannot be converted completely!\n",progname);
    fprintf(stderr,"Some parts of this environment have to be converted and corrected by hand\n");

    if (code == TABULAR)
      {
      fprintf(stderr,"Begin of environment: TABULAR\n");
      fprintf(fRtf,"{\\v\\par ***begin of environment: TABULAR ***\\par}");
      }
    if (code == TABULAR_1)
      {
      fprintf(stderr,"Begin of environment: TABULAR*  \n");
      fprintf(fRtf,"{\\v\\par ***begin of environment: TABULAR*   ***\\par}");
      }
  }
  else /* off switch */
  {
    code &= ~(OFF);  /* mask MSB */
    if (code == TABULAR)
      {
      fprintf(stderr,"End of environment: TABULAR\n");
      fprintf(fRtf,"{\\v\\par ***end of environment: TABULAR ***\\par}");
      }
    if (code == TABULAR_1)
      {
      fprintf(stderr,"End of environment: TABULAR*  \n");
      fprintf(fRtf,"{\\v\\par ***end of environment: TABULAR*  ***\\par}");
      }
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
  if (code & ON)  /* on switch */
  {
    code &= ~(ON);  /* mask MSB */
    fprintf(stderr,"%s: WARNING: Following environment cannot be converted completely!\n",progname);
    fprintf(stderr,"Some parts of this environment have to be converted and corrected by hand\n");

    if (code == TABLE)
      {
      fprintf(stderr,"Begin of environment: TABLE\n");
      fprintf(fRtf,"{\\v\\par ***begin of environment: TABLE ***\\par}");
      }
    if (code == TABLE_1)
      {
      fprintf(stderr,"Begin of environment: TABLE*  \n");
      fprintf(fRtf,"{\\v\\par ***begin of environment: TABLE*   ***\\par}");
      }
  }
  else /* off switch */
  {
    code &= ~(OFF);  /* mask MSB */
    if (code == TABLE)
      {
      fprintf(stderr,"End of environment: TABLE\n");
      fprintf(fRtf,"{\\v\\par ***end of environment: TABLE ***\\par}");
      }
    if (code == TABLE_1)
      {
      fprintf(stderr,"End of environment: TABLE*  \n");
      fprintf(fRtf,"{\\v\\par ***end of environment: TABLE*  ***\\par}");
      }
  }
}
