
/* commands.c - Defines subroutines to translate LaTeX commands to RTF

Copyright (C) 1995-2002 The Free Software Foundation

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

This file is available from http://sourceforge.net/projects/latex2rtf/
 
Authors:
    1995-1997 Ralf Schlatterbeck
    1998-2000 Georg Lehner
    2001-2002 Scott Prahl
*/

#include <stdlib.h>
#include <string.h>
#include "cfg.h"
#include "main.h"
#include "convert.h"
#include "chars.h"
#include "fonts.h"
#include "preamble.h"
#include "funct1.h"
#include "tables.h"
#include "equations.h"
#include "letterformat.h"
#include "commands.h"
#include "parser.h"
#include "xrefs.h"
#include "ignore.h"
#include "lengths.h"
#include "definitions.h"
#include "graphics.h"
#include "vertical.h"

typedef struct commandtag {
    char *cmd_name;            /* LaTeX command name without \ */
    void (*func) (int);         /* function to convert LaTeX to RTF */
    int param;                  /* used in various ways */
} CommandArray;

static int iEnvCount = 0;               /* number of current environments */
static CommandArray *Environments[100]; /* call chain for current environments */
static int g_par_indent_array[100];
static int g_left_indent_array[100];
static int g_right_indent_array[100];
static char g_align_array[100];

