/*
 * $Id: funct1.c,v 1.1 2001/08/12 15:32:24 prahl Exp $
 * History:
 * $Log: funct1.c,v $
 * Revision 1.1  2001/08/12 15:32:24  prahl
 * Initial revision
 *
 * Revision 1.2  1994/06/21  08:14:11  ralf
 * Corrected Bug in keyword search
 *
 * Revision 1.1  1994/06/17  11:26:29  ralf
 * Initial revision
 *
 */
/***************************************************************************
   name : funct1.c
 author : DORNER Fernando, GRANZER Andreas
purpose : includes besides funct2.c all functions which are called from the programm commands.c;
 ****************************************************************************/

/********************************* includes *********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "funct1.h"
#include "funct2.h"
#include "commands.h"
#include "stack.h"
#include "fonts.h"
/*****************************************************************************/

/**************************** extern variables ******************************/
extern FILE *fRtf;                       /* Rtf-File-Pointer */
extern FILE *fTex;                       /* LaTex-File-Pointer */
extern int bPard;                        /* true if \pard-command is necessary */
extern BOOL bInDocument;                 /* true if File-Pointer is in the document */
extern int BracketLevel;                 /* counts open braces */
extern int RecursLevel;                  /* counts returns occured by closing braces */
extern BOOL MathMode;                    /* true at a formula-convertion */
extern int fontsize;                     /* includes the actual fontsize in points */
extern BOOL twocolumn;                   /* true if twocolumn-mode is enabled */
extern BOOL titlepage;                   /* true if titlepage-mode is set */
extern BOOL article;                     /* true if article-mode is set */
extern int indent;                       /* includes the left margin e.g. for itemize-commands */
extern int NoNewLine;
extern BOOL bNewPar;
extern BOOL TABBING_ON;
extern BOOL TITLE_AUTHOR_ON;
extern char *progname;
extern char *latexname;
extern char alignment;
extern BOOL GermanMode;
extern BOOL mbox;
extern long linenumber;
/***************************************************************************/

/***************************  prototypes     ********************************/
void ConvertFormula();
/****************************************************************************/

/****************************************************************************/
void CmdCharFormat(int code)
/****************************************************************************
     purpose : sets the characterformat to bold, italic, underlined...
   parameter : code includes the character-format-style
 ****************************************************************************/
{
  if (TABBING_ON == FALSE)
    {
     switch(code)
     {
       case CMD_BOLD: fprintf(fRtf,"{\\b ");
		      break;
       case CMD_ITALIC: fprintf(fRtf,"{\\i ");
			break;
       case CMD_UNDERLINE: fprintf(fRtf,"{\\ul ");
			   break;
       case CMD_CAPS: fprintf(fRtf,"{\\scaps ");
		      break;
     }
     Convert();
     fprintf(fRtf,"}");
   }
}

/******************       helping function for CmdBegin               ***/
/**************************************************************************/
void GetParam(char *string, int size)
/**************************************************************************
     purpose: returns the parameter after the \begin-command
	      for instance: \begin{environment}
		    return: -> string = "environment"
   parameter: string: look at purpose
		 int: maximal number of characters from string
     returns: success: string
	      miss : string = ""
 **************************************************************************/
{
  char cThis;
  int i,PopLevel,PopBrack;
  int bracket=0;

  if ( (fread(&cThis,1,1,fTex) < 1))
    numerror(ERR_EOF_INPUT);
  if ( cThis != '{' )
  {
    string[0] = cThis;
    string[1] = '\0';
    return;
  }
  else
  {
    bracket++;
    ++BracketLevel;
    Push(RecursLevel,BracketLevel);
  }
  /* Varibale Bracket is 1 here */
  for (i = 0; ;i++)   /* get param from input stream */
  {
    if (fread(&cThis,1,1,fTex) < 1)
       numerror(ERR_EOF_INPUT);
    if (cThis == '}')
    {
      bracket--;
      if (bracket == 0)
      {
	--BracketLevel;
	Pop(&PopLevel,&PopBrack);
       break;
      }
    }
    if (cThis == '{')
    {
      bracket++;
    }

    /* \and-command handling routine
    if (cThis == '\\')
       {
       /* command is overread !
	for(;;)
	{
	if (fread(&cThis,1,1,fTex) < 1)
	   numerror(ERR_EOF_INPUT);
	if (!isalpha(cThis))
	    break;
	}
       fseek(fTex,-1L,SEEK_CUR); /* reread last character
       continue;
       }
     */
    if (cThis == '%')
	{
	IgnoreTo('\n');
	i--;
	continue;
	}

    if (size-- > 0)
      string[i] = cThis;
  }
  string[i] = '\0';
}

/***************************************************************************/
void CmdBeginEnd(int code)
/***************************************************************************
   purpose: reads the parameter after the \begin or \end-command; ( see also GetParam )
	    after reading the parameter the CallParamFunc-function calls the
	    handling-routine for that special environment
 parameter: code: CMD_BEGIN: start of environment
		  CMD_END:   end of environment
 ***************************************************************************/
{
  char cParam[50];
  switch(code)
  {
    case CMD_BEGIN:
		    GetParam(cParam,49);
		    CallParamFunc(cParam,ON);
		    break;
    case CMD_END:
		    GetParam(cParam,49);
		    CallParamFunc(cParam,OFF);
		    break;
  }
}

