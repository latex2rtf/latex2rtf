
/* l2r_fonts.c - LaTeX commands that alter font size, style, or face

Copyright (C) 2001-2002 The Free Software Foundation

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
    2001-2002 Scott Prahl
*/

/*
    All changes to font size, font style, and font face are 
    handled in this file.  Explicit changing of font characteristics
    should not be done elsewhere.
    
    Font handling in LaTeX uses five independent parameters 
    
        * Font encoding    --- OT1, OT
        * Font family      --- roman, typewriter, sans serif
        * Font size        --- normal, large, ...
        * Font shape       --- upright, italics, small caps
        * Font series      --- medium, bold
        
    Font changes in LaTeX use commands that fall into three categories.  
    
        commands that are independent of the previous state, e.g.,
        {\sc before {\it some text}} will typeset in "some text"
        in italics 
        
        commands that add to the previous state {\sc before \textit{some text}}
        will typeset "some text" in italic small caps (if available)
        
        commands that are affected by the previous state {\it before \emph{some text}}
        will typeset "some text" in an upright font
        
    RTF has no commands to directly emulate this third type of command.  The first
    type is readily simulated by resetting the font properties before setting the
    desired setting.  The second type of command is the normal way that RTF handles
    fonts, and therefore is not a problem.

    Limiting the extent of font changes is handled by braces in the RTF file.  This
    leads to the following problem,
    
        \textit{some text {\em roman text} more italic text {\em more roman text}} 
        
    which should be translated to
    
        {\i some text {\i0 roman text} more italic text {\i0 more roman text}}
        
    when \em is encountered by latex2rtf, the extent of the emphasis is unknown:
    it may continue to the next brace, it may continue to the end of an environment
    \end{center}, or it may continue to the end of the document.  In the example above,
    the text will be reset to italics by the first closing brace.  This is easy, but
    the problem is that the at the next \em, it is necessary to know that the font has 
    been changed back.
    
    Consequently, it is necessary to know the current latex font setting *for each
    RTF brace level*.  The easiest way to do this is to filter everything fprintf'ed 
    to the RTF file
                
*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "convert.h"
#include "fonts.h"
#include "funct1.h"
#include "commands.h"
#include "cfg.h"
#include "parser.h"
#include "stack.h"
#include "vertical.h"

typedef struct RtfFontInfoType {
    int family;
    int shape;
    int series;
    int size;
    int encoding;
} RtfFontInfoType;

#define MAX_FONT_INFO_DEPTH 301
static RtfFontInfoType RtfFontInfo[MAX_FONT_INFO_DEPTH];
static int FontInfoDepth = 0;

/****************************************************************************
 *   purpose: returns the RTF font number from an RTF font name
     example: RtfFontNumber("Times")
 ****************************************************************************/
int RtfFontNumber(const char *Fname)
{
    ConfigEntryT **config_handle;
    char *font_type, *font_name;
    int font_id;

    diagnostics(6, "seeking=%s", Fname);

    config_handle = CfgStartIterate();

    while ((config_handle = CfgNext(FONT_A, config_handle)) != NULL) {
        font_type = (char *) (*config_handle)->TexCommand;
        font_name = (char *) (*config_handle)->RtfCommand;
        font_id   = (int   ) (*config_handle)->original_id;
        
        diagnostics(6, "RTF font %d has name='%s' of type='%s'", font_id, font_name, font_type);

        if (strcmp(font_name, Fname) == 0) 
            return font_id;
    }
    return TexFontNumber("Roman");  /* default font */
}

int TexFontNumber(const char *Fname)

/****************************************************************************
  purpose: returns the RTF font number for a particular LaTeX font
  example: TexFontNumber("Roman")
 ****************************************************************************/
{
    ConfigEntryT **p;
    int number=0;
    
    if (strcmp(Fname,"CurrentFontSize")==0)
        return CurrentFontSize();

    if (strcmp(Fname,"DefaultFontSize")==0)
        return DefaultFontSize();
                
    p = SearchCfgEntry(Fname, FONT_A);
    
    if (p) number = (**p).original_id;
        
    diagnostics(6,"TexFontNumber for '%s' is %d", Fname, number);
    
    return number;
}

