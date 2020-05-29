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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

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
#include "encodings.h"

#include "labels.h"
#include "acronyms.h"
#include "biblio.h"

static int iEnvCount = 0;               /* number of current environments */
static CommandArray *Environments[101]; /* call chain for current environments */
static int g_par_indent_array[100];
static int g_left_indent_array[100];
static int g_right_indent_array[100];
static char g_align_array[100];

static CommandArray commands[] = {
    {"begin", CmdBeginEnd, CMD_BEGIN},
    {"end", CmdBeginEnd, CMD_END},
    {"today", CmdToday, 0},
    {"footnote", CmdFootNote, FOOTNOTE},
    {"endnote", CmdFootNote, ENDNOTE},

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
    {"bm", CmdFontSeries, F_SERIES_BOLD_2},

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
    {"accent", CmdRingChar, 1},
    {"b", CmdUnderbarChar, 0},
    {"c", CmdCedillaChar, 0},
    {"i", CmdDotlessChar, 0},
    {"j", CmdDotlessChar, 1},
    {"H", CmdDoubleAcuteChar, 0},
    {"l", CmdUnicodeChar, 323},
    {"L", CmdUnicodeChar, 322},
    
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

    {"ldots", CmdUnicodeChar, 8230},
    {"dots", CmdUnicodeChar, 8230},
    {"dotfill", CmdUnicodeChar, 8230},
    {"textellipsis", CmdUnicodeChar, 8230},

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
    {"makebox", CmdBox, BOX_MAKEBOX},
    {"framebox", CmdBox, BOX_FRAMEBOX},
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
    {"refstepcounter", CmdCounter, COUNTER_INCR},
    {"fnsymbol", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"Alph", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"alph", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"Roman", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"roman", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"arabic", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"newcount", CmdIgnoreDef, 0},
    {"output", CmdIgnoreDef, 0},
    {"value", CmdCounter, COUNTER_VALUE},
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
    {"rightleftharpoons", CmdArrows, RIGHT_LEFT_HARPOONS},
    {"psset", CmdPsset, 0},
    {"newpsstyle", CmdNewPsStyle, 0},

    {"nonumber", CmdNonumber, EQN_NO_NUMBER},
    {"notag", CmdNonumber, EQN_NO_NUMBER},
    {"char", CmdSymbol, 0},
    {"symbol", CmdSymbol, 1},
    {"rlap", CmdLap, 0},
    {"llap", CmdLap, 1},
    {"not", CmdNot, 0},

    {"url",               CmdHtml, LABEL_URL},
    {"urlstyle",          CmdHtml, LABEL_URLSTYLE},
    {"htmladdnormallink", CmdHtml, LABEL_HTMLADDNORMALREF},
    {"htmlref",           CmdHtml, LABEL_HTMLREF},
    
    {"nobreakspace", CmdNonBreakSpace, 100},
    {"thinspace", CmdNonBreakSpace, 50},
    {"abstract", CmdAbstract, ABSTRACT_SIMPLE},
    {"keywords", CmdKeywords, 0},
    {"endinput", CmdEndInput, 0},
    {"color", CmdTextColor, 0},
    {"textcolor", CmdTextColor, 1},
    {"tableofcontents", CmdTableOfContents, 0},
    {"listoffigures", CmdListOf, LIST_OF_FIGURES},
    {"listoftables", CmdListOf, LIST_OF_TABLES},
    {"numberline", CmdNumberLine, 0},
    {"contentsline", CmdContentsLine, 0},
    {"centering", CmdAlign, PAR_CENTERING},
    
    {"halign", CmdHAlign, 0},
        
    {"efloatseparator", CmdIgnoreParameter,0},
    {"pagestyle", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"pagenumbering", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"markboth", CmdIgnoreParameter, No_Opt_Two_NormParam},
    {"markright", CmdIgnoreParameter, No_Opt_One_NormParam},

    /************ commands for auxfile.c *******************/
    { "newlabel",      CmdNewLabel,       0 },
    { "newacro",       CmdAcrodef,        ACRONYM_NEWACRO },
    { "newacroplural", CmdAcrodef,        ACRONYM_NEWACROPLURAL },
    { "harvardcite",   CmdAuxHarvardCite, 0 },
    { "bibcite",       CmdBibCite,        0 },
    
    {"usetikzlibrary",CmdTikzlib, 0},
    
    {"", NULL, 0}
};