static CommandArray commands[] = {
    {"begin", CmdBeginEnd, CMD_BEGIN},
    {"end", CmdBeginEnd, CMD_END},
    {"today", CmdToday, 0},
	{"footnote", CmdFootNote, FOOTNOTE},
	{"endnote", CmdFootNote, FOOTNOTE | FOOTNOTE_ENDNOTE},

	{"rmfamily", CmdFontFamily, F_FAMILY_ROMAN  },
    {"rm", CmdFontFamily, F_FAMILY_ROMAN_1},
    {"mathrm", CmdFontFamily, F_FAMILY_ROMAN_2},
    {"textrm", CmdFontFamily, F_FAMILY_ROMAN_2},

    {"sffamily", CmdFontFamily, F_FAMILY_SANSSERIF},
    {"sf", CmdFontFamily, F_FAMILY_SANSSERIF_1},
    {"mathsf", CmdFontFamily, F_FAMILY_SANSSERIF_2},
    {"textsf", CmdFontFamily, F_FAMILY_SANSSERIF_2},

    {"ttfamily", CmdFontFamily, F_FAMILY_TYPEWRITER},
    {"tt", CmdFontFamily, F_FAMILY_TYPEWRITER_1},
    {"mathtt", CmdFontFamily, F_FAMILY_TYPEWRITER_2},
    {"texttt", CmdFontFamily, F_FAMILY_TYPEWRITER_2},

    {"cal", CmdFontFamily, F_FAMILY_CALLIGRAPHIC_1},
    {"mathcal", CmdFontFamily, F_FAMILY_CALLIGRAPHIC_2},

    {"bfseries", CmdFontSeries, F_SERIES_BOLD},
    {"bf", CmdFontSeries, F_SERIES_BOLD_1},
    {"textbf", CmdFontSeries, F_SERIES_BOLD_2},
    {"mathbf", CmdFontSeries, F_SERIES_BOLD_2},

    {"mdseries", CmdFontSeries, F_SERIES_MEDIUM},
    {"textmd", CmdFontSeries, F_SERIES_MEDIUM_2},
    {"mathmd", CmdFontSeries, F_SERIES_MEDIUM_2},

    {"itshape", CmdFontShape, F_SHAPE_ITALIC},
    {"it", CmdFontShape, F_SHAPE_ITALIC_1},
    {"mit", CmdFontShape, F_SHAPE_ITALIC_1},
    {"textit", CmdFontShape, F_SHAPE_ITALIC_2},
    {"mathit", CmdFontShape, F_SHAPE_ITALIC_2},

    {"upshape", CmdFontShape, F_SHAPE_UPRIGHT},
    {"textup", CmdFontShape, F_SHAPE_UPRIGHT_2},
    {"mathup", CmdFontShape, F_SHAPE_UPRIGHT_2},

    {"scfamily", CmdFontShape, F_SHAPE_CAPS},
    {"scshape", CmdFontShape, F_SHAPE_CAPS},
    {"sc", CmdFontShape, F_SHAPE_CAPS_1},
    {"textsc", CmdFontShape, F_SHAPE_CAPS_2},
    {"mathsc", CmdFontShape, F_SHAPE_CAPS_2},

    {"slshape", CmdFontShape, F_SHAPE_SLANTED},
    {"sl", CmdFontShape, F_SHAPE_SLANTED_1},
    {"textsl", CmdFontShape, F_SHAPE_SLANTED_2},
    {"mathsl", CmdFontShape, F_SHAPE_SLANTED_2},

    {"tiny", CmdFontSize, 10},
    {"ssmall", CmdFontSize, 12},    /* from moresize.sty */
    {"scriptsize", CmdFontSize, 14},
    {"footnotesize", CmdFontSize, 16},
    {"enotesize", CmdFontSize, 16},
    {"small", CmdFontSize, 18},
    {"normalsize", CmdFontSize, 20},
    {"large", CmdFontSize, 24},
    {"Large", CmdFontSize, 28},
    {"LARGE", CmdFontSize, 34},
    {"huge", CmdFontSize, 40},
    {"Huge", CmdFontSize, 50},
    {"HUGE", CmdFontSize, 60},  /* from moresize.sty */

    /* ---------- OTHER FONT STUFF ------------------- */
    {"em", CmdEmphasize, F_EMPHASIZE_1},
    {"emph", CmdEmphasize, F_EMPHASIZE_2},
    {"underline", CmdUnderline, 0},
    {"underbar", CmdUnderline, 0},
    {"textnormal", CmdTextNormal, F_TEXT_NORMAL_2},
    {"normalfont", CmdTextNormal, F_TEXT_NORMAL_2},
    {"mathnormal", CmdTextNormal, F_TEXT_NORMAL_3},
    {"textfont", CmdTextFont, 0},
    {"the", CmdThe, 0},

    {"raggedright", CmdAlign, PAR_RAGGEDRIGHT},
    {"centerline", CmdAlign, PAR_CENTERLINE},
    {"vcenter", CmdAlign, PAR_VCENTER},

    /* ---------- LOGOS ------------------- */
    {"latex", CmdLogo, CMD_LATEX},
    {"LaTeX", CmdLogo, CMD_LATEX},
    {"LaTeXe", CmdLogo, CMD_LATEXE},
    {"TeX", CmdLogo, CMD_TEX},
    {"SLiTeX", CmdLogo, CMD_SLITEX},
    {"BibTeX", CmdLogo, CMD_BIBTEX},
    {"AmSTeX", CmdLogo, CMD_AMSTEX},
    {"AmSLaTeX", CmdLogo, CMD_AMSLATEX},
    {"LyX", CmdLogo, CMD_LYX},
    {"lower",CmdSubscript,2},
    {"kern",CmdKern,0},

    /* ---------- SPECIAL CHARACTERS ------------------- */
    {"hat", CmdHatChar, 0},
    {"check", CmdCaronChar, 0},
    {"breve", CmdBreveChar, 0},
    {"acute", CmdAcuteChar, 0},
    {"grave", CmdGraveChar, 0},
    {"tilde", CmdTildeChar, 0},
    {"bar", CmdMacronChar, 0},
    {"vec", CmdVecChar, 0},
    {"overrightarrow", CmdVecChar, 0},
    {"dot", CmdDotChar, 0},
    {"ddot", CmdUmlauteChar, 0},
    {"\"", CmdUmlauteChar, 0},
    {"u", CmdBreveChar, 0},
    {"d", CmdUnderdotChar, 0},
    {"v", CmdCaronChar, 0},
    {"r", CmdRingChar, 0},
    {"b", CmdUnderbarChar, 0},
    {"c", CmdCedillaChar, 0},
    {"i", CmdDotlessChar, 0},
    {"j", CmdDotlessChar, 1},
	{"H", CmdDoubleAcuteChar, 0},
    {"l", CmdPolishL, 0},
    {"L", CmdPolishL, 1},
	
/* sectioning commands */
    {"part", CmdSection, SECT_PART},
    {"part*", CmdSection, SECT_PART_STAR},
    {"chapter", CmdSection, SECT_CHAPTER},
    {"chapter*", CmdSection, SECT_CHAPTER_STAR},
    {"section", CmdSection, SECT_NORM},
    {"section*", CmdSection, SECT_NORM_STAR},
    {"subsection", CmdSection, SECT_SUB},
    {"subsection*", CmdSection, SECT_SUB_STAR},
    {"subsubsection", CmdSection, SECT_SUBSUB},
    {"subsubsection*", CmdSection, SECT_SUBSUB_STAR},
    {"paragraph", CmdSection, SECT_SUBSUBSUB},
    {"paragraph*", CmdSection, SECT_SUBSUBSUB_STAR},
    {"subparagraph", CmdSection, SECT_SUBSUBSUBSUB},
    {"subparagraph*", CmdSection, SECT_SUBSUBSUBSUB_STAR},

    {"ldots", CmdLdots, 0},
    {"dots", CmdLdots, 0},
    {"dotfill", CmdLdots, 1},
    {"textellipsis", CmdLdots, 2},

    {"maketitle", CmdMakeTitle, 0},
    {"par", CmdEndParagraph, 0},
    {"noindent", CmdIndent, INDENT_NONE},
    {"indent", CmdIndent, INDENT_USUAL},
    {"caption", CmdCaption, 0},
    {"appendix", CmdAppendix, 0},
    {"protect", CmdIgnore, 0},
    {"clearpage", CmdNewPage, NewPage},
    {"efloatseparator", CmdNewPage, NewPage},
    {"cleardoublepage", CmdNewPage, NewPage},
    {"newpage", CmdNewPage, NewColumn},
    {"pagebreak", CmdNewPage, NewPage},
    {"mbox", CmdBox, BOX_MBOX},
    {"hbox", CmdBox, BOX_HBOX},
    {"vbox", CmdBox, BOX_VBOX},
    {"fbox", CmdBox, BOX_FBOX},
    {"parbox", CmdBox, BOX_PARBOX},
    {"frenchspacing", CmdIgnore, 0},
    {"nonfrenchspacing", CmdIgnore, 0},
    {"include", CmdIgnoreParameter, No_Opt_One_NormParam},  /* should not happen */
    {"input", CmdIgnoreParameter, No_Opt_One_NormParam},    /* should not happen */
    {"verb", CmdVerb, VERB_VERB},
    {"verb*", CmdVerb, VERB_STAR},
    {"onecolumn", CmdColumn, One_Column},
    {"twocolumn", CmdColumn, Two_Column},
    {"includegraphics", CmdGraphics, FIGURE_INCLUDEGRAPHICS},
    {"epsffile", CmdGraphics, FIGURE_EPSFFILE},
    {"epsfbox", CmdGraphics, FIGURE_EPSFBOX},
    {"BoxedEPSF", CmdGraphics, FIGURE_BOXEDEPSF},
    {"psfig", CmdGraphics, FIGURE_PSFIG},
    {"includegraphics*", CmdGraphics, FIGURE_INCLUDEGRAPHICS},
    {"moveleft", CmdLength, 0},
    {"moveright", CmdLength, 0},
    {"hsize", CmdLength, 0},
    {"letterspace", CmdLength, 0},
    {"footnotemark", CmdIgnoreParameter, One_Opt_No_NormParam},
    {"endnotemark", CmdIgnoreParameter, One_Opt_No_NormParam},
    {"label", CmdLabel, LABEL_LABEL},
    {"ref", CmdLabel, LABEL_REF},
    {"vref", CmdLabel, LABEL_VREF},
    {"eqref", CmdLabel, LABEL_EQREF},
    {"pageref", CmdLabel, LABEL_PAGEREF},
    {"nameref", CmdLabel, LABEL_NAMEREF},
    {"cite", CmdCite, CITE_CITE},
	{"onlinecite", CmdCite, CITE_CITE},
	{"citeonline", CmdCite, CITE_CITE},
    {"nobibliography", CmdIgnoreParameter, No_Opt_One_NormParam},  
    {"bibliography", CmdBibliography, 0},
    {"bibitem", CmdBibitem, 0},
    {"bibentry", CmdBibEntry, 0},
    {"newblock", CmdNewblock, 0},
    {"newsavebox", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"usebox", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"subfigure", CmdSubFigure, 0},

/*	{"fbox", CmdIgnoreParameter, No_Opt_One_NormParam}, */
    {"quad", CmdQuad, 1},
    {"qquad", CmdQuad, 2},
    {"textsuperscript", CmdSuperscript, 1},
    {"textsubscript", CmdSubscript, 1},
    {"hspace", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"hspace*", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"vspace", CmdVspace, VSPACE_VSPACE},
    {"vspace*", CmdVspace, VSPACE_VSPACE},
    {"vskip", CmdVspace, VSPACE_VSKIP},
    {"smallskip", CmdVspace, VSPACE_SMALL_SKIP},
    {"medskip", CmdVspace, VSPACE_MEDIUM_SKIP},
    {"bigskip", CmdVspace, VSPACE_BIG_SKIP},
    {"addvspace", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"addcontentsline", CmdIgnoreParameter, No_Opt_Three_NormParam},
    {"addcontents", CmdIgnoreParameter, No_Opt_Two_NormParam},
    {"stretch", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"typeaout", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"index", CmdIndex, 0},
    {"printindex", CmdPrintIndex, 0},
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
    {"rule", CmdRule,0},
    {"raisebox", CmdIgnoreParameter, Two_Opt_Two_NormParam},
    {"newfont", CmdIgnoreParameter, No_Opt_Two_NormParam},
    {"settowidth", CmdIgnoreParameter, No_Opt_Two_NormParam},
    {"nopagebreak", CmdIgnoreParameter, One_Opt_No_NormParam},
    {"samepage", CmdIgnore, 0},
    {"expandafter", CmdIgnore, 0},
    {"long", CmdIgnore, 0},
    {"nobreak", CmdIgnore, 0},
    {"linebreak", CmdIgnoreParameter, One_Opt_No_NormParam},
    {"nolinebreak", CmdIgnoreParameter, One_Opt_No_NormParam},
    {"typein", CmdIgnoreParameter, One_Opt_One_NormParam},
    {"marginpar", CmdIgnoreParameter, One_Opt_One_NormParam},
    {"baselineskip", Cmd_OptParam_Without_braces, 0},
    {"psfrag", CmdIgnoreParameter, No_Opt_Two_NormParam},
    {"lineskip", Cmd_OptParam_Without_braces, 0},
    {"vsize", Cmd_OptParam_Without_braces, 0},
    {"setbox", Cmd_OptParam_Without_braces, 0},
    {"thanks", CmdFootNote, FOOTNOTE_THANKS},
    {"bibliographystyle", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"let", CmdIgnoreLet, 0},
    {"multicolumn", CmdMultiCol, 0},
    {"ensuremath", CmdEnsuremath, 0},
    {"frac", CmdFraction, 0},
    {"dfrac", CmdFraction, 0},
    {"Frac", CmdFraction, 0},
    {"sqrt", CmdRoot, 0},
    {"lim", CmdLim, 0},
    {"limsup", CmdLim, 1},
    {"liminf", CmdLim, 2},
    {"int", CmdIntegral, 0},
    {"iint", CmdIntegral, 3},
    {"iiint", CmdIntegral, 4},
    {"sum", CmdIntegral, 1},
    {"prod", CmdIntegral, 2},
    {"left", CmdLeftRight, 0},
    {"right", CmdLeftRight, 1},
    {"stackrel", CmdStackrel, 0},
    {"matrix", CmdMatrix, 0},
    {"overline",CmdOverLine,0},
    {"leftrightarrows", CmdArrows, LEFT_RIGHT},
    {"leftleftarrows", CmdArrows, LEFT_LEFT},
    {"rightrightarrows", CmdArrows, RIGHT_RIGHT},
    {"longleftrightarrows", CmdArrows, LONG_LEFTRIGHT},
    {"longleftarrow", CmdArrows, LONG_LEFT},
    {"longrightarrow", CmdArrows, LONG_RIGHT},
    {"psset", CmdPsset, 0},
    {"newpsstyle", CmdNewPsStyle, 0},

    {"mho",               CmdMTExtraChar, MTEXTRA_MHO},
    {"ddots",             CmdMTExtraChar, MTEXTRA_DDOTS},
    {"mapsto",            CmdMTExtraChar, MTEXTRA_MAPSTO},
    {"updownarrow",       CmdMTExtraChar, MTEXTRA_UPDOWNARROW},
    {"nwarrow",           CmdMTExtraChar, MTEXTRA_NWARROW},
    {"nearrow",           CmdMTExtraChar, MTEXTRA_NEARROW},
    {"searrow",           CmdMTExtraChar, MTEXTRA_SEARROW},
    {"swarrow",           CmdMTExtraChar, MTEXTRA_SWARROW},

    {"triangleleft",      CmdMTExtraChar, MTEXTRA_TRIANGLELEFT},
    {"triangleright",     CmdMTExtraChar, MTEXTRA_TRIANGLERIGHT},
    {"cdots",             CmdMTExtraChar, MTEXTRA_CDOTS},
    {"vdots",             CmdMTExtraChar, MTEXTRA_VDOTS},
    {"hbar",              CmdMTExtraChar, MTEXTRA_HBAR},
    {"hslash",            CmdMTExtraChar, MTEXTRA_HBAR},
    {"ell",               CmdMTExtraChar, MTEXTRA_ELL},
    {"mp",                CmdMTExtraChar, MTEXTRA_MP},
    {"succ",              CmdMTExtraChar, MTEXTRA_SUCC},
    {"prec",              CmdMTExtraChar, MTEXTRA_PREC},
    {"amalg",             CmdMTExtraChar, MTEXTRA_COPROD},
    {"coprod",            CmdMTExtraChar, MTEXTRA_COPROD},
 
    {"simeq",             CmdMTExtraChar, MTEXTRA_SIMEQ},
    {"ll",                CmdMTExtraChar, MTEXTRA_LL},
    {"gg",                CmdMTExtraChar, MTEXTRA_GG},
    {"doteq",             CmdMTExtraChar, MTEXTRA_DOTEQ},
    {"because",           CmdMTExtraChar, MTEXTRA_BECAUSE},
    {"measuredangle",     CmdMTExtraChar, MTEXTRA_MEASUREDANGLE},
    {"Updownarrow",       CmdMTExtraChar, MTEXTRA_DOUBLEUPDOWNARROW},
    {"rightleftarrows",   CmdMTExtraChar, MTEXTRA_RIGHTLEFTARROWS},
    {"rightleftharpoons", CmdMTExtraChar, MTEXTRA_RIGHTLEFTHARPOONS},
    {"rightharpoonup",    CmdMTExtraChar, MTEXTRA_RIGHTHARPOONUP},
    {"leftharpoondown",   CmdMTExtraChar, MTEXTRA_LEFTHARPOONDOWN},

    {"nonumber", CmdNonumber, EQN_NO_NUMBER},
    {"notag", CmdNonumber, EQN_NO_NUMBER},
    {"char", CmdSymbol, 0},
    {"symbol", CmdSymbol, 1},

    {"url",               CmdHtml, LABEL_URL},
    {"urlstyle",          CmdHtml, LABEL_URLSTYLE},
    {"htmladdnormallink", CmdHtml, LABEL_HTMLADDNORMALREF},
    {"htmlref",           CmdHtml, LABEL_HTMLREF},
	
    {"nobreakspace", CmdNonBreakSpace, 100},
    {"thinspace", CmdNonBreakSpace, 50},
    {"abstract", CmdAbstract, 2},
    {"endinput", CmdEndInput, 0},
    {"textcolor", CmdTextColor, 0},
    {"tableofcontents", CmdTableOfContents, 0},
    {"listoffigures", CmdListOf, LIST_OF_FIGURES},
    {"listoftables", CmdListOf, LIST_OF_TABLES},
    {"numberline", CmdNumberLine, 0},
    {"contentsline", CmdContentsLine, 0},
    {"centering", CmdAlign, PAR_CENTERING},
    {"pagestyle", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"pagenumbering", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"markboth", CmdIgnoreParameter, No_Opt_Two_NormParam},
    {"markright", CmdIgnoreParameter, No_Opt_One_NormParam},
    
	{"aleph", CmdSymbolChar, 0xc0},
	{"alpha", CmdSymbolChar, (int) 'a'},
	{"Alpha", CmdSymbolChar, (int) 'A'},
	{"angle", CmdSymbolChar, 0xd0},
	{"approx", CmdSymbolChar, 0xbb},
	{"ast", CmdSymbolChar, 0x2a},
	{"beta", CmdSymbolChar, (int) 'b'},
	{"Beta", CmdSymbolChar, (int) 'B'},
	{"bot", CmdSymbolChar, 0x5e},
	{"bullet", CmdSymbolChar, 0xb7},
	{"cap", CmdSymbolChar, 0xc7}, 
	{"cdot", CmdSymbolChar, 0xd7},
	{"chi", CmdSymbolChar, (int) 'c'},
	{"Chi", CmdSymbolChar, (int) 'C'},
	{"circ", CmdSymbolChar, 0x6f},
	{"clubsuit", CmdSymbolChar, 0xa7},
	{"cong", CmdSymbolChar, 0x40},
	{"cup", CmdSymbolChar, 0xc8},      
	{"delta", CmdSymbolChar, (int) 'd'},
	{"Delta", CmdSymbolChar, (int) 'D'},
	{"Diamond", CmdSymbolChar, 0xe0},
	{"diamondsuit", CmdSymbolChar, 0xa8},
	{"div", CmdSymbolChar, 0xb8},
	{"dots", CmdSymbolChar, 0xbc},
	{"downarrow", CmdSymbolChar, 0xaf},
	{"Downarrow", CmdSymbolChar, 0xdf},
	{"emptyset", CmdSymbolChar, 0xc6},
	{"epsilon", CmdSymbolChar, 0x65},
	{"equiv", CmdSymbolChar, 0xba},
	{"eta", CmdSymbolChar, (int) 'h'},
	{"exists", CmdSymbolChar, 0x24},
	{"forall", CmdSymbolChar, 0x22},
	{"gamma", CmdSymbolChar, (int) 'g'},
	{"Gamma", CmdSymbolChar, (int) 'G'},
	{"ge", CmdSymbolChar, 0xb3},
	{"geq", CmdSymbolChar, 0xb3},
	{"heartsuit", CmdSymbolChar, 0xa9},
	{"Im", CmdSymbolChar, 0xc1},
	{"in", CmdSymbolChar, 0xce},
	{"infty", CmdSymbolChar, 0xa5},
	{"int", CmdSymbolChar, 0xf2},
	{"iota", CmdSymbolChar, (int) 'i'},
	{"kappa", CmdSymbolChar, (int) 'k'},
	{"lambda", CmdSymbolChar, (int) 'l'},
	{"Lambda", CmdSymbolChar, (int) 'L'},
	{"land", CmdSymbolChar, 0xd9},
	{"langle", CmdSymbolChar, 0xe1},
	{"lceil", CmdSymbolChar, 0xe9},
	{"le", CmdSymbolChar, 0xa3},
	{"leftarrow", CmdSymbolChar, 0xac},
	{"Leftarrow", CmdSymbolChar, 0xdc},
	{"leftrightarrow", CmdSymbolChar, 0xab},
	{"Leftrightarrow", CmdSymbolChar, 0xdb},
	{"leq", CmdSymbolChar, 0xa3},
	{"lfloor", CmdSymbolChar, 0xeb},
	{"lor", CmdSymbolChar, 0xda},
	{"mu", CmdSymbolChar, (int) 'm'},
	{"nabla", CmdSymbolChar, 0xd1},
	{"ne", CmdSymbolChar, 0xb9},
	{"neg", CmdSymbolChar, 0xd8},             
	{"neq", CmdSymbolChar, 0xb9},
	{"nu", CmdSymbolChar, (int) 'n'},
	{"omega", CmdSymbolChar, (int) 'w'},
	{"Omega", CmdSymbolChar, (int) 'W'},
	{"omicron", CmdSymbolChar, (int) 'o'},    
	{"oplus", CmdSymbolChar, 0xc5},
	{"oslash", CmdSymbolChar, 0xc6},  
	{"otimes", CmdSymbolChar, 0xc4},
	{"partial", CmdSymbolChar, 0xb6},
	{"perp", CmdSymbolChar, 0x5e},
	{"phi", CmdSymbolChar, (int) 'f'},
	{"Phi", CmdSymbolChar, (int) 'F'},
	{"pi", CmdSymbolChar, (int) 'p'},
	{"Pi", CmdSymbolChar, (int) 'P'},
	{"pm", CmdSymbolChar, 0xb1},
	{"prod", CmdSymbolChar, 0xd5},
	{"propto", CmdSymbolChar, 0xb5},
	{"psi", CmdSymbolChar, (int) 'y'},
	{"Psi", CmdSymbolChar, (int) 'Y'},
	{"rangle", CmdSymbolChar, 0xf1},
	{"rceil", CmdSymbolChar, 0xf9},
	{"Re", CmdSymbolChar, 0xc2},
	{"rfloor", CmdSymbolChar, 0xfb},
	{"rho", CmdSymbolChar, (int) 'r'},
	{"Rightarrow", CmdSymbolChar, 0xde},
	{"rightarrow", CmdSymbolChar, 0xae},
	{"sigma", CmdSymbolChar, (int) 's'},
	{"Sigma", CmdSymbolChar, (int) 'S'},
	{"sim", CmdSymbolChar, 0x7e},
	{"spadesuit", CmdSymbolChar, 0xaa},
	{"subset", CmdSymbolChar, 0xcc},
	{"subseteq", CmdSymbolChar, 0xcd},
	{"sum", CmdSymbolChar, 0xe5},
	{"supset", CmdSymbolChar, 0xc9},
	{"supseteq", CmdSymbolChar, 0xca},
	{"surd", CmdSymbolChar, 0xd6},
	{"tau", CmdSymbolChar, (int) 't'},
	{"textalpha", CmdSymbolChar, (int) 'a'},
	{"textbeta", CmdSymbolChar, (int) 'b'},
	{"textbullet", CmdSymbolChar, 0xb7},
	{"textchi", CmdSymbolChar, (int) 'c'},
	{"textDelta", CmdSymbolChar, (int) 'D'},
	{"textdelta", CmdSymbolChar, (int) 'd'},
	{"textepsilon", CmdSymbolChar, (int) 'e'},
	{"texteta", CmdSymbolChar, (int) 'h'},
	{"textGamma", CmdSymbolChar, (int) 'G'},
	{"textgamma", CmdSymbolChar, (int) 'g'},
	{"textiota", CmdSymbolChar, (int) 'i'},
	{"textkappa", CmdSymbolChar, (int) 'k'},
	{"textLambda", CmdSymbolChar, (int) 'L'},
	{"textlambda", CmdSymbolChar, (int) 'l'},
	{"textmu", CmdSymbolChar, (int) 'm'},
	{"textnu", CmdSymbolChar, (int) 'n'},
	{"textOmega", CmdSymbolChar, (int) 'W'},
	{"textomega", CmdSymbolChar, (int) 'w'},
	{"textperiodcentered", CmdSymbolChar, 0xd7},
	{"textPhi", CmdSymbolChar, (int) 'F'},
	{"textphi", CmdSymbolChar, (int) 'f'},
	{"textPi", CmdSymbolChar, (int) 'P'},
	{"textpi", CmdSymbolChar, (int) 'p'},
	{"textPsi", CmdSymbolChar, (int) 'Y'},
	{"textpsi", CmdSymbolChar, (int) 'y'},
	{"textSigma", CmdSymbolChar, (int) 'S'},
	{"textsigma", CmdSymbolChar, (int) 's'},
	{"textTau", CmdSymbolChar, (int) 'T'},
	{"texttau", CmdSymbolChar, (int) 't'},
	{"textTheta", CmdSymbolChar, (int) 'Q'},
	{"texttheta", CmdSymbolChar, (int) 'q'},
	{"textXi", CmdSymbolChar, (int) 'X'},
	{"textxi", CmdSymbolChar, (int) 'x'},
	{"textzeta", CmdSymbolChar, (int) 'z'},
	{"therefore", CmdSymbolChar, 0x5c},
	{"Theta", CmdSymbolChar, (int) 'Q'},
	{"theta", CmdSymbolChar, (int) 'q'},
	{"times", CmdSymbolChar, 0xb4},
	{"to", CmdSymbolChar, 0xae},
	{"Uparrow", CmdSymbolChar, 0xdd},
	{"uparrow", CmdSymbolChar, 0xad},
	{"upsilon", CmdSymbolChar, (int) 'u'},
	{"Upsilon", CmdSymbolChar, (int) 'U'},
	{"varepsilon", CmdSymbolChar, (int) 'e'},
	{"varnothing", CmdSymbolChar, 0xc6},
	{"varphi", CmdSymbolChar, (int) 'j'},
	{"varpi", CmdSymbolChar, (int) 'v'},
	{"varpropto", CmdSymbolChar, 0xb5},
	{"varsigma", CmdSymbolChar, (int) 'V'},
	{"vartheta", CmdSymbolChar, (int) 'J'},
	{"vee", CmdSymbolChar, 0xda},
	{"wedge", CmdSymbolChar, 0xd9},
	{"wp", CmdSymbolChar, 0xc3},
	{"xi", CmdSymbolChar, (int) 'x'},
	{"Xi", CmdSymbolChar, (int) 'X'},
	{"zeta", CmdSymbolChar, (int) 'z'},
	{"efloatseparator", CmdIgnoreParameter,0},
    {"", NULL, 0}
};

