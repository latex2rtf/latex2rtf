/*  $Id: commands.c,v 1.24 2001/09/26 03:31:50 prahl Exp $

    Defines subroutines to translate LaTeX commands to RTF
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cfg.h"
#include "main.h"
#include "convert.h"
#include "chars.h"
#include "l2r_fonts.h"
#include "preamble.h"
#include "funct1.h"
#include "tables.h"
#include "equation.h"
#include "letterformat.h"
#include "commands.h"
#include "parser.h"
#include "biblio.h"
#include "ignore.h"

void            error(char *);

typedef struct commandtag {
	char           *cpCommand;			/* LaTeX command name without \ */
	void (*func) (int);	        		/* function converting LaTex-cmd to Rtf-cmd */
	int             param;	    		/* used in various ways */
} CommandArray;

static int      iEnvCount = 0;			/* number of active environments */
static CommandArray *Environments[100];	/* list of active environments */

static CommandArray commands[] = {
	{"begin", CmdBeginEnd, CMD_BEGIN},
	{"end", CmdBeginEnd, CMD_END},
	{"today", CmdToday, 0},
	{"footnote", CmdFootNote, FOOTN},

	{"rmfamily", CmdFontFamily, F_FAMILY_ROMAN  },
	{"rm",       CmdFontFamily, F_FAMILY_ROMAN_1},
	{"mathrm",   CmdFontFamily, F_FAMILY_ROMAN_2},
	{"textrm",   CmdFontFamily, F_FAMILY_ROMAN_2},

	{"sffamily", CmdFontFamily, F_FAMILY_SANSSERIF  },
	{"sf",       CmdFontFamily, F_FAMILY_SANSSERIF_1},
	{"mathsf",   CmdFontFamily, F_FAMILY_SANSSERIF_2},
	{"textsf",   CmdFontFamily, F_FAMILY_SANSSERIF_2},

	{"ttfamily", CmdFontFamily, F_FAMILY_TYPEWRITER},
	{"tt",       CmdFontFamily, F_FAMILY_TYPEWRITER_1},
	{"mathtt",   CmdFontFamily, F_FAMILY_TYPEWRITER_2},
	{"texttt",   CmdFontFamily, F_FAMILY_TYPEWRITER_2},

	{"cal",      CmdFontFamily, F_FAMILY_CALLIGRAPHIC_1},
	{"mathcal",  CmdFontFamily, F_FAMILY_CALLIGRAPHIC_2},

	{"bfseries", CmdFontSeries, F_SERIES_BOLD  },
	{"bf",       CmdFontSeries, F_SERIES_BOLD_1},
	{"textbf",   CmdFontSeries, F_SERIES_BOLD_2},
	{"mathbf",   CmdFontSeries, F_SERIES_BOLD_2},

	{"mdseries", CmdFontSeries, F_SERIES_MEDIUM  },
	{"textmd",   CmdFontSeries, F_SERIES_MEDIUM_2},
	{"mathmd",   CmdFontSeries, F_SERIES_MEDIUM_2},

	{"itshape",  CmdFontShape, F_SHAPE_ITALIC  },
	{"it",       CmdFontShape, F_SHAPE_ITALIC_1},
	{"mit",      CmdFontShape, F_SHAPE_ITALIC_1},
	{"textit",   CmdFontShape, F_SHAPE_ITALIC_2},
	{"mathit",   CmdFontShape, F_SHAPE_ITALIC_2},

	{"upshape",  CmdFontShape, F_SHAPE_UPRIGHT  },
	{"textup",   CmdFontShape, F_SHAPE_UPRIGHT_2},
	{"mathup",   CmdFontShape, F_SHAPE_UPRIGHT_2},

	{"scfamily", CmdFontShape, F_SHAPE_CAPS  },
	{"scshape",  CmdFontShape, F_SHAPE_CAPS  },
	{"sc",       CmdFontShape, F_SHAPE_CAPS_1},
	{"textsc",   CmdFontShape, F_SHAPE_CAPS_2},
	{"mathsc",   CmdFontShape, F_SHAPE_CAPS_2},

	{"slshape",  CmdFontShape, F_SHAPE_SLANTED  },
	{"sl",       CmdFontShape, F_SHAPE_SLANTED_1},
	{"textsl",   CmdFontShape, F_SHAPE_SLANTED_2},
	{"mathsl",   CmdFontShape, F_SHAPE_SLANTED_2},
	
	{"tiny",         CmdFontSize, 10},
	{"ssmall",   	 CmdFontSize, 12},  /* from moresize.sty */
	{"scriptsize",   CmdFontSize, 14},
	{"footnotesize", CmdFontSize, 16},
	{"small",        CmdFontSize, 18},
	{"normalsize",   CmdFontSize, 20},
	{"large",        CmdFontSize, 24},
	{"Large",        CmdFontSize, 28},
	{"LARGE",        CmdFontSize, 34},
	{"huge",         CmdFontSize, 40},
	{"Huge",         CmdFontSize, 50},
    {"HUGE",         CmdFontSize, 60},  /* from moresize.sty */
	
	/* ---------- OTHER FONT STUFF ------------------- */
	{"em",           CmdEmphasize, F_EMPHASIZE_1},
	{"emph",         CmdEmphasize, F_EMPHASIZE_2},
	{"underline",    CmdUnderline, 0},
	{"textnormal",   CmdTextNormal, F_TEXT_NORMAL_2},
	{"normalfont",   CmdTextNormal, F_TEXT_NORMAL_2},
	{"mathnormal",   CmdTextNormal, F_TEXT_NORMAL_3},

	{"centerline", CmdAlign, PAR_CENTERLINE},
	{"vcenter",    CmdAlign, PAR_VCENTER},
	
	/* ---------- LOGOS ------------------- */
	{"LaTeX", CmdLogo, CMD_LATEX},
	{"LaTeXe", CmdLogo, CMD_LATEXE},
	{"TeX", CmdLogo, CMD_TEX},
	{"SLiTeX", CmdLogo, CMD_SLITEX},
	{"BibTeX", CmdLogo, CMD_BIBTEX},
	{"AmSTeX", CmdLogo, CMD_AMSTEX},
	{"AmSLaTeX", CmdLogo, CMD_AMSLATEX},
	
	/* ---------- SPECIAL CHARACTERS ------------------- */
	{"hat", CmdHatChar, 0},
	{"check", CmdHacekChar, 0},
	{"breve", CmdBreveChar, 0},
	{"acute", CmdRApostrophChar, 0},
	{"grave", CmdLApostrophChar, 0},
	{"tilde", CmdTildeChar, 0},
	{"bar", CmdMacronChar, 0},
	{"vec", CmdVecChar, 0},
	{"dot", CmdDotChar, 0},
	{"ddot", CmdUmlauteChar, 0},
	{"u", CmdBreveChar, 0},
	{"d", CmdUnderdotChar, 0},
	{"v", CmdHacekChar, 0},
	{"r", CmdOaccentChar, 0},
	{"b", CmdUnderbarChar, 0},
	{"c", CmdCedillaChar, 0},

	{"ldots", CmdLdots, 0},
	{"maketitle", CmdMakeTitle, 0},
	{"par", CmdEndParagraph, 0},
	{"noindent", CmdIndent, INDENT_NONE},
	{"indent", CmdIndent, INDENT_USUAL},
	{"section", CmdSection, SECT_NORM},
	{"section*", CmdSection, SECT_NORM},
	{"caption", CmdCaption, 0},
	{"chapter", CmdSection, SECT_CHAPTER},
	{"subsection", CmdSection, SECT_SUB},
	{"subsection*", CmdSection, SECT_SUB},
	{"subsubsection", CmdSection, SECT_SUBSUB},
	{"subsubsection*", CmdSection, SECT_SUBSUB},
	{"part", CmdSection, SECT_PART},
	{"appendix", CmdIgnore, 0},
	{"protect", CmdIgnore, 0},
	{"paragraph", CmdSection, SECT_SUB},
	{"subparagraph", CmdSection, SECT_SUBSUB},
	{"clearpage", CmdNewPage, NewPage},
	{"cleardoublepage", CmdNewPage, NewPage},
	{"newpage", CmdNewPage, NewColumn},
	{"pagebreak", CmdNewPage, NewPage},
	{"mbox", CmdBox, 0},
	{"hbox", CmdBox, 0},
	{"vbox", CmdBox, 0},
	{"frenchspacing", CmdIgnore, 0},
	{"nonfrenchspacing", CmdIgnore, 0},
	{"include", CmdInclude, 0},	/* include Latex file */
	{"input", CmdInclude, 0},	/* include Latex file */
	{"verb", CmdVerb, 0},
	{"verb*", CmdVerb, AST_FORM},
	{"onecolumn", CmdColumn, One_Column},
	{"twocolumn", CmdColumn, Two_Column},
	{"includegraphics", CmdGraphics, 0},
	{"includegraphics*", CmdGraphics, 0},
	{"moveleft", CmdLength, 0},
	{"moveright", CmdLength, 0},
	{"footnotemark", CmdIgnoreParameter, One_Opt_No_NormParam},
	{"footnotetext", CmdIgnoreParameter, One_Opt_One_NormParam},
	{"label", CmdLabel, LABEL},
	{"ref", CmdLabel, REF},
	{"pageref", CmdLabel, PAGEREF},
	{"bibliography", CmdBibliography, 0},
	{"bibitem", CmdBibitem, 0},
	{"newblock", CmdNewblock, 0},
	{"newsavebox", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"usebox", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"fbox", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"quad", CmdQuad, 1},
	{"qquad", CmdQuad, 2},
	{"textsuperscript", CmdSuperscript, 0},
	{"hspace*", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"vspace", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"vspace*", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"addvspace", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"addcontentsline", CmdIgnoreParameter, No_Opt_Three_NormParam},
	{"addcontents", CmdIgnoreParameter, No_Opt_Two_NormParam},
	{"numberline", CmdIgnoreParameter, No_Opt_Two_NormParam},
	{"stretch", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"typeaout", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"index", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"indexentry", CmdIgnoreParameter, No_Opt_Two_NormParam},
	{"glossary", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"glossaryentry", CmdIgnoreParameter, No_Opt_Two_NormParam},
	{"typeout", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"Typein", CmdIgnoreParameter, One_Opt_One_NormParam},
	{"includeonly", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"nocite", CmdNoCite, No_Opt_One_NormParam},
	{"stepcounter", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"refstepcounter", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"fnsymbol", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"Alph", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"alph", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"Roman", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"roman", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"arabic", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"newcount", CmdIgnoreDef, 0},
	{"output", CmdIgnoreDef, 0},
	{"value", CmdCounter, COUNTER_VALUE},
	{"makebox", CmdIgnoreParameter, Two_Opt_One_NormParam},
	{"framebox", CmdIgnoreParameter, Two_Opt_One_NormParam},
	{"sbox", CmdIgnoreParameter, No_Opt_Two_NormParam},
	{"savebox", CmdIgnoreParameter, Two_Opt_Two_NormParam},
	{"parbox", CmdIgnoreParameter, One_Opt_Two_NormParam},
	{"rule", CmdIgnoreParameter, One_Opt_Two_NormParam},
	{"raisebox", CmdIgnoreParameter, Two_Opt_Two_NormParam},
	{"newfont", CmdIgnoreParameter, No_Opt_Two_NormParam},
	{"settowidth", CmdIgnoreParameter, No_Opt_Two_NormParam},
	{"nopagebreak", CmdIgnoreParameter, One_Opt_No_NormParam},
	{"samepage", CmdIgnore, 0},
	{"linebreak", CmdIgnoreParameter, One_Opt_No_NormParam},
	{"nolinebreak", CmdIgnoreParameter, One_Opt_No_NormParam},
	{"typein", CmdIgnoreParameter, One_Opt_One_NormParam},
	{"cite", CmdCite, 0},
	{"marginpar", CmdIgnoreParameter, One_Opt_One_NormParam},
	{"baselineskip", Cmd_OptParam_Without_braces, 0},
	{"lineskip", Cmd_OptParam_Without_braces, 0},
	{"vsize", Cmd_OptParam_Without_braces, 0},
	{"setbox", Cmd_OptParam_Without_braces, 0},
	{"thanks", CmdFootNote, THANKS},
	{"bibliographystyle", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"let", CmdIgnoreLet, 0},
	{"cline", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"multicolumn", CmdMultiCol, 0},
	{"frac", CmdFraction, 0},
    {"Frac", CmdFraction, 0},
	{"sqrt", CmdRoot, 0},
    {"int",  CmdIntegral, 0},
	{"nonumber",CmdNonumber, EQN_NO_NUMBER},
	{"", NULL, 0}
};

