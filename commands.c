/*
 * $Id: commands.c,v 1.5 2001/08/12 17:50:50 prahl Exp $
 * History:
 * $Log: commands.c,v $
 * Revision 1.5  2001/08/12 17:50:50  prahl
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
 * Revision 1.7  1998/10/28 06:30:53  glehner
 * changed thebibliography from CmdIgnore... to CmdConvertBiblio.
 *
 * Revision 1.6  1998/07/03 06:59:05  glehner
 * LaTeX2e support, documentclass, usepackage
 * added ignoring environment
 *
 * Revision 1.5  1997/02/15 21:34:50  ralf
 * Fixed bug in german quotes as corrected by Oliver Moldenhauer
 *
 * Revision 1.4  1997/02/15 20:42:58  ralf
 * Corrected some declarations found by lclint.
 *
 * Revision 1.3  1995/05/24 15:35:32  ralf
 * Changes by Vladimir Menkov for DOS port
 * Added include of stdlib.h
 *
 * Revision 1.2  1995/03/23  15:58:08  ralf
 * Reworked version by Friedrich Polzer and Gerhard Trisko
 *
 * Revision 1.1  1994/06/17  11:26:29  ralf
 * Initial revision
 */
/***************************************************************************
     name : commands.c
   author : DORNER Fernando, GRANZER Andreas
            POLZER Friedrich,TRISKO Gerhard
  * \centerline, \c, commands for citation, table & title
  purpose : includes all LaTex-Commands which can be converted into Rtf-Commands
 ******************************************************************************/

/****************************** includes ************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cfg.h"
#include "main.h"
#include "funct1.h"
#include "commands.h"
#include "funct2.h"
#include "parser.h"
/****************************************************************************/
void error(char *);
/*********************** typedefs *******************************************/
typedef struct commandtag {
    char *cpCommand;	 	 /* LaTeX-commandname without \ */
    /*@null@*/void (*func)(int); /* function converting LaTex-cmd to Rtf-cmd */
    int param;		 	 /* used in various ways */
} CommandArray;
/****************************************************************************/

/***************** file-global variables ************************************/
static CommandArray *Environments[100]; /* list of active environments */
static int iEnvCount = 0;               /* number of active environments */