void CmdFontFamily(int code)

/******************************************************************************
  purpose: selects the appropriate font family
                F_FAMILY_ROMAN    for \rmfamily
                F_FAMILY_ROMAN_1  for \rm
                F_FAMILY_ROMAN_2  for \textrm{...}
                F_FAMILY_ROMAN_3  for \begin{rmfamily} or \end{rmfamily}
 ******************************************************************************/
{
    char *s;
    int num, true_code;

    true_code = code & ~ON;

    diagnostics(6, "CmdFontFamily (before) depth=%d, family=%d, size=%d, shape=%d, series=%d",
      FontInfoDepth, RtfFontInfo[FontInfoDepth].family,
      RtfFontInfo[FontInfoDepth].size, RtfFontInfo[FontInfoDepth].shape, RtfFontInfo[FontInfoDepth].series);

    if (!(code & ON) &&
      (true_code == F_FAMILY_CALLIGRAPHIC_3 || true_code == F_FAMILY_TYPEWRITER_3 ||
        true_code == F_FAMILY_SANSSERIF_3 || true_code == F_FAMILY_ROMAN_3 ||
        true_code == F_FAMILY_TYPEWRITER_4 || true_code == F_FAMILY_SANSSERIF_4 || true_code == F_FAMILY_ROMAN_4))
        return;

    switch (true_code) {

        case F_FAMILY_SANSSERIF:
        case F_FAMILY_SANSSERIF_1:
        case F_FAMILY_SANSSERIF_2:
        case F_FAMILY_SANSSERIF_3:
        case F_FAMILY_SANSSERIF_4:
            num = TexFontNumber("Sans Serif");
            break;

        case F_FAMILY_TYPEWRITER:
        case F_FAMILY_TYPEWRITER_1:
        case F_FAMILY_TYPEWRITER_2:
        case F_FAMILY_TYPEWRITER_3:
        case F_FAMILY_TYPEWRITER_4:
            num = TexFontNumber("Typewriter");
            break;

        case F_FAMILY_CALLIGRAPHIC:
        case F_FAMILY_CALLIGRAPHIC_1:
        case F_FAMILY_CALLIGRAPHIC_2:
        case F_FAMILY_CALLIGRAPHIC_3:
            num = TexFontNumber("Calligraphic");
            break;
        
        case F_FAMILY_ROMAN:
        case F_FAMILY_ROMAN_1:
        case F_FAMILY_ROMAN_2:
        case F_FAMILY_ROMAN_3:
        case F_FAMILY_ROMAN_4:
        default:
            num = TexFontNumber("Roman");
            break;
    }


    switch (true_code) {
        case F_FAMILY_ROMAN:
        case F_FAMILY_SANSSERIF:
        case F_FAMILY_TYPEWRITER:
        case F_FAMILY_CALLIGRAPHIC:
        case F_FAMILY_ROMAN_3:
        case F_FAMILY_SANSSERIF_3:
        case F_FAMILY_TYPEWRITER_3:
        case F_FAMILY_CALLIGRAPHIC_3:
            fprintRTF("\\f%d ", num);
            break;

        case F_FAMILY_ROMAN_1:
        case F_FAMILY_SANSSERIF_1:
        case F_FAMILY_TYPEWRITER_1:
        case F_FAMILY_ROMAN_4:
        case F_FAMILY_SANSSERIF_4:
        case F_FAMILY_TYPEWRITER_4:
            fprintRTF("\\i0\\scaps0\\b0\\f%d ", num);
            break;

        case F_FAMILY_ROMAN_2:
        case F_FAMILY_SANSSERIF_2:
        case F_FAMILY_TYPEWRITER_2:
        case F_FAMILY_CALLIGRAPHIC_2:
            fprintRTF("{\\f%d ", num);
            s = getBraceParam();
            ConvertString(s);
            free(s);
            fprintRTF("}");
            break;
    }

    diagnostics(6, "CmdFontFamily (after) depth=%d, family=%d, size=%d, shape=%d, series=%d",
      FontInfoDepth, RtfFontInfo[FontInfoDepth].family,
      RtfFontInfo[FontInfoDepth].size, RtfFontInfo[FontInfoDepth].shape, RtfFontInfo[FontInfoDepth].series);
}