/********************************************************************
  commands found in the preamble of the LaTeX file
 ********************************************************************/
static CommandArray PreambleCommands[] = {
	{"documentclass", CmdDocumentStyle, 0},
	{"documentstyle", CmdDocumentStyle, 0},
	{"usepackage", CmdUsepackage, 0},
	{"begin", CmdPreambleBeginEnd, CMD_BEGIN},
	{"title", CmdTitle, TITLE_TITLE},
	{"author", CmdTitle, TITLE_AUTHOR},
	{"date", CmdTitle, TITLE_DATE},
	{"flushbottom", CmdBottom, 0},
	{"raggedbottom", CmdBottom, 0},
	{"addtolength", CmdLength, LENGTH_ADD},
	{"setlength", CmdLength, LENGTH_SET},
	{"newlength", CmdLength, LENGTH_NEW},
	{"newcounter", CmdCounter, COUNTER_NEW},
	{"setcounter", CmdCounter, COUNTER_SET},
	{"addtocounter", CmdCounter, COUNTER_ADD},
    {"cfoot", CmdHeadFoot, CFOOT},
    {"rfoot", CmdHeadFoot, RFOOT},
    {"lfoot", CmdHeadFoot, LFOOT},
    {"chead", CmdHeadFoot, CHEAD},
    {"rhead", CmdHeadFoot, RHEAD},
    {"lhead", CmdHeadFoot, LHEAD},
    {"thepage", CmdThePage, 0},
	{"hyphenation", CmdHyphenation, 0},
	{"def", CmdIgnoreDef, 0},
	{"newcommand", CmdIgnoreParameter, One_Opt_Three_NormParam},
	{"renewcommand", CmdIgnoreParameter, One_Opt_Two_NormParam},
	{"newenvironment", CmdIgnoreParameter, One_Opt_Three_NormParam},
	{"renewenvironment", CmdIgnoreParameter, One_Opt_Three_NormParam},
	{"newtheorem", CmdIgnoreParameter, One_Opt_Two_NormParam},
	{"renewtheorem", CmdIgnoreParameter, One_Opt_Two_NormParam},
	{"pagestyle", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"thispagestyle", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"pagenumbering", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"markboth", CmdIgnoreParameter, No_Opt_Two_NormParam},
	{"markright", CmdIgnoreParameter, No_Opt_One_NormParam},
	{"makeindex",CmdIgnoreParameter,0},
	{"makeglossary",CmdIgnoreParameter,0},
	{"listoffiles",CmdIgnoreParameter,0},
	{"nofiles",CmdIgnoreParameter,0},
	{"makelabels",CmdIgnoreParameter,0},
	{"verbositylevel",CmdVerbosityLevel,0},
	{"", NULL, 0}
};				/* end of list */

