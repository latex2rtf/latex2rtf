/*
 * $Id: funct1.c,v 1.3 2001/08/12 15:56:56 prahl Exp $
 * History:
 * $Log: funct1.c,v $
 * Revision 1.3  2001/08/12 15:56:56  prahl
 * latex2rtf version 1.5 by Ralf Schlatterbeck
 *
 * Revision 1.5  1995/05/24  17:06:47  ralf
 * Removed bug with generation of include filenames
 *
 * Revision 1.4  1995/05/24  12:06:22  ralf
 * Changed two wrong checks after malloc
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
 */
/***************************************************************************
   name : funct1.c
 author : DORNER Fernando, GRANZER Andreas
          POLZER Friedrich,TRISKO Gerhard
 * in footnote: special characters treated correctly
 * now produces section-numbers
 * GetParam allocates only the needed amount of memory
 * \c
 * footnotes treats \"o etc correctly
 * paragraph formatting properties of itemize/liste environment corrected
 *
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
extern BOOL bBlankLine;
extern BOOL TABBING_ON;
extern BOOL TITLE_AUTHOR_ON;
extern char *progname;
extern char *latexname;
extern char alignment;
extern BOOL GermanMode;
extern BOOL mbox;
extern long linenumber;
extern int DefFont;
/***************************************************************************/

/***************************  prototypes     ********************************/
void ConvertFormula();
/****************************************************************************/

/****************************************************************************/
void CmdCharFormat(int code)
/****************************************************************************
     purpose : sets the characterformat to bold, italic, underlined...
   parameter : code includes the character-format-style
     globals : fRtf: Rtf-File-Pointer
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
/* ------------------------------------------ */
       case CMD_CENTERED: fprintf(fRtf,"{\\qc ");
                          break;
/* ------------------------------------------ */
     }
     Convert();
     fprintf(fRtf,"}");
   }
}

/******************       helping function for CmdBegin               ***/
/**************************************************************************/
void GetParam(char **string)
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

  oldpos = ftell (fTex);
  /* read Param to get needed length, allocate it and position
     the file-pointer back to the beginning of the param in the TeX-file */
  if ( (fread(&cThis,1,1,fTex) < 1))
    numerror(ERR_EOF_INPUT);
  if ( cThis != '{' )
  {
    (*string) = malloc (2*sizeof(char));
    if (*string == NULL)
       error(" malloc error -> out of memory!\n");
    (*string)[0] = cThis;
    (*string)[1] = '\0';
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
    size++;
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

    if (cThis == '%')
	{
	IgnoreTo('\n');
	i--;
	continue;
	}

  } /* for */
  (*string) = malloc ( (size+1) * sizeof(char) );
  if (*string == NULL)
     error(" malloc error -> out of memory!\n");

  fseek(fTex, oldpos, SEEK_SET);





  if ( (fTexRead(&cThis,1,1,fTex) < 1))
    numerror(ERR_EOF_INPUT);
  else
  {
    bracket++;
    ++BracketLevel;
    Push(RecursLevel,BracketLevel);
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
        Pop(&PopLevel,&PopBrack);
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

    (*string)[i] = cThis;
  }

  (*string)[i] = '\0';
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
  char *cParam;
  switch(code)
  {
    case CMD_BEGIN:
		    GetParam(&cParam);
		    CallParamFunc(cParam,ON);
		    break;
    case CMD_END:
		    GetParam(&cParam);
		    CallParamFunc(cParam,OFF);
		    break;
  }
  free (cParam);
}