/********************************************************************************/
void Paragraph(int code)
/*****************************************************************************
    purpose : sets the alignment for a paragraph
  parameter : code: alignment centered, justified, left or right
   globals  : bpard: after such a paragraph-mode the default has to be set back
		     bpard would do this in the function convert in the file main.c
 ********************************************************************************/
{ static char old_alignment_before_center = JUSTIFIED;
  static char old_alignment_before_right = JUSTIFIED;
  static char old_alignment_before_left = JUSTIFIED;


  switch(code)
  {
    case (PAR_CENTER | ON):
      old_alignment_before_center = alignment;
      alignment = CENTERED;
      fprintf(fRtf,"\\pard \\q%c ",alignment);
      break;
    case (PAR_CENTER | OFF):
      bPard = TRUE;
      alignment = old_alignment_before_center;
      break;

    case (PAR_RIGHT | ON):
      old_alignment_before_right = alignment;
      alignment = RIGHT;
      fprintf(fRtf,"\\pard \\q%c ",alignment);
      break;
    case (PAR_RIGHT | OFF):
      bPard = TRUE;
      alignment = old_alignment_before_right;
      break;

    case (PAR_LEFT | ON):
      old_alignment_before_left = alignment;
      alignment = LEFT;
      fprintf(fRtf,"\\pard \\q%c ",alignment);
      break;
    case (PAR_LEFT | OFF):
      bPard = TRUE;
      alignment = old_alignment_before_left;
      break;
  }
}

/******************************************************************************/
void CmdToday(int code)
/******************************************************************************
    purpose: converts the LaTex-date-command into a Rtf-chdate-command which
	     prints the current date into an document
 ******************************************************************************/
{
  fprintf(fRtf,"\\chdate ");
}


/******************************************************************************/
void CmdUmlaute(int code)
/******************************************************************************
 purpose : converts german symbols from LaTeX to Rtf
 ******************************************************************************/
{
  static char cHexDigits[16] = {'0', '1', '2' ,'3', '4', '5', '6', '7', '8',
				'9', 'a', 'b', 'c', 'd', 'e', 'f' };
  char cParam[10];

  GetParam(cParam,9);

  switch(cParam[0])
  {
    case 'o': fprintf(fRtf,"\\'%c%c ",cHexDigits[('”'>>4)&0x0f],
		cHexDigits[('”'&0x0f)]);
	      break;
    case 'O': fprintf(fRtf,"\\'%c%c ",cHexDigits[('™'>>4)&0x0f],
		cHexDigits[('™'&0x0f)]);
	      break;
    case 'a': fprintf(fRtf,"\\'%c%c ",cHexDigits[('„'>>4)&0x0f],
		cHexDigits[('„'&0x0f)]);
	      break;
    case 'A': fprintf(fRtf,"\\'%c%c ",cHexDigits[('Ž'>>4)&0x0f],
		cHexDigits[('Ž'&0x0f)]);
	      break;
    case 'u': fprintf(fRtf,"\\'%c%c ",cHexDigits[(''>>4)&0x0f],
		cHexDigits[(''&0x0f)]);
	      break;
    case 'U': fprintf(fRtf,"\\'%c%c ",cHexDigits[('š'>>4)&0x0f],
		cHexDigits[('š'&0x0f)]);
	      break;
    case 'E':fprintf(fRtf, "\\ansi\\'cb\\pc ");
	     break;
    case 'I':fprintf(fRtf, "\\ansi\\'cf\\pc ");
	     break;
    case 'e':fprintf(fRtf, "\\ansi\\'eb\\pc ");
	     break;
    case 'i':fprintf(fRtf, "\\ansi\\'ef\\pc ");
	     break;
    case 'y':fprintf(fRtf, "\\ansi\\'ff\\pc ");
	     break;
  }
}

/******************************************************************************/
void CmdLApostrophChar(int code)
/******************************************************************************
 purpose: converts special symbols from LaTex to Rtf
 ******************************************************************************/
{
  char cParam[10];

  GetParam(cParam,9);
  switch(cParam[0])
  {
    case 'A':fprintf(fRtf, "\\ansi\\'c0\\pc ");
	     break;
    case 'E':fprintf(fRtf, "\\ansi\\'c8\\pc ");
	     break;
    case 'I':fprintf(fRtf, "\\ansi\\'cc\\pc ");
	     break;
    case 'O':fprintf(fRtf, "\\ansi\\'d2\\pc ");
	     break;
    case 'U':fprintf(fRtf, "\\ansi\\'d9\\pc ");
	     break;
    case 'a':fprintf(fRtf, "\\ansi\\'e0\\pc ");
	     break;
    case 'e':fprintf(fRtf, "\\ansi\\'e8\\pc ");
	     break;
    case 'i':fprintf(fRtf, "\\ansi\\'ec\\pc ");
	     break;
    case 'o':fprintf(fRtf, "\\ansi\\'f2\\pc ");
	     break;
    case 'u':fprintf(fRtf, "\\ansi\\'f9\\pc ");
	     break;
  }
}