/********************************************************************
  commands found in the preamble of the LaTeX file
 ********************************************************************/
static CommandArray PreambleCommands[] = {
    {"documentclass", CmdDocumentStyle, 0},
    {"documentstyle", CmdDocumentStyle, 0},
    {"usepackage", CmdUsepackage, 0},
/*    {"begin", CmdPreambleBeginEnd, CMD_BEGIN},*/
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
    {"fancyfoot", CmdHeadFoot, CFOOT},
    {"fancyhead", CmdHeadFoot, CHEAD},   
    {"thepage", CmdThePage, 0},
    {"hyphenation", CmdHyphenation, 0},
    {"def", CmdNewDef, DEF_DEF},
    {"newcommand", CmdNewDef, DEF_NEW},
    {"providecommand", CmdNewDef, DEF_NEW},
    {"DeclareRobustCommand", CmdNewDef, DEF_NEW},
    {"DeclareRobustCommand*", CmdNewDef, DEF_NEW},
    {"renewcommand", CmdNewDef, DEF_RENEW},
    {"newenvironment", CmdNewEnvironment, DEF_NEW},
    {"renewenvironment", CmdNewEnvironment, DEF_RENEW},
    {"newtheorem", CmdNewTheorem, 0},
    {"renewtheorem", CmdIgnoreParameter, One_Opt_Two_NormParam},
    {"pagestyle", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"thispagestyle", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"pagenumbering", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"markboth", CmdIgnoreParameter, No_Opt_Two_NormParam},
    {"markright", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"makeindex", CmdIgnoreParameter, 0},
    {"makeglossary", CmdIgnoreParameter, 0},
    {"listoffiles", CmdIgnoreParameter, 0},
    {"nofiles", CmdIgnoreParameter, 0},
    {"makelabels", CmdIgnoreParameter, 0},
    {"hoffset", CmdSetTexLength, SL_HOFFSET},
    {"voffset", CmdSetTexLength, SL_VOFFSET},
    {"parindent", CmdSetTexLength, SL_PARINDENT},
    {"parskip", CmdSetTexLength, SL_PARSKIP},
    {"baselineskip", CmdSetTexLength, SL_BASELINESKIP},
    {"topmargin", CmdSetTexLength, SL_TOPMARGIN},
    {"textheight", CmdSetTexLength, SL_TEXTHEIGHT},
    {"headheight", CmdSetTexLength, SL_HEADHEIGHT},
    {"headsep", CmdSetTexLength, SL_HEADSEP},
    {"textwidth", CmdSetTexLength, SL_TEXTWIDTH},
    {"oddsidemargin", CmdSetTexLength, SL_ODDSIDEMARGIN},
    {"evensidemargin", CmdSetTexLength, SL_EVENSIDEMARGIN},
    {"footnotetext", CmdFootNote, FOOTNOTE_TEXT},
    {"endnotetext", CmdFootNote, FOOTNOTE_TEXT | FOOTNOTE_ENDNOTE},
    {"include", CmdInclude, 0},
    {"input", CmdInclude, 1},
    {"nobreakspace", CmdNonBreakSpace, 100},
    {"signature", CmdSignature, 0},
    {"hline", CmdHline, 0},
    {"cline", CmdHline, 1},
    {"ifx", CmdIf, 0},
    {"theendnotes", CmdTheEndNotes, 0},
    {"euro", CmdEuro, 0},
    {"EUR", CmdEuro, 1},
    {"celsius", CmdDegreeCelsius},
    {"degreecelsius", CmdDegreeCelsius},
    {"resizebox", CmdResizeBox, 0},
    {"resizebox*", CmdResizeBox, 1},    
    {"geometry",CmdGeometry,0},
	{"doublespacing", CmdDoubleSpacing, 0},
    {"verbositylevel", CmdVerbosityLevel, 0},
    {"iflatextortf",CmdIflatextortf,0},
    {"latextortftrue",CmdIgnore,1}, 
    {"latextortffalse",CmdIgnore,0}, 
    {"newif",CmdNewif,0},
    {"title", CmdTitle, TITLE_TITLE},
    {"author", CmdTitle, TITLE_AUTHOR},
    {"and", CmdAnd, 0},
    {"date", CmdTitle, TITLE_DATE},
    {"affiliation", CmdTitle, TITLE_AFFILIATION},
    {"abstract", CmdTitle, TITLE_ABSTRACT},
	{"acknowledgements", CmdTitle, TITLE_ACKNOWLEDGE},
    {"bibliographystyle", CmdBibliographyStyle, 0},
    {"extrasfrench", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"AtEndDocument", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"docnumber", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"", NULL, 0}
};                              /* end of list */

