/*
 * $Id: commands.c,v 1.1 2001/08/12 15:32:17 prahl Exp $
 * History:
 * $Log: commands.c,v $
 * Revision 1.1  2001/08/12 15:32:17  prahl
 * Initial revision
 *
 * Revision 1.1  1994/06/17  11:26:29  ralf
 * Initial revision
 *
 */
/***************************************************************************
     name : commands.c
   author : DORNER Fernando, GRANZER Andreas
  purpose : includes all LaTex-Commands which can be converted into Rtf-Commands
 ******************************************************************************/

/****************************** includes ************************************/
#include <stdio.h>
#include "main.h"
#include "funct1.h"
#include "commands.h"
#include "funct2.h"
/****************************************************************************/
void error(char *);
/*********************** typedefs *******************************************/
typedef struct commandtag
	  { char *cpCommand;		 /* LaTeX-commandname without \ */
	    void (*func)(int);		 /* name of function which converts an LaTex-command to an Rtf-command */
	    int param;			 /* optional parameter can be used in various ways */
	  } CommandArray;
/****************************************************************************/

/********************** global variables ************************************/
CommandArray *Environments[100] = {NULL}; /* list of active environments */
int iEnvCount = 0;			 /* number of active environments */
extern char *progname;
extern BOOL GermanMode;

CommandArray commands[300] = {
/********************************************************************/
/* commands for environment document */
/********************************************************************/
   { "begin", CmdBeginEnd, CMD_BEGIN },
   { "end", CmdBeginEnd, CMD_END },
   { "today", CmdToday, 0 },
   { "footnote", CmdFootNote, 0 },
   { "bf", CmdCharFormat, CMD_BOLD },
   { "it", CmdCharFormat, CMD_ITALIC },
   { "underline", CmdCharFormat, CMD_UNDERLINE },
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
   { "caption", CmdSection, SECT_SUB },
   { "chapter", CmdSection, SECT_SUB },
   { "subsection", CmdSection, SECT_SUB },
   { "subsection*", CmdSection, SECT_SUB },
   { "subsubsection", CmdSection, SECT_SUBSUB },
   { "subsubsection*", CmdSection, SECT_SUBSUB },
   { "part", CmdSection, SECT_SUBSUB },
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
   { "include", CmdInclude, 0}, 		     /* include Latex file */
   { "input", CmdInclude, 0},			     /* include Latex file */
   { "verb", CmdVerb, 0},
   { "verb*", CmdVerb, AST_FORM},
   { "onecolumn", CmdColumn, One_Column },
   { "twocolumn", CmdColumn, Two_Column },
   { "flushbottom", CmdBottom, 0},
   { "raggedbottom", CmdBottom, 0},
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
   { "nocite", CmdIgnoreParameter, No_Opt_One_NormParam },
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
   { "cite", CmdIgnoreParameter, One_Opt_One_NormParam },
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
   { "thanks", CmdIgnoreParameter, No_Opt_One_NormParam },
   { "bibliographystyle", CmdIgnoreParameter,No_Opt_One_NormParam },
   { "let", CmdIgnoreLet, 0},
   { "cline",CmdIgnoreParameter,No_Opt_One_NormParam},
   { "", NULL } };

CommandArray HeaderCommands[100] = {
/********************************************************************/
/* commands for environment LaTeX-header */
/********************************************************************/
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
   { "", NULL } };    /* end of list */

CommandArray ItemizeCommands[10] = {
/********************************************************************/
/* commands for environment Itemize */
/********************************************************************/
   { "item", CmdItem, ITEMIZE },
   { "", NULL } };    /* end of list */

CommandArray DescriptionCommands[10] = {
/********************************************************************/
/* commands for environment Description */
/********************************************************************/
   { "item", CmdItem, DESCRIPTION },
   { "", NULL } };    /* end of list */

CommandArray EnumerateCommands[10] = {
/********************************************************************/
/* commands for environment Enumerate */
/********************************************************************/
   { "item", CmdItem, ENUMERATE },
   { "", NULL } };    /* end of list */

CommandArray TabbingCommands[10] = {
/********************************************************************/
/* commands for environment tabbing */
/********************************************************************/
    { "kill" , CmdTabkill, 0 },  /* a line that ends with a \kill command produces no output */
    { "", NULL } };   /* end of list */