/******************************************************************************/
void CmdRApostrophChar(int code)
/******************************************************************************
 purpose: converts special symbols from LaTex to Rtf
 ******************************************************************************/
{
  char cParam[10];

  GetParam(cParam,9);
  switch(cParam[0])
  {
    case 'A':fprintf(fRtf, "\\ansi\\'c1\\pc ");
	     break;
    case 'E':fprintf(fRtf, "\\ansi\\'c9\\pc ");
	     break;
    case 'I':fprintf(fRtf, "\\ansi\\'cd\\pc ");
	     break;
    case 'O':fprintf(fRtf, "\\ansi\\'d3\\pc ");
	     break;
    case 'U':fprintf(fRtf, "\\ansi\\'da\\pc ");
	     break;
    case 'a':fprintf(fRtf, "\\ansi\\'e1\\pc ");
	     break;
    case 'e':fprintf(fRtf, "\\ansi\\'e9\\pc ");
	     break;
    case 'i':fprintf(fRtf, "\\ansi\\'ed\\pc ");
	     break;
    case 'o':fprintf(fRtf, "\\ansi\\'f3\\pc ");
	     break;
    case 'u':fprintf(fRtf, "\\ansi\\'fa\\pc ");
	     break;
    case 'y':fprintf(fRtf, "\\ansi\\'fd\\pc ");
	     break;
    case 'Y':fprintf(fRtf, "\\ansi\\'dd\\pc ");
	     break;
  }
}

/******************************************************************************/
void CmdSpitzeChar(int code)
/******************************************************************************
 purpose: converts special symbols from LaTex to Rtf
 ******************************************************************************/
{
  char cParam[10];

  GetParam(cParam,9);
  switch(cParam[0])
  {
    case 'A':fprintf(fRtf, "\\ansi\\'c2\\pc ");
	     break;
    case 'E':fprintf(fRtf, "\\ansi\\'ca\\pc ");
	     break;
    case 'I':fprintf(fRtf, "\\ansi\\'ce\\pc ");
	     break;
    case 'O':fprintf(fRtf, "\\ansi\\'d4\\pc ");
	     break;
    case 'U':fprintf(fRtf, "\\ansi\\'db\\pc ");
	     break;
    case 'a':fprintf(fRtf, "\\ansi\\'e2\\pc ");
	     break;
    case 'e':fprintf(fRtf, "\\ansi\\'ea\\pc ");
	     break;
    case 'i':fprintf(fRtf, "\\ansi\\'ee\\pc ");
	     break;
    case 'o':fprintf(fRtf, "\\ansi\\'f4\\pc ");
	     break;
    case 'u':fprintf(fRtf, "\\ansi\\'fb\\pc ");
	     break;
  }
}

/******************************************************************************/
void CmdTildeChar(int code)
/******************************************************************************
 purpose: converts special symbols from LaTex to Rtf
 ******************************************************************************/
{
  char cParam[10];

  GetParam(cParam,9);
  switch(cParam[0])
  {
    case 'A':fprintf(fRtf, "\\ansi\\'c3\\pc ");
	     break;
    case 'O':fprintf(fRtf, "\\ansi\\'d5\\pc ");
	     break;
    case 'a':fprintf(fRtf, "\\ansi\\'e3\\pc ");
	     break;
    case 'o':fprintf(fRtf, "\\ansi\\'f5\\pc ");
	     break;
  }
}

/******************************************************************************/
void CmdFontSize(int code)
/******************************************************************************
 purpose : sets the fontsize to the point-size given by the LaTex-\fs_size-command
 globals : fontsize : includes the actual fontsize in the document
 ******************************************************************************/
{
  code = (code*fontsize)/20;
  fprintf(fRtf,"\\fs%d ",code);
}

/******************************************************************************/
void CmdLogo(int code)
/******************************************************************************
 purpose : prints the LaTex, Tex, SLiTex and BibTex-Logos as an ordinary text
	   in the Rtf-File
 ******************************************************************************/
{
  switch(code)
  {
    case CMD_TEX: fprintf(fRtf, "TeX"); break;
    case CMD_LATEX: fprintf(fRtf, "LaTeX"); break;
    case CMD_SLITEX: fprintf(fRtf, "SLiTeX"); break;
    case CMD_BIBTEX: fprintf(fRtf, "BibTeX");break;
  }
}

/******************************************************************************/
void CmdFormula(int code)
/******************************************************************************
 purpose: sets the Math-Formula-Mode depending on the code-parameter
 parameter : code: type of braces which include the formula
 ******************************************************************************/
{
  switch(code)
  {
    case FORM_DOLLAR:
      fprintf(fRtf," ");
      if (MathMode == TRUE)
	MathMode = FALSE;
      else
	MathMode = TRUE;
      break;
    case FORM_RND_OPEN:
      fprintf(fRtf," ");
      if (MathMode == TRUE)
      {
	fprintf(stderr,"\n%s: ERROR: warning - nested formulas (1) in File: %s at linenumber: %ld\n",progname,latexname,linenumber);
	fprintf(stderr,"\nprogram aborted\n");
	exit(-1);
      }
      MathMode = TRUE;
      break;
    case FORM_RND_CLOSE:
      fprintf(fRtf," ");
      if (MathMode == FALSE)
      {
	fprintf(stderr,"\n%s: ERROR: warning - nested formulas (2) in File: %s at linenumber: %ld\n",progname,latexname,linenumber);
	fprintf(stderr,"\nprogram aborted\n");
	exit(-1);
      }
      MathMode = FALSE;
      break;
    case FORM_ECK_OPEN:
      fprintf(fRtf,"\n\r\\par ");
      if (MathMode == TRUE)
      {
	fprintf(stderr,"\n%s: ERROR: warning - nested formulas (3) in File: %s at linenumber: %ld\n",progname,latexname,linenumber);
	fprintf(stderr,"\nprogram aborted\n");
	exit(-1);
      }
      MathMode = TRUE;
      break;
    case FORM_ECK_CLOSE:
      fprintf(fRtf,"\n\r\\par ");
      if (MathMode == FALSE)
      {
	fprintf(stderr,"\n%s: ERROR: warning - nested formulas (4) in File: %s at linenumber: %ld\n",progname,latexname,linenumber);
	fprintf(stderr,"\nprogram aborted\n");
	exit(-1);
      }
      MathMode = FALSE;
      bNewPar = TRUE;
      break;
  }
}