/********************************************************************
  commands found in the preamble of the LaTeX file
 ********************************************************************/
static CommandArray PreambleCommands[] = {
    {"documentclass", CmdDocumentStyle, 0},
    {"documentstyle", CmdDocumentStyle, 0},
    {"usepackage", CmdUsepackage, 0},
    {"RequirePackage", CmdUsepackage, 0},
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
    {"hfil", CmdHfill, 0},
    {"hfill", CmdHfill, 1},
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
    {"linewidth", CmdSetTexLength, SL_LINEWIDTH},
    {"oddsidemargin", CmdSetTexLength, SL_ODDSIDEMARGIN},
    {"evensidemargin", CmdSetTexLength, SL_EVENSIDEMARGIN},
    {"footnotetext", CmdFootNote, FOOTNOTE_TEXT},
    {"endnotetext", CmdFootNote, ENDNOTE_TEXT},
    {"include", CmdInclude, 0},
    {"input", CmdInclude, 1},
    {"nobreakspace", CmdNonBreakSpace, 100},
    {"signature", CmdSignature, 0},
    {"hline", CmdHline, 0},
    {"cline", CmdHline, 1},
    {"ifx", CmdIf, 0},
    {"ifpdf", CmdIf, 0},
    {"theendnotes", CmdTheEndNotes, 0},
    {"euro", CmdEuro, 0},
    {"EUR", CmdEuro, 1},
    {"celsius", CmdDegreeCelsius, 0},
    {"degreecelsius", CmdDegreeCelsius, 0},
    {"resizebox", CmdResizeBox, 0},
    {"resizebox*", CmdResizeBox, 1},    
    {"geometry",CmdGeometry,0},
    {"singlespacing", CmdLineSpacing, LINE_SPACING_SINGLE},
    {"onehalfspacing", CmdLineSpacing, LINE_SPACING_ONE_AND_A_HALF},
    {"doublespacing", CmdLineSpacing, LINE_SPACING_DOUBLE},
    {"verbositylevel", CmdVerbosityLevel, 0},
    {"iflatextortf",CmdIflatextortf,0},
    {"latextortftrue",CmdIgnore,1}, 
    {"latextortffalse",CmdIgnore,0}, 
    {"newif",CmdNewif,0},
    {"else",CmdElse,0},
    {"fi",CmdFi,0},
    {"title", CmdTitle, TITLE_TITLE},
    {"author", CmdTitle, TITLE_AUTHOR},
    {"and", CmdAnd, 0},
    {"date", CmdTitle, TITLE_DATE},
    {"affiliation", CmdTitle, TITLE_AFFILIATION},
    {"abstract", CmdTitle, TITLE_ABSTRACT},
    {"keywords", CmdKeywords, 0},
    {"acknowledgements", CmdTitle, TITLE_ACKNOWLEDGE},
    {"bibliographystyle", CmdBibliographyStyle, 0},
    {"bibstyle", CmdBibStyle, 0},
    {"extrasfrench", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"AtEndDocument", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"docnumber", CmdIgnoreParameter, No_Opt_One_NormParam},
    {"graphicspath",  CmdGraphicsPath, 0 },
    {"", NULL, 0}
};                              /* end of list */

static CommandArray ItemizeCommands[] = {
    {"item", CmdItem, ITEMIZE_MODE},
    {"", NULL, 0}
};