/********************************************************************/
/* commands for environment document */
/********************************************************************/
static CommandArray commands[] = {
   { "begin", CmdBeginEnd, CMD_BEGIN },
   { "end", CmdBeginEnd, CMD_END },
   { "today", CmdToday, 0 },
   { "footnote", CmdFootNote, FOOTN },
   { "bf", CmdCharFormat, CMD_BOLD },
   { "it", CmdCharFormat, CMD_ITALIC },
   { "sc", CmdCharFormat, CMD_CAPS },
/* ------------------------------------------ */
   { "centerline", Paragraph, PAR_CENTERLINE },
   { "c", CmdC, 0},
/* ------------------------------------------ */
   { "underline", CmdCharFormat, CMD_UNDERLINE },
   { "textit", CmdCharFormat, CMD_ITALIC },
   { "mathit", CmdCharFormat, CMD_ITALIC},
   { "mathbf", CmdCharFormat, CMD_BOLD},
   { "textbf", CmdCharFormat, CMD_BOLD },
   { "textsc", CmdCharFormat, CMD_CAPS },
   { "tiny", CmdFontSize, 10 },
   { "scriptsize", CmdFontSize, 12 },
   { "footnotesize", CmdFontSize, 14 },
   { "small", CmdFontSize, 16 },
   { "normalsize", CmdFontSize, 20 },
   { "large", CmdFontSize, 25 },
   { "Large", CmdFontSize, 30 },
   { "LARGE", CmdFontSize, 40 },
   { "huge", CmdFontSize, 50 },
   { "Huge", CmdFontSize, 60 },
   { "LaTeX", CmdLogo, CMD_LATEX },
   { "TeX", CmdLogo, CMD_TEX },
   { "SLiTeX", CmdLogo, CMD_SLITEX },
   { "BibTeX", CmdLogo, CMD_BIBTEX },
   { "acute", CmdLApostrophChar, 0 },
   { "grave", CmdRApostrophChar, 0 },
   { "hat", CmdRApostrophChar, 0 },
   { "tilde", CmdTildeChar, 0 },
   { "ldots", CmdLdots, 0 },
   { "em", CmdEmphasize, 0},
   { "maketitle", CmdTitle, TITLE_MAKE },
   { "section", CmdSection, SECT_NORM },
   { "section*", CmdSection, SECT_NORM },
   { "caption", CmdSection, SECT_CAPTION },
   { "chapter", CmdSection, SECT_CHAPTER },
   { "subsection", CmdSection, SECT_SUB },
   { "subsection*", CmdSection, SECT_SUB },
   { "subsubsection", CmdSection, SECT_SUBSUB },
   { "subsubsection*", CmdSection, SECT_SUBSUB },
   { "part", CmdSection, SECT_PART },
   { "appendix",CmdIgnore,0},
   { "protect", CmdIgnore,0},
   { "paragraph", CmdSection, SECT_SUB },
   { "subparagraph", CmdSection, SECT_SUBSUB },
   { "clearpage", CmdNewPage, NewPage },
   { "cleardoublepage", CmdNewPage, NewPage },
   { "newpage", CmdNewPage, NewColumn },
   { "mbox", CmdMbox, 0},
   { "hbox", CmdMbox, 0},
   { "frenchspacing", CmdIgnore, 0},
   { "nonfrenchspacing", CmdIgnore, 0},
   { "rm", CmdSetFont, F_ROMAN},		     /* set font Roman */
   { "sl", CmdSetFont, F_SLANTED},		     /* set font Slanted */
   { "sf", CmdSetFont, F_SANSSERIF},		     /* set font Sans Serif */
   { "tt", CmdSetFont, F_TYPEWRITER},		     /* set font Typewriter */
   { "mathrm", CmdSetFont, F_ROMAN},		     /* set font Roman */
   { "include", CmdInclude, 0}, 		     /* include Latex file */
   { "input", CmdInclude, 0},			     /* include Latex file */
   { "verb", CmdVerb, 0},
   { "verb*", CmdVerb, AST_FORM},
   { "onecolumn", CmdColumn, One_Column },
   { "twocolumn", CmdColumn, Two_Column },
   { "flushbottom", CmdBottom, 0},
   { "raggedbottom", CmdBottom, 0},
   { "includegraphics", CmdGraphics, 0},
   { "includegraphics*", CmdGraphics, 0},
   { "setlength", CmdIgnoreParameter, No_Opt_Two_NormParam },
   { "footnotemark", CmdIgnoreParameter, One_Opt_No_NormParam },
   { "footnotetext", CmdIgnoreParameter, One_Opt_One_NormParam },
   { "newlength", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "label", CmdLabel, LABEL },
   { "ref", CmdLabel, REF },
   { "pageref", CmdLabel, PAGEREF },
   { "bibliography", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "newsavebox", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "usebox", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "fbox", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "hspace", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "hspace*", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "vspace", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "vspace*", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "addvspace", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "addcontentsline", CmdIgnoreParameter, No_Opt_Three_NormParam },
   { "addcontents", CmdIgnoreParameter, No_Opt_Two_NormParam },
   { "numberline", CmdIgnoreParameter, No_Opt_Two_NormParam },
   { "stretch", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "hyphenation", CmdHyphenation, 0 },
   { "typeaout", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "index", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "indexentry", CmdIgnoreParameter, No_Opt_Two_NormParam},
   { "glossary", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "glossaryentry", CmdIgnoreParameter, No_Opt_Two_NormParam },
   { "typeout", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "Typein", CmdIgnoreParameter, One_Opt_One_NormParam },
   { "includeonly", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "nocite", CmdNoCite, No_Opt_One_NormParam },
   { "bibliography", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "stepcounter", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "refstepcounter", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "fnsymbol", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "Alph", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "alph", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "Roman", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "roman", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "arabic", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "value", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "def", CmdIgnoreDef, 0},
   { "newcommand", IgnoreNewCmd, 0 },
   { "renewcommand", IgnoreNewCmd,0 },
   { "newenvironment", CmdIgnoreParameter, One_Opt_Three_NormParam },
   { "renewenvironment", CmdIgnoreParameter, One_Opt_Three_NormParam },
   { "newtheorem", CmdIgnoreParameter, One_Opt_Two_NormParam },
   { "renewtheorem", CmdIgnoreParameter, One_Opt_Two_NormParam },
   { "newcount", CmdIgnoreDef, 0 },
   { "output", CmdIgnoreDef, 0 },
   { "newcounter", CmdIgnoreParameter, One_Opt_One_NormParam },
   { "value", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "makebox", CmdIgnoreParameter, Two_Opt_One_NormParam },
   { "framebox", CmdIgnoreParameter, Two_Opt_One_NormParam },
   { "sbox", CmdIgnoreParameter, No_Opt_Two_NormParam },
   { "savebox", CmdIgnoreParameter, Two_Opt_Two_NormParam },
   { "parbox", CmdIgnoreParameter, One_Opt_Two_NormParam },
   { "rule", CmdIgnoreParameter, One_Opt_Two_NormParam },
   { "raisebox", CmdIgnoreParameter, Two_Opt_Two_NormParam },
   { "newfont", CmdIgnoreParameter, No_Opt_Two_NormParam },
   { "addtolength", CmdIgnoreParameter, No_Opt_Two_NormParam },
   { "settowidth", CmdIgnoreParameter, No_Opt_Two_NormParam },
   { "pagebreak", CmdIgnoreParameter, One_Opt_No_NormParam },
   { "nopagebreak", CmdIgnoreParameter, One_Opt_No_NormParam },
   { "samepage", CmdIgnore, 0 },
   { "linebreak", CmdIgnoreParameter, One_Opt_No_NormParam },
   { "nolinebreak", CmdIgnoreParameter, One_Opt_No_NormParam },
   { "typein", CmdIgnoreParameter, One_Opt_One_NormParam },
   { "cite", CmdCite, 0 },
   { "marginpar", CmdIgnoreParameter, One_Opt_One_NormParam },
   { "caption", CmdIgnoreParameter, One_Opt_One_NormParam },
   { "addtocounter", CmdIgnoreParameter, No_Opt_Two_NormParam },
   { "setcounter", CmdIgnoreParameter, No_Opt_Two_NormParam },
   { "newcounter", CmdIgnoreParameter, One_Opt_One_NormParam },
   { "baselineskip", Cmd_OptParam_Without_braces, 0 },
   { "lineskip", Cmd_OptParam_Without_braces, 0 },
   { "vsize", Cmd_OptParam_Without_braces, 0 },
   { "setbox", Cmd_OptParam_Without_braces, 0 },
   { "pagestyle", CmdIgnoreParameter, No_Opt_One_NormParam }, /* pagestyle is ignored because there is a variable calculation
								 in the RTF-header which can't be converted by reading the LATEX-File. */
   { "pagenumbering", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "markboth", CmdIgnoreParameter, No_Opt_Two_NormParam},
   { "markright", CmdIgnoreParameter, No_Opt_One_NormParam},
   { "thanks", CmdFootNote, THANKS },
   { "bibliographystyle", CmdIgnoreParameter,No_Opt_One_NormParam },
   { "let", CmdIgnoreLet, 0},
   { "cline",CmdIgnoreParameter,No_Opt_One_NormParam},
/* begin pftg 23.11.94 ----------------------------------- */
   { "title", CmdTitle, TITLE_TITLE },
   { "author", CmdTitle, TITLE_AUTHOR },
   { "date", CmdTitle, TITLE_DATE },
/* end pftg ---------------------------------- */
   { "multicolumn", CmdMultiCol, 0 },
   { "newcommand", CmdIgnoreParameter, 13 }, /* one optional 3 required */
   { "newenvironment", CmdIgnoreParameter, 13 }, /* ditto */
   { "newtheorem", CmdIgnoreParameter, 12 }, /* one optional two required */
   { "newcounter", CmdIgnoreParameter, 11 }, /* one optional one required */
   { "", NULL, 0 }
};