static CommandArray ItemizeCommands[] = {
    {"item", CmdItem, ITEMIZE_MODE},
    {"", NULL, 0}
};

static CommandArray DescriptionCommands[] = {
    {"item", CmdItem, DESCRIPTION_MODE},
    {"", NULL, 0}
};

static CommandArray EnumerateCommands[] = {
    {"item", CmdItem, ENUMERATE_MODE},
    {"", NULL, 0}
};

static CommandArray InparaenumCommands[] = {
    {"item", CmdItem, INPARAENUM_MODE},
    {"", NULL, 0}
};

static CommandArray FigureCommands[] = {
    {"caption", CmdCaption, 0},
    {"center", CmdAlign, PAR_CENTER},
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

static CommandArray CzechModeCommands[] = {
    {"uv", CmdCzechAbbrev, 0},
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
    {"inferieura", CmdFrenchAbbrev, INFERIEURA},
    {"superieura", CmdFrenchAbbrev, SUPERIEURA},
    {"lq", CmdFrenchAbbrev, FRENCH_LQ},
    {"rq", CmdFrenchAbbrev, FRENCH_RQ},
    {"lqq", CmdFrenchAbbrev, FRENCH_LQQ},
    {"rqq", CmdFrenchAbbrev, FRENCH_RQQ},
    {"pointvirgule", CmdFrenchAbbrev, POINT_VIRGULE},
    {"pointexclamation", CmdFrenchAbbrev, POINT_EXCLAMATION},
    {"pointinterrogation", CmdFrenchAbbrev, POINT_INTERROGATION},
    {"dittomark", CmdFrenchAbbrev, DITTO_MARK},
    {"deuxpoints", CmdFrenchAbbrev, DEUX_POINTS},
    {"fup", CmdFrenchAbbrev, FUP},
    {"up", CmdFrenchAbbrev, FUP},
    {"LCS", CmdFrenchAbbrev, LCS},
    {"FCS", CmdFrenchAbbrev, FCS},
    {"og", CmdFrenchAbbrev, FRENCH_OG},
    {"fg", CmdFrenchAbbrev, FRENCH_FG},
    {"", NULL, 0}
};

/********************************************************************/

/* commands for Russian Mode */

/********************************************************************/
static CommandArray RussianModeCommands[] = {
    {"CYRA", CmdCyrillicChar, 0xC0},
    {"CYRB", CmdCyrillicChar, 0xC1},
    {"CYRV", CmdCyrillicChar, 0xC2},
    {"CYRG", CmdCyrillicChar, 0xC3},
    {"CYRD", CmdCyrillicChar, 0xC4},
    {"CYRE", CmdCyrillicChar, 0xC5},
    {"CYRZH", CmdCyrillicChar, 0xC6},
    {"CYRZ", CmdCyrillicChar, 0xC7},
    {"CYRI", CmdCyrillicChar, 0xC8},
    {"CYRISHRT", CmdCyrillicChar, 0xC9},
    {"CYRK", CmdCyrillicChar, 0xCA},
    {"CYRL", CmdCyrillicChar, 0xCB},
    {"CYRM", CmdCyrillicChar, 0xCC},
    {"CYRN", CmdCyrillicChar, 0xCD},
    {"CYRO", CmdCyrillicChar, 0xCE},
    {"CYRP", CmdCyrillicChar, 0xCF},
    {"CYRR", CmdCyrillicChar, 0xD0},
    {"CYRS", CmdCyrillicChar, 0xD1},
    {"CYRT", CmdCyrillicChar, 0xD2},
    {"CYRU", CmdCyrillicChar, 0xD3},
    {"CYRF", CmdCyrillicChar, 0xD4},
    {"CYRH", CmdCyrillicChar, 0xD5},
    {"CYRC", CmdCyrillicChar, 0xD6},
    {"CYRCH", CmdCyrillicChar, 0xD7},
    {"CYRSH", CmdCyrillicChar, 0xD8},
    {"CYRCHSH", CmdCyrillicChar, 0xD9},
    {"CYRHRDSN", CmdCyrillicChar, 0xDA},
    {"CYRERY", CmdCyrillicChar, 0xDB},
    {"CYRSFTSN", CmdCyrillicChar, 0xDC},
    {"CYREREV", CmdCyrillicChar, 0xDD},
    {"CYRYU", CmdCyrillicChar, 0xDE},
    {"CYRYA", CmdCyrillicChar, 0xDF},
    {"cyra", CmdCyrillicChar, 0xE0},
    {"cyrb", CmdCyrillicChar, 0xE1},
    {"cyrv", CmdCyrillicChar, 0xE2},
    {"cyrg", CmdCyrillicChar, 0xE3},
    {"cyrd", CmdCyrillicChar, 0xE4},
    {"cyre", CmdCyrillicChar, 0xE5},
    {"cyrzh", CmdCyrillicChar, 0xE6},
    {"cyrz", CmdCyrillicChar, 0xE7},
    {"cyri", CmdCyrillicChar, 0xE8},
    {"cyrishrt", CmdCyrillicChar, 0xE9},
    {"cyrk", CmdCyrillicChar, 0xEA},
    {"cyrl", CmdCyrillicChar, 0xEB},
    {"cyrm", CmdCyrillicChar, 0xEC},
    {"cyrn", CmdCyrillicChar, 0xED},
    {"cyro", CmdCyrillicChar, 0xEE},
    {"cyrp", CmdCyrillicChar, 0xEF},
    {"cyrr", CmdCyrillicChar, 0xF0},
    {"cyrs", CmdCyrillicChar, 0xF1},
    {"cyrt", CmdCyrillicChar, 0xF2},
    {"cyru", CmdCyrillicChar, 0xF3},
    {"cyrf", CmdCyrillicChar, 0xF4},
    {"cyrh", CmdCyrillicChar, 0xF5},
    {"cyrc", CmdCyrillicChar, 0xF6},
    {"cyrch", CmdCyrillicChar, 0xF7},
    {"cyrsh", CmdCyrillicChar, 0xF8},
    {"cyrchsh", CmdCyrillicChar, 0xF9},
    {"cyrhrdsn", CmdCyrillicChar, 0xFA},
    {"cyrery", CmdCyrillicChar, 0xFB},
    {"cyrsftsn", CmdCyrillicChar, 0xFC},
    {"cyrerev", CmdCyrillicChar, 0xFD},
    {"cyryu", CmdCyrillicChar, 0xFE},
    {"cyrya", CmdCyrillicChar, 0xFF},
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
    {"document", Environment, DOCUMENT_MODE},
    {"tabbing", CmdTabbing, TABBING},
    {"figure", CmdFigure, FIGURE},
    {"figure*", CmdFigure, FIGURE_1},
    {"picture", CmdPicture, 0},
    {"minipage", CmdMinipage, 0},
    {"music", CmdMusic, 0},
    {"small", CmdTolerateEnviron, 0},
    {"pspicture", CmdPsPicture, 0},
    {"psgraph", CmdPsGraph, 0},

    {"quote", CmdQuote, QUOTE_MODE},
    {"quotation", CmdQuote, QUOTATION_MODE},
    {"enumerate", CmdList, ENUMERATE_MODE},
    {"list", CmdList, ITEMIZE_MODE},
    {"itemize", CmdList, ITEMIZE_MODE},
	{"compactitem", CmdList, ITEMIZE_MODE},
    {"description", CmdList, DESCRIPTION_MODE},
    
    {"asparaenum", CmdList, ENUMERATE_MODE},
	{"inparaenum", CmdList, INPARAENUM_MODE},
	{"compactenum", CmdList, ENUMERATE_MODE},
	{"compactdesc", CmdList, DESCRIPTION_MODE},
	{"compactitem", CmdList, ITEMIZE_MODE},

    {"verbatim", CmdVerbatim, VERBATIM_1},
    {"comment", CmdVerbatim, VERBATIM_4},
    {"verse", CmdVerse, 0},
    {"tabular", CmdTabular, TABULAR},
    {"tabular*", CmdTabular, TABULAR_STAR},
    {"longtable", CmdTabular, TABULAR_LONG},
    {"longtable*", CmdTabular, TABULAR_LONG_STAR},
    {"array", CmdArray, 1},

    {"displaymath", CmdEquation, EQN_DISPLAYMATH},
    {"equation", CmdEquation, EQN_EQUATION},
    {"equation*", CmdEquation, EQN_EQUATION_STAR},
    {"eqnarray*", CmdEquation, EQN_ARRAY_STAR},
    {"eqnarray", CmdEquation, EQN_ARRAY},
    {"align*", CmdEquation, EQN_ALIGN_STAR},
    {"align", CmdEquation, EQN_ALIGN},
    {"math", CmdEquation, EQN_MATH},

    {"multicolumn", CmdMultiCol, 0},
    {"letter", CmdLetter, 0},
    {"table", CmdTable, TABLE},
    {"table*", CmdTable, TABLE_STAR},
    {"thebibliography", CmdThebibliography, 0},
    {"abstract", CmdAbstract, 1},
	{"acknowledgments", CmdAcknowledgments, 0},
    {"titlepage", CmdTitlepage, 0},

    {"em", CmdEmphasize, F_EMPHASIZE_3},
    {"rmfamily", CmdFontFamily, F_FAMILY_ROMAN_3},
    {"sffamily", CmdFontFamily, F_FAMILY_SANSSERIF_3},
    {"ttfamily", CmdFontFamily, F_FAMILY_TYPEWRITER_3},
    {"bfseries", CmdFontSeries, F_SERIES_BOLD_3},
    {"mdseries", CmdFontSeries, F_SERIES_MEDIUM_3},
    {"itshape", CmdFontShape, F_SHAPE_ITALIC_3},
    {"scshape", CmdFontShape, F_SHAPE_CAPS_3},
    {"slshape", CmdFontShape, F_SHAPE_SLANTED_3},
    {"it", CmdFontShape, F_SHAPE_ITALIC_4},
    {"sc", CmdFontShape, F_SHAPE_CAPS_4},
    {"sl", CmdFontShape, F_SHAPE_SLANTED_4},
    {"bf", CmdFontShape, F_SERIES_BOLD_4},
    {"sf", CmdFontFamily, F_FAMILY_ROMAN_4},
    {"tt", CmdFontFamily, F_FAMILY_SANSSERIF_4},
    {"rm", CmdFontFamily, F_FAMILY_TYPEWRITER_4},
    {"Verbatim", CmdVerbatim, VERBATIM_2},
    {"alltt", CmdVerbatim, VERBATIM_3},
    {"latexonly", CmdIgnore, 0},
    {"htmlonly", CmdIgnoreEnviron, IGNORE_HTMLONLY},
    {"rawhtml", CmdIgnoreEnviron, IGNORE_RAWHTML},
    {"theindex", CmdIgnoreEnviron, 0},
    {"landscape", CmdTolerateEnviron, 0},
    {"sloppypar", CmdTolerateEnviron, 0},
    {"doublespace", CmdTolerateEnviron, 0},
    {"", NULL, 0}
};                              /* end of list */


/********************************************************************
purpose: commands for hyperlatex package 
********************************************************************/
static CommandArray hyperlatexCommands[] = {
    {"link", CmdLink, 0},
    {"xlink", CmdLink, 0},
    {"Cite", CmdLabel, LABEL_HYPERCITE},
    {"Ref", CmdLabel, LABEL_HYPERREF},
    {"Pageref", CmdLabel, LABEL_HYPERPAGEREF},
    {"S", CmdColsep, 0},
    {"", NULL, 0}
};                              /* end of list */

/********************************************************************
purpose: commands for hyperref package 
********************************************************************/
static CommandArray hyperrefCommands[] = {
    {"url",               CmdHtml, LABEL_URL_HYPER},
    {"href",              CmdHtml, LABEL_HREF},
	{"hypersetup",        CmdIgnoreParameter, No_Opt_One_NormParam},
	{"nolinkurl",         CmdHtml, LABEL_NO_LINK_URL},
	{"hyperbaseurl",      CmdHtml, LABEL_BASE_URL},
	{"hyperref",          CmdHtml, LABEL_HYPERREF},
    {"", NULL, 0}
};                              /* end of list */

/********************************************************************
purpose: commands for apacite package 
********************************************************************/
static CommandArray apaciteCommands[] = {
    {"BBOP", CmdApaCite, 0},    /* Open parenthesis Default is "(" */
    {"BBAA", CmdApaCite, 1},    /* Last ``and'' Default is "\&" */
    {"BBAB", CmdApaCite, 2},    /* Last ``and'' Default is "and" */
    {"BBAY", CmdApaCite, 3},    /* Punctuation Default is ", " */
    {"BBC", CmdApaCite, 4},     /* Punctuation Default is "; " */
    {"BBN", CmdApaCite, 5},     /* Punctuation Default is ", " */
    {"BBCP", CmdApaCite, 6},    /* Closing parenthesis, Default is ")" */
    {"BBOQ", CmdApaCite, 7},    /* Opening quote Default is the empty string */
    {"BBCQ", CmdApaCite, 8},    /* Closing quote Default is the empty string */
    {"BCBT", CmdApaCite, 9},    /* Comma Default is "," */
    {"BCBL", CmdApaCite, 10},   /* Comma Default is "," */
    {"BOthers", CmdApaCite, 11},    /* Used for ``others'' Default is "et~al." */
    {"BIP", CmdApaCite, 12},    /* ``In press'', Default is "in press" */
    {"BAnd", CmdApaCite, 13},   /* Used as ``and'' Default is "&" */
    {"BED", CmdApaCite, 14},    /* Editor Default is "Ed." */
    {"BEDS", CmdApaCite, 15},   /* Editors Default is "Eds." */
    {"BTRANS", CmdApaCite, 16}, /* Translator. Default is "Trans." */
    {"BTRANSS", CmdApaCite, 17},    /* Translators. Default is "Trans." */
    {"BCHAIR", CmdApaCite, 18}, /* Chair Default is "Chair" */
    {"BCHAIRS", CmdApaCite, 19},    /* Chairs. Default is "Chairs" */
    {"BVOL", CmdApaCite, 20},   /* Volume, Default is "Vol." */
    {"BVOLS", CmdApaCite, 21},  /* Volumes, Default is "Vols." */
    {"BNUM", CmdApaCite, 22},   /* Number, Default is "No." */
    {"BNUMS", CmdApaCite, 23},  /* Numbers, Default is "Nos." */
    {"BEd", CmdApaCite, 24},    /* Edition, Default is "ed." */
    {"BPG", CmdApaCite, 25},    /* Page, default is "p." */
    {"BPGS", CmdApaCite, 26},   /* Pages, default is "pp." */
    {"BTR", CmdApaCite, 27},    /* technical report Default is "Tech.\ Rep." */
    {"BPhD", CmdApaCite, 28},   /* Default is "Doctoral dissertation" */
    {"BUPhD", CmdApaCite, 29},  /* Unpublished PhD Default is "Unpublished doctoral dissertation" */
    {"BMTh", CmdApaCite, 30},   /* MS thesis Default is "Master's thesis" */
    {"BUMTh", CmdApaCite, 31},  /* unpublished MS Default is "Unpublished master's thesis" */
    {"BOWP", CmdApaCite, 32},   /* default is "Original work published " */
    {"BREPR", CmdApaCite, 33},  /* default is "Reprinted from " */
    {"BCnt", CmdApaCite, 34},   /* convert number to letter */
    {"BCntIP", CmdApaCite, 34}, /* convert number to letter */
    {"BBA", CmdApaCite, 35},    /* "&" in paren, "and" otherwise */
    {"AX", CmdApaCite, 36},     /* index name */
    {"BPBI", CmdApaCite,37},    /* Period between initials */
	{"BIn", CmdApaCite, 38},
    
    {"APACyear", CmdApaCite, CITE_APA_CITE_YEAR},
    {"APACmetastar", CmdApaCite, CITE_APA_CITE_METASTAR},
	{"APACciteatitle", CmdApaCite, CITE_APA_CITE_A_TITLE},
	{"APACcitebtitle", CmdApaCite, CITE_APA_CITE_B_TITLE},
	{"APACinsertmetastar", CmdApaCite, CITE_APA_CITE_INSERT},
	{"APACrefYearMonthDay", CmdApaCite, CITE_APA_YMD},
	{"APACrefatitle", CmdApaCite, CITE_APA_REF_A_TITLE},
	{"APACrefbtitle", CmdApaCite, CITE_APA_REF_B_TITLE},
	{"APACjournalVolNumPages", CmdApaCite, CITE_APA_JVNP},
	{"APACrefYear", CmdApaCite, CITE_APA_REF_YEAR},
	{"APACaddressPublisher", CmdApaCite, CITE_APA_ADD_PUB},
	{"PrintBackRefs", CmdApaCite, CITE_PRINT_BACK_REFS}, 
	{"PrintCardinal", CmdApaCite, CITE_PRINT_CARDINAL},
	{"APACaddressPublisherEqAuth", CmdApaCite, CITE_APA_ADD_PUB_EQ_AUTHOR},
	{"APACrefaetitle", CmdApaCite, CITE_APA_REF_A_E_TITLE},
	{"APACrefbetitle", CmdApaCite, CITE_APA_REF_B_E_TITLE},
	{"APACmonth", CmdApaCite, CITE_APA_MONTH},
	{"APACbVolEdTR", CmdApaCite, CITE_APA_B_VOL_ED_TR},
	{"APACaddressInstitution", CmdApaCite, CITE_APA_ADD_INST},
	{"APAChowpublished", CmdApaCite, CITE_APA_HOW},
	{"APACorigyearnote", CmdApaCite, CITE_APA_ORIG_YEAR_NOTE},
	{"APACrefnote", CmdApaCite, CITE_APA_REF_NOTE},
	{"APACbVolEdTRpgs", CmdApaCite, CITE_APA_B_VOL_ED_TR_PGS},
	{"APACorigjournalnote", CmdApaCite, CITE_APA_ORIG_JOUR},
	{"APACaddressInstitutionEqAuth", CmdApaCite, CITE_APA_ADD_PUB_EQ_AUTHOR},
	{"unskip", CmdApaCite, CITE_APA_UNSKIP},
	
	{"Bem", CmdEmphasize, F_EMPHASIZE_2},
    {"BCAY", CmdBCAY, 0},
    {"citeauthoryear", CmdBCAY, 0},
    {"fullcite", CmdCite, CITE_FULL},
    {"shortcite", CmdCite, CITE_SHORT},
    {"citeNP", CmdCite, CITE_CITE_NP},
    {"fullciteNP", CmdCite, CITE_FULL_NP},
    {"shortciteNP", CmdCite, CITE_SHORT_NP},
    {"citeA", CmdCite, CITE_CITE_A},
    {"fullciteA", CmdCite, CITE_FULL_A},
    {"shortciteA", CmdCite, CITE_SHORT_A},
    {"citeauthor", CmdCite, CITE_CITE_AUTHOR},
    {"fullciteauthor", CmdCite, CITE_FULL_AUTHOR},
    {"shortciteauthor", CmdCite, CITE_SHORT_AUTHOR},
    {"citeyear", CmdCite, CITE_YEAR},
    {"citeyearNP", CmdCite, CITE_YEAR_NP},
    {"shorttitle", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"rightheader", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"leftheader", CmdIgnoreParameter, No_Opt_One_NormParam},

    {"", NULL, 0}
};

/********************************************************************
purpose: commands for apacite package 
********************************************************************/
static CommandArray natbibCommands[] = {
    {"cite", CmdNatbibCite, CITE_CITE},
    {"citet", CmdNatbibCite, CITE_T},
    {"citet*", CmdNatbibCite, CITE_T_STAR},
    {"citep", CmdNatbibCite, CITE_P},
    {"citep*", CmdNatbibCite, CITE_P_STAR},
    {"citealt", CmdNatbibCite, CITE_ALT},
    {"citealp", CmdNatbibCite, CITE_ALP},
    {"citealt*", CmdNatbibCite, CITE_ALT_STAR},
    {"citealp*", CmdNatbibCite, CITE_ALP_STAR},
    {"citetext", CmdNatbibCite, CITE_TEXT},
    {"citeauthor", CmdNatbibCite, CITE_AUTHOR},
    {"citeauthor*", CmdNatbibCite, CITE_AUTHOR_STAR},
    {"citeyear", CmdNatbibCite, CITE_YEAR},
    {"citeyearpar", CmdNatbibCite, CITE_YEAR_P},
    {"Citet", CmdNatbibCite, CITE_T_CAP},
    {"Citep", CmdNatbibCite, CITE_P_CAP},
    {"Citealt", CmdNatbibCite, CITE_ALT_CAP},
    {"Citealp", CmdNatbibCite, CITE_ALP_CAP},
    {"Citeauthor", CmdNatbibCite, CITE_AUTHOR_CAP},
    {"bibpunct", CmdBibpunct, 0},
    {"", NULL, 0}
};

/********************************************************************
purpose: commands for harvard package 
********************************************************************/
static CommandArray harvardCommands[] = {
    {"cite", CmdHarvardCite, CITE_CITE},
    {"citeasnoun", CmdHarvardCite, CITE_AS_NOUN},
    {"possessivecite", CmdHarvardCite, CITE_POSSESSIVE},
    {"citeaffixed", CmdHarvardCite, CITE_AFFIXED},
    {"citeyear", CmdHarvardCite, CITE_YEAR},
    {"citeyear*", CmdHarvardCite, CITE_YEAR_STAR},
    {"citename", CmdHarvardCite, CITE_NAME},
    {"harvarditem", CmdHarvard, CITE_HARVARD_ITEM},
    {"harvardand", CmdHarvard, CITE_HARVARD_AND},
    {"harvardyearleft", CmdHarvard, CITE_HARVARD_YEAR_LEFT},
    {"harvardyearright", CmdHarvard, CITE_HARVARD_YEAR_RIGHT},
    {"", NULL, 0}
};

/********************************************************************
purpose: commands for authordate package 
********************************************************************/
static CommandArray authordateCommands[] = {
    {"citename", CmdCiteName, 0},
    {"shortcite", CmdCite, CITE_SHORT},
    {"", NULL, 0}
};

/********************************************************************
purpose: commands for verbatim commands (placeholder) 
********************************************************************/
static CommandArray verbatimCommands[] = {
    {"", NULL, 0}
};

/********************************************************************
purpose: commands for quote commands (placeholder) 
********************************************************************/
static CommandArray quoteCommands[] = {
    {"", NULL, 0}
};

/********************************************************************
purpose: commands for quotation commands (placeholder) 
********************************************************************/
static CommandArray quotationCommands[] = {
    {"", NULL, 0}
};

/********************************************************************
purpose: commands for verse commands (placeholder) 
********************************************************************/
static CommandArray verseCommands[] = {
    {"", NULL, 0}
};

/********************************************************************
purpose: commands for generic commands (placeholder) 
********************************************************************/
static CommandArray genericCommands[] = {
    {"", NULL, 0}
};

/********************************************************************
purpose: commands for bibliography commands (placeholder) 
********************************************************************/
static CommandArray bibliographyCommands[] = {
    {"", NULL, 0}
};

/********************************************************************
purpose: commands for ignored commands (placeholder) 
********************************************************************/
static CommandArray ignoreCommands[] = {
    {"", NULL, 0}
};

bool CallCommandFunc(char *cCommand)

/****************************************************************************
purpose: Tries to call the command-function for the commandname
params:  string with command name
returns: success or failure
globals: command-functions have side effects or recursive calls
 ****************************************************************************/
{
    int iCommand, iEnv,user_def_index;
    char *macro_string;

    diagnostics(5, "CallCommandFunc seeking <%s> (%d environments to look through)", cCommand, iEnvCount);

    user_def_index = existsDefinition(cCommand);
    if (user_def_index > -1) {
        macro_string = expandDefinition(user_def_index);
        diagnostics(5, "CallCommandFunc <%s> expanded to <%s>", cCommand, macro_string);
        ConvertString(macro_string);
        free(macro_string);
        return TRUE;
    }

	/* search backwards through chain of environments*/
    for (iEnv = iEnvCount - 1; iEnv >= 0; iEnv--) {
    
    	/* test every command in the current enviroment */
        iCommand = 0;
        while (strcmp(Environments[iEnv][iCommand].cmd_name, "") != 0) {

          /*  if (iCommand<10)
            	diagnostics(8,"CallCommandFunc (%d,%3d) Trying %s",iEnv,iCommand,Environments[iEnv][iCommand].cmd_name);
		*/
		
            if (strcmp(Environments[iEnv][iCommand].cmd_name, cCommand) == 0) {
                if (Environments[iEnv][iCommand].func == NULL)
                    return FALSE;
                if (*Environments[iEnv][iCommand].func == CmdIgnoreParameter) {
                    diagnostics(2, "Unknown command '\\%s'", cCommand);
                }

                diagnostics(5, "CallCommandFunc Found '%s' iEnvCommand=%d number=%d", Environments[iEnv][iCommand].cmd_name, iEnv, iCommand);
                (*Environments[iEnv][iCommand].func) ((Environments[iEnv][iCommand].param));
                return TRUE;    /* Command Function found */
            }
            ++iCommand;
        }
    }
    return FALSE;
}


void CallParamFunc(char *cCommand, int AddParam)

/****************************************************************************
purpose: Try to call the environment-function for the commandname
params:  cCommand - string with command name
	 AddParam - param "ORed"(||) to the int param of command-funct
returns: sucess or not
globals: command-functions have side effects or recursive calls
 ****************************************************************************/
{
    int i = 0;
    char unknown_environment[100];

    while (strcmp(params[i].cmd_name, "") != 0) {
        if (strcmp(params[i].cmd_name, cCommand) == 0) {
            assert(params[i].func != NULL);
            (*params[i].func) ((params[i].param) | AddParam);
            return;             /* command function found */
        }
        ++i;
    }

    /* unknown environment must be ignored */
    if (AddParam == ON) {
        snprintf(unknown_environment, 100, "\\%s%s%s", "end{", cCommand, "}");
        Ignore_Environment(cCommand);
        diagnostics(WARNING, "Unknown environment \\begin{%s} ... \end{%s}", cCommand, cCommand);
    }
}

int CurrentEnvironmentCount(void)

/****************************************************************************
purpose: to eliminate the iEnvCount global variable 
****************************************************************************/
{
    return iEnvCount;
}

/****************************************************************************
purpose: returns a name for the current environment
 ****************************************************************************/
static char *EnvironmentName(CommandArray *code)
{
	if (code == PreambleCommands)
		return strdup("preamble");
	if (code == commands)
		return strdup("document");
	if (code == ItemizeCommands)
		return strdup("itemize");
	if (code == EnumerateCommands)
		return strdup("enumerate");
	if (code == DescriptionCommands)
		return strdup("description");
	if (code == InparaenumCommands)
		return strdup("inparaenum");
	if (code == LetterCommands)
		return strdup("letter");
	if (code == GermanModeCommands)
		return strdup("german");
	if (code == FrenchModeCommands)
		return strdup("french");
	if (code == RussianModeCommands)
		return strdup("russian");
	if (code == CzechModeCommands)
		return strdup("czech");
	if (code == FigureCommands)
		return strdup("figure");
	if (code == ignoreCommands)
		return strdup("unknown environment");
	if (code == hyperlatexCommands)
		return strdup("hyperlatex");
	if (code == apaciteCommands)
		return strdup("apacite");
	if (code == natbibCommands)
		return strdup("natbib");
	if (code == harvardCommands)
		return strdup("harvard");
	if (code == authordateCommands)
		return strdup("authordate");
	if (code == verbatimCommands)
		return strdup("verbatim");
	if (code == quoteCommands)
		return strdup("quote");
	if (code == quotationCommands)
		return strdup("quotation");
	if (code == bibliographyCommands)
		return strdup("bibliography");
	if (code == verseCommands)
		return strdup("verse");
	if (code == genericCommands)
		return strdup("generic");

	return strdup("unknown");
}

/****************************************************************************
purpose: returns a name for the current environment number
 ****************************************************************************/
static char *EnvironmentNameByNumber(int n)
{
    if (n<0) return "";
	return EnvironmentName(Environments[n]);
}

/****************************************************************************
purpose: prints the names of all the current environments
 ****************************************************************************/
/*
static void WriteEnvironmentStack(void)
{
    int i;
    char *s;
        
    for (i=0; i<iEnvCount; i++) {
    	s=EnvironmentName(Environments[i]);
    	diagnostics(WARNING, "Environments[%2d]=\"%12s\"", i, s);
    	free(s);
    }
}
*/

void PushEnvironment(int code)

/****************************************************************************
purpose: adds the command list for a specific environment to the list
	 of commands searched through.
params:  constant identifying the environment
globals: changes Environment - array of active environments
		 iEnvCount   - counter of active environments
 ****************************************************************************/
{
    char *diag;
	
    g_par_indent_array[iEnvCount] = getLength("parindent");
    g_left_indent_array[iEnvCount] = getLeftMarginIndent();
    g_right_indent_array[iEnvCount] = getRightMarginIndent();
    g_align_array[iEnvCount] = getAlignment();

    PushFontSettings();

    switch (code) {
        case PREAMBLE_MODE:
            Environments[iEnvCount] = PreambleCommands;
            break;
        case DOCUMENT_MODE:
            Environments[iEnvCount] = commands;
            break;
        case ITEMIZE_MODE:
            Environments[iEnvCount] = ItemizeCommands;
            break;
        case ENUMERATE_MODE:
            Environments[iEnvCount] = EnumerateCommands;
            break;
        case INPARAENUM_MODE:
            Environments[iEnvCount] = InparaenumCommands;
            break;
		case LETTER_MODE:
            Environments[iEnvCount] = LetterCommands;
            break;
        case DESCRIPTION_MODE:
            Environments[iEnvCount] = DescriptionCommands;
            break;
        case GERMAN_MODE:
            Environments[iEnvCount] = GermanModeCommands;
            break;
        case FRENCH_MODE:
            Environments[iEnvCount] = FrenchModeCommands;
            break;
        case RUSSIAN_MODE:
            Environments[iEnvCount] = RussianModeCommands;
            break;
        case CZECH_MODE:
            Environments[iEnvCount] = CzechModeCommands;
            break;
        case FIGURE_MODE:
            Environments[iEnvCount] = FigureCommands;
            break;
        case HYPERLATEX_MODE:
            Environments[iEnvCount] = hyperlatexCommands;
            break;
        case APACITE_MODE:
            Environments[iEnvCount] = apaciteCommands;
            break;
        case NATBIB_MODE:
            Environments[iEnvCount] = natbibCommands;
            break;
        case HARVARD_MODE:
            Environments[iEnvCount] = harvardCommands;
            break;
        case AUTHORDATE_MODE:
            Environments[iEnvCount] = authordateCommands;
            break;            
        case VERBATIM_MODE:
            Environments[iEnvCount] = verbatimCommands;
            break;            
        case QUOTATION_MODE:
            Environments[iEnvCount] = quotationCommands;
            break;            
        case QUOTE_MODE:
            Environments[iEnvCount] = quoteCommands;
            break;            
        case VERSE_MODE:
            Environments[iEnvCount] = verseCommands;
            break;            
        case BIBLIOGRAPHY_MODE:
            Environments[iEnvCount] = bibliographyCommands;
            break;            
        case GENERIC_MODE:
            Environments[iEnvCount] = genericCommands;
            break;
        case HYPERREF_MODE:
            Environments[iEnvCount] = hyperrefCommands;
            break;
        case IGNORE_MODE:
            Environments[iEnvCount] = ignoreCommands;
            break;

        default:
            diagnostics(ERROR, "assertion failed at function PushEnvironment");
    }
     
    iEnvCount++;
    diag = EnvironmentNameByNumber(iEnvCount-1);
    diagnostics(4, "\\begin{%s} [%d]", diag, iEnvCount-1);
	free(diag);

  /*  WriteEnvironmentStack();*/
}

/****************************************************************************
purpose: removes the environment-commands list added by last PushEnvironment;
globals: changes Environment - array of active environments
		 iEnvCount - counter of active environments
 ****************************************************************************/
void PopEnvironment()
{
	char *this_env, *last_env;

	this_env = EnvironmentNameByNumber(iEnvCount-1);
	last_env = EnvironmentNameByNumber(iEnvCount-2);
    
    /* always pop the current environment */
    --iEnvCount;
    Environments[iEnvCount] = NULL;
    
    setLength("parindent", g_par_indent_array[iEnvCount]);
    setLeftMarginIndent(g_left_indent_array[iEnvCount]);
    setRightMarginIndent(g_right_indent_array[iEnvCount]);
    setAlignment(g_align_array[iEnvCount]);
    PopFontSettings();

    diagnostics(3, "\\end{%s} [%d]", this_env, iEnvCount-1);
   
	free(this_env);
	free(last_env);
	
  /*  WriteEnvironmentStack(); */
}
