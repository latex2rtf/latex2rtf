/*
 * $Id: funct1.c,v 1.10 2001/08/12 19:32:24 prahl Exp $
 * History:
 * $Log: funct1.c,v $
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
 * getParam allocates only the needed amount of memory
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
#include "parser.h"
/*****************************************************************************/

/**************************** extern variables ******************************/
extern bool bInDocument;                 /* true if File-Pointer is in the document */
extern int BracketLevel;                 /* counts open braces */
extern int RecursLevel;                  /* counts returns occured by closing braces */
extern bool g_processing_equation;                    /* true at a formula-convertion */
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
extern int curr_fontbold[MAXENVIRONS];
extern int curr_fontital[MAXENVIRONS];
extern int curr_fontscap[MAXENVIRONS];
extern int curr_fontnumb[MAXENVIRONS];
/***************************************************************************/

/***************************  prototypes     ********************************/
void ConvertFormula();
static void CmdLabel1_4(int code, char *text);
static void CmdLabelOld(int code, char *text);
static void RtfHeader(int where, /*@null@*/ char *what);
static void PlainPagestyle(void);
void CmdPagestyle(/*@unused@*/ int code);
void CmdHeader(int code);


/***************************************************************************/
void CmdBeginEnd(int code)
/***************************************************************************
   purpose: reads the parameter after the \begin or \end-command; ( see also getParam )
	    after reading the parameter the CallParamFunc-function calls the
	    handling-routine for that special environment
 parameter: code: CMD_BEGIN: start of environment
		  CMD_END:   end of environment
 ***************************************************************************/
{
  char *cParam = getParam();
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
    case TITLE_TITLE: title = getParam();
		      break;
    case TITLE_AUTHOR: TITLE_AUTHOR_ON = TRUE; /* is used for the \and command */
		       author = getParam();
		       TITLE_AUTHOR_ON = FALSE;
		       break;
    case TITLE_DATE: date = getParam();
		     break;
    case TITLE_MAKE:
      sprintf(title_begin,"%s%2d", "\\fs", (30*CurrentFontSize())/20);
      sprintf(author_begin,"%s%2d", "\\fs", (24*CurrentFontSize())/20);
      sprintf(date_begin,"%s%2d", "\\fs", (24*CurrentFontSize())/20);

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

static void setDocumentOptions(char *optionlist)
{
    char *option;

    option = strtok(optionlist, ",");
    
    while (option)
    {               
        diagnostics(4, "                    option   %s", option);
       if (strcmp(option,"11pt") == 0)
            SetDocumentFontSize(22);
        else if (strcmp(option,"12pt") == 0)
            SetDocumentFontSize(24);
        else if (strcmp(option,"german") == 0)
        { 
            GermanMode = TRUE; 
            PushEnvironment(GERMANMODE);
            language = strdup(option);
        }
        else if (strcmp(option,"spanish") == 0 || strcmp(option,"french") == 0)
        {
            language = strdup(option);
        }
        else if (strcmp(option,"twoside") == 0)
        { 
            twoside = TRUE;
            fprintf(stderr, "\n  rtf1.5 token `\\facingp' used");          /*LEG diag(1,)*/
            fprintf(fRtf,"\\facingp");
        }
        else if (strcmp(option,"twocolumn") == 0)
        {
            fprintf(fRtf,"\\cols2\\colsx709 "); /* two columns -- space between columns 709 */
            twocolumn = TRUE;
        }
        else if (strcmp(option,"titlepage") == 0)
        {
            titlepage = TRUE;
        }
        else if (strcmp(option,"isolatin1") == 0)
        {
            TexCharSet = ISO_8859_1;
            fprintf(stderr,"\nisolatin1 style option encountered.");
            fprintf(stderr,"\nLatin-1 (= ISO 8859-1) special characters will be ");
            fprintf(stderr,"converted into RTF-Commands!\n");
        }
        else if (strcmp(option, "hyperlatex") == 0)
        {
            PushEnvironment(HYPERLATEX);
        }
        else if (!TryVariableIgnore(option, fTex))
        {
            fprintf(stderr,"\n%s: WARNING: unknown style option %s ignored", progname, option);
        }
        
        option = strtok(NULL, ",");
    }
}

void CmdDocumentStyle(/*@unused@*/ int code)
/******************************************************************************
 purpose: reads the information from the LaTeX \documentstyle command and
	  converts it to a similar Rtf-information
 globals : fRtf
           article: (article, report, bookstyle)
           GermanMode: support for germanstyle
           twocolumn; titlepage
 ******************************************************************************/
{
    char format[100];
    char optionlist[100]; 
    
    getBracketParam(optionlist,99);
    getBraceParam(format,99);
  
    diagnostics(4, "Documentstyle/class options [%s]", optionlist);
    diagnostics(4, "Documentstyle/class format  {%s}", format);
    
    setDocumentOptions(optionlist);
        
    if (strcmp(format,"article") == 0)
        article = TRUE;
    else
        article = FALSE;
 }

/******************************************************************************
 CmdUsepackage:
 purpose: reads the LaTex2e-usepackage-command parameters and
	  converts it to a similar Rtf-information
	  styled after CmdDocumentStyle
 globals : fRtf
           article: (article, report, bookstyle)
           GermanMode: support for germanstyle
           twocolumn; titlepage
 ******************************************************************************/
void CmdUsepackage(/*@unused@*/ int code)
{
  char package[100];
  char optionlist[100]; 
 
  getBracketParam(optionlist,99);
  getBraceParam(package,99);
  
  diagnostics(4, "Package {%s} with options [%s] encountered", package, optionlist);
  
  if (strcmp(package, "inputenc") == 0 || strcmp(package, "babel") == 0)
      setDocumentOptions(optionlist);
  else
      setDocumentOptions(package);
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

  getBracketParam(optparam,99);
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
    
    cNext=getTexChar();
    cNext=getNonBlank();
    rewind_one(cNext);
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

       cNext=getTexChar();
       skipSpaces();
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
       cNext=getTexChar();
       skipSpaces();
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
       cNext=getTexChar();
       skipSpaces();
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
       cNext=getTexChar();
       skipSpaces();
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
       cNext=getTexChar();
       skipSpaces();
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
       cNext=getTexChar();
       skipSpaces();
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
          DefFont
 ******************************************************************************/
{
  char number[255];
  static int thankno = 1;
  int text_ref_upsize, foot_ref_upsize;
  
  getBracketParam(number,254); /* is ignored because of the automatic footnumber-generation */

  text_ref_upsize = (6*CurrentFontSize())/20;
  foot_ref_upsize = (6*CurrentFontSize())/20;

  if (code == THANKS)
  {
     fprintf(fRtf,"{\\up%d %d}",text_ref_upsize,thankno++);
     fprintf(fRtf,"{\\up%d %d}\n{\\*\\footnote \\pard\\plain\\q%c ",foot_ref_upsize,thankno,alignment);
     fprintf(fRtf,"{\\s246 \\f%u \\fs%d ",(unsigned int)DefFont,CurrentFontSize());
     Convert();
     fprintf(fRtf,"}\n ");
  }
  else
  {
     fprintf(fRtf,"{\\up%d\\chftn }",text_ref_upsize);
     fprintf(fRtf,"{\\up%d\\chftn}\n{\\*\\footnote \\pard\\plain\\q%c ",foot_ref_upsize,alignment);
     fprintf(fRtf,"\\s246 \\f%ld \\fs%d ",DefFont,CurrentFontSize());
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
  style = getParam();
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
  
  if (getBracketParam(itemlabel,99)) /* \item[label] */
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
  fTex = fp;         		
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


  while (cThis=getTexChar())
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

  while (cThis=getTexChar())
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
    cThis=getTexChar();
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
  while (cThis=getTexChar())
  {
    if (cThis == '{')
      break;
  }
  if (cThis != '{')
    numerror(ERR_EOF_INPUT);
  bracket = 1;
  while (cThis=getTexChar())
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
  cThis=getTexChar();

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

  while (cThis=getTexChar())
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
    
  skipSpaces();
    
  /* skip non-space characters */
  while ((cThis=getTexChar()) && cThis != ' ');
  
  skipSpaces();
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
 * returns 1 if target is found otherwise 0
 ************************************************************************/

int ScanAux(char *token, char* reference, int code)
{
  static FILE* fAux = NULL;
  static char openbrace = '{';
  static char closebrace = '}';
  char tokenref[255];      /* should really be allocated dynmically */
  char AuxLine[1024];
  char *s;
      
  if (g_aux_file_missing || strlen(reference)==0)
  {
    fprintf(fRtf, "?");
    return 0;
  }
  
  if (fAux == NULL && (fAux = fopen(AuxName,"r")) == NULL)   /* open .aux file if not opened before */
  {
          fprintf(stderr,"Error opening AUX-file: %s\n", AuxName);
          fprintf(stderr,"Try running LaTeX first\n");
          g_aux_file_missing = TRUE;
          fprintf(fRtf, "?");
	  return 0;
  }

  rewind(fAux);
  sprintf(tokenref, "\\%s{%s}", token, reference);
  
  while (fgets (AuxLine, 1023, fAux) != NULL)
  {
      
      if ( (s=strstr (AuxLine, tokenref)) != NULL)
      {
          s += strlen(tokenref);

          for (; code>=0; code--)          /* Do once or twice depending on code */
          {
              s = strchr(s, openbrace);
              if (s==NULL)                 /* no parameter found, just print '?' and return */
              {
                fprintf(fRtf, "?");
                return 0;
              }
              s++;                         
          }
          
	  while (*s != closebrace && *s != '\0')  /* print the number and exit */
	      fprintf (fRtf, "%c", *s++);

	  return 1;
      }
  }
  
  fprintf(stderr, "Warning: \\%s{%s} not found in %s\n", token, reference, AuxName);
  fprintf(fRtf, "?");
  return 0;
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
    text = getParam();
    rewind_one(text[strlen(text)-1]);   /*somewhat screwy */
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

  cThis = getTexChar();

  cThis = getNonSpace();
  
  cThis = getTexChar();

  if (cThis != '\n')
    rewind_one(cThis);

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

/******************************************************************************/
void IgnoreNewCmd(/*@unused@*/ int code)
/******************************************************************************
     purpose : ignore \newcmd
   parameter : code  not used 
     globals : fTex
 ******************************************************************************/
{
  char cThis;
  
/* ignore first '{' */
  cThis = getTexChar();
  rewind_one(cThis);
  
  if (cThis == '\\')
    CmdIgnoreDef(0);
  else
    CmdIgnoreParameter(No_Opt_Two_NormParam );
}