/******************************************************************************/
void CmdIgnore(int code)
/******************************************************************************
 purpose: LaTeX-commands which can't be converted in Rtf-commands are overread
	  as such
 ******************************************************************************/
{
}

/******************************************************************************/
void CmdLdots(int code)
/******************************************************************************
 purpose: converts the LaTex-\ldots-command into "..." in Rtf
 ******************************************************************************/
{
  fprintf(fRtf, "...");
}

/******************************************************************************/
void CmdEmphasize(int code)
/******************************************************************************
 purpose: turn on/off the emphasized style for characters
 ******************************************************************************/
{
  if (TABBING_ON == FALSE) /* TABBING-environment ignores emphasized-style */
    {
    static Em_on = 0;
    if (!(Em_on))
      fprintf(fRtf,"{\\i ");
    else
      fprintf(fRtf,"{\\plain ");
    Em_on = ~Em_on;
    Convert();
    fprintf(fRtf,"}");
    Em_on = ~Em_on;
    }
}

/******************************************************************************/
void Format(int code)
/******************************************************************************
 purpose: makes the same as the function CmdEmphasize above
	  but this is an environment-handling-routine in contrast
	  to the function above which converts an ordinary \em-command
 parameter: code: EMPHASIZED with ON at environment start;
				  OFF at environment end
 ******************************************************************************/
{
  if (TABBING_ON == FALSE) /* TABBING-environment ignores emphasized-style */
    {
    static Em_on = 0;
    switch(code)
    {
      case (EMPHASIZE | ON):
	if (!(Em_on))
	  fprintf(fRtf,"{\\i ");
	else
	 fprintf(fRtf,"} ");
	Em_on = ~Em_on;
	break;
      case (EMPHASIZE | OFF):
	if ((Em_on))
	  fprintf(fRtf,"} ");
	Em_on = 0;
	break;
    }
    }
}

/******************************************************************************/
void Environment(int code)
/******************************************************************************
  purpose: pushes/pops the new environment-commands on/from the stack
parameter: code includes the type of the environment
globals  : bIndocument
 ******************************************************************************/
{
  if (code & ON) /* on switch */
  {
    code &= ~(ON);   /* mask MSB */
    if (code == DOCUMENT)
    {
      ClearEnvironment();
      bInDocument = TRUE;
    }
    PushEnvironment(code);
  }
  else /* off switch */
  {
    PopEnvironment();
  }
}

/******************************************************************************/
void CmdTitle(int code)
/******************************************************************************
  purpose: converts the tite, author and date-information from LaTex to Rtf
parameter: code includes which type of the information listed above will be converted
 ******************************************************************************/
{
#define TITLE_END ""
#define AUTHOR_END ""
#define DATE_END ""

  char TITLE_BEGIN[10];
  char AUTHOR_BEGIN[10];
  char DATE_BEGIN[10];
  static char title[1000] = "";
  static char author[1000] = "";
  static char date[500] = "";

  switch (code)
  {
    case TITLE_TITLE: GetParam(title,999);
		      break;
    case TITLE_AUTHOR: TITLE_AUTHOR_ON = TRUE; /* is used for the \and command */
		       GetParam(author,999);
		       TITLE_AUTHOR_ON = FALSE;
		       break;
    case TITLE_DATE: GetParam(date,499);
		     break;
    case TITLE_MAKE:
      sprintf(TITLE_BEGIN,"%s%2d", "\\fs", (30*fontsize)/20);
      sprintf(AUTHOR_BEGIN,"%s%2d", "\\fs", (24*fontsize)/20);
      sprintf(DATE_BEGIN,"%s%2d", "\\fs", (24*fontsize)/20);

      fprintf(fRtf,"\n\r\\par\\pard\\qc {%s ",TITLE_BEGIN);
      ConvertString(title);
      fprintf(fRtf,"}%s",TITLE_END);
      fprintf(fRtf,"\n\r\\par\\qc {%s ",AUTHOR_BEGIN);
      ConvertString(author);
      fprintf(fRtf,"}%s",AUTHOR_END);
      fprintf(fRtf,"\n\r\\par\\qc {%s ",DATE_BEGIN);
      ConvertString(date);
      fprintf(fRtf,"}%s",DATE_END);
      fprintf(fRtf,"\n\r\\par\n\\par\\pard\\q%c ",alignment);
      if (titlepage == TRUE)
	fprintf(fRtf,"\\page ");
      break;
  }
}