void CmdFontShape(int code)

/****************************************************************************
     purpose : sets the font to upright, italic, or small caps
                F_SHAPE_ITALIC    for \itshape
                F_SHAPE_ITALIC_1  for \it
                F_SHAPE_ITALIC_2  for \textit{...}
                F_SHAPE_ITALIC_3  for \begin{itshape}
                F_SHAPE_ITALIC_4  for \begin{it}

 ****************************************************************************/
{
    int true_code = code & ~ON;

    diagnostics(5, "CmdFontShape (before) depth=%d, family=%d, size=%d, shape=%d, series=%d",
      FontInfoDepth, RtfFontInfo[FontInfoDepth].family,
      RtfFontInfo[FontInfoDepth].size, RtfFontInfo[FontInfoDepth].shape, RtfFontInfo[FontInfoDepth].series);

    /* \end{itshape}, \end{sc} ... */
    if (!(code & ON) &&
      (true_code == F_SHAPE_UPRIGHT_3 || true_code == F_SHAPE_ITALIC_3 ||
        true_code == F_SHAPE_SLANTED_3 || true_code == F_SHAPE_CAPS_3 ||
        true_code == F_SHAPE_ITALIC_4 || true_code == F_SHAPE_SLANTED_4 || true_code == F_SHAPE_CAPS_4))
        return;

    switch (true_code) {

        case F_SHAPE_UPRIGHT:
        case F_SHAPE_UPRIGHT_3:
            fprintRTF("\\i0\\scaps0 ");
            break;

        case F_SHAPE_UPRIGHT_1:
            fprintRTF("\\i0\\scaps0\\b0 ");
            break;

        case F_SHAPE_UPRIGHT_2:
            fprintRTF("{\\i0\\b0\\scaps0 ");
            break;

        case F_SHAPE_SLANTED:
        case F_SHAPE_ITALIC:
            fprintRTF("\\scaps0\\i ");
            break;

        case F_SHAPE_SLANTED_1:
        case F_SHAPE_ITALIC_1:
            fprintRTF("\\scaps0\\b0\\i ");
            break;

        case F_SHAPE_SLANTED_2:
        case F_SHAPE_ITALIC_2:
            fprintRTF("{\\i ");
            break;

        case F_SHAPE_SLANTED_3:
        case F_SHAPE_ITALIC_3:
            fprintRTF("\\scaps0\\i ");
            break;

        case F_SHAPE_SLANTED_4:
        case F_SHAPE_ITALIC_4:
            fprintRTF("\\scaps0\\b0\\i ");
            break;

        case F_SHAPE_CAPS:
        case F_SHAPE_CAPS_3:
            fprintRTF("\\scaps ");
            break;

        case F_SHAPE_CAPS_1:
        case F_SHAPE_CAPS_4:
            fprintRTF("\\i0\\b0\\scaps ");
            break;

        case F_SHAPE_CAPS_2:
            fprintRTF("{\\scaps ");
            break;
    }

    if (true_code == F_SHAPE_UPRIGHT_2 || true_code == F_SHAPE_ITALIC_2 ||
      true_code == F_SHAPE_SLANTED_2 || true_code == F_SHAPE_CAPS_2) {
        char *s;

        s = getBraceParam();
        ConvertString(s);
        fprintRTF("}");
        free(s);
    }

    diagnostics(5, "CmdFontShape (after) depth=%d, family=%d, size=%d, shape=%d, series=%d",
      FontInfoDepth, RtfFontInfo[FontInfoDepth].family,
      RtfFontInfo[FontInfoDepth].size, RtfFontInfo[FontInfoDepth].shape, RtfFontInfo[FontInfoDepth].series);
}