static CommandArray ItemizeCommands[] = {
	{"item", CmdItem, ITEMIZE},
	{"", NULL, 0}
};

static CommandArray DescriptionCommands[] = {
	{"item", CmdItem, DESCRIPTION},
	{"", NULL, 0}
};

static CommandArray EnumerateCommands[] = {
	{"item", CmdItem, ENUMERATE},
	{"", NULL, 0}
};

static CommandArray FigureCommands[] = {
	{"caption", CmdCaption, 0},
	{"center", CmdAlign, PAR_CENTER},
	{"", NULL, 0}
};

static CommandArray TabbingCommands[] = {
	{"kill", CmdTabkill, 0},/* a line that ends with a \kill command produces no output */
	{"", NULL, 0}
};

static CommandArray LetterCommands[] = {
	{"opening", CmdOpening, 0},
	{"closing", CmdClosing, 0},
	{"address", CmdAddress, 0},
	{"signature", CmdSignature, 0},
	{"ps", CmdPs, LETTER_PS},
	{"cc", CmdPs, LETTER_CC},
	{"encl", CmdPs, LETTER_ENCL},
	{"", NULL, 0}
};

static CommandArray GermanModeCommands[] = {
	{"ck", GermanPrint, GP_CK},
	{"glqq", GermanPrint, GP_LDBL},
	{"glq", GermanPrint, GP_L},
	{"grq", GermanPrint, GP_R},
	{"grqq", GermanPrint, GP_RDBL},
	{"", NULL, 0}
};