CommandArray LetterCommands[10] = {
/********************************************************************/
/* commands for environment letter */
/********************************************************************/
    { "address" , CmdAddress, 0 },
    { "signature", CmdSignature, 0},
    { "opening", CmdOpening, 0},
    { "closing", CmdClosing, 0},
    { "ps", CmdPs, 0},
    { "", NULL } };   /* end of list */

CommandArray GermanModeCommands[10] = {
/********************************************************************/
/* commands for German Mode */
/********************************************************************/
 /*   { "ck" , CmdPrintRtf , (int)"ck" },
    { "glqq" , CmdPrintRtf , (int)"\\ldblquote "},
    { "glq" , CmdPrintRtf , (int)"\\lquote "},
    { "grqq" , CmdPrintRtf , (int)"\\rdblquote "},
    { "grq" , CmdPrintRtf , (int)"\\rquote "}, */
    { "ck", GermanPrint, GP_CK},
    { "glqq", GermanPrint, GP_LDBL},
    { "glq", GermanPrint, GP_L},
    { "grqq", GermanPrint, GP_R},
    { "grq", GermanPrint, GP_RDBL},
    { "", NULL } };   /* end of list */

CommandArray params[100] = {
/********************************************************************/
/* commands for begin-end environments */
/* only strings used in the form \begin{text} or \end{text} */
/********************************************************************/
   { "center", Paragraph, PAR_CENTER },
   { "flushright", Paragraph, PAR_RIGHT },
   { "flushleft", Paragraph, PAR_LEFT},
   { "document", Environment, DOCUMENT },
   { "tabbing", Tabbing, TABBING },
   { "figure", CmdIgnoreFigure, FIGURE },
   { "figure*", CmdIgnoreFigure, FIGURE_1 },
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
   { "math", CmdFormula2, 0 },
   { "displaymath" , CmdFormula2, 0 },
   { "equation", CmdFormula2, 0 },
   { "letter", CmdLetter, 0 },
   { "table", CmdTable, TABLE },
   { "table*", CmdTable, TABLE_1 },
   { "caption", CmdSection, SECT_SUB },
   { "thebibliography", CmdIgnoreEnvironment, BIBLIOGRAPHY },
   { "abstract", CmdAbstract, 0},
   { "titlepage", CmdTitlepage, 0},
   { "array", CmdArray, ARRAY },
   { "eqnarray", CmdArray, EQNARRAY },
   { "eqnarray*", CmdArray, EQNARRAY_1},
   { "", NULL } };    /* end of list */



/****************************************************************************/
BOOL CallCommandFunc(char *cCommand)
/****************************************************************************
purpose: The tries to call the command-function for the commandname
params:  string with command name
returns: sucess or not
globals: command-functions have side effects or recursive calls
 ****************************************************************************/
{
  int i=0, j;

  for (j = iEnvCount-1; j >= 0; j--, i = 0)
  {
    while (strcmp(Environments[j][i].cpCommand,"")!= 0)
    {
      if (strcmp(Environments[j][i].cpCommand,cCommand)== 0)
      {
	if (Environments[j][i].func == NULL)
	  return FALSE;
	(*Environments[j][i].func)((Environments[j][i].param));
	return TRUE; /* Command Function found */
      }
      ++i;
    }
  }
  return FALSE;
}


/****************************************************************************/
BOOL CallParamFunc(char *cCommand, int AddParam)
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
      Ignore_Environment(unknown_environment);
      fprintf(stderr,"\n%s: WARNING: Environment %s ignored, because it's not defined in the command-list\n",progname,cCommand);
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
  switch(code)
  {
    case HEADER: Environments[iEnvCount++]=HeaderCommands;
		 break;
    case DOCUMENT:
		   Environments[iEnvCount++]=commands;
		   if (GermanMode)
		      Environments[iEnvCount++]=GermanModeCommands;
		   break;
    case ITEMIZE:  Environments[iEnvCount++]=ItemizeCommands;
		   break;
    case ENUMERATE: Environments[iEnvCount++]=EnumerateCommands;
		    break;
    case TABBING : Environments[iEnvCount++]=TabbingCommands;
		   break;
    case LETTER : Environments[iEnvCount++]=LetterCommands;
		   break;
    case DESCRIPTION : Environments[iEnvCount++]=DescriptionCommands;
		   break;
    case GERMANMODE : Environments[iEnvCount++]=GermanModeCommands;
		   break;

    default: error("assertion failed at function PushEnvironment");
  }
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