void CmdFontSeries(int code)

/****************************************************************************
     purpose : sets the font weight to medium or bold
     
     F_SERIES_BOLD        for  \bfseries ... 
     F_SERIES_BOLD_1      for  \bf ... 
     F_SERIES_BOLD_2      for  \textbf{...}
     F_SERIES_BOLD_3      for  \begin{bfseries} ... \end{bfseries}

 ****************************************************************************/
{
    int true_code = code & ~ON;

    diagnostics(5, "CmdFontSeries (before) depth=%d, family=%d, size=%d, shape=%d, series=%d",
      FontInfoDepth, RtfFontInfo[FontInfoDepth].family,
      RtfFontInfo[FontInfoDepth].size, RtfFontInfo[FontInfoDepth].shape, RtfFontInfo[FontInfoDepth].series);

    /* either \end{bfseries} or \end{mdseries} */
    if ((true_code == F_SERIES_MEDIUM_3 || true_code == F_SERIES_BOLD_3 ||
        true_code == F_SERIES_BOLD_4) && !(code & ON))
        return;

    if (getTexMode() == MODE_VERTICAL)
        changeTexMode(MODE_HORIZONTAL);

    switch (code) {
        case F_SERIES_MEDIUM_3:
        case F_SERIES_MEDIUM:
            fprintRTF("\\b0 ");
            break;

        case F_SERIES_MEDIUM_1:
            fprintRTF("\\i0\\scaps0\\b0 ");
            break;

        case F_SERIES_MEDIUM_2:
            fprintRTF("{\\b0 ");
            break;

        case F_SERIES_BOLD:
        case F_SERIES_BOLD_3:
            fprintRTF("\\b ");
            break;

        case F_SERIES_BOLD_1:
        case F_SERIES_BOLD_4:
            fprintRTF("\\i0\\scaps0\\b ");
            break;

        case F_SERIES_BOLD_2:
            fprintRTF("{\\b ");
            break;
    }

    if (true_code == F_SERIES_BOLD_2 || true_code == F_SERIES_MEDIUM_2) {
        char *s;

        s = getBraceParam();
        ConvertString(s);
        fprintRTF("}");
        free(s);
    }

    diagnostics(5, "CmdFontShape (after) depth=%d, family=%d, size=%d, shape=%d, series=%d",
      FontInfoDepth, RtfFontInfo[FontInfoDepth].family,
      RtfFontInfo[FontInfoDepth].size, RtfFontInfo[FontInfoDepth].shape, RtfFontInfo[FontInfoDepth].series);
}

void CmdFontSize(int code)

/******************************************************************************
 purpose : handles LaTeX commands that change the font size
******************************************************************************/
{
    int scaled_size;

    diagnostics(5, "CmdFontSize (before) depth=%d, family=%d, size=%d, shape=%d, series=%d",
      FontInfoDepth, RtfFontInfo[FontInfoDepth].family,
      RtfFontInfo[FontInfoDepth].size, RtfFontInfo[FontInfoDepth].shape, RtfFontInfo[FontInfoDepth].series);

//    if (getTexMode() == MODE_VERTICAL)
//        changeTexMode(MODE_HORIZONTAL);

    if (code == F_SMALLER)
        scaled_size = (int) (CurrentFontSize() / 1.2 + 0.5);
    else if (code == F_LARGER)
        scaled_size = (int) (CurrentFontSize() * 1.2 + 0.5);
    else
        scaled_size = (int) (code * DefaultFontSize() / 20.0 + 0.5);

    fprintRTF("\\fs%d ", scaled_size);

    diagnostics(5, "CmdFontSize (after) depth=%d, family=%d, size=%d, shape=%d, series=%d",
      FontInfoDepth, RtfFontInfo[FontInfoDepth].family,
      RtfFontInfo[FontInfoDepth].size, RtfFontInfo[FontInfoDepth].shape, RtfFontInfo[FontInfoDepth].series);
}