/********************************************************************/
/* commands for environment LaTeX-header */
/********************************************************************/
static CommandArray HeaderCommands[] = {
  { "documentclass", CmdDocumentStyle, 0 },
  { "usepackage", CmdUsepackage, 0 },
   { "documentstyle", CmdDocumentStyle, 0 },
   { "setlength", CmdIgnoreParameter, No_Opt_Two_NormParam },
   { "pagestyle", CmdIgnoreParameter, No_Opt_One_NormParam }, /* pagestyle is ignored because there is a variable calculation
								 in the RTF-header which can't be converted by reading the LATEX-File. */
   { "pagenumbering", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "markboth", CmdIgnoreParameter, No_Opt_Two_NormParam},
   { "markright", CmdIgnoreParameter, No_Opt_One_NormParam},
   { "newlength", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "title", CmdTitle, TITLE_TITLE },
   { "author", CmdTitle, TITLE_AUTHOR },
   { "date", CmdTitle, TITLE_DATE },
   { "begin", CmdBeginEnd, CMD_BEGIN },
   { "newcommand", CmdIgnoreParameter, One_Opt_Two_NormParam },
   { "renewcommand", CmdIgnoreParameter, One_Opt_Two_NormParam },
   { "def", CmdIgnoreDef, 0},
   { "flushbottom", CmdBottom, 0},
   { "raggedbottom", CmdBottom, 0},
   { "thanks", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "address" , CmdAddress, 0 },
   { "signature", CmdSignature, 0},
   { "opening", CmdOpening, 0},
   { "closing", CmdClosing, 0},
   { "ps", CmdPs, 0},
   { "nonumber", CmdFormula, FORM_NO_NUMBER },
   { "", NULL, 0 }
};    /* end of list */

