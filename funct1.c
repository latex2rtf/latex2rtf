/*
 * $Id: funct1.c,v 1.6 2001/08/12 18:25:13 prahl Exp $
 * History:
 * $Log: funct1.c,v $
 * Revision 1.6  2001/08/12 18:25:13  prahl
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
 * Revision 1.9  1998/10/28 06:19:38  glehner
 * Changed all occurences of eol-output to "\r\n" to allow
 * transport across different platforms.
 * Fixed (CmdTitle) which caused a "Could not write tempfile" error.
 * Fixed (CmdFormula) for \[ and \{, nested Formula - error.
 * Added support for \encode package with [isolatin] option (only).
 * Some fixes for character conversion and \emph.
 * Fixed (CmdLabel) and helpers, to not do swallow the parameter.
 * Converting now optional argument of \item in description env.3
 *
 * Revision 1.8  1998/07/03 06:58:24  glehner
 * LaTeX2e support, CmdUsepackage
 * twoside, language support
 *
 * Revision 1.7  1997/02/15 21:49:13  ralf
 * Corrected printing of german umlaut-accent characters as reported by
 * Oliver Moldenhauer
 *
 * Revision 1.6  1997/02/15 20:59:48  ralf
 * Mainly lclint-suggested changes
 *
 * Revision 1.5  1995/05/24 17:06:47  ralf
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
#include "l2r_fonts.h"
#include "cfg.h"
#include "ignore.h"
#include "util.h"
#include "encode.h"
/*****************************************************************************/

/**************************** extern variables ******************************/
extern bool bInDocument;                 /* true if File-Pointer is in the document */
extern int BracketLevel;                 /* counts open braces */
extern int RecursLevel;                  /* counts returns occured by closing braces */
extern bool g_processing_equation;                    /* true at a formula-convertion */
extern int fontsize;                     /* includes the actual fontsize in points */
extern bool twocolumn;                   /* true if twocolumn-mode is enabled */
extern bool titlepage;                   /* true if titlepage-mode is set */
extern bool article;                     /* true if article-mode is set */
extern int indent;                       /* includes the left margin e.g. for itemize-commands */
static bool NoNewLine;
extern bool bNewPar;
extern bool TITLE_AUTHOR_ON;
extern bool mbox;
extern size_t DefFont;
extern char *language;
extern enum TexCharSetKind TexCharSet;
/***************************************************************************/

/***************************  prototypes     ********************************/
void ConvertFormula();
/*@out@*/ static char *stralloc(size_t len);
static void CmdLabel1_4(int code, char *text);
static void CmdLabelOld(int code, char *text);
static void RtfHeader(int where, /*@null@*/ char *what);
static void PlainPagestyle(void);
static void ScanAux(char *token, char* reference, int code);
void CmdPagestyle(/*@unused@*/ int code);
void CmdHeader(int code);


/****************************************************************************/