void CmdFontSizeEnviron(int code)

/******************************************************************************
  purpose: sets the font size e.g., \begin{small} ... \end{small}
 ******************************************************************************/
{
    int true_code = code & ~ON;

    diagnostics(6, "CmdFontSizeEnviron (before) depth=%d, family=%d, size=%d, shape=%d, series=%d",
      FontInfoDepth, RtfFontInfo[FontInfoDepth].family,
      RtfFontInfo[FontInfoDepth].size, RtfFontInfo[FontInfoDepth].shape, RtfFontInfo[FontInfoDepth].series);

    if (!(code & ON)) return;
	CmdFontSize(true_code);

    diagnostics(6, "CmdFontSizeEnviron (after) depth=%d, family=%d, size=%d, shape=%d, series=%d",
      FontInfoDepth, RtfFontInfo[FontInfoDepth].family,
      RtfFontInfo[FontInfoDepth].size, RtfFontInfo[FontInfoDepth].shape, RtfFontInfo[FontInfoDepth].series);
}

void CmdEmphasize(int code)

/****************************************************************************
 purpose: LaTeX commands \em, \emph, and \begin{em} ... \end{em}
 
          the \emph{string} construction is handled by \textit{string} or \textup{string}
          
          {\em string} should be properly localized by brace mechanisms
          
          \begin{em} ... \end{em} will be localized by environment mechanisms

     F_EMPHASIZE_1        for  \em ... 
     F_EMPHASIZE_2        for  \emph{...}
     F_EMPHASIZE_3        for  \begin{em} ... \end{em}
 ******************************************************************************/
{
    int true_code = code & ~ON;

    diagnostics(5, "CmdEmphasize (before) depth=%d, family=%d, size=%d, shape=%d, series=%d",
      FontInfoDepth, RtfFontInfo[FontInfoDepth].family,
      RtfFontInfo[FontInfoDepth].size, RtfFontInfo[FontInfoDepth].shape, RtfFontInfo[FontInfoDepth].series);

    if (getTexMode() == MODE_VERTICAL)
        changeTexMode(MODE_HORIZONTAL);

    if (true_code == F_EMPHASIZE_3 && !(code & ON))
        return;

    if (true_code == F_EMPHASIZE_2) {

        if (CurrentFontShape() == F_SHAPE_UPRIGHT)
            CmdFontShape(F_SHAPE_ITALIC_2);
        else
            CmdFontShape(F_SHAPE_UPRIGHT_2);

    } else {

        if (CurrentFontShape() == F_SHAPE_UPRIGHT)
            fprintRTF("\\i ");
        else
            fprintRTF("\\i0 ");

    }

    diagnostics(5, "CmdEmphasize (after) depth=%d, family=%d, size=%d, shape=%d, series=%d",
      FontInfoDepth, RtfFontInfo[FontInfoDepth].family,
      RtfFontInfo[FontInfoDepth].size, RtfFontInfo[FontInfoDepth].shape, RtfFontInfo[FontInfoDepth].series);

}

void CmdUnderline(int code)

/****************************************************************************
 purpose: handle \underline{text}
 ******************************************************************************/
{
    char *s;

    diagnostics(5, "Entering CmdUnderline");

    if (getTexMode() == MODE_VERTICAL)
        changeTexMode(MODE_HORIZONTAL);

    fprintRTF("{\\ul ");
    s = getBraceParam();
    ConvertString(s);
    free(s);
    fprintRTF("}");
    diagnostics(5, "Exiting CmdUnderline");
}