static CommandArray FrenchModeCommands[] = {
/*    {"degree", CmdFrenchAbbrev, DEGREE}, */
    {"ier", CmdFrenchAbbrev, IERF},
    {"iere", CmdFrenchAbbrev, IEREF},
    {"iers", CmdFrenchAbbrev, IERSF},
    {"ieres", CmdFrenchAbbrev, IERESF},
    {"ieme", CmdFrenchAbbrev, IEMEF},
    {"iemes", CmdFrenchAbbrev, IEMESF},
    {"numero", CmdFrenchAbbrev, NUMERO},
    {"numeros", CmdFrenchAbbrev, NUMEROS},
    {"Numero", CmdFrenchAbbrev, CNUMERO},
    {"Numeros", CmdFrenchAbbrev, CNUMEROS},
/*    {"degres", CmdFrenchAbbrev, DEGREE}, */
/*    {"textdegree", CmdFrenchAbbrev, DEGREE}, */
    {"primo", CmdFrenchAbbrev, PRIMO},
    {"secundo", CmdFrenchAbbrev, SECUNDO},
    {"tertio", CmdFrenchAbbrev, TERTIO},
    {"quarto", CmdFrenchAbbrev, QUARTO},
    {"fup", CmdFrenchAbbrev, FUP},
	{"", NULL, 0}
};