/****************************************************************************/
void CmdCharFormat(int code)
/****************************************************************************
     purpose : sets the characterformat to bold, italic, underlined...
   parameter : code includes the character-format-style
     globals : fRtf: Rtf-File-Pointer
 ****************************************************************************/
{
  if (tabbing_on || 1)
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
  char *cParam = GetParam();
  switch(code)
  {
    case CMD_BEGIN:
		    (void)CallParamFunc(cParam,ON);
		    break;
    case CMD_END:
		    (void)CallParamFunc(cParam,OFF);
		    break;
    default:
	assert(0);
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
void CmdToday(/*@unused@*/ int code)
/******************************************************************************
    purpose: converts the LaTex-date-command into a Rtf-chdate-command which
	     prints the current date into an document
 ******************************************************************************/
{
  fprintf(fRtf,"\\chdate ");
}



/*****************************************************************************
CmdC
 purpose : converts /c
  params : not used
 globals : fRtf
 ******************************************************************************/
void CmdC(/*@unused@*/ int code)
{
char *cParam;

  cParam = GetParam();

  switch(cParam[0])
  {
  case 'c': fprintf(fRtf,"{\\ansi\\'e7}");
            break;
  }
  free (cParam);
}

/*****************************************************************************
CmdUmlaute

 purpose : converts german symbols from LaTeX to Rtf
 globals : fRtf
 ******************************************************************************/
void CmdUmlaute(void)
{
  char *cParam = GetParam();

  switch(cParam[0])
  {
    case 'o': fprintf(fRtf,"{\\ansi\\'f6}");
	      break;
    case 'O': fprintf(fRtf,"{\\ansi\\'d6}");
	      break;
    case 'a': fprintf(fRtf,"{\\ansi\\'e4}");
	      break;
    case 'A': fprintf(fRtf,"{\\ansi\\'c4}");
	      break;
    case 'u': fprintf(fRtf,"{\\ansi\\'fc}");
	      break;
    case 'U': fprintf(fRtf,"{\\ansi\\'dc}");
	      break;
    case 'E':fprintf(fRtf, "{\\ansi\\'cb}");
	     break;
    case 'I':fprintf(fRtf, "{\\ansi\\'cf}");
	     break;
    case 'e':fprintf(fRtf, "{\\ansi\\'eb}");
	     break;
    case 'i':fprintf(fRtf, "{\\ansi\\'ef}");
	     break;
    case 'y':fprintf(fRtf, "{\\ansi\\'ff}");
	     break;
  }
  free (cParam);
}

/******************************************************************************/
void CmdLApostrophChar(/*@unused@*/ int code)
/******************************************************************************
 purpose: converts special symbols from LaTex to Rtf
 globals : fRtf
 ******************************************************************************/
{
  char *cParam;

  cParam = GetParam();
  switch(cParam[0])
  {
    case 'A':fprintf(fRtf, "{\\ansi\\'c0}");
	     break;
    case 'E':fprintf(fRtf, "{\\ansi\\'c8}");
	     break;
    case 'I':fprintf(fRtf, "{\\ansi\\'cc}");
	     break;
    case 'O':fprintf(fRtf, "{\\ansi\\'d2}");
	     break;
    case 'U':fprintf(fRtf, "{\\ansi\\'d9}");
	     break;
    case 'a':fprintf(fRtf, "{\\ansi\\'e0}");
	     break;
    case 'e':fprintf(fRtf, "{\\ansi\\'e8}");
	     break;
    case 'i':fprintf(fRtf, "{\\ansi\\'ec}");
	     break;
    case 'o':fprintf(fRtf, "{\\ansi\\'f2}");
	     break;
    case 'u':fprintf(fRtf, "{\\ansi\\'f9}");
	     break;
  }
  free (cParam);
}

/******************************************************************************/
void CmdRApostrophChar(/*@unused@*/ int code)
/******************************************************************************
 purpose: converts special symbols from LaTex to Rtf
 globals : fRtf
 ******************************************************************************/
{
  char *cParam;

  cParam = GetParam();
  switch(cParam[0])
  {
    case 'A':fprintf(fRtf, "{\\ansi\\'c1}");
	     break;
    case 'E':fprintf(fRtf, "{\\ansi\\'c9}");
	     break;
    case 'I':fprintf(fRtf, "{\\ansi\\'cd}");
	     break;
    case 'O':fprintf(fRtf, "{\\ansi\\'d3}");
	     break;
    case 'U':fprintf(fRtf, "{\\ansi\\'da}");
	     break;
    case 'a':fprintf(fRtf, "{\\ansi\\'e1}");
	     break;
    case 'e':fprintf(fRtf, "{\\ansi\\'e9}");
	     break;
    case 'i':fprintf(fRtf, "{\\ansi\\'ed}");
	     break;
    case 'o':fprintf(fRtf, "{\\ansi\\'f3}");
	     break;
    case 'u':fprintf(fRtf, "{\\ansi\\'fa}");
	     break;
    case 'y':fprintf(fRtf, "{\\ansi\\'fd}");
	     break;
    case 'Y':fprintf(fRtf, "{\\ansi\\'dd}");
	     break;
  }
  free (cParam);
}

/******************************************************************************/
void CmdSpitzeChar()
/******************************************************************************
 purpose: converts special symbols from LaTex to Rtf
 globals : fRtf
 ******************************************************************************/
{
  char *cParam;

  cParam = GetParam();
  switch(cParam[0])
  {
    case 'A':fprintf(fRtf, "{\\ansi\\'c2}");
	     break;
    case 'E':fprintf(fRtf, "{\\ansi\\'ca}");
	     break;
    case 'I':fprintf(fRtf, "{\\ansi\\'ce}");
	     break;
    case 'O':fprintf(fRtf, "{\\ansi\\'d4}");
	     break;
    case 'U':fprintf(fRtf, "{\\ansi\\'db}");
	     break;
    case 'a':fprintf(fRtf, "{\\ansi\\'e2}");
	     break;
    case 'e':fprintf(fRtf, "{\\ansi\\'ea}");
	     break;
    case 'i':fprintf(fRtf, "{\\ansi\\'ee}");
	     break;
    case 'o':fprintf(fRtf, "{\\ansi\\'f4}");
	     break;
    case 'u':fprintf(fRtf, "{\\ansi\\'fb}");
	     break;
  }
  free (cParam);
}

/******************************************************************************/
void CmdTildeChar(/*@unused@*/ int code)
/******************************************************************************
 purpose: converts special symbols from LaTex to Rtf
 globals : fRtf
 ******************************************************************************/
{
  char *cParam;

  cParam = GetParam();
  switch(cParam[0])
  {
    case 'A':fprintf(fRtf, "{\\ansi\\'c3}");
	     break;
    case 'O':fprintf(fRtf, "{\\ansi\\'d5}");
	     break;
    case 'a':fprintf(fRtf, "{\\ansi\\'e3}");
	     break;
    case 'o':fprintf(fRtf, "{\\ansi\\'f5}");
	     break;
    case 'n':fprintf(fRtf, "{\\ansi\\'f1}");
	     break;
    case 'N':fprintf(fRtf, "{\\ansi\\'d1}");
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

/******************************************************************************
 purpose: sets the Math-Formula-Mode depending on the code-parameter
 parameter : code: type of braces which include the formula
 globals : fRtf
           g_processing_equation
           progname; latexname;
           linenumber
           bNewPar
 ******************************************************************************/
void
CmdFormula(int code)
{

  if (code & ON)			/* this is only true if starting \begin{math} */
  {
  	fprintf(fRtf,"{\\i ");
  	g_processing_equation = TRUE;
  	BracketLevel++;
    diagnostics(4, "Switching g_processing_equation on with \\begin{math}");
    return;
  }
     
  switch(code)
  {
    case FORM_NO_NUMBER:
	    if (g_processing_eqnarray)
    		g_suppress_equation_number = TRUE;
    	break;
    	
    case FORM_DOLLAR:
      if (g_processing_equation)
      {
      	fprintf(fRtf,"}");
      	g_processing_equation = FALSE;
      	BracketLevel--;
        diagnostics(4, "Switching g_processing_equation off with $");
      }
      else
      {
      	fprintf(fRtf,"{\\i ");
      	g_processing_equation = TRUE;
      	BracketLevel++;
        diagnostics(4, "Switching g_processing_equation on with $");
      }
      break;
      
    case FORM_RND_OPEN:  /* \( */
      fprintf(fRtf,"{\\i ");
      g_processing_equation = TRUE;
      diagnostics(4, "Switching g_processing_equation on with \\(");
      break;
      
    case FORM_RND_CLOSE:   /* \) */
      fprintf(fRtf,"}");
      g_processing_equation = FALSE;
      diagnostics(4, "Switching g_processing_equation off with \\)");
      break;
      
    case FORM_ECK_OPEN:   /* \[ */
      fprintf(fRtf,"\n\\par{\\i  ");
      g_processing_equation = TRUE;
      diagnostics(4, "Switching g_processing_equation on with \\[");
      break;
      
    case FORM_ECK_CLOSE:      /* \] */
      fprintf(fRtf,"}\n\\par ");
      g_processing_equation = FALSE;
      bNewPar = TRUE;
      diagnostics(4, "Switching g_processing_equation off with \\]");
      break;

    case FORM_MATH:   /* will only be encountered for \end{math} */
	  fprintf(fRtf,"}");
	  g_processing_equation = FALSE;
	  BracketLevel--;
	  diagnostics(4, "Switching g_processing_equation off with \\end{math}");
      break;
  }
}

/******************************************************************************/
void CmdIgnore(/*@unused@*/ int code)
/******************************************************************************
 purpose: LaTeX-commands which can't be converted in Rtf-commands are overread
	  as such
 ******************************************************************************/
{
}

/******************************************************************************/
void CmdLdots(/*@unused@*/ int code)
/******************************************************************************
 purpose: converts the LaTex-\ldots-command into "..." in Rtf
 globals : fRtf
 ******************************************************************************/
{
  fprintf(fRtf, "...");
}

/****************************************************************************
CmdEmphasize
 purpose: turn on/off the emphasized style for characters
 globals : fRtf
 ******************************************************************************/
void CmdEmphasize(/*@unused@*/ int code)
{
  if (!tabbing_on) /* tabbing-environment ignores emphasized-style */
    {
    static bool Em_on = FALSE;
    if (!(Em_on))
      fprintf(fRtf,"{\\i ");
    else
      fprintf(fRtf,"{\\plain ");
    Em_on = !Em_on;
    Convert();
    fprintf(fRtf,"}");
    Em_on = !Em_on;
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
           tabbing_on
 ******************************************************************************/
{
  if (tabbing_on) /* tabbing-environment ignores emphasized-style */
    {
    static bool Em_on = FALSE;
    switch(code)
    {
      case (EMPHASIZE | ON):
	if (!(Em_on))
	  fprintf(fRtf,"{\\i ");
	else
	 fprintf(fRtf,"} ");
	Em_on = !Em_on;
	break;
      case (EMPHASIZE | OFF):
	if ((Em_on))
	  fprintf(fRtf,"} ");
	Em_on = FALSE;
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
      /* LEG Meanwhile commented out    ClearEnvironment();*/
      bInDocument = TRUE;
      if(!pagestyledefined) {
//	diagnostics(WARNING, "rtf 1.4 codes generated, funct1.c (Environment)");
	PlainPagestyle();
      }
      ReadLg(language);
    }
    PushEnvironment(code);
  }
  else /* off switch */
  {
    PopEnvironment();
  }
}

/******************************************************************************
  purpose: converts the tite, author and date-information from LaTex to Rtf
parameter: code includes which type of the information listed above will be converted
 globals : fRtf
           TITLE_AUTHOR_ON
           fontsize
           alignment
           titlepage: if true a page-break is inserted
 ******************************************************************************/
void
CmdTitle(/*@unused@*/ int code)
{
  char title_begin[10];
  char author_begin[10];
  char date_begin[10];

  static /*@owned@*//*@null@*/ char *title  = NULL;
  static /*@owned@*//*@null@*/ char *author = NULL;
  static /*@owned@*//*@null@*/ char *date   = NULL;

  
  switch (code)
  {
    case TITLE_TITLE: title = GetParam();
		      break;
    case TITLE_AUTHOR: TITLE_AUTHOR_ON = TRUE; /* is used for the \and command */
		       author = GetParam();
		       TITLE_AUTHOR_ON = FALSE;
		       break;
    case TITLE_DATE: date = GetParam();
		     break;
    case TITLE_MAKE:
      sprintf(title_begin,"%s%2d", "\\fs", (30*fontsize)/20);
      sprintf(author_begin,"%s%2d", "\\fs", (24*fontsize)/20);
      sprintf(date_begin,"%s%2d", "\\fs", (24*fontsize)/20);

      fprintf(fRtf,"\n\\par\\pard\\qc {%s ", title_begin);
      if (title != NULL && strcmp(title, "") !=0) ConvertString(title);
      fprintf(fRtf,"}");
      fprintf(fRtf,"\n\\par\\qc {%s ", author_begin);

      if (author != NULL && strcmp(author, "") !=0) ConvertString(author);
      fprintf(fRtf,"}");
      fprintf(fRtf,"\n\\par\\qc {%s ", date_begin);

      if (date != NULL && strcmp(date, "") !=0) ConvertString(date);
      fprintf(fRtf,"}");
      fprintf(fRtf,"\n\\par\n\\par\\pard\\q%c ",alignment);
      if (titlepage)
	fprintf(fRtf,"\\page ");
      break;
  }
}

/******************************************************************************/
void CmdDocumentStyle(/*@unused@*/ int code)
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
  /*@owned@*//*@null@*/ static char *style = NULL;
  char cThis = ' ';
  char optstring[30] = { '\0' };
  bool has_optparam = FALSE;


     for(;;) /* do forever */
     {
        while ( cThis == ' ')
        {
   	   if ( (fread(&cThis,1,1,fTex) < 1))
	      numerror(ERR_EOF_INPUT);
        }
	if(fseek(fTex,-1L,SEEK_CUR) != 0) /* reread last character */
	  diagnostics(ERROR, "Seek failed in LaTeX-file");

	switch (cThis)
	{
	   case '{' : style = GetParam();  
		      if (strcmp(style,"article") == 0)
                      {
			  article = TRUE;
                      }
		      else
                      {
			  article = FALSE;
                      }
		      diagnostics(4, "Documentstyle/class `%s' encountered", style);

		      break;
	   case '[' : 
	   			style = (char*) malloc (100*sizeof(char));
                if (style == NULL)
                     error(" malloc error -> out of memory!\n");
                rewind_one();
                GetBracketParam(style,99);
		      has_optparam = TRUE;
		      diagnostics(4,"Documentstyle/classoptions: `%s'", style);
		      break;
	   default :  /* last character was read again above.
			 this character will be written in the rtf-file in the
			 convert-routine (main.c) */

		      fprintf(fRtf,"\\fs%d ",fontsize); /* default or new fontsize */
		      return;
		      /*@notreached@*/
		      break;
	} /* switch */

	if (has_optparam)
	{
	    /* returnstring of GetOptParam will be seperated in his components */
	    do
	    {
		 strcpy(optstring,GetSubString
		                            (style == NULL ? "" : style,','));

		 if (strcmp(optstring,"11pt") == 0)
		    fontsize = 22; /* fontsize 11-TEX -> 22-RTF */

		 else if (strcmp(optstring,"12pt") == 0)
		    fontsize = 24; /* fontsize 12-TEX -> 24-RTF */

		 else if (strcmp(optstring,"german") == 0)
		    { GermanMode = TRUE; PushEnvironment(GERMANMODE);
		      language = stralloc(strlen(optstring));
		      strcpy(language,optstring);
		    }
		 else if (strcmp(optstring,"spanish") == 0)
		   {
		     language = stralloc(strlen(optstring));
		     strcpy(language,optstring);
		   }
		 else if (strcmp(optstring,"twoside") == 0)
		   { twoside = TRUE;
		   /*LEG diag(1,)*/
		   fprintf(stderr, "\n  rtf1.5 token `\\facingp' used");
		   fprintf(fRtf,"\\facingp");
		   }
		 
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
		 else if (strcmp(optstring,"isolatin1") == 0)
		   {
		     TexCharSet = ISO_8859_1;
		     fprintf(stderr,"\nisolatin1 style option encountered.");
                     fprintf(stderr,"\nLatin-1 (= ISO 8859-1) special characters will be ");
                     fprintf(stderr,"converted into RTF-Commands!\n");
                   }
		 else if (strcmp(optstring, "hyperlatex") == 0)
		   {
		     PushEnvironment(HYPERLATEX);
		   }
		 else if (!TryVariableIgnore(optstring, fTex))
		   {
		     fprintf(stderr,"\n%s: WARNING: unknown style option %s ignored",
			     progname, optstring);
		   }
	    }
	     while (strcmp(style,"") != 0);
      } /* if */

      if ( (fread(&cThis,1,1,fTex) < 1)) 
	   numerror(ERR_EOF_INPUT); 

    } /* for */
}


/******************************************************************************/
/*LEG030598*/
char *stralloc(size_t len)
/******************************************************************************
 purpose: allocates memory for a string, asserts that there is memory
 ******************************************************************************/
{
  char *str = NULL;
  str = malloc(len);
  if (str == NULL)
    error(" malloc error -> out of memory!\n");
  return(str);
}

/******************************************************************************/
/*LEG190498*/

/******************************************************************************
 CmdUsepackage:
 purpose: reads the LaTex2e-usepackage-command parameters and
	  converts it to a similar Rtf-information
	  styled after CmdDocumentStyle
 globals : fRtf
           fontsize
           article: (article, report, bookstyle)
           GermanMode: support for germanstyle
           twocolumn; titlepage
 ******************************************************************************/
void CmdUsepackage(/*@unused@*/ int code)
{
  char package[100];
  char options[100]; 
 
  GetBracketParam(options,99);
  GetBraceParam(package,99);
  
  diagnostics(4, "Package {%s} with options [%s] encountered", package, options);
  
  if (strcmp(package,"11pt") == 0) {
    fontsize = 22; /* fontsize 11-TEX -> 22-RTF */
    fprintf(fRtf,"\\fs%d ",fontsize); /* default or new fontsize */
  }
  else if (strcmp(package,"12pt") == 0) {
    fontsize = 24; /* fontsize 12-TEX -> 24-RTF */
    fprintf(fRtf,"\\fs%d ",fontsize); /* default or new fontsize */
  }
  else if (strcmp(package,"german") == 0)
    {
      GermanMode = TRUE;
      PushEnvironment(GERMANMODE);
      language = stralloc(strlen(package));
      strcpy(language,package);
    }
  else if (strcmp(package,"spanish") == 0) {
    language = stralloc(strlen(package));
    strcpy(language,package);
  }
  else if (strcmp(package,"twoside") == 0)
    {
      twoside = TRUE;
      /*LEG diag(1,)*/
      fprintf(stderr, "\n  rtf1.5 token `\\facingp' used");
      fprintf(fRtf,"\\facingp");
    }
  else if (strcmp(package,"twocolumn") == 0)
    {
      fprintf(fRtf,"\\cols2\\colsx709 "); /* two columns */
      /* space between columns 709 */
      twocolumn = TRUE;
    }
  else if (strcmp(package,"titlepage") == 0)
    {
      titlepage = TRUE;
    }
/*LEG190498*/
  else if (strcmp(package,"isolatin1") == 0)
    {	
      TexCharSet = ISO_8859_1;
      fprintf(stderr,"\nisolatin1 package encountered.");
      fprintf(stderr,"\nLatin-1 (= ISO 8859-1) special characters will be ");
      fprintf(stderr,"converted into RTF-Commands!\n");
    }
  else if (strcmp(package, "inputenc") == 0)
    {
      diagnostics(WARNING, "inputenc package with parameter(s) `%s' encountered.",
		  options);

      if (strcmp(options,"latin1") == 0)
	{
	  TexCharSet = ISO_8859_1;
	  fprintf(stderr,"\nLatin-1 (= ISO 8859-1) special characters will be ");
	  fprintf(stderr,"converted into RTF-Commands!\n");
	}
    }
  /*  would be nice, but needs language option from documentclass
  else if (strcmp(package, "babel") == 0)
    {
      if (strcmp(options, "german") == 0 || 
	  strcmp(options, "spanish") == 0)
	{
	  language = options; <-- dumps core
	  optparam = FALSE;
	}
      else
	{
	  diagnostics(WARNING,
		      "unknown language `%s' as option to babel package",
		      options);
	}
    }
    */
  else if (strcmp(package, "hyperlatex") == 0)
    {
      PushEnvironment(HYPERLATEX);
    }
  else if (!TryVariableIgnore(package, fTex))
    {
      fprintf(stderr,"\n%s: WARNING: unknown \\usepackage{%s} ignored",
	      progname, package);
    }
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
  static int table_caption = 0;
  static int figure_caption = 0;
  char optparam[100] = "";
  char cNext = ' ';

  GetBracketParam(optparam,99);
  switch (code)
  {
  case SECT_PART:
    part++;
    fprintf(fRtf, "\\par\\pard\\page");
    fprintf(fRtf, "{\\qc\\b\\fs40 ");
    fprintf (fRtf, "%s %d\\par ",
	     TranslateName("PART"), part);
    Convert();
    fprintf(fRtf, "\\par}\\page\\par \\pard\\q%c",alignment);
    break;

  case SECT_CHAPTER:
    chapter++;
    section = 0;
    subsection = 0;
    fprintf (fRtf, "\\par \\pard\\page\\pard{\\pntext\\pard\\plain\\b\\fs32\\kerning28 ");
    fprintf (fRtf, "%d\\par \\par }\\pard\\plain\n", chapter);
    fprintf (fRtf, "%s%u%s{", HEADER11,(unsigned int)DefFont,HEADER12);
    Convert();
    fprintf( fRtf,"}\n\\par \\pard\\plain\\f%u\\q%c\n"
           , (unsigned int)DefFont,alignment);
    do 
    {
       if ( (fread(&cNext,1,1,fTex) < 1))
          numerror(ERR_EOF_INPUT);
    }
    while ((cNext == ' ') || (cNext == '\n'));
    rewind_one();
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
       fprintf (fRtf, "%s%u%s{", HEADER11,(unsigned int)DefFont,HEADER12);
       Convert();
       fprintf(fRtf,"}\n\\par \\pard\\plain\\f%u\\q%c\n"
              , (unsigned int)DefFont,alignment);
       do 
       {
          if ( (fread(&cNext,1,1,fTex) < 1))
             numerror(ERR_EOF_INPUT);
       }
       while ((cNext == ' ') || (cNext == '\n'));
       rewind_one(); /* reread last character */
       bNewPar = FALSE;
       bBlankLine = TRUE;
    }
    else
    {
       section++;
       subsection = 0;
       fprintf (fRtf, "{\\par \\pard\\pntext\\pard\\plain\\b\\fs24 %d.%d\\tab}\\pard\\plain\n", chapter, section);
       fprintf (fRtf, "%s%u%s{", HEADER21,(unsigned int)DefFont,HEADER22);
       Convert();
       fprintf( fRtf,"}\n\\par \\pard\\plain\\f%u\\q%c\n"
              , (unsigned int)DefFont,alignment);
       do 
       {
          if ( (fread(&cNext,1,1,fTex) < 1))
             numerror(ERR_EOF_INPUT);
       }
       while ((cNext == ' ') || (cNext == '\n'));
       rewind_one(); /* reread last character */
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
       fprintf (fRtf, "%s%u%s{", HEADER21,(unsigned int)DefFont,HEADER22);
       Convert();
       fprintf( fRtf,"}\n\\par \\pard\\plain\\f%u\\q%c\n"
              , (unsigned int)DefFont,alignment);
       do 
       {
          if ( (fread(&cNext,1,1,fTex) < 1))
             numerror(ERR_EOF_INPUT);
       }
       while ((cNext == ' ') || (cNext == '\n'));
       rewind_one(); /* reread last character */
       bNewPar = FALSE;
       bBlankLine = TRUE;
    }
    else
    {
       subsection++;
       fprintf (fRtf, "{\\par \\pard\\pntext\\pard\\plain\\b\\fs24 %d.%d.%d\\tab}\\pard\\plain\n", chapter, section, subsection);
       fprintf (fRtf, "%s%u%s{", HEADER31,(unsigned int)DefFont,HEADER32);
       Convert();
       fprintf( fRtf,"}\n\\par \\pard\\plain\\f%u\\q%c\n"
              , (unsigned int)DefFont,alignment);
       do 
       {
          if ( (fread(&cNext,1,1,fTex) < 1))
             numerror(ERR_EOF_INPUT);
       }
       while ((cNext == ' ') || (cNext == '\n'));
       rewind_one(); /* reread last character */
       bNewPar = FALSE;
       bBlankLine = TRUE;
    }
    break;

  case SECT_SUBSUB:
    if (article)
    {
       subsubsection++;
       fprintf (fRtf, "{\\par \\pard\\pntext\\pard\\plain\\b\\fs24 %d.%d.%d\\tab}\\pard\\plain\n", section, subsection, subsubsection);
       fprintf (fRtf, "%s%u%s{", HEADER31,(unsigned int)DefFont,HEADER32);
       Convert();
       fprintf( fRtf,"}\n\\par \\pard\\plain\\f%u\\q%c\n"
              , (unsigned int)DefFont,alignment);
       do 
       {
          if ( (fread(&cNext,1,1,fTex) < 1))
             numerror(ERR_EOF_INPUT);
       }
       while ((cNext == ' ') || (cNext == '\n'));
       rewind_one(); /* reread last character */
       bNewPar = FALSE;
       bBlankLine = TRUE;
    }
    else
    {
       subsubsection++;
       fprintf (fRtf, "{\\par \\pard\\pntext\\pard\\plain\\b\\fs24 %d.%d.%d.%d\\tab}\\pard\\plain\n", chapter, section, subsection, subsubsection);
       fprintf (fRtf, "%s%u%s{", HEADER41,(unsigned int)DefFont,HEADER42);
       Convert();
       fprintf( fRtf,"}\n\\par \\pard\\plain\\f%u\\q%c\n"
              , (unsigned int)DefFont,alignment);
       do 
       {
          if ( (fread(&cNext,1,1,fTex) < 1))
             numerror(ERR_EOF_INPUT);
       }
       while ((cNext == ' ') || (cNext == '\n'));
       rewind_one(); /* reread last character */
       bNewPar = FALSE;
       bBlankLine = TRUE;
    }
    break;

  case SECT_CAPTION:
/*SAP Change to produce count figures and tables separately*/
    if (g_processing_figure)
    {
	    figure_caption++;
	    if (GermanMode)
	       fprintf(fRtf,"\n\\par \\pard\\qc {Abbildung %d: ", figure_caption);
	    else
	       fprintf(fRtf,"\n\\par \\pard\\qc {Figure %d: ", figure_caption);
	}
	else
	{
	    table_caption++;
	    if (GermanMode)
	       fprintf(fRtf,"\n\\par \\pard\\qc {Tabelle %d: ", table_caption);
	    else
	       fprintf(fRtf,"\n\\par \\pard\\qc {Table %d: ", table_caption);
	}
/*SAP end fix */
    Convert();
    fprintf(fRtf,"} \\par \\pard\\q%c\n",alignment);
    break;
  }
}


/******************************************************************************/
void CmdFootNote(int code)
/******************************************************************************
 purpose: converts footnotes from LaTeX to Rtf
 params : code specifies whether it is a footnote or a thanks-mark
 globals: fTex, fRtf
          fontsize 
          DefFont
 ******************************************************************************/
{
  char number[255];
  char cNext;
  static int thankno = 1;
  
  GetBracketParam(number,254); /* is ignored because of the automatic footnumber-generation */

  if (code == THANKS)
  {
     fprintf(fRtf,"{\\up6 %d}\n{\\*\\footnote \\pard\\plain\\q%c \\s246 \\f%u \\fs%d",thankno,alignment,(unsigned int)DefFont,fontsize);
     fprintf(fRtf," {\\up6 %d}",thankno++);
     Convert();
     fprintf(fRtf,"}\n ");
  }
  else
  {
     fprintf(fRtf,"{\\up6\\chftn}\n{\\*\\footnote \\pard\\plain\\q%c \\s246 \\f%u \\fs%d",alignment,(unsigned int)DefFont,fontsize);
     fprintf(fRtf," {\\up6\\chftn }");
     Convert();
     fprintf(fRtf,"}\n ");
  }
}


/******************************************************************************
  LEG030598
  purpose: sets centered page numbering at bottom in rtf-output

  globals : pagenumbering set to TRUE if pagenumbering is to occur, default
 ******************************************************************************/
void
PlainPagestyle(void)
{
  pagenumbering = TRUE;
  
  if(twoside) {
    fprintf(fRtf, "{\n\\footerr\\pard\\plain\\f2\\qc"
	    "{\\field{\\*\\fldinst PAGE}{\\fldrslt ?}}\\par}");
    fprintf(fRtf, "{\n\\footerl \\pard\\plain \\f2\\qc "
	    "{\\field{\\*\\fldinst PAGE}{\\fldrslt ?}}\\par}");
  }
  else {
    fprintf(fRtf, "{\n\\footer\\pard\\plain\\qc\\f2"
	    "{\\field{\\*\\fldinst PAGE}{\\fldrslt ?}}\\par}");
  }
}

/******************************************************************************
 * LEG030598
 purpose: sets page numbering in rtf-output
 parameter: 

 globals : headings  set to TRUE if the pagenumber is to go into the header
           pagenumbering set to TRUE if pagenumbering is to occur- default
	   pagestyledefined, flag, set to true

Produces latex-like headers and footers.
Needs to be terminated for:
- headings chapter, section informations and page numbering
- myheadings page nunmbering, combined with markboth, markright.

 ******************************************************************************/
void
CmdPagestyle(/*@unused@*/ int code)
{
  static char *style = "";

  pagestyledefined = TRUE;
  style = GetParam();
  if(strcmp(style,"empty") == 0)
    {
      if(pagenumbering) {
	fprintf(fRtf, "{\\footer}");
	pagenumbering = FALSE;
      }
    }
  else if (strcmp(style,"plain") == 0)
    PlainPagestyle();
  else if (strcmp(style,"headings") == 0)
    {
      headings = TRUE;
      /*--- but here code to put section information in header, pagenumbering
	    in header */
    }
  else if (strcmp(style,"myheadings") == 0)
    {
      headings = TRUE;
      /*--- but here code to put empty section information in header, will be
	    provided by markboth, markright
	    pagenumbering in header */
    }
  else {
    fprintf(stderr, "\nWarning: unknown \\pagestyle{%s} ignored", style);
  }
}



/******************************************************************************
 * LEG030598
 purpose: converts the \markboth and \markright Command in Header information
 parameter: code: BOTH_SIDES, RIGHT_SIDE

 globals : twoside, 
 ******************************************************************************/
void
CmdHeader(int code)
{
  if(code == BOTH_SIDES) {
    if(twoside) {
      RtfHeader(LEFT_SIDE, NULL);
      RtfHeader(RIGHT_SIDE, NULL);
    }
    else
      fprintf(stderr, "\nWarning: \\markboth used in onesided documentstyle");
  }
  else 
    RtfHeader(BOTH_SIDES, NULL);
}

/******************************************************************************
  LEG030598
  purpose: generates the header command in the rtf-output
  parameter: where: RIGHT_SIDE, LEFT_SIDE -handed page, BOTH_SIDES
           what:  NULL - Convert from LaTeX input, else put "what" into rtf
                  output
 ******************************************************************************/
void
RtfHeader(int where, char *what)
{
  switch (where) {
  case RIGHT_SIDE:
    fprintf(fRtf,"\n{\\headerr \\pard\\plain\\f2 ");
    break;
  case LEFT_SIDE:
    fprintf(fRtf,"\n{\\headerl \\pard\\plain\\f2 ");
    break;
  case BOTH_SIDES:
    fprintf(fRtf,"\n{\\header \\pard\\plain\\f2 ");
    break;
    default:
    error("\n error -> called RtfHeader with illegal parameter\n ");
  }
  if(what == NULL) {
    Convert();
    fprintf(fRtf,"}");
  }
  else 
    fprintf(fRtf,"%s}", what);
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
      fprintf(fRtf,"\n\\par\\li%d",indent);
      break;
    case (QUOTE | OFF):
      indent -= 512;
      NoNewLine = FALSE;
      fprintf(fRtf,"\n\\par\\li%d",indent);
      break;
    case (QUOTATION | ON):
      indent += 512;
      NoNewLine = TRUE;
      fprintf(fRtf,"\n\\par\\li%d",indent);
      break;
    case (QUOTATION | OFF):
      indent -= 512;
      NoNewLine = FALSE;
      fprintf(fRtf,"\n\\par\\li%d",indent);
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
      g_enumerate_depth++;
      CmdItem(RESET_ITEM_COUNTER);
      indent += 512;
      NoNewLine = TRUE;
      bNewPar = FALSE;
      break;
    case (ENUMERATE | OFF):
      PopEnvironment();
      g_enumerate_depth--;
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
 globals  : nonewline, indent: look at function CmdQuote
            bNewPar: 
            fRtf: Rtf-file-pointer
 ******************************************************************************/
{ 
  char itemlabel[100];
  static int item_number[4];
  
  if (code == RESET_ITEM_COUNTER)
  {
      item_number[g_enumerate_depth] = 1;
      return;
  }
  
  fprintf(fRtf,"\n\\par\\fi0\\li%d ",indent); 
  
  if (GetBracketParam(itemlabel,99)) /* \item[label] */
  {
    int i;
 	fprintf(fRtf,"{\\b "); /*bold on */

	for(i = 0; itemlabel[i] != '\0'; i++)
	{
		if (TexCharSet==ISO_8859_1)
			Write_ISO_8859_1(itemlabel[i]);
		else
		    Write_Default_Charset(itemlabel[i]);
	}

 	fprintf(fRtf,"}\\tab "); /*bold off*/
  }
  else									/* \item */
  {
	switch (code)
	{      
	    case ITEMIZE:
	       fprintf(fRtf,"\\bullet\\tab ");
	       break;

	    case ENUMERATE:
	       switch (g_enumerate_depth)
	       {
	       	   case 1:
	           fprintf(fRtf,"%d.",item_number[g_enumerate_depth]);
	           break;
	           
	           case 2:
 	 	   	   fprintf(fRtf,"(%c)",'a'+item_number[g_enumerate_depth]-1); 
 	 	   	   break;

	           case 3:
	           roman_item(item_number[g_enumerate_depth], itemlabel);
 	 	   	   fprintf(fRtf,"%s.",itemlabel); 
 	 	   	   break;

	           case 4:
 	 	   	   fprintf(fRtf,"%c.",'A'+item_number[g_enumerate_depth]-1); 
 	 	   	   break;
 	 	   }
	       fprintf(fRtf,"\\tab ");
 	 	   item_number[g_enumerate_depth]++;
	       break;

	    case DESCRIPTION: 
 	 	   fprintf(fRtf,"\\tab "); /*indent*/
	       break;
	 }
   }
   Convert();
   bNewPar = FALSE;
}

/******************************************************************************/
void CmdMbox(/*@unused@*/ int code)
/******************************************************************************
  purpose: converts the LaTeX \mbox-command into  an similar Rtf-style
  globals: mbox
 ******************************************************************************/
{
bool was_processing_equation = g_processing_equation;
  if (g_processing_equation)
  {
  	g_processing_equation = FALSE;
 	fprintf(fRtf,"}"); /*close italics*/
  }
  
  mbox = TRUE;
  Convert();
  mbox = FALSE;

  if (was_processing_equation)
  {
  	g_processing_equation = TRUE;
 	fprintf(fRtf,"{\\i "); /*reopen math italics*/
  }
}

/******************************************************************************/
void ConvertFormula()
/******************************************************************************
 purpose : necessary commands for the formula-environment are pushed onto a stack
 globals : g_processing_equation
 ******************************************************************************/
{
  (void)Push(RecursLevel,BracketLevel);
  ++BracketLevel;    /* Math End char is treated as } math begin as { */
  (void)Push(RecursLevel,BracketLevel);
  g_processing_equation = TRUE;
  Convert();
  g_processing_equation = FALSE;
}


/******************************************************************************/
void CmdSetFont(int code)
/******************************************************************************
  purpose: sets an font for the actual character-style
parameter: code: includes the font-type
  globals: fRtf
 ******************************************************************************/
{
  size_t num;

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
  fprintf(fRtf,"{\\f%u ", (unsigned int)num);
  Convert();
  fprintf(fRtf,"}");
}
/******************************************************************************
 purpose: reads an extern-LaTex-File from the into the actual document and converts it to
	  an similar Rtf-style
 globals: GermanMode: is set if germanstyles are included
          fTex
          progname
 ******************************************************************************/
void
CmdInclude(/*@unused@*/ int code)
{
  char fname[1024];
  char fullpath[1024];
  char *dp;
  
  /*@dependent@*/      FILE *fp;
  /*@dependent@*/      FILE *LatexFile;
  char *olatexname;
  long oldlinenumber;

  strcpy(fname, "");


# ifdef no_longer_needed

  fgetpos(fTex,&aktpos);
  if ( (n=(fread(fname,99,1,fTex)) < 1))
    numerror(ERR_EOF_INPUT);
  fname[n+1] = '\0';

  if (strstr(fname,"german.sty")!=NULL)
  {
    GermanMode = TRUE;
    PushEnvironment(GERMANMODE);
    return;
  }
  fsetpos(fTex,&aktpos);

# endif /* no_longer_needed */

  GetInputParam(fname,99);
  if (strstr(fname,"german.sty")!=NULL)
  {
    GermanMode = TRUE;
    PushEnvironment(GERMANMODE);
    return;
  }
  assert(fname != NULL);


  if (strcmp(fname,"") == 0)
     {
     fprintf(stderr,"\n%s: WARNING: invalid filename after \\include: %s\n",progname,fname);
     return;
     }

/* extension .tex is appended automatically */
  if(strchr(fname,'.')==NULL)
    strcat(fname,".tex");

/*SAP fixes for Mac Platform*/
  strcpy(fullpath,latexname);
  dp=strrchr(fullpath,':');
  if (dp!=NULL) {dp++;*dp='\0';} else strcpy(fullpath,"");
  strcat(fullpath,fname);
  strcpy(fname,fullpath);
/*SAP end fix*/
  
  if (g_processing_include)
  {
  	fprintf(stderr,"\nWARNING: cannot process nested include file: %s\n",fullpath);
  	return;
  }
  g_processing_include = TRUE;
  
  
  if ( (fp=(fopen(fname,"rb")))==NULL )
  {
    fprintf(stderr,"\nWARNING: cannot open include file: %s\n",fullpath);
    return;
  }

  fprintf(stderr,"\nProcessing include file: %s\n",fullpath);
    
  LatexFile = fTex;					/* Save current file pointer */
  WriteTemp(fp);         			/* Normalize eol in fp, close fp, and make fTex = tmpfile() */
  oldlinenumber = linenumber;
  linenumber = 1;
  olatexname = latexname;
  latexname = fname;
  BracketLevel++;
  fprintf(fRtf,"{");
  Convert();
  fprintf(fRtf,"}");
  BracketLevel--;

  fclose(fTex);
  fTex = LatexFile;
  latexname = olatexname;
  linenumber = oldlinenumber;
  g_processing_include=FALSE;
//  (void)fclose(fp);
}

/******************************************************************************/
void CmdVerb(int code)
/******************************************************************************
 purpose: converts the LaTex-verb-environment to a similar Rtf-style
 ******************************************************************************/
{
  char cThis;
  char markingchar = '\177';
  size_t num;


  while (fTexRead(&cThis, 1,1,fTex) == 1)
  {
    if ((cThis != ' ') && (cThis != '*') && !isalpha((unsigned char) cThis))
    {
      markingchar = cThis;
      break;
    }
  }
  if (cThis != markingchar)
    numerror(ERR_EOF_INPUT);
    
  num = GetTexFontNumber("Typewriter");
  fprintf(fRtf,"{\\f%u ", (unsigned int)num);

  while (fTexRead(&cThis, 1,1,fTex) == 1)
  {
    if (cThis == markingchar)
      break;

    if (cThis == '\\')
		fprintf(fRtf,"\\\\");
    else if (cThis == '{')
		fprintf(fRtf,"\\{");
    else if (cThis == '}')
		fprintf(fRtf,"\\}");
	else if (code == AST_FORM && cThis == ' ')  /* print dot instead of space */
	    fprintf(fRtf,"{\\ansi\\'b7}");
	else
	  fprintf(fRtf,"%c",cThis);
  }
  fprintf(fRtf,"}");
  
  if (cThis != markingchar)
    numerror(ERR_EOF_INPUT);
}

/******************************************************************************/
void CmdVerbatim(/*@unused@*/ int code)  /* write anything till \end{verbatim} */
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
void CmdIgnoreDef(/*@unused@*/ int code)
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
    case 'a': fprintf(fRtf, "{\\ansi\\'e4}");
	      break;
    case 'o': fprintf(fRtf, "{\\ansi\\'f6}");
	      break;
    case 'u': fprintf(fRtf, "{\\ansi\\'fc}");
	      break;
    case 's': fprintf(fRtf, "{\\ansi\\'df}");
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
void CmdIgnoreLet(/*@unused@*/ int code)
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
  rewind_one(); /* reread last character */
}







/******************************************************************************/
void IgnoreNewCmd(/*@unused@*/ int code)
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
  rewind_one(); /* reread last character */
  if (cThis == '\\')
    CmdIgnoreDef(0);
  else
    CmdIgnoreParameter(No_Opt_Two_NormParam );
}



/*************************************************************************
 *LEG250498
 * purpose: opens existing aux-file and scans for token, with value
 * reference.  Then outputs the corresponding ref-information to the
 * rtf-file parameters: code 0 = get the first parameter ( {refnumber}
 * -> refnumber)
 *                           1 = get the first par. of the first {
 * {{sectref}{line}} } -> sectref ) globals: input (name of
 * LaTeX-Inputfile) fRtf rtf-Outputfile
 ************************************************************************/

void
ScanAux(char *token, char* reference, int code)
{
  static FILE* fAux = NULL;
  char help[255];      /* should really be allocated dynmically */
  char AuxLine[255];
  size_t i;
      
  if (fAux == NULL)
    {
      if ((fAux = fopen(AuxName,"r")) == NULL)   /* open file */
	{
	  fprintf(stderr,"Error opening AUX-file: %s - while trying to"
		         "search for \\%s{%s}...",
		  AuxName, token, reference);
	  return;
	}
    }


  i = 0; 

  sprintf(help, "\\%s{%s}", token, reference);
  /*DBG1 fprintf(stderr, "\n-> Searching .aux for %s", help); */
  /*DBG3 fprintf(stderr, "\n"); */
  
  do
    {
      if (fgets (AuxLine, 255, fAux) == NULL)
	if (feof(fAux))
	  break;
	else
	  error ("Error reading AUX-File!\n");
      	/*DBG2 fprintf(stderr,"--> in line: %s", AuxLine); */
      if (strstr (AuxLine, help) )
	{
	  i = strlen(help); /* supposing that we start on start of line !*/
	  while (AuxLine[i++] != '{') ; /* scan to "{" */
	  if (code == 1)
	    while (AuxLine[i++] != '{') ; /* scan to "{" */
	  /*DBG2 fprintf(stderr, "\n--> Now scanning argument: "); */
	  while (AuxLine[i] != '}')
	    {
	      /*DBG2 fprintf(stderr, "%c", AuxLine[i]);*/
	      fprintf (fRtf, "%c", AuxLine[i++]);
	    }
	  break; /* don't continue searching */
	}
      
    } while (!feof(fAux));
  if (feof(fAux)) {
    fprintf(stderr, "\nWarning: \\%s{%s} not found in %s", token, reference, AuxName);
    diagnostics(WARNING, "\\%s{%s} not found in %s",
		token, reference, AuxName);
  }
  rewind(fAux);
  return;
}




/******************************************************************************
  purpose : label, produce rtf-output in dependency of the rtfversion.
  parameter : code  kind of label, passed through

  LEG250498
 ******************************************************************************/
void
CmdLabel(int code)
{
  char *text;
  char cThis;

  if (code < HYPER) {
    text = GetParam();
    rewind_one();
  }
  else {
    text = hyperref;
  }

  diagnostics(4, "Generating label/bookmark `%s'", text);

  if (rtf_restrict(1, 1))
    CmdLabelOld(code, text);
  if (rtf_restrict(1, 4))
    CmdLabel1_4(code, text);
  
  if (code >= HYPER) free(text);

  if ( (fread(&cThis,1,1,fTex) < 1))
    numerror(ERR_EOF_INPUT);
  while (cThis == ' ')
    {
      if ( (fread(&cThis,1,1,fTex) < 1))
	numerror(ERR_EOF_INPUT);
    }
  if ( (fread(&cThis,1,1,fTex) < 1))
    numerror(ERR_EOF_INPUT);
  if (cThis != '\n')
    rewind_one();
  else
    ++linenumber;
  /*LEG190498*/
}

/******************************************************************************
  purpose : label
  parameter : code  kind of label, text name of the label
  globals : fRtf
            linenumber
  LEG250498
 ******************************************************************************/
void
CmdLabel1_4(int code, char *text)
{
  switch (code)
    {
    case LABEL:
      /* Note that Hyperlabels do not exist, if they are encountered
	 it's a severe bug */

      fprintf(fRtf,"{\\*\\bkmkstart %s}"
	           "{\\*\\bkmkend %s}",
	      text, text);
      break;                         
    case REF:
    case HYPERREF:
      fprintf(fRtf,"{\\field\\fldlock{\\*\\fldinst REF %s  \\\\n}{\\fldrslt ",
	      text);
      ScanAux("newlabel", text, 1);
      fprintf(fRtf,"}}");
      break;
    case PAGEREF:
    case HYPERPAGEREF:
      fprintf(fRtf,"{\\field{\\*\\fldinst PAGEREF %s}{\\fldrslt ?}}",
	      text);
      break;
    default:
      diagnostics(ERROR, "Called CmdLabel with wrong Code %d", code);
    }
}


/******************************************************************************
     purpose : label
   parameter : code  kind of label
     globals : fRtf
               linenumber
 ******************************************************************************/
void
CmdLabelOld(int code, char *text)
{
  switch (code)
    {
    case LABEL:
      fprintf(fRtf,"{\\v[LABEL: %s]}",text);
      break;
    case REF:
    case HYPERREF:
      fprintf(fRtf,"{\\v[REF: %s]}",text);
      break;
    case PAGEREF:
    case HYPERPAGEREF:
      fprintf(fRtf,"{\\v[PAGEREF: %s]}",text);
      break;
    default:
      diagnostics(ERROR, "Called CmdLabel with wrong Code %d", code);
    }
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
  int test;

// tmpname = tempnam(getenv("TMPDIR"), "l2r");
// if ((fp = fopen(tempname,"w+")) == NULL)
  if ((fp = tmpfile()) == NULL)
  {
    fprintf(stderr,"%s: Fatal Error: cannot create temporary file\n", progname);
    exit(EXIT_FAILURE);
  }
  
  test = fwrite(string,strlen(string),1,fp);
  if(test != 1)
    diagnostics(WARNING, 
		"(ConvertString): "
		"Could not write `%s' to tempfile %s, "
		"fwrite returns %d, should be 1",
		string, "tmpfile()", test);
  if(fseek(fp,0L,SEEK_SET) != 0)
    diagnostics(ERROR, "Could not position to 0 in tempfile (ConvertString)");

  LatexFile = fTex;
  fTex = fp;
  oldlinenumber = linenumber;
  linenumber = 1;
  BracketLevel++;
  Convert();

  BracketLevel--;

  fTex = LatexFile;
  linenumber = oldlinenumber;
  if(fclose(fp) != 0)
    diagnostics(ERROR, "Could not close tempfile, (ConvertString).");
//  if(remove(tmpname) != 0)
//    diagnostics(ERROR, "Could not remove tempfile, (ConvertString).");
//  free(tmpname);
}