void CmdTextNormal(int code)

/****************************************************************************
 purpose: handle \textnormal{text}  {\normalfont ...} commands

     F_TEXT_NORMAL        for  \normalfont ... 
     F_TEXT_NORMAL_1
     F_TEXT_NORMAL_2      for  \textnormal{...}
     F_TEXT_NORMAL_3      for  \begin{normalfont} ... \end{normalfont}

 ******************************************************************************/
{
    int true_code = code & ~ON;

    diagnostics(5, "CmdTextNormal (before) depth=%d, family=%d, size=%d, shape=%d, series=%d",
      FontInfoDepth, RtfFontInfo[FontInfoDepth].family,
      RtfFontInfo[FontInfoDepth].size, RtfFontInfo[FontInfoDepth].shape, RtfFontInfo[FontInfoDepth].series);

    if (true_code == F_TEXT_NORMAL_3 && !(code & ON))
        return;

    if (getTexMode() == MODE_VERTICAL)
        changeTexMode(MODE_HORIZONTAL);

    if (code == F_TEXT_NORMAL_2)
        fprintRTF("{");

    if (CurrentFontShape() != DefaultFontShape())
        CmdFontShape(DefaultFontShape());

    if (CurrentFontSeries() != DefaultFontSeries())
        CmdFontSeries(DefaultFontSeries());

    if (CurrentFontSize() != DefaultFontSize())
        CmdFontSize(DefaultFontSize());

    if (CurrentFontFamily() != DefaultFontFamily())
        CmdFontFamily(DefaultFontFamily());

    if (code == F_TEXT_NORMAL_2) {
        char *s;

        s = getBraceParam();
        ConvertString(s);
        free(s);
        fprintRTF("}");
    }

    diagnostics(5, "CmdTextNormal (after) depth=%d, family=%d, size=%d, shape=%d, series=%d",
      FontInfoDepth, RtfFontInfo[FontInfoDepth].family,
      RtfFontInfo[FontInfoDepth].size, RtfFontInfo[FontInfoDepth].shape, RtfFontInfo[FontInfoDepth].series);
}

static int strstart(const unsigned char *text, const char *str)

/* returns true if text begins with str */
{
    while (*str && (*str == *text)) {
        str++;
        text++;
    }

    if (*str)
        return FALSE;
    else
        return TRUE;
}

static int strstartnum(const unsigned char *text, const char *str, int *num)

/* returns true if text begins with str and followed by an integer*/
{
    const unsigned char *numptr;

    *num = 0;

    if (!strstart(text, str))
        return FALSE;

    text += strlen(str);
    numptr = text;

    while (isdigit((int) *numptr)) {
        *num = (*num * 10) + (*numptr - '0');
        numptr++;
    }

    if (numptr == text)
        return FALSE;
    else
        return TRUE;
}


void InitializeDocumentFont(int family, int size, int shape, int series, int encoding)

/******************************************************************************
  purpose: Initialize the basic font properties for a document
           pass -1 to avoid setting any parameter
 ******************************************************************************/
{
    if (size >= 0)
        RtfFontInfo[0].size = size;
    if (family >= 0)
        RtfFontInfo[0].family = family;
    if (shape >= 0)
        RtfFontInfo[0].shape = shape;
    if (series >= 0)
        RtfFontInfo[0].series = series;
    if (encoding >= 0)
        RtfFontInfo[0].encoding = encoding;

    diagnostics(5, "InitializeDocumentFont family=%d, size=%d, shape=%d, series=%d",
      RtfFontInfo[0].family, RtfFontInfo[0].size, RtfFontInfo[0].shape, RtfFontInfo[0].series);
}