static CommandArray DescriptionCommands[] = {
    {"item", CmdItem, DESCRIPTION_MODE},
    /* {"acro", CmdAcronymItem, 0}, */
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
    {"cyr", CmdFontEncoding, ENCODING_OT2},
    {"CYRYO",CmdUnicodeChar,0x0401},
    {"CYRDJE",CmdUnicodeChar,0x0402},
    {"CYRIE",CmdUnicodeChar,0x0404},
    {"CYRDZE",CmdUnicodeChar,0x0405},
    {"CYRII",CmdUnicodeChar,0x0406},
    {"CYRYI",CmdUnicodeChar,0x0407},
    {"CYRJE",CmdUnicodeChar,0x0408},
    {"CYRLJE",CmdUnicodeChar,0x0409},
    {"CYRNJE",CmdUnicodeChar,0x040A},
    {"CYRTSHE",CmdUnicodeChar,0x040B},
    {"CYRUSHRT",CmdUnicodeChar,0x040E},
    {"CYRDZHE",CmdUnicodeChar,0x040F},
    {"CYRA",CmdUnicodeChar,0x0410},
    {"CYRB",CmdUnicodeChar,0x0411},
    {"CYRV",CmdUnicodeChar,0x0412},
    {"CYRG",CmdUnicodeChar,0x0413},
    {"CYRD",CmdUnicodeChar,0x0414},
    {"CYRE",CmdUnicodeChar,0x0415},
    {"CYRZH",CmdUnicodeChar,0x0416},
    {"CYRZ",CmdUnicodeChar,0x0417},
    {"CYRI",CmdUnicodeChar,0x0418},
    {"CYRISHRT",CmdUnicodeChar,0x0419},
    {"CYRK",CmdUnicodeChar,0x041A},
    {"CYRL",CmdUnicodeChar,0x041B},
    {"CYRM",CmdUnicodeChar,0x041C},
    {"CYRN",CmdUnicodeChar,0x041D},
    {"CYRO",CmdUnicodeChar,0x041E},
    {"CYRP",CmdUnicodeChar,0x041F},
    {"CYRR",CmdUnicodeChar,0x0420},
    {"CYRS",CmdUnicodeChar,0x0421},
    {"CYRT",CmdUnicodeChar,0x0422},
    {"CYRU",CmdUnicodeChar,0x0423},
    {"CYRF",CmdUnicodeChar,0x0424},
    {"CYRH",CmdUnicodeChar,0x0425},
    {"CYRC",CmdUnicodeChar,0x0426},
    {"CYRCH",CmdUnicodeChar,0x0427},
    {"CYRSH",CmdUnicodeChar,0x0428},
    {"CYRSHCH",CmdUnicodeChar,0x0429},
    {"CYRHRDSN",CmdUnicodeChar,0x042A},
    {"CYRERY",CmdUnicodeChar,0x042B},
    {"CYRSFTSN",CmdUnicodeChar,0x042C},
    {"CYREREV",CmdUnicodeChar,0x042D},
    {"CYRYU",CmdUnicodeChar,0x042E},
    {"CYRYA",CmdUnicodeChar,0x042F},
    {"cyra",CmdUnicodeChar,0x0430},
    {"cyrb",CmdUnicodeChar,0x0431},
    {"cyrv",CmdUnicodeChar,0x0432},
    {"cyrg",CmdUnicodeChar,0x0433},
    {"cyrd",CmdUnicodeChar,0x0434},
    {"cyre",CmdUnicodeChar,0x0435},
    {"cyrzh",CmdUnicodeChar,0x0436},
    {"cyrz",CmdUnicodeChar,0x0437},
    {"cyri",CmdUnicodeChar,0x0438},
    {"cyrishrt",CmdUnicodeChar,0x0439},
    {"cyrk",CmdUnicodeChar,0x043A},
    {"cyrl",CmdUnicodeChar,0x043B},
    {"cyrm",CmdUnicodeChar,0x043C},
    {"cyrn",CmdUnicodeChar,0x043D},
    {"cyro",CmdUnicodeChar,0x043E},
    {"cyrp",CmdUnicodeChar,0x043F},
    {"cyrr",CmdUnicodeChar,0x0440},
    {"cyrs",CmdUnicodeChar,0x0441},
    {"cyrt",CmdUnicodeChar,0x0442},
    {"cyru",CmdUnicodeChar,0x0443},
    {"cyrf",CmdUnicodeChar,0x0444},
    {"cyrh",CmdUnicodeChar,0x0445},
    {"cyrc",CmdUnicodeChar,0x0446},
    {"cyrch",CmdUnicodeChar,0x0447},
    {"cyrsh",CmdUnicodeChar,0x0448},
    {"cyrshch",CmdUnicodeChar,0x0449},
    {"cyrhrdsn",CmdUnicodeChar,0x044A},
    {"cyrery",CmdUnicodeChar,0x044B},
    {"cyrsftsn",CmdUnicodeChar,0x044C},
    {"cyrerev",CmdUnicodeChar,0x044D},
    {"cyryu",CmdUnicodeChar,0x044E},
    {"cyrya",CmdUnicodeChar,0x044F},
    {"cyryo",CmdUnicodeChar,0x0451},
    {"cyrdje",CmdUnicodeChar,0x0452},
    {"cyrie",CmdUnicodeChar,0x0454},
    {"cyrdze",CmdUnicodeChar,0x0455},
    {"cyrii",CmdUnicodeChar,0x0456},
    {"cyryi",CmdUnicodeChar,0x0457},
    {"cyrje",CmdUnicodeChar,0x0458},
    {"cyrlje",CmdUnicodeChar,0x0459},
    {"cyrnje",CmdUnicodeChar,0x045A},
    {"cyrtshe",CmdUnicodeChar,0x045B},
    {"cyrushrt",CmdUnicodeChar,0x045E},
    {"cyrdzhe",CmdUnicodeChar,0x045F},
    {"CYRYAT",CmdUnicodeChar,0x0462},
    {"cyryat",CmdUnicodeChar,0x0463},
    {"CYRBYUS",CmdUnicodeChar,0x046A},
    {"cyrbyus",CmdUnicodeChar,0x046B},    
    {"CYRFITA",CmdUnicodeChar,0x0472},    
    {"cyrfita",CmdUnicodeChar,0x0473},    
    {"CYRIZH",CmdUnicodeChar,0x0474},    
    {"cyrizh",CmdUnicodeChar,0x0475},    
    {"U", CmdWideBreveChar, 0},
    {"", NULL, 0}
};