/********************************************************************/
/* commands for environment Itemize */
/********************************************************************/
static CommandArray ItemizeCommands[] = {
   { "item", CmdItem, ITEMIZE },
   { "", NULL, 0 }
};    /* end of list */

/********************************************************************/
/* commands for environment Description */
/********************************************************************/
static CommandArray DescriptionCommands[] = {
   { "item", CmdItem, DESCRIPTION },
   { "", NULL, 0 }
};    /* end of list */

/********************************************************************/
/* commands for environment Enumerate */
/********************************************************************/
static CommandArray EnumerateCommands[] = {
   { "item", CmdItem, ENUMERATE },
   { "", NULL, 0 }
};    /* end of list */

/********************************************************************/
/* commands for environment tabbing */
/********************************************************************/
static CommandArray TabbingCommands[] = {
    { "kill" , CmdTabkill, 0 },  /* a line that ends with a \kill command produces no output */
    { "", NULL, 0 }
};   /* end of list */

/********************************************************************/
/* commands for environment letter */
/********************************************************************/
static CommandArray LetterCommands[] = {
    { "address" , CmdAddress, 0 },
    { "signature", CmdSignature, 0},
    { "opening", CmdOpening, 0},
    { "closing", CmdClosing, 0},
    { "ps", CmdPs, 0},
    { "", NULL, 0 }
};   /* end of list */

/********************************************************************/
/* commands for German Mode */
/********************************************************************/
static CommandArray GermanModeCommands[] = {
 /*   { "ck" , CmdPrintRtf , (int)"ck" },
    { "glqq" , CmdPrintRtf , (int)"\\ldblquote "},
    { "glq" , CmdPrintRtf , (int)"\\lquote "},
    { "grqq" , CmdPrintRtf , (int)"\\rdblquote "},
    { "grq" , CmdPrintRtf , (int)"\\rquote "}, */
    { "ck", GermanPrint, GP_CK},
    { "glqq", GermanPrint, GP_LDBL},
    { "glq", GermanPrint, GP_L},
    { "grq", GermanPrint, GP_R},
    { "grqq", GermanPrint, GP_RDBL},
    { "", NULL, 0 }
};   /* end of list */