/********************************************************************************/
void Paragraph(int code)
/*****************************************************************************
    purpose : sets the alignment for a paragraph
  parameter : code: alignment centered, justified, left or right
     globals: fRtf: Rtf-file-pointer
              alignment: alignment of paragraphs
              bNewPar
 ********************************************************************************/
{ static char old_alignment_before_center = JUSTIFIED;
  static char old_alignment_before_right = JUSTIFIED;
  static char old_alignment_before_left = JUSTIFIED;
  static char old_alignment_before_centerline = JUSTIFIED;

  char *centertext;

  switch(code)
  {
    case (PAR_CENTERLINE):
      old_alignment_before_centerline = alignment;
      alignment = CENTERED;
      fprintf(fRtf,"\\par \\pard\\q%c{",alignment);
      Convert();
      alignment = old_alignment_before_centerline;
      fprintf(fRtf,"}\\par \\pard\\q%c\n",alignment);
      bNewPar = TRUE;
      break;

    case (PAR_CENTER | ON):
      old_alignment_before_center = alignment;
      alignment = CENTERED;
      fprintf(fRtf,"\\pard \\q%c ",alignment);
      break;
    case (PAR_CENTER | OFF):
      alignment = old_alignment_before_center;
      fprintf(fRtf,"\\par \\pard \\q%c\n",alignment);
      bNewPar = TRUE;
      break;

    case (PAR_RIGHT | ON):
      old_alignment_before_right = alignment;
      alignment = RIGHT;
      fprintf(fRtf,"\\pard \\q%c ",alignment);
      break;
    case (PAR_RIGHT | OFF):
      alignment = old_alignment_before_right;
      fprintf(fRtf,"\\par \\pard\\q%c\n",alignment);
      bNewPar = TRUE;
      break;

    case (PAR_LEFT | ON):
      old_alignment_before_left = alignment;
      alignment = LEFT;
      fprintf(fRtf,"\\pard\\q%c\n",alignment);
      break;
    case (PAR_LEFT | OFF):
      alignment = old_alignment_before_left;
      fprintf(fRtf,"\\par \\pard\\q%c\n",alignment);
      bNewPar = TRUE;
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
void CmdC(int code)
/******************************************************************************
 purpose : converts /c
  params : not used
 globals : fRtf
 ******************************************************************************/
{
char *cParam;

  GetParam(&cParam);

  switch(cParam[0])
  {
  case 'c': fprintf(fRtf,"\\ansi\\'e7");
            break;
  }
  free (cParam);
}

/******************************************************************************/
void CmdUmlaute(int code)
/******************************************************************************
 purpose : converts german symbols from LaTeX to Rtf
 globals : fRtf
 ******************************************************************************/
{
  static char cHexDigits[16] = {'0', '1', '2' ,'3', '4', '5', '6', '7', '8',
				'9', 'a', 'b', 'c', 'd', 'e', 'f' };
  char *cParam;

  GetParam(&cParam);

  switch(cParam[0])
  {
    case 'o': fprintf(fRtf,"\\'%c%c",cHexDigits[('”'>>4)&0x0f],
		cHexDigits[('”'&0x0f)]);
	      break;
    case 'O': fprintf(fRtf,"\\'%c%c",cHexDigits[('™'>>4)&0x0f],
		cHexDigits[('™'&0x0f)]);
	      break;
    case 'a': fprintf(fRtf,"\\'%c%c",cHexDigits[('„'>>4)&0x0f],
		cHexDigits[('„'&0x0f)]);
	      break;
    case 'A': fprintf(fRtf,"\\'%c%c",cHexDigits[('Ž'>>4)&0x0f],
		cHexDigits[('Ž'&0x0f)]);
	      break;
    case 'u': fprintf(fRtf,"\\'%c%c",cHexDigits[(''>>4)&0x0f],
		cHexDigits[(''&0x0f)]);
	      break;
    case 'U': fprintf(fRtf,"\\'%c%c",cHexDigits[('š'>>4)&0x0f],
		cHexDigits[('š'&0x0f)]);
	      break;
    case 'E':fprintf(fRtf, "\\ansi\\'cb\\pc");
	     break;
    case 'I':fprintf(fRtf, "\\ansi\\'cf\\pc");
	     break;
    case 'e':fprintf(fRtf, "\\ansi\\'eb\\pc");
	     break;
    case 'i':fprintf(fRtf, "\\ansi\\'ef\\pc");
	     break;
    case 'y':fprintf(fRtf, "\\ansi\\'ff\\pc");
	     break;
  }
  free (cParam);
}

/******************************************************************************/
void CmdLApostrophChar(int code)
/******************************************************************************
 purpose: converts special symbols from LaTex to Rtf
 globals : fRtf
 ******************************************************************************/
{
  char *cParam;

  GetParam(&cParam);
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
  free (cParam);
}


/******************************************************************************/
void CmdRApostrophChar(int code)
/******************************************************************************
 purpose: converts special symbols from LaTex to Rtf
 globals : fRtf
 ******************************************************************************/
{
  char *cParam;

  GetParam(&cParam);
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
  free (cParam);
}

/******************************************************************************/
void CmdSpitzeChar(int code)
/******************************************************************************
 purpose: converts special symbols from LaTex to Rtf
 globals : fRtf
 ******************************************************************************/
{
  char *cParam;

  GetParam(&cParam);
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
  free (cParam);
}

/******************************************************************************/
void CmdTildeChar(int code)
/******************************************************************************
 purpose: converts special symbols from LaTex to Rtf
 globals : fRtf
 ******************************************************************************/
{
  char *cParam;

  GetParam(&cParam);
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
  free (cParam);
}

/******************************************************************************/
void CmdFontSize(int code)
/******************************************************************************
 purpose : sets the fontsize to the point-size given by the LaTex-\fs_size-command
 globals : fontsize : includes the actual fontsize in the document
           fRtf 
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
 globals : fRtf
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
 globals : fRtf
           MathMode
           progname; latexname;
           linenumber
           bNewPar
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
	fprintf(stderr,"\n%s: ERROR: warning - nested formulas (1) in File: %s at linenumber: %ld\n",progname,latexname,getLinenumber());
	fprintf(stderr,"\nprogram aborted\n");
	exit(-1);
      }
      MathMode = TRUE;
      break;
    case FORM_RND_CLOSE:
      fprintf(fRtf," ");
      if (MathMode == FALSE)
      {
	fprintf(stderr,"\n%s: ERROR: warning - nested formulas (2) in File: %s at linenumber: %ld\n",progname,latexname,getLinenumber());
	fprintf(stderr,"\nprogram aborted\n");
	exit(-1);
      }
      MathMode = FALSE;
      break;
    case FORM_ECK_OPEN:
      fprintf(fRtf,"\n\r\\par ");
      if (MathMode == TRUE)
      {
	fprintf(stderr,"\n%s: ERROR: warning - nested formulas (3) in File: %s at linenumber: %ld\n",progname,latexname,getLinenumber());
	fprintf(stderr,"\nprogram aborted\n");
	exit(-1);
      }
      MathMode = TRUE;
      break;
    case FORM_ECK_CLOSE:
      fprintf(fRtf,"\n\r\\par ");
      if (MathMode == FALSE)
      {
	fprintf(stderr,"\n%s: ERROR: warning - nested formulas (4) in File: %s at linenumber: %ld\n",progname,latexname,getLinenumber());
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
 globals : fRtf
 ******************************************************************************/
{
  fprintf(fRtf, "...");
}

/******************************************************************************/
void CmdEmphasize(int code)
/******************************************************************************
 purpose: turn on/off the emphasized style for characters
 globals : fRtf
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
 globals : fRtf
           TABBING_ON
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
 globals : fRtf
           TITLE_AUTHOR_ON
           fontsize
           alignment
           titlepage: if true a page-break is inserted
 ******************************************************************************/
{
#define TITLE_END ""
#define AUTHOR_END ""
#define DATE_END ""

  char TITLE_BEGIN[10];
  char AUTHOR_BEGIN[10];
  char DATE_BEGIN[10];

  static char *title = "";
  static char *author = "";
  static char *date = "";

  
  switch (code)
  {
    case TITLE_TITLE: GetParam(&title);
		      break;
    case TITLE_AUTHOR: TITLE_AUTHOR_ON = TRUE; /* is used for the \and command */
		       GetParam(&author);
		       TITLE_AUTHOR_ON = FALSE;
		       break;
    case TITLE_DATE: GetParam(&date);
		     break;
    case TITLE_MAKE:
      sprintf(TITLE_BEGIN,"%s%2d", "\\fs", (30*fontsize)/20);
      sprintf(AUTHOR_BEGIN,"%s%2d", "\\fs", (24*fontsize)/20);
      sprintf(DATE_BEGIN,"%s%2d", "\\fs", (24*fontsize)/20);

      fprintf(fRtf,"\n\\par\\pard\\qc {%s ",TITLE_BEGIN);
      ConvertString(title);
      fprintf(fRtf,"}%s",TITLE_END);
      fprintf(fRtf,"\n\\par\\qc {%s ",AUTHOR_BEGIN);
      ConvertString(author);
      fprintf(fRtf,"}%s",AUTHOR_END);
      fprintf(fRtf,"\n\\par\\qc {%s ",DATE_BEGIN);
      ConvertString(date);
      fprintf(fRtf,"}%s",DATE_END);
      fprintf(fRtf,"\n\\par\n\\par\\pard\\q%c ",alignment);
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
 globals : fRtf
           fontsize
           article: (article, report, bookstyle)
           GermanMode: support for germanstyle
           twocolumn; titlepage
 ******************************************************************************/
{
  static char *style = "";
  char cThis = ' ';
  char optstring[30] = "";
  int optparam = FALSE;


     for(;;) /* do forever */
     {
        while ( cThis == ' ')
        {
   	   if ( (fread(&cThis,1,1,fTex) < 1))
	      numerror(ERR_EOF_INPUT);
        }
	fseek(fTex,-1L,SEEK_CUR); /* reread last character */

	switch (cThis)
	{
	   case '{' : GetParam(&style);  
		      if (strcmp(style,"article") == 0)
                      {
			  article = TRUE;
                      }
		      else
                      {
			  article = FALSE;
                      }
		      break;
	   case '[' : style = (char*) malloc (100*sizeof(char));
                      if (style == NULL)
                         error(" malloc error -> out of memory!\n");
                      GetOptParam(style,99);
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

      if ( (fread(&cThis,1,1,fTex) < 1)) 
	   numerror(ERR_EOF_INPUT); 

    } /* for */
}


/******************************************************************************/
void CmdSection(int code)
/******************************************************************************
  purpose: converts the LaTex-section-commands into similar Rtf-styles
parameter: code: type of section-recursion-level
 globals : fRtf
           DefFont: default-font-number
           bNewPar
           bBlankLine
 ******************************************************************************/
{
  static int part = 0;
  static int chapter = 0;
  static int section = 0;
  static int subsection = 0;
  static int subsubsection = 0;
  static int caption = 0;
  char optparam[100] = "";
  char cNext = ' ';

  if ( (fread(&cNext,1,1,fTex) < 1))
	 numerror(ERR_EOF_INPUT);
  fseek(fTex,-1L,SEEK_CUR); /* reread last character */
  if (cNext == '[')
     GetOptParam(optparam,99);
  switch (code)
  {
  case SECT_PART:
    part++;
    fprintf(fRtf, "\\par\\pard\\page");
    fprintf(fRtf, "{\\qc\\b\\fs40 ");
    if (GermanMode)
       fprintf(fRtf, "Teil %d\\par ",part);
    else
       fprintf(fRtf, "Part %d\\par ",part);
    Convert();
    fprintf(fRtf, "\\par}\\page\\par \\pard\\q%c",alignment);
    break;

  case SECT_CHAPTER:
    chapter++;
    section = 0;
    subsection = 0;
    fprintf (fRtf, "\\par \\pard\\page\\pard{\\pntext\\pard\\plain\\b\\fs32\\kerning28 ");
    fprintf (fRtf, "%d\\par \\par }\\pard\\plain\n", chapter);
    fprintf (fRtf, "%s%d%s{", HEADER11,DefFont,HEADER12);
    Convert();
    fprintf(fRtf,"}\n\\par \\pard\\plain\\f%d\\q%c\n", DefFont,alignment);
    do 
    {
       if ( (fread(&cNext,1,1,fTex) < 1))
          numerror(ERR_EOF_INPUT);
    }
    while ((cNext == ' ') || (cNext == '\n'));
    fseek(fTex,-1L,SEEK_CUR); /* reread last character */
    bNewPar = FALSE;
    bBlankLine = TRUE;
    break;

  case SECT_NORM:
    if (article)
    {
       section++;
       subsection = 0;
       subsubsection = 0;
       fprintf (fRtf, "{\\par \\pard\\pntext\\pard\\plain\\b\\fs32\\kerning28 %d\\tab}\\pard\\plain\n", section);
       fprintf (fRtf, "%s%d%s{", HEADER11,DefFont,HEADER12);
       Convert();
       fprintf(fRtf,"}\n\\par \\pard\\plain\\f%d\\q%c\n", DefFont,alignment);
       do 
       {
          if ( (fread(&cNext,1,1,fTex) < 1))
             numerror(ERR_EOF_INPUT);
       }
       while ((cNext == ' ') || (cNext == '\n'));
       fseek(fTex,-1L,SEEK_CUR); /* reread last character */
       bNewPar = FALSE;
       bBlankLine = TRUE;
    }
    else
    {
       section++;
       subsection = 0;
       fprintf (fRtf, "{\\par \\pard\\pntext\\pard\\plain\\b\\fs24 %d.%d\\tab}\\pard\\plain\n", chapter, section);
       fprintf (fRtf, "%s%d%s{", HEADER21,DefFont,HEADER22);
       Convert();
       fprintf(fRtf,"}\n\\par \\pard\\plain\\f%d\\q%c\n", DefFont,alignment);
       do 
       {
          if ( (fread(&cNext,1,1,fTex) < 1))
             numerror(ERR_EOF_INPUT);
       }
       while ((cNext == ' ') || (cNext == '\n'));
       fseek(fTex,-1L,SEEK_CUR); /* reread last character */
       bNewPar = FALSE;
       bBlankLine = TRUE;
    }
    break;

  case SECT_SUB:
    if (article)
    {
       subsection++;
       subsubsection = 0;
       fprintf (fRtf, "{\\par \\pard\\pntext\\pard\\plain\\b\\fs24 %d.%d\\tab}\\pard\\plain\n", section, subsection);
       fprintf (fRtf, "%s%d%s{", HEADER21,DefFont,HEADER22);
       Convert();
       fprintf(fRtf,"}\n\\par \\pard\\plain\\f%d\\q%c\n", DefFont,alignment);
       do 
       {
          if ( (fread(&cNext,1,1,fTex) < 1))
             numerror(ERR_EOF_INPUT);
       }
       while ((cNext == ' ') || (cNext == '\n'));
       fseek(fTex,-1L,SEEK_CUR); /* reread last character */
       bNewPar = FALSE;
       bBlankLine = TRUE;
    }
    else
    {
       subsection++;
       fprintf (fRtf, "{\\par \\pard\\pntext\\pard\\plain\\b\\fs24 %d.%d.%d\\tab}\\pard\\plain\n", chapter, section, subsection);
       fprintf (fRtf, "%s%d%s{", HEADER31,DefFont,HEADER32);
       Convert();
       fprintf(fRtf,"}\n\\par \\pard\\plain\\f%d\\q%c\n", DefFont,alignment);
       do 
       {
          if ( (fread(&cNext,1,1,fTex) < 1))
             numerror(ERR_EOF_INPUT);
       }
       while ((cNext == ' ') || (cNext == '\n'));
       fseek(fTex,-1L,SEEK_CUR); /* reread last character */
       bNewPar = FALSE;
       bBlankLine = TRUE;
    }
    break;

  case SECT_SUBSUB:
    if (article)
    {
       subsubsection++;
       fprintf (fRtf, "{\\par \\pard\\pntext\\pard\\plain\\b\\fs24 %d.%d.%d\\tab}\\pard\\plain\n", section, subsection, subsubsection);
       fprintf (fRtf, "%s%d%s{", HEADER31,DefFont,HEADER32);
       Convert();
       fprintf(fRtf,"}\n\\par \\pard\\plain\\f%d\\q%c\n", DefFont,alignment);
       do 
       {
          if ( (fread(&cNext,1,1,fTex) < 1))
             numerror(ERR_EOF_INPUT);
       }
       while ((cNext == ' ') || (cNext == '\n'));
       fseek(fTex,-1L,SEEK_CUR); /* reread last character */
       bNewPar = FALSE;
       bBlankLine = TRUE;
    }
    else
    {
       subsubsection++;
       fprintf (fRtf, "{\\par \\pard\\pntext\\pard\\plain\\b\\fs24 %d.%d.%d.%d\\tab}\\pard\\plain\n", chapter, section, subsection, subsubsection);
       fprintf (fRtf, "%s%d%s{", HEADER41,DefFont,HEADER42);
       Convert();
       fprintf(fRtf,"}\n\\par \\pard\\plain\\f%d\\q%c\n", DefFont,alignment);
       do 
       {
          if ( (fread(&cNext,1,1,fTex) < 1))
             numerror(ERR_EOF_INPUT);
       }
       while ((cNext == ' ') || (cNext == '\n'));
       fseek(fTex,-1L,SEEK_CUR); /* reread last character */
       bNewPar = FALSE;
       bBlankLine = TRUE;
    }
    break;

  case SECT_CAPTION:
    caption++;
    if (GermanMode)
       fprintf(fRtf,"\n\\par \\pard\\qc {Tabelle %d: ", caption);
    else
       fprintf(fRtf,"\n\\par \\pard\\qc {Table %d: ", caption);
    Convert();
    fprintf(fRtf,"} \\par \\pard\\q%c\n",alignment);
    break;
  }
}


/******************************************************************************/
void CmdFootNote(int code)
/******************************************************************************
 purpose: converts footnotes from LaTex to Rtf
 params : code specifies whether it is a footnote or a thanks-mark
 globals: fTex, fRtf
          fontsize 
          DefFont
 ******************************************************************************/
{
  char number[255];
  char cNext;
  static int thankno = 1;
  

  if ( (fread(&cNext,1,1,fTex) < 1))
	 numerror(ERR_EOF_INPUT);
  fseek(fTex,-1L,SEEK_CUR); /* reread last character */

  if (cNext == '[')
      GetOptParam(number,254); /* is ignored because of the automatic footnumber-generation */

  if (code == THANKS)
  {
     fprintf(fRtf,"{\\up6 %d}\n{\\*\\footnote \\pard\\plain\\q%c \\s246 \\f%d \\fs%d",thankno,alignment,DefFont,fontsize);
     fprintf(fRtf," {\\up6 %d}",thankno++);
     Convert();
     fprintf(fRtf,"}\n ");
  }
  else
  {
     fprintf(fRtf,"{\\up6\\chftn}\n{\\*\\footnote \\pard\\plain\\q%c \\s246 \\f%d \\fs%d",alignment,DefFont,fontsize);
     fprintf(fRtf," {\\up6\\chftn }");
     Convert();
     fprintf(fRtf,"}\n ");
  }
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
      NoNewLine = FALSE;
      bNewPar = FALSE;
      break;
    case (ITEMIZE | OFF):
      PopEnvironment();
      indent -= 512;
      NoNewLine = FALSE;
      fprintf(fRtf,"\n\\par\\fi0\\li%d ",indent);
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
      fprintf(fRtf,"\n\\par\\fi0\\li%d ",indent);
      bNewPar = TRUE;
      break;
    case (DESCRIPTION | ON):
      PushEnvironment(DESCRIPTION);
      indent += 512;
      NoNewLine = FALSE;
      bNewPar = FALSE;
      break;
    case (DESCRIPTION | OFF):
      PopEnvironment();
      indent -= 512;
      NoNewLine = FALSE;
      fprintf(fRtf,"\n\\par\\fi0\\li%d ",indent);
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
            bNewPar: 
            fRtf: Rtf-file-pointer
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
       fprintf(fRtf,"\n\\par ");
       if (cNext == '[')
       {
	   GetOptParam(labeltext,99);
           fprintf(fRtf,"{\\b %s} ",labeltext);
       }
       fprintf(fRtf,"\\fi-512\\li%d \\bullet  ",indent);
       Convert();
       bNewPar = FALSE;
       break;
    case ENUMERATE:
       fprintf(fRtf,"\n\\par\\fi-512\\li%d %d. \\~",indent,number++);
       bNewPar = FALSE;
       break;
    case DESCRIPTION:
       if ( (fread(&cNext,1,1,fTex) < 1))
	      numerror(ERR_EOF_INPUT);
       fseek(fTex,-1L,SEEK_CUR); /* reread last character */
       labeltext[0] = '\0';
       fprintf(fRtf,"\n\\par ");
       if (cNext == '[')
       {
	   GetOptParam(labeltext,99);
           fprintf(fRtf,"{\\b %s} ",labeltext);
       }
       fprintf(fRtf,"\\fi-512\\li%d ",indent);
       Convert();
       bNewPar = FALSE;
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
  globals: fRtf
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
          fTex
          progname
 ******************************************************************************/
{
  char filename[255] = "";
  FILE *fp,*LatexFile;
  /*fpos_t aktpos;*/
  char *oldlatexfilename;
  long oldlinenumber;


# ifdef no_longer_needed

  fgetpos(fTex,&aktpos);
  if ( (n=(fread(filename,99,1,fTex)) < 1))
    numerror(ERR_EOF_INPUT);
  filename[n+1] = '\0';

  if (strstr(filename,"german.sty")!=NULL)
  {
    GermanMode = TRUE;
    PushEnvironment(GERMANMODE);
    return;
  }
  fsetpos(fTex,&aktpos);

# endif /* no_longer_needed */

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
  oldlatexfilename = latexname;
  latexname = filename;
  BracketLevel++;
  fprintf(fRtf,"{");
  Convert();
  fprintf(fRtf,"}");
  BracketLevel--;

  fTex = LatexFile;
  latexname = oldlatexfilename;
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
  char markingchar;
  while (fTexRead(&cThis, 1,1,fTex) == 1)
  {
    if ((cThis != ' ') && (cThis != '*') && !isalpha(cThis))
    {
      markingchar = cThis;
      break;
    }
  }
  if (cThis != markingchar)
    numerror(ERR_EOF_INPUT);
  while (fTexRead(&cThis, 1,1,fTex) == 1)
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
  globals: fRtf: Rtf-file-pointer
           NoNewLine
           bNewPar
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
 globals: linenumber
          fTex
 ******************************************************************************/
{
  char cThis;
  int bracket;
  /* ignore till '{'  */
  while (fTexRead(&cThis, 1,1,fTex) == 1)
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
  if (fTexRead(&cThis, 1,1,fTex) != 1)
    numerror(ERR_EOF_INPUT);

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


/******************************************************************************/
void CmdIgnoreLet(int code)
/******************************************************************************
     purpose : ignore \let
   parameter : code  not used 
     globals : linenumber
               fTex
 ******************************************************************************/
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
 while (fTexRead(&cThis, 1,1,fTex) == 1)
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
  }
  if ((cThis != ' ') && (cThis != '\n'))
    numerror(ERR_EOF_INPUT);
  /* seek back 1 */
  fseek(fTex,-1L,SEEK_CUR); /* reread last character */
}







/******************************************************************************/
void IgnoreNewCmd(int code)
/******************************************************************************
     purpose : ignore \newcmd
   parameter : code  not used 
     globals : fTex
 ******************************************************************************/
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
/******************************************************************************/
void CmdLabel(int code)
/******************************************************************************
     purpose : label
   parameter : code  kind of label
     globals : fRtf
               linenumber
 ******************************************************************************/
{
  char *text;
  char cThis;
  switch (code)
  {
   case LABEL: GetParam(&text);
	       fprintf(fRtf,"{\\v[LABEL: %s]}",text);
	       break;
   case REF:   GetParam(&text);
	       fprintf(fRtf,"{\\v[REF: %s]}",text);
	       break;
   case PAGEREF:GetParam(&text);
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
  free (text);
}




/******************************************************************************/
void ConvertString(char *string)
/******************************************************************************
     purpose : converts string in TeX-format to Rtf-format
   parameter : string   will be converted, result is written to Rtf-file
     globals : linenumber
               fTex
 ******************************************************************************/
{
  char *tmpname;
  FILE *fp, *LatexFile;
  long oldlinenumber;
  BOOL bOldInDocument;

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