/******************************************************************************/
void CmdDocumentStyle(int code)
/******************************************************************************
 purpose: reads the information from the LaTex-documentsstyle-command and
	  converts it to a similar Rtf-information
 ******************************************************************************/
{
  static char style[100] = "";
  char cThis = ' ';
  char optstring[30] = "";
  int optparam = FALSE;

     while ( cThis == ' ')
     {
	if ( (fread(&cThis,1,1,fTex) < 1))
	 numerror(ERR_EOF_INPUT);
     }

     for(;;) /* do forever */
     {
	fseek(fTex,-1L,SEEK_CUR); /* reread last character */

	switch (cThis)
	{
	   case '{' : GetParam(style,99);  /* article, report, bookstyle are the same */
		      if (strcmp(style,"article") == 0)
			  article = TRUE;
		      else
			  article = FALSE;
		      break;
	   case '[' : GetOptParam(style,99);
		      optparam = TRUE;
		      break;
	   default :  /* last character was read again above.
			 this character will be written in the rtf-file in the
			 convert-routine (main.c) */

		      fprintf(fRtf,"\\fs%d ",fontsize); /* default or new fontsize */
		      return;
		      break;
	} /* switch */

	if (optparam)
	{
	    /* returnstring of GetOptParam will be seperated in his components */
	    do
	    {
		 strcpy(optstring,(char*)GetSubString(style,','));

		 if (strcmp(optstring,"11pt") == 0)
		    fontsize = 22; /* fontsize 11-TEX -> 22-RTF */

		 else if (strcmp(optstring,"12pt") == 0)
		    fontsize = 24; /* fontsize 12-TEX -> 24-RTF */

		 else if (strcmp(optstring,"german") == 0)
		    { GermanMode = TRUE; PushEnvironment(GERMANMODE); }

		 else if (strcmp(optstring,"twoside") == 0)
		     /* default */;

		 else if (strcmp(optstring,"twocolumn") == 0)
		    {
		    fprintf(fRtf,"\\cols2\\colsx709 "); /* two columns */
						     /* space between columns 709 */
		    twocolumn = TRUE;
		    }
		else if (strcmp(optstring,"titlepage") == 0)
		    {
		    titlepage = TRUE;
		    }

	    }
	     while (strcmp(optstring,"") != 0);
      } /* if */

      if ( (fread(&cThis,1,1,fTex) < 1)) /* read next character */
	   numerror(ERR_EOF_INPUT);

    } /* for */
}


/******************************************************************************/
void CmdSection(int code)
/******************************************************************************
  purpose: converts the LaTex-section-commands into similar Rtf-styles
parameter: code: type of section-recursion-level
 ******************************************************************************/
{
#define SECTNORM_END "}"
#define SECTSUB_END "}"
#define SECTSUBSUB_END "}"

  char SECTNORM_BEGIN[10];
  char SECTSUB_BEGIN[10];
  char SECTSUBSUB_BEGIN[10];
  char optparam[100] = "";
  char cNext = ' ';

  if ( (fread(&cNext,1,1,fTex) < 1))
	 numerror(ERR_EOF_INPUT);
  fseek(fTex,-1L,SEEK_CUR); /* reread last character */
  if (cNext == '[')
     GetOptParam(optparam,99);
  switch (code)
  {
  case SECT_NORM:
    sprintf(SECTNORM_BEGIN, "%s%2d%s", "{\\fs", (40*fontsize)/20, "");
    fprintf(fRtf,"\n\r\\par %s ",SECTNORM_BEGIN);
    Convert();
    fprintf(fRtf,"%s\n\r\\par ",SECTNORM_END);
    bNewPar = TRUE;
    break;
  case SECT_SUB:
    sprintf(SECTSUB_BEGIN, "%s%2d%s", "{\\fs", (30*fontsize)/20, "");
    fprintf(fRtf,"\n\r\\par %s ",SECTSUB_BEGIN);
    Convert();
    fprintf(fRtf,"%s\n\r\\par ",SECTSUB_END);
    bNewPar = TRUE;
    break;
  case SECT_SUBSUB:
    sprintf(SECTSUBSUB_BEGIN, "%s%2d%s", "{\\fs", (24*fontsize)/20, "");
    fprintf(fRtf,"\n\r\\par %s ",SECTSUBSUB_BEGIN);
    Convert();
    fprintf(fRtf,"%s\n\r\\par ",SECTSUBSUB_END);
    bNewPar = TRUE;
    break;
  }
}


/******************************************************************************/
void CmdFootNote(int code)
/******************************************************************************
 purpose: converts footnotes from LaTex to Rtf
 ******************************************************************************/
{
  char cText[1000] = "";
  char number[255];
  char cNext;

  if ( (fread(&cNext,1,1,fTex) < 1))
	 numerror(ERR_EOF_INPUT);
  fseek(fTex,-1L,SEEK_CUR); /* reread last character */

  if (cNext == '[')
      GetOptParam(number,254); /* is ignored because of the automatic footnumber-generation */

  GetParam(cText,999);
  fprintf(fRtf,"{\\up6\\chftn}\n\r{\\*\\footnote \\pard\\plain\\q%c \\s246 \\fs%d",alignment,fontsize);
  fprintf(fRtf," {\\up6\\chftn } %s }\n\r ",cText);
}

/******************************************************************************/
void CmdQuote(int code)
/******************************************************************************
  purpose: converts the LaTex-Quote-commands into similar Rtf-commands
parameter: code: QUOTE and QUOTATION On/Off
		 specifies the recursion-level of these commands
 globals : NoNewLine: true if no newline should be printed into the Rtf-File
	   indent : includes the left-indent-position
 ******************************************************************************/
{
  switch(code)
  {
    case (QUOTE | ON):
      indent += 512;
      NoNewLine = TRUE;
      fprintf(fRtf,"\n\r\\par\\li%d",indent);
      break;
    case (QUOTE | OFF):
      indent -= 512;
      NoNewLine = FALSE;
      fprintf(fRtf,"\n\r\\par\\li%d",indent);
      break;
    case (QUOTATION | ON):
      indent += 512;
      NoNewLine = TRUE;
      fprintf(fRtf,"\n\r\\par\\li%d",indent);
      break;
    case (QUOTATION | OFF):
      indent -= 512;
      NoNewLine = FALSE;
      fprintf(fRtf,"\n\r\\par\\li%d",indent);
      break;
  }
}