int DefaultFontFamily(void)
{
    diagnostics(5, "DefaultFontFamily -- family=%d", RtfFontInfo[0].family);
    return RtfFontInfo[0].family;
}

int DefaultFontSize(void)
{
    diagnostics(5, "DefaultFontSize -- size=%d", RtfFontInfo[0].size);
    return RtfFontInfo[0].size;
}

int DefaultFontShape(void)
{
    diagnostics(5, "DefaultFontShape -- shape=%d", RtfFontInfo[0].shape);
    return RtfFontInfo[0].shape;
}

int DefaultFontSeries(void)
{
    diagnostics(5, "DefaultFontSeries -- series=%d", RtfFontInfo[0].series);
    return RtfFontInfo[0].series;
}

int DefaultFontEncoding(void)
{
    diagnostics(6, "DefaultFontSeries -- series=%d", RtfFontInfo[0].encoding);
    return RtfFontInfo[0].encoding;
}

int CurrentFontFamily(void)

/******************************************************************************
  purpose: returns the current RTF family
 ******************************************************************************/
{
    diagnostics(5, "CurrentFontFamily -- family=%d", RtfFontInfo[FontInfoDepth].family);
    return RtfFontInfo[FontInfoDepth].family;
}

int CurrentFontShape(void)

/******************************************************************************
  purpose: returns the current RTF shape
 ******************************************************************************/
{
    diagnostics(5, "CurrentFontShape -- shape=%d", RtfFontInfo[FontInfoDepth].shape);
    return RtfFontInfo[FontInfoDepth].shape;
}

int CurrentFontSize(void)

/******************************************************************************
  purpose: returns the current RTF size in twips
 ******************************************************************************/
{
    diagnostics(5, "CurrentFontSize -- size=%d", RtfFontInfo[FontInfoDepth].size);

    return RtfFontInfo[FontInfoDepth].size;
}

int CurrentFontSeries(void)

/******************************************************************************
  purpose: returns the current RTF series
 ******************************************************************************/
{
    diagnostics(6, "CurrentFontSeries -- series=%d", RtfFontInfo[FontInfoDepth].series);
    return RtfFontInfo[FontInfoDepth].series;
}

int CurrentFontEncoding(void)

/******************************************************************************
  purpose: returns the current RTF encoding
 ******************************************************************************/
{
    diagnostics(6, "CurrentFontSeries -- encoding=%d", RtfFontInfo[FontInfoDepth].encoding);
    return RtfFontInfo[FontInfoDepth].encoding;
}

void CmdFontEncoding(int code)

/******************************************************************************
  purpose: sets the current RTF encoding
 ******************************************************************************/
{
/* 	fprintf(stderr,"setting encoding from %d to %d\n",RtfFontInfo[FontInfoDepth].encoding,code);*/
    RtfFontInfo[FontInfoDepth].encoding = code;
    diagnostics(6, "CurrentFontSeries -- encoding=%d", RtfFontInfo[FontInfoDepth].encoding);
}

void PushFontSettings(void)
{
    if (FontInfoDepth == MAX_FONT_INFO_DEPTH)
        diagnostics(ERROR, "FontInfoDepth too large, cannot PushFontSettings()!");

    RtfFontInfo[FontInfoDepth + 1].size = RtfFontInfo[FontInfoDepth].size;
    RtfFontInfo[FontInfoDepth + 1].family = RtfFontInfo[FontInfoDepth].family;
    RtfFontInfo[FontInfoDepth + 1].shape = RtfFontInfo[FontInfoDepth].shape;
    RtfFontInfo[FontInfoDepth + 1].series = RtfFontInfo[FontInfoDepth].series;
    RtfFontInfo[FontInfoDepth + 1].encoding = RtfFontInfo[FontInfoDepth].encoding;
    FontInfoDepth++;

    diagnostics(6, "PushFontSettings depth=%2d, family=%2d, size=%2d, shape=%2d, series=%2d, encoding=%2d",
      FontInfoDepth, RtfFontInfo[FontInfoDepth].family,
      RtfFontInfo[FontInfoDepth].size, 
      RtfFontInfo[FontInfoDepth].shape, 
      RtfFontInfo[FontInfoDepth].series,
      RtfFontInfo[FontInfoDepth].encoding);
}