static CommandArray spacingCommands[] = {
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
    {"wrapfigure", CmdFigure, WRAP_FIGURE},
    {"figure*", CmdFigure, FIGURE_1},
    {"picture", CmdPicture, 0},
    {"minipage", CmdMinipage, 0},
    {"music", CmdMusic, 0},
    {"pspicture", CmdPsPicture, 0},
    {"psgraph", CmdPsGraph, 0},

    {"quote", CmdQuote, QUOTE_MODE},
    {"quotation", CmdQuote, QUOTATION_MODE},
    {"enumerate", CmdList, ENUMERATE_MODE},
    {"list", CmdList, ITEMIZE_MODE},
    {"itemize", CmdList, ITEMIZE_MODE},
    {"compactitem", CmdList, ITEMIZE_MODE},
    {"description", CmdList, DESCRIPTION_MODE},
    
    {"enumerate*", CmdList, ENUMERATE_MODE},
    {"itemize*", CmdList, ITEMIZE_MODE},
    {"description*", CmdList, DESCRIPTION_MODE},
    {"basedescipt", CmdList, DESCRIPTION_MODE},
    
    {"acronym", CmdBeginAcronym, 0},
    
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
    {"abstract", CmdAbstract, ABSTRACT_BEGIN_END},
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
    {"rm", CmdFontFamily, F_FAMILY_ROMAN_4},
    {"sf", CmdFontFamily, F_FAMILY_SANSSERIF_4},
    {"tt", CmdFontFamily, F_FAMILY_TYPEWRITER_4},
    {"Verbatim", CmdVerbatim, VERBATIM_2},
    {"alltt", CmdVerbatim, VERBATIM_3},
    {"latexonly", CmdIgnore, 0},
    {"htmlonly", CmdIgnoreEnviron, IGNORE_HTMLONLY},
    {"rawhtml", CmdIgnoreEnviron, IGNORE_RAWHTML},
    {"theindex", CmdIgnoreEnviron, 0},
    {"landscape", CmdTolerateEnviron, 0},
    {"sloppypar", CmdTolerateEnviron, 0},
    {"doublespace", CmdSpacingEnviron, 2},
    {"spacing", CmdSpacingEnviron, 0},
    
    {"small", CmdFontSizeEnviron, 12},
	{"tiny", CmdFontSizeEnviron, 10},
	{"scriptsize", CmdFontSizeEnviron, 14},
	{"footnotesize", CmdFontSizeEnviron, 16},
	{"normalsize", CmdFontSizeEnviron, 20},
	{"large", CmdFontSizeEnviron, 24},
	{"Large", CmdFontSizeEnviron, 28},
	{"LARGE", CmdFontSizeEnviron, 34},
	{"huge", CmdFontSizeEnviron, 40},
	{"Huge", CmdFontSizeEnviron, 50}, 

    {"tikzpicture",CmdTikzPicture, 0},
	
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
    {"PrintOrdinal", CmdApaCite, CITE_PRINT_ORDINAL},
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
    {"natexlab", CmdNatexlab, 0},
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
        return strdup("figure or wrapfigure");
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
    if (code == acronymCommands)
        return strdup("acronym");
    if (code == spacingCommands)
        return strdup("setpace");
    return strdup("unknown");
}