/******************************************************************************/
void CmdList(int code)
/******************************************************************************
  purpose : converts the LaTeX-environments itemize, description and enumerate
	    to similar Rtf-styles
	    (only the begin/end-commands and not the commands inside the environment
	     see also function CmdItem)
parameter : code : type of environment and on/off-state
 globals  : nonewline, indent: look at funtction CmdQuote
 ******************************************************************************/
{
  switch (code)
  {
    case (ITEMIZE | ON):
      PushEnvironment(ITEMIZE);
      indent += 512;
      NoNewLine = TRUE;
      bNewPar = FALSE;
      break;
    case (ITEMIZE | OFF):
      PopEnvironment();
      indent -= 512;
      NoNewLine = FALSE;
      fprintf(fRtf,"\n\r\\par\\fi0\\li%d ",indent);
      bNewPar = TRUE;
      break;
   case (ENUMERATE | ON):
      PushEnvironment(ENUMERATE);
      CmdItem(RESET);
      indent += 512;
      NoNewLine = TRUE;
      bNewPar = FALSE;
      break;
    case (ENUMERATE | OFF):
      PopEnvironment();
      indent -= 512;
      NoNewLine = FALSE;
      fprintf(fRtf,"\n\r\\par\\fi0\\li%d ",indent);
      bNewPar = TRUE;
      break;
    case (DESCRIPTION | ON):
      PushEnvironment(DESCRIPTION);
      indent += 512;
      NoNewLine = TRUE;
      bNewPar = FALSE;
      break;
    case (DESCRIPTION | OFF):
      PopEnvironment();
      indent -= 512;
      NoNewLine = FALSE;
      fprintf(fRtf,"\n\r\\par\\fi0\\li%d ",indent);
      bNewPar = TRUE;
      break;
  }
}

/******************************************************************************/
void CmdItem(int code)
/******************************************************************************
 purpose : makes the same as the function CmdList except that
	   here are only \-commands are handled and in the function
	   CmdList only the \begin or \end{environment}-command is handled
parameter : code : type of environment and on/off-state
 globals  : nonewline, indent: look at funtction CmdQuote
 ******************************************************************************/
{ char labeltext[100];
  char cNext;
  static int number = 0;
  switch (code)
  {
    case RESET:
      number = 1;
      break;
    case ITEMIZE:
       if ( (fread(&cNext,1,1,fTex) < 1))
	      numerror(ERR_EOF_INPUT);
       fseek(fTex,-1L,SEEK_CUR); /* reread last character */
       labeltext[0] = '\0';
       if (cNext == '[')
	   GetOptParam(labeltext,99);
      fprintf(fRtf,"\n\\par\\fi-340\\li%d{\\b ",indent);
      ConvertString(labeltext);
      fprintf(fRtf,"}\\~");
      bNewPar = TRUE;
      break;
    case ENUMERATE:
      fprintf(fRtf,"\n\\par\\fi-340\\li%d %d \\~",indent,number++);
      bNewPar = TRUE;
      break;
    case DESCRIPTION:
       if ( (fread(&cNext,1,1,fTex) < 1))
	      numerror(ERR_EOF_INPUT);
       fseek(fTex,-1L,SEEK_CUR); /* reread last character */
       labeltext[0] = '\0';
       if (cNext == '[')
	   GetOptParam(labeltext,99);
      fprintf(fRtf,"\n\\par\\fi-340\\li%d{\\b ",indent);
      ConvertString(labeltext);
      fprintf(fRtf,"}\\~");
      bNewPar = TRUE;
      break;
  }
}

/******************************************************************************/
void CmdMbox(int code)
/******************************************************************************
  purpose: converts the LaTeX \mbox-command into  an similar Rtf-style
  globals: mbox
 ******************************************************************************/
{
  mbox = TRUE;
  Convert();
  mbox = FALSE;
}

/******************************************************************************/
void ConvertFormula()
/******************************************************************************
 purpose : necessary commands for the formula-environment are pushed onto a stack
 globals : MathMode
 ******************************************************************************/
{
  Push(RecursLevel,BracketLevel);
  ++BracketLevel;    /* Math End char is treated as } math begin as { */
  Push(RecursLevel,BracketLevel);
  MathMode = TRUE;
  Convert();
  MathMode = FALSE;
}


/******************************************************************************/
void CmdSetFont(int code)
/******************************************************************************
  purpose: sets an font for the actual character-style
parameter: code: includes the font-type
 ******************************************************************************/
{
  int num;

  switch(code)
  {
    case F_ROMAN: num = GetTexFontNumber("Roman");
		  break;
    case F_SLANTED: num = GetTexFontNumber("Slanted");
		  break;
    case F_SANSSERIF: num = GetTexFontNumber("Sans Serif");
		  break;
    case F_TYPEWRITER: num = GetTexFontNumber("Typewriter");
		  break;
    default: num = 0;
  }
  fprintf(fRtf,"{\\f%d ",num);
  Convert();
  fprintf(fRtf,"}");
}