void PopFontSettings(void)
{
    if (FontInfoDepth == 0)
        diagnostics(WARNING, "FontInfoDepth = 0, cannot PopFontSettings()!");

    FontInfoDepth--;
    diagnostics(6, "PopFontSettings  depth=%2d, family=%2d, size=%2d, shape=%2d, series=%2d, encoding=%2d",
      FontInfoDepth, RtfFontInfo[FontInfoDepth].family,
      RtfFontInfo[FontInfoDepth].size, 
      RtfFontInfo[FontInfoDepth].shape, 
      RtfFontInfo[FontInfoDepth].series,
      RtfFontInfo[FontInfoDepth].encoding);
}

void MonitorFontChanges(const unsigned char *text)
{
    int n;

    diagnostics(6, "\nMonitorFont %10s\n", text);
    diagnostics(6, "MonitorFont before depth=%d, family=%d, size=%d, shape=%d, series=%d",
      FontInfoDepth, RtfFontInfo[FontInfoDepth].family,
      RtfFontInfo[FontInfoDepth].size, RtfFontInfo[FontInfoDepth].shape, RtfFontInfo[FontInfoDepth].series);

    if (strstart(text, "\\b0"))
        RtfFontInfo[FontInfoDepth].series = F_SERIES_MEDIUM;

    else if (strstart(text, "\\b ") || strstart(text, "\\b\\"))
        RtfFontInfo[FontInfoDepth].series = F_SERIES_BOLD;

    else if (strstart(text, "\\i0")) {
        int mode=getTexMode();
        if (mode==MODE_MATH || mode==MODE_DISPLAYMATH)
            RtfFontInfo[FontInfoDepth].shape = F_SHAPE_MATH_UPRIGHT;
        else
            RtfFontInfo[FontInfoDepth].shape = F_SHAPE_UPRIGHT;
    }
    else if (strstart(text, "\\i ") || strstart(text, "\\i\\"))
        RtfFontInfo[FontInfoDepth].shape = F_SHAPE_ITALIC;

    else if (strstart(text, "\\scaps0")){
        int mode=getTexMode();
        if (mode==MODE_MATH || mode==MODE_DISPLAYMATH)
            RtfFontInfo[FontInfoDepth].shape = F_SHAPE_MATH_UPRIGHT;
        else
            RtfFontInfo[FontInfoDepth].shape = F_SHAPE_UPRIGHT;
    }

    else if (strstart(text, "\\scaps ") || strstart(text, "\\scaps\\"))
        RtfFontInfo[FontInfoDepth].shape = F_SHAPE_CAPS;

    else if (strstartnum(text, "\\fs", &n))
        RtfFontInfo[FontInfoDepth].size = n;

    else if (strstartnum(text, "\\f", &n))
        RtfFontInfo[FontInfoDepth].family = n;

    else if (strstart(text, "\\plain")) {
        RtfFontInfo[FontInfoDepth].size = RtfFontInfo[0].size;
        RtfFontInfo[FontInfoDepth].family = RtfFontInfo[0].family;
        RtfFontInfo[FontInfoDepth].shape = RtfFontInfo[0].shape;
        RtfFontInfo[FontInfoDepth].series = RtfFontInfo[0].series;
    }

    diagnostics(6, "MonitorFont after depth=%d, family=%d, size=%d, shape=%d, series=%d",
      FontInfoDepth, RtfFontInfo[FontInfoDepth].family,
      RtfFontInfo[FontInfoDepth].size, RtfFontInfo[FontInfoDepth].shape, RtfFontInfo[FontInfoDepth].series);

}