/****************************************************************************
purpose: prints the names of all the current environments
 ****************************************************************************/
#if 0
static void WriteEnvironmentStack(void)
{
    int i;
    char *s;
        
    for (i=0; i<iEnvCount; i++) {
        s=EnvironmentName(Environments[i]);
        diagnostics(6, "Environments[%2d]=\"%12s\"", i, s);
        free(s);
    }
}
#endif

int CallCommandFunc(char *cCommand)

/****************************************************************************
purpose: Tries to call the command-function for the commandname
params:  string with command name
returns: success or failure
globals: command-functions have side effects or recursive calls
 ****************************************************************************/
{
    int iCommand, iEnv,user_def_index;
    char *macro_string;

    diagnostics(4, "CallCommandFunc seeking <%s> (%d environments to look through)", cCommand, iEnvCount);

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

            /*
            if (iCommand<3)
                diagnostics(1,"CallCommandFunc (%d,%3d) Trying %s",iEnv,iCommand,Environments[iEnv][iCommand].cmd_name);
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
        diagnostics(WARNING, "Unknown environment \\begin{%s} ... \\end{%s}", cCommand, cCommand);
    }
}

/****************************************************************************
purpose: returns a name for the current environment number
 ****************************************************************************/
static char *EnvironmentNameByNumber(int n)
{
    if (n<0) return "";
    return EnvironmentName(Environments[n]);
}

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
        case ACRONYM_MODE:
            Environments[iEnvCount] = acronymCommands;
            break;
        case SPACING_MODE:
            Environments[iEnvCount] = spacingCommands;
            break;

        default:
            diagnostics(ERROR, "assertion failed at function PushEnvironment");
    }
     
    iEnvCount++;
    diag = EnvironmentNameByNumber(iEnvCount-1);
    diagnostics(4, "\\begin{%s} [%d]", diag, iEnvCount-1);
    free(diag);

/*    WriteEnvironmentStack();*/
}

/****************************************************************************
purpose: removes the environment-commands list added by last PushEnvironment;
globals: changes Environment - array of active environments
         iEnvCount - counter of active environments
 ****************************************************************************/
void PopEnvironment(void)
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
    
/*    WriteEnvironmentStack(); */
}