/******************************************************************************/
void CmdInclude(int code)
/******************************************************************************
 purpose: reads an extern-LaTex-File from the into the actual document and converts it to
	  an similar Rtf-style
 globals: GermanMode: is set if germanstyles are included
 ******************************************************************************/
{
  char filename[255] = "";
  FILE *fp,*LatexFile;
  /*fpos_t aktpos;*/
  char oldlatexfilename[PATHMAX];
  long oldlinenumber;

/*  fgetpos(fTex,&aktpos);
  if ( (n=(fread(filename,99,1,fTex)) < 1))
    numerror(ERR_EOF_INPUT);
  filename[n+1] = '\0';

  if (strstr(filename,"german.sty")!=NULL)
  {
    GermanMode = TRUE;
    PushEnvironment(GERMANMODE);
    return;
  }
  fsetpos(fTex,&aktpos);*/

  GetInputParam(filename,99);
  if (strstr(filename,"german.sty")!=NULL)
  {
    GermanMode = TRUE;
    PushEnvironment(GERMANMODE);
    return;
  }


  if (strcmp(filename,"") == 0)
     {
     fprintf(stderr,"\n%s: WARNING: invalid filename after \\include: %s\n",progname,filename);
     return;
     }

  /* extension .tex is appended automaticly */
  if(strchr(filename,'.')==NULL)
    strcat(filename,".tex");
  if ( (fp=(fopen(filename,"r")))==NULL )
  {
    fprintf(stderr,"\n%s: WARNING: cannot open include file: %s\n",progname,filename);
    return;
  }

  LatexFile = fTex;
  fTex = fp;
  oldlinenumber = linenumber;
  linenumber = 1;
  strcpy(oldlatexfilename,latexname);
  strcpy(latexname,filename);
  BracketLevel++;
  fprintf(fRtf,"{");
  Convert();
  fprintf(fRtf,"}");
  BracketLevel--;

  fTex = LatexFile;
  strcpy(latexname,oldlatexfilename);
  linenumber = oldlinenumber;
  fclose(fp);
}

/******************************************************************************/
void CmdVerb(int code)
/******************************************************************************
 purpose: converts the LaTex-verb-environment to a similar Rtf-style
 ******************************************************************************/
{
  char cThis;
  char markingchar='|';   /* Verb-Text is between | or " */
  while (fread(&cThis, 1,1,fTex) == 1)
  {
    if ((cThis == '\"') || (cThis == '|'))
    {
      markingchar = cThis;
      break;
    }
  }
  if (cThis != markingchar)
    numerror(ERR_EOF_INPUT);
  while (fread(&cThis, 1,1,fTex) == 1)
  {
    if (cThis == markingchar)
      break;
    else
    {   /* print character */
      if (cThis == '\\')    /* care for \\ */
	fprintf(fRtf,"\\\\");
      else
      {
	if (code == AST_FORM)
	{
	  if (cThis == ' ')   /* print dot instead of space */
	    fprintf(fRtf,"{\\ansi\\'b7\\pc}");
	  else
	    fprintf(fRtf,"%c",cThis);
	}
	else
	  fprintf(fRtf,"%c",cThis);
      }
    }
  }
  if (cThis != markingchar)
    numerror(ERR_EOF_INPUT);
}

/******************************************************************************/
void CmdVerbatim(int code)  /* write anything till \end{verbatim} */
/******************************************************************************
 purpose: in this verb-environment each character is converted 1:1 from LaTex
	  to Rtf without converting any LaTex-commands
 ******************************************************************************/
{
  char endstring[]="\\end{verbatim}";
  int i=0,j=0;
  char cThis;
  for(;;)
  {
    if (fread(&cThis, 1,1,fTex) != 1)
      numerror(ERR_EOF_INPUT);
    if ( (cThis != endstring[i]) || ( (i>0) && (cThis == ' ') ) )
    {
      if (i > 0)
      {
	for(j=0;j<i;j++)
	{
	  if (j==0)
	    fprintf(fRtf,"\\\\");
	  else
	    fprintf(fRtf,"%c",endstring[j]);
	}
	i = 0;
      }
      if (cThis == '\\')    /* care for \\ */
	fprintf(fRtf,"\\\\");
      else
      {
	if (cThis == '\n')
	  {
	  linenumber++;
	  fprintf(fRtf,"\\par ");
	  }
	else
	  fprintf(fRtf,"%c",cThis);
      }
    }
    else
    {
      if (cThis != ' ')
	++i;
      if ((int)i >=(int)strlen(endstring))
	return;
    }
  } /* for */
}

/******************************************************************************/
void CmdVerse(int code)
/******************************************************************************
  purpose: converts the LaTex-Verse-environment to a similar Rtf-style
parameter: code: turns on/off handling routine
 ******************************************************************************/
{
  switch (code)
  {
    case ON :
	      fprintf(fRtf,"\n\\par\\pard\\q%c\\fi-567\\li1134\\ri1134\\keep ",alignment);
	      NoNewLine = FALSE;
	      bNewPar = TRUE;
	      break;
    case OFF: fprintf(fRtf,"\n\\par\\pard\\q%c ",alignment);
	      bNewPar = TRUE;
	      break;
  }
}


/******************************************************************************/
void CmdIgnoreDef(int code)
/*****************************************************************************
 purpose: newenvironments or newcommands which are defined by the user can't
	  be converted into Rtf and so they've to be ignored
 ******************************************************************************/
{
  char cThis;
  int bracket;
  /* ignore till '{'  */
  while (fread(&cThis, 1,1,fTex) == 1)
  {
    if (cThis == '{')
      break;
  }
  if (cThis != '{')
    numerror(ERR_EOF_INPUT);
  bracket = 1;
  while (fread(&cThis, 1,1,fTex) == 1)
  {
    if (cThis == '{')
      bracket++;
    if (cThis == '}')
      bracket--;
    if (cThis == '\n')
	linenumber++;
    if (bracket == 0)
      return;
   /* if (cThis == '%')
    {  in file latex.tex '%' in def means `%` no comment
      IgnoreTo('\n');
    }*/
  }
  if (cThis != '}')
    numerror(ERR_EOF_INPUT);
}