/********************************************************************/
/* commands for begin-end environments */
/* only strings used in the form \begin{text} or \end{text} */
/********************************************************************/
static CommandArray params[] = {
   { "center", Paragraph, PAR_CENTER },
   { "flushright", Paragraph, PAR_RIGHT },
   { "flushleft", Paragraph, PAR_LEFT},
   { "document", Environment, DOCUMENT },
   { "tabbing", Tabbing, TABBING },
   { "figure", CmdTable, FIGURE },
   { "figure*", CmdTable, FIGURE_1 },
   { "picture", CmdIgnoreFigure, PICTURE },
   { "minipage", CmdIgnoreFigure, MINIPAGE },
   /*{ "thebibliography", CmdIgnoreFigure, THEBIBLIOGRAPHY },*/
   { "em", Format, EMPHASIZE },
   { "quote", CmdQuote, QUOTE },
   { "quotation", CmdQuote, QUOTATION },
   { "enumerate", CmdList, ENUMERATE },
   { "list", CmdList, ITEMIZE },
   { "itemize", CmdList, ITEMIZE },
   { "description", CmdList, DESCRIPTION },
   { "verbatim", CmdVerbatim, 0 },
   { "verse", CmdVerse, 0 },
   { "tabular", CmdTabular, TABULAR },
   { "tabular*", CmdTabular, TABULAR_1 },
   { "longtable", CmdTabular, TABULAR },
   { "longtable*", CmdTabular, TABULAR_1 },
   { "array", CmdTabular, TABULAR_2},
   
   { "multicolumn", CmdMultiCol, 0 },
   { "math", CmdFormula, FORM_MATH },
   { "displaymath" , CmdFormula2, FORM_DOLLAR },
   { "equation", CmdFormula2, EQUATION },
   { "equation*", CmdFormula2, EQUATION_1 },
   { "eqnarray*" , CmdFormula2, EQNARRAY_1 },
   { "eqnarray", CmdFormula2, EQNARRAY },
   { "letter", CmdLetter, 0 },
   { "table", CmdTable, TABLE },
   { "table*", CmdTable, TABLE_1 },
   { "thebibliography", CmdConvertBiblio, 0 },
   /*   { "thebibliography", CmdIgnoreEnvironment, BIBLIOGRAPHY },*/
   { "abstract", CmdAbstract, 0},
   { "titlepage", CmdTitlepage, 0},
   { "", NULL, 0 }
};    /* end of list */


/********************************************************************/
/* commands for hyperlatex package */
/* */
/********************************************************************/
static CommandArray hyperlatex[] = {
   { "link", CmdLink, 0 },
   { "xlink", CmdLink, 0 },
   { "Cite", CmdCite, HYPERLATEX },
   { "Ref", CmdLabel, HYPERREF },
   { "Pageref", CmdLabel, HYPERPAGEREF },
   { "S", CmdColsep, 0 },
   { "", NULL, 0 }
};    /* end of list */


/****************************************************************************/
bool CallCommandFunc(char *cCommand)
/****************************************************************************
purpose: The tries to call the command-function for the commandname
params:  string with command name
returns: sucess or not
globals: command-functions have side effects or recursive calls
 ****************************************************************************/
{
  int i=0, j;

/* ------------------------------------
fprintf(stderr,"%s %d\n ",cCommand,iEnvCount-1);
 ------------------------------------ */
  for (j = iEnvCount-1; j >= 0; j--, i = 0)
  {
    while (strcmp(Environments[j][i].cpCommand,"")!= 0)
    {
/* ------------------------------------ 
fprintf(stderr,"%s ",Environments[j][i].cpCommand);
 ------------------------------------ */
      if (strcmp(Environments[j][i].cpCommand,cCommand)== 0)
      {
	if (Environments[j][i].func == NULL)
	  return FALSE;
	if(*Environments[j][i].func == CmdIgnoreParameter) {
	  diagnostics (4, "Ignoring command %s.", cCommand);
	}
	(*Environments[j][i].func)((Environments[j][i].param));
	return TRUE; /* Command Function found */
      }
      ++i;
    }
  }
  return FALSE;
}