/********************************************************************/
/* commands for begin-end environments */
/* only strings used in the form \begin{text} or \end{text} */
/********************************************************************/
static CommandArray params[] = {
	{"center", CmdAlign, PAR_CENTER},
	{"flushright", CmdAlign, PAR_RIGHT},
	{"flushleft", CmdAlign, PAR_LEFT},
	{"document", Environment, DOCUMENT},
	{"tabbing", CmdTabbing, TABBING},
	{"figure", CmdFigure, FIGURE},
	{"figure*", CmdFigure, FIGURE_1},
	{"picture", CmdIgnoreFigure, PICTURE},
	{"minipage", CmdIgnoreFigure, MINIPAGE},

	{"quote", CmdQuote, QUOTE},
	{"quotation", CmdQuote, QUOTATION},
	{"enumerate", CmdList, ENUMERATE},
	{"list", CmdList, ITEMIZE},
	{"itemize", CmdList, ITEMIZE},
	{"description", CmdList, DESCRIPTION},
	{"verbatim", CmdVerbatim, 1},
	{"verse", CmdVerse, 0},
	{"tabular", CmdTabular, TABULAR},
	{"tabular*", CmdTabular, TABULAR_1},
	{"longtable", CmdTabular, TABULAR},
	{"longtable*", CmdTabular, TABULAR_1},
	{"array", CmdTabular, TABULAR_2},

	{"multicolumn", CmdMultiCol, 0},
	{"displaymath", CmdDisplayMath, EQN_DISPLAYMATH},
	{"equation", CmdDisplayMath, EQN_EQUATION},
	{"equation*", CmdDisplayMath, EQN_EQUATION_STAR},
	{"eqnarray*", CmdDisplayMath, EQN_ARRAY_STAR},
	{"eqnarray", CmdDisplayMath, EQN_ARRAY},
	{"letter", CmdLetter, 0},
	{"table", CmdTable, TABLE},
	{"table*", CmdTable, TABLE_1},
	{"thebibliography", CmdThebibliography, 0},
	{"abstract", CmdAbstract, 0},
	{"titlepage", CmdTitlepage, 0},
	
	{"math", CmdMath, EQN_MATH},
	{"em", CmdEmphasize, F_EMPHASIZE_3},
	{"rmfamily", CmdFontFamily, F_FAMILY_ROMAN_3},
	{"sffamily", CmdFontFamily, F_FAMILY_SANSSERIF_3},
	{"ttfamily", CmdFontFamily, F_FAMILY_TYPEWRITER_3},
	{"bfseries", CmdFontSeries, F_SERIES_BOLD_3},
	{"mdseries", CmdFontSeries, F_SERIES_MEDIUM_3},
	{"itshape",  CmdFontShape, F_SHAPE_ITALIC_3},
	{"scshape",  CmdFontShape, F_SHAPE_CAPS_3},
	{"slshape",  CmdFontShape, F_SHAPE_SLANTED_3},
	{"it",  CmdFontShape, F_SHAPE_ITALIC_4},
	{"sc",  CmdFontShape, F_SHAPE_CAPS_4},
	{"sl",  CmdFontShape, F_SHAPE_SLANTED_4},
	{"bf",  CmdFontShape, F_SERIES_BOLD_4},
	{"sf", CmdFontFamily, F_FAMILY_ROMAN_4},
	{"tt", CmdFontFamily, F_FAMILY_SANSSERIF_4},
	{"rm", CmdFontFamily, F_FAMILY_TYPEWRITER_4},
	{"", NULL, 0}
};				/* end of list */