/******************************************************************************/
void TranslateGerman(void)
/***************************************************************************
purpose: called on active german-mode and " character in input file to
	 handle " as an active (meta-)character.
globals: reads from fTex and writes to fRtf
 ***************************************************************************/
{
  char cThis;
  while (fread(&cThis, 1,1,fTex) != 1)
  {
    numerror(ERR_EOF_INPUT);
  }
  switch(cThis)
  {
    case 'a': fprintf(fRtf, "\\ansi\\'e4\\pc ");
	      break;
    case 'o': fprintf(fRtf, "\\ansi\\'f6\\pc ");
	      break;
    case 'u': fprintf(fRtf, "\\ansi\\'fc\\pc ");
	      break;
    case 's': fprintf(fRtf, "\\ansi\\'df\\pc ");
	      break;
    case '|': break;  /* ignore */
    case '-': break;  /* ignore */
    case '"': break;  /* ignore */
    case '\'':fprintf(fRtf, "\\ldblquote ");
	      break;
    case '`': fprintf(fRtf, "\\rdblquote ");
	      break;
    case '<': break;
    case '>': break;
    default:  fprintf(fRtf,"%c",cThis);
  }
}

/******************************************************************************/
void CmdPrintRtf(int code)
/***************************************************************************
purpose: writes string to RTF file
globals: writes to fRtf
 ***************************************************************************/
{
  fprintf(fRtf,(char*)code);
}

void GermanPrint(int code)
{
  switch(code)
  {
    case GP_CK:fprintf(fRtf,"ck");
		break;
    case GP_LDBL: fprintf(fRtf,"\\ldblquote");
		  break;
    case GP_L: fprintf(fRtf,"\\lquote");
	       break;
    case GP_R: fprintf(fRtf,"\\rquote");
	       break;
    case GP_RDBL: fprintf(fRtf,"\\rdblquote");
  }
}


void CmdIgnoreLet(int code)
{
  char cThis;
  int count=0;
  /* Format: \let\XXXXX = \YYYYYY or \let\XXXXX\YYYYYY
  /* ignore till 2x '\' */

  while (fread(&cThis, 1,1,fTex) == 1)
  {
    if (cThis == '\\')
      count++;
    if (count == 2)
	break;
    if (cThis == '\n')
      linenumber++;
  }
  if (cThis != '\\')
    numerror(ERR_EOF_INPUT);
  /* ignore all following spaces */
 while (fread(&cThis, 1,1,fTex) == 1)
  {
    if (cThis != ' ')
      break;
  }
  if (cThis == ' ')
    numerror(ERR_EOF_INPUT);
  /* ignore till next space */
  while (fread(&cThis, 1,1,fTex) == 1)
  {
    if (cThis == ' ')
      break;
    if (cThis == '\n')
    {
      linenumber++;
      break;
    }
  }
  if ((cThis != ' ') && (cThis != '\n'))
    numerror(ERR_EOF_INPUT);
  /* seek back 1 */
  fseek(fTex,-1L,SEEK_CUR); /* reread last character */
}


void IgnoreNewCmd(int code)
{
  char cThis;
  /* ignore till '{' */
  if (fread(&cThis, 1,1,fTex) != 1)
    numerror(ERR_EOF_INPUT);
  fseek(fTex,-1L,SEEK_CUR); /* reread last character */
  if (cThis == '\\')
    CmdIgnoreDef(0);
  else
    CmdIgnoreParameter(No_Opt_Two_NormParam );
}

#define LABEL 1
#define REF 2
#define PAGEREF 3
void CmdLabel(int code)
{
  char text[500];
  char cThis;
  switch (code)
  {
   case LABEL: GetParam(text,499);
	       fprintf(fRtf,"{\\v[LABEL: %s]}",text);
	       break;
   case REF:   GetParam(text,499);
	       fprintf(fRtf,"{\\v[REF: %s]}",text);
	       break;
   case PAGEREF:GetParam(text,499);
		fprintf(fRtf,"{\\v[PAGEREF: %s]}",text);
		break;
  }
  if ( (fread(&cThis,1,1,fTex) < 1))
    numerror(ERR_EOF_INPUT);
  while (cThis == ' ')
  {
    if ( (fread(&cThis,1,1,fTex) < 1))
      numerror(ERR_EOF_INPUT);
  }
  if (cThis != '\n')
    fseek(fTex,-1L,SEEK_CUR);
  else
    ++linenumber;
}

void ConvertString(char *string)
{
  char *tmpname;
  FILE *fp, *LatexFile;
  long oldlinenumber;

  tmpname = tempnam(getenv("TMPDIR"), "l2r");
  if ((fp = fopen(tmpname,"w+")) == NULL)
  {
    fprintf(stderr,"%s: Fatal Error: cannot create temporary file\n", progname);
    exit(1);
  }
  fwrite(string,strlen(string),1,fp);
  fseek(fp,0L,SEEK_SET);

  LatexFile = fTex;
  fTex = fp;
  oldlinenumber = linenumber;
  linenumber = 1;
  BracketLevel++;
  Convert();
  BracketLevel--;

  fTex = LatexFile;
  linenumber = oldlinenumber;
  fclose(fp);
  remove(tmpname);
  free(tmpname);
}