/****************************************************************************/
bool CallParamFunc(char *cCommand, int AddParam)
/****************************************************************************
purpose: The tries to call the environment-function for the commandname
params:  cCommand - string with command name
	 AddParam - param "ORed"(||) to the int param of command-funct
returns: sucess or not
globals: command-functions have side effects or recursive calls
 ****************************************************************************/
{
  int i=0;
  char unknown_environment[100];

  while (strcmp(params[i].cpCommand,"")!= 0)
  {
    if (strcmp(params[i].cpCommand,cCommand)== 0)
      {
	 assert(params[i].func != NULL);
	 (*params[i].func)((params[i].param)|AddParam);
	 return TRUE;	 /* command function found */
      }
    ++i;
  }

  /* unknown environment must be ignored !!! */
  /* ========================================*/
  if (AddParam == ON)
      {
      sprintf(unknown_environment,"%s%s%s","end{",cCommand,"}");
/*      Ignore_Environment(unknown_environment); */
      fprintf(stderr,"\n%s: WARNING: Environment \"%s\" ignored, because it's not defined in the command-list\n",progname,cCommand);
      }

  return FALSE;
}


/****************************************************************************/
void PushEnvironment(int code)
/****************************************************************************
purpose: adds the command list for a specific environment to the list
	 of commands searched through.
params:  constant identifying the environment
globals: changes Environment - array of active environments
		 iEnvCount   - counter of active environments
 ****************************************************************************/
{
  char *diag = "";

  switch(code)
    {
    case HEADER: Environments[iEnvCount++]=HeaderCommands;
      diag = "preample";
      break;
    case DOCUMENT:
      Environments[iEnvCount++]=commands;
      if (GermanMode)
	Environments[iEnvCount++]=GermanModeCommands;
      diag = "document";
      break;
    case ITEMIZE:  Environments[iEnvCount++]=ItemizeCommands;
      diag = "itemize";
      break;
    case ENUMERATE: Environments[iEnvCount++]=EnumerateCommands;
      diag = "enumerate";
      break;
    case TABBING : Environments[iEnvCount++]=TabbingCommands;
      diag = "tabbing";
      break;
    case LETTER : Environments[iEnvCount++]=LetterCommands;
      diag = "letter";
      break;
    case DESCRIPTION : Environments[iEnvCount++]=DescriptionCommands;
      diag = "description";
      break;
    case GERMANMODE : Environments[iEnvCount++]=GermanModeCommands;
      diag = "?german?";
      break;
      /*LEG160698 Maybe we should push "nothing" 
	but I think it doesn't harm*/
    case IGN_ENV_CMD: Environments[iEnvCount++]=commands;
      diag = "*latex2rtf ignored*";
      break;
    case HYPERLATEX: Environments[iEnvCount++]=hyperlatex;
      diag = "hyperlatex";
      break;
      
    default: error("assertion failed at function PushEnvironment");
    }
  
  diagnostics(4, "Entering %s environment, Depth: %d.", diag, iEnvCount);
  
}


/****************************************************************************/
void PopEnvironment()
/****************************************************************************
purpose: removes the environment-commands list added by the last call to
	 PushEnvironemnt;
globals: changes Environment - array of active environments
		 iEnvCount - counter of active environments
 ****************************************************************************/
{
  diagnostics(4, "Leaving environment, Depth: %d", iEnvCount);

  --iEnvCount;
  Environments[iEnvCount]=NULL;

     /* overlapping environments are not allowed !!!
	example: \begin{verse}\begin{enumerate}\end{verse}\end{enumerate}
	==> undefined result
	extension possible
     */

  return ;
}


/****************************************************************************/
void ClearEnvironment()
/****************************************************************************
purpose: removes all environment-commands lists
globals: changes Environment - array of active environments
		 iEnvCount - counter of active environments
 ****************************************************************************/
{
  Environments[0] = NULL;
  iEnvCount = 0;
  return;
}