/********************************************************************
purpose: commands for hyperlatex package 
********************************************************************/
static CommandArray hyperlatex[] = {
	{"link", CmdLink, 0},
	{"xlink", CmdLink, 0},
	{"Cite", CmdCite, HYPERLATEX},
	{"Ref", CmdLabel, HYPERREF},
	{"Pageref", CmdLabel, HYPERPAGEREF},
	{"S", CmdColsep, 0},
	{"", NULL, 0}
};				/* end of list */


bool 
CallCommandFunc(char *cCommand)
/****************************************************************************
purpose: Tries to call the command-function for the commandname
params:  string with command name
returns: success or failure
globals: command-functions have side effects or recursive calls
 ****************************************************************************/
{
	int             i = 0, j;

	diagnostics(5,"CallCommandFunc %s, iEnvCount = %d",cCommand,iEnvCount);
	
	for (j = iEnvCount - 1; j >= 0; j--) {
		i = 0;
		while (strcmp(Environments[j][i].cpCommand, "") != 0) {

		/*	diagnostics(5,"CallCommandFunc Trying %s",Environments[j][i].cpCommand); */
			
			if (strcmp(Environments[j][i].cpCommand, cCommand) == 0) {
				if (Environments[j][i].func == NULL)
					return FALSE;
				if (*Environments[j][i].func == CmdIgnoreParameter) {
					diagnostics(WARNING, "Command \\%s ignored", cCommand);
				}

			diagnostics(5,"CallCommandFunc Found %s iEnvCount=%d number=%d",Environments[j][i].cpCommand, j, i); 
				(*Environments[j][i].func) ((Environments[j][i].param));
				return TRUE;	/* Command Function found */
			}
			++i;
		}
	}
	return FALSE;
}


bool 
CallParamFunc(char *cCommand, int AddParam)
/****************************************************************************
purpose: Try to call the environment-function for the commandname
params:  cCommand - string with command name
	 AddParam - param "ORed"(||) to the int param of command-funct
returns: sucess or not
globals: command-functions have side effects or recursive calls
 ****************************************************************************/
{
	int             i = 0;
	char            unknown_environment[100];

	while (strcmp(params[i].cpCommand, "") != 0) {
		if (strcmp(params[i].cpCommand, cCommand) == 0) {
			assert(params[i].func != NULL);
			(*params[i].func) ((params[i].param) | AddParam);
			return TRUE;	/* command function found */
		}
		++i;
	}

	/* unknown environment must be ignored */
	if (AddParam == ON) {
		sprintf(unknown_environment, "\\%s%s%s", "end{", cCommand, "}");
		Ignore_Environment(cCommand);
		diagnostics(WARNING, "Environment <%s> ignored.  Not defined in commands.c", cCommand);
	}
	return FALSE;
}

int 
CurrentEnvironmentCount(void)
/****************************************************************************
purpose: to eliminate the iEnvCount global variable 
****************************************************************************/
{
	return iEnvCount;
}

void 
PushEnvironment(int code)
/****************************************************************************
purpose: adds the command list for a specific environment to the list
	 of commands searched through.
params:  constant identifying the environment
globals: changes Environment - array of active environments
		 iEnvCount   - counter of active environments
 ****************************************************************************/
{
	char           *diag = "";

	switch (code) {
	case PREAMBLE:
		Environments[iEnvCount] = PreambleCommands;
		diag = "preamble";
		break;
	case DOCUMENT:
		Environments[iEnvCount] = commands;
		diag = "document";
		break;
	case ITEMIZE:
		Environments[iEnvCount] = ItemizeCommands;
		diag = "itemize";
		break;
	case ENUMERATE:
		Environments[iEnvCount] = EnumerateCommands;
		diag = "enumerate";
		break;
	case TABBING:
		Environments[iEnvCount] = TabbingCommands;
		diag = "tabbing";
		break;
	case LETTER:
		Environments[iEnvCount] = LetterCommands;
		diag = "letter";
		break;
	case DESCRIPTION:
		Environments[iEnvCount] = DescriptionCommands;
		diag = "description";
		break;
	case GERMAN_MODE:
		Environments[iEnvCount] = GermanModeCommands;
		diag = "german";
		break;
	case FRENCH_MODE:
		Environments[iEnvCount] = FrenchModeCommands;
		diag = "french";
		break;
	case FIGURE_ENV:
		Environments[iEnvCount] = FigureCommands;
		diag = "figure";
		break;
	case IGN_ENV_CMD:
		Environments[iEnvCount] = commands;
		diag = "*latex2rtf ignored*";
		break;
	case HYPERLATEX:
		Environments[iEnvCount] = hyperlatex;
		diag = "hyperlatex";
		break;

	default:
		error("assertion failed at function PushEnvironment");
	}

	iEnvCount++;
	diagnostics(3, "Entered %s environment, iEnvCount now = %d", diag, iEnvCount);
}


void 
PopEnvironment()
/****************************************************************************
purpose: removes the environment-commands list added by last PushEnvironment;
globals: changes Environment - array of active environments
		 iEnvCount - counter of active environments
 ****************************************************************************/
{
	--iEnvCount;
	Environments[iEnvCount] = NULL;

	/*
	 * overlapping environments are not allowed !!! example:
	 * \begin{verse}\begin{enumerate}\end{verse}\end{enumerate} ==>
	 * undefined result extension possible
	 */

	diagnostics(3, "Exited environment, iEnvCount now = %d", iEnvCount);
	return;
}


void 
ClearEnvironment()
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
