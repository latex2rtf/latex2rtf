/* $Id: l2r_fonts.c,v 1.12 2001/08/22 05:50:23 prahl Exp $

	All changes to font size, font style, and font face are 
	handled in this file.  Explicit changing of font characteristics
	should not be done elsewhere.
	
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "l2r_fonts.h"
#include "funct1.h"
#include "commands.h"
#include "cfg.h"
#include "parser.h"

static char     sitiny[20] = "\\tiny";
static char     siscriptsize[20] = "\\scriptsize";
static char     sifootnotesize[20] = "\\footnotesize";
static char     sismall[20] = "\\small";
static char     sinormalsize[20] = "\\normalsize";
static char     silarge[20] = "\\large";
static char     siLarge[20] = "\\Large";
static char     siLARGE[20] = "\\LARGE";
static char     sihuge[20] = "\\huge";
static char     siHuge[20] = "\\Huge";
static char     siHUGE[20] = "\\HUGE";

int		 default_document_font_size = 22;
int		 default_document_font_family = 0;

int      FontFamily[MAXENVIRONS];
int      FontShape[MAXENVIRONS];
int      FontSeries[MAXENVIRONS];
int      FontSize[MAXENVIRONS];
char    *LatexFontSize[MAXENVIRONS];

void 	ResetFontSeries(void);
void 	ResetFontShape(void);

int 
RtfFontNumber(char *Fname)
/****************************************************************************
 *   purpose: returns the RTF font number from an RTF font name
     example: getTexFontNumber("Times")
 ****************************************************************************/
{
	int          num = 0;
	ConfigEntryT **config_handle = CfgStartIterate(FONT_A);

	while ((config_handle = CfgNext(FONT_A, config_handle)) != NULL) {
		if (strcmp((*config_handle)->RtfCommand, Fname) == 0) {
			return num;
		}
		num++;
	}
	return TexFontNumber("Roman");	/* default font */
}

int 
TexFontNumber(char *Fname)
/****************************************************************************
  purpose: returns the RTF font number for a particular LaTeX font
  example: TexFontNumber("Roman")
 ****************************************************************************/
{
	return SearchRtfIndex(Fname, FONT_A);
}

void 
CmdSetFontFamily(int code)
/******************************************************************************
  purpose: selects the appropriate font family
 ******************************************************************************/
{
	int          	num,old_family,temp;
	char           *s;
	int             iEnvCount = CurrentEnvironmentCount();
	temp = code;
	if (code & ON) code &= ~(ON);
	
	if ((code == F_FAMILY_CALLIGRAPHIC_3 || code ==F_FAMILY_TYPEWRITER_3 ||
	     code == F_FAMILY_SANSSERIF_3    || code == F_FAMILY_ROMAN_3       ) && !(temp & ON)) 
	{
	     fprintf(fRtf, "}");
	     return;
	}

	old_family = FontFamily[iEnvCount];
	FontFamily[iEnvCount] = code;
	
	diagnostics(4, "Entering CmdSetFontFamily");
	switch (code) {
		case F_FAMILY_ROMAN:
		case F_FAMILY_ROMAN_1:
		case F_FAMILY_ROMAN_2:
		case F_FAMILY_ROMAN_3:
			num = TexFontNumber("Roman");
			break;
			
		case F_FAMILY_SANSSERIF:
		case F_FAMILY_SANSSERIF_1:
		case F_FAMILY_SANSSERIF_2:
		case F_FAMILY_SANSSERIF_3:
			num = TexFontNumber("Sans Serif");
			break;
			
		case F_FAMILY_TYPEWRITER:
		case F_FAMILY_TYPEWRITER_1:
		case F_FAMILY_TYPEWRITER_2:
		case F_FAMILY_TYPEWRITER_3:
			num = TexFontNumber("Typewriter");
			break;
			
		case F_FAMILY_CALLIGRAPHIC:
		case F_FAMILY_CALLIGRAPHIC_1:
		case F_FAMILY_CALLIGRAPHIC_2:
		case F_FAMILY_CALLIGRAPHIC_3:
			num = TexFontNumber("Calligraphic");
			break;
	}
	
	
	switch (code) {
		case F_FAMILY_ROMAN:   
		case F_FAMILY_SANSSERIF:
		case F_FAMILY_TYPEWRITER:
		case F_FAMILY_CALLIGRAPHIC:
			fprintf(fRtf, "\\plain\\f%d ", num);          
			break;
			
		case F_FAMILY_ROMAN_1:   
		case F_FAMILY_SANSSERIF_1:
		case F_FAMILY_TYPEWRITER_1:
		case F_FAMILY_CALLIGRAPHIC_1:
			fprintf(fRtf, "{\\f%d\\plain ", num);
			FontSeries[iEnvCount] = F_SERIES_MEDIUM;
			FontShape[iEnvCount] = F_SHAPE_UPRIGHT;
			Convert();
			fprintf(fRtf, "}");
			break;

		case F_FAMILY_ROMAN_2:   
		case F_FAMILY_SANSSERIF_2:
		case F_FAMILY_TYPEWRITER_2:
		case F_FAMILY_CALLIGRAPHIC_2:
			fprintf(fRtf, "{\\f%d ", num);          
			s = getParam();
			ConvertString(s);
			free(s);
			fprintf(fRtf, "}");
			FontFamily[iEnvCount] = old_family;
			break;

		case F_FAMILY_ROMAN_3:   
		case F_FAMILY_SANSSERIF_3:
		case F_FAMILY_TYPEWRITER_3:
		case F_FAMILY_CALLIGRAPHIC_3:
			fprintf(fRtf, "{\\f%d\\plain ", num);
			FontSeries[iEnvCount] = F_SERIES_MEDIUM;
			FontShape[iEnvCount] = F_SHAPE_UPRIGHT;
			break;
	}

	diagnostics(4, "Exiting CmdSetFontFamily");
}

int 
CurrentFontFamily(void)
/******************************************************************************
  purpose: returns the current font style being used
 ******************************************************************************/
{
	int             iEnvCount = CurrentEnvironmentCount();
	return FontFamily[iEnvCount];
}

void 
CmdSetFontShape(int code)
/****************************************************************************
     purpose : sets the font to upright, italic, or small caps
     			F_SHAPE_ITALIC    for \begin{itshape}
     			F_SHAPE_ITALIC_1  for \it and \itfamily
     			F_SHAPE_ITALIC_2  for \textit{...}
 ****************************************************************************/
{
	int             iEnvCount, old_shape, temp;
	
	iEnvCount = CurrentEnvironmentCount();
	old_shape = FontShape[iEnvCount];

	temp = code;
	if (code & ON) code &= ~(ON);
	
	if ((code == F_SHAPE_UPRIGHT_3 || code ==F_SHAPE_ITALIC_3 ||
	     code == F_SHAPE_SLANTED_3 || code == F_SHAPE_CAPS_3    ) && !(temp & ON)) 
	{
	     fprintf(fRtf, "}");
	     return;
	}

	FontShape[iEnvCount] = code;
	
	diagnostics(4, "Entering CmdSetFontShape shape=%d",code);
	
	switch (code) {
		case F_SHAPE_UPRIGHT:   fprintf(fRtf, "\\plain ");  break;
		case F_SHAPE_UPRIGHT_1: fprintf(fRtf, "{\\plain ");  break;
		case F_SHAPE_UPRIGHT_2: 
		case F_SHAPE_UPRIGHT_3: fprintf(fRtf, "{\\plain ");  break;
	
		case F_SHAPE_ITALIC:    fprintf(fRtf, "\\i ");         break;
		case F_SHAPE_ITALIC_1:  fprintf(fRtf, "{\\plain\\i ");  break;
		case F_SHAPE_ITALIC_2:
		case F_SHAPE_ITALIC_3:  fprintf(fRtf, "{\\i ");        break;
	
		case F_SHAPE_SLANTED:   fprintf(fRtf, "\\i ");         break;
		case F_SHAPE_SLANTED_1: fprintf(fRtf, "{\\plain\\i ");  break;
		case F_SHAPE_SLANTED_2:
		case F_SHAPE_SLANTED_3: fprintf(fRtf, "{\\i ");        break;
	
		case F_SHAPE_CAPS:      fprintf(fRtf, "\\scaps ");         break;
		case F_SHAPE_CAPS_1:    fprintf(fRtf, "{\\plain\\scaps ");  break;
		case F_SHAPE_CAPS_2:
		case F_SHAPE_CAPS_3:    fprintf(fRtf, "{\\scaps ");        break;
	}

	if (code == F_SHAPE_UPRIGHT_1 || code == F_SHAPE_ITALIC_1 || 
	    code == F_SHAPE_SLANTED_1 || code == F_SHAPE_CAPS_1)
				FontSeries[iEnvCount] = F_SERIES_MEDIUM;

	if (code == F_SHAPE_UPRIGHT_2 || code == F_SHAPE_ITALIC_2 || 
	    code == F_SHAPE_SLANTED_2 || code == F_SHAPE_CAPS_2)
	{
		char *s;
		s = getParam();
		ConvertString(s);
		fprintf(fRtf, "}");
		FontShape[iEnvCount] = old_shape;
		free(s);
	} else 	if (code == F_SHAPE_UPRIGHT_1 || code == F_SHAPE_ITALIC_1 || 
	            code == F_SHAPE_SLANTED_1 || code == F_SHAPE_CAPS_1) 
	{
		Convert();
		fprintf(fRtf,"}");
		FontShape[iEnvCount] = old_shape;
	} 

	diagnostics(4, "Exiting CmdSetFontShape");
}

int 
CurrentFontShape(void)
/******************************************************************************
  purpose: returns the current font style being used
 ******************************************************************************/
{
	int             iEnvCount = CurrentEnvironmentCount();
	return FontShape[iEnvCount];
}

void 
CmdSetFontSeries(int code)
/****************************************************************************
     purpose : sets the font weight to medium or bold
     
     F_SERIES_BOLD        for  {\bfseries ... }
	 F_SERIES_BOLD_1      for  {\bf ... }
	 F_SERIES_BOLD_2      for  \textbf{...}
	 F_SERIES_BOLD_3      for  \begin{bfseries} ... \end{bfseries}

 ****************************************************************************/
{
	int iEnvCount, old_series, temp;
	
	iEnvCount = CurrentEnvironmentCount();
	old_series = FontSeries[iEnvCount];
	FontSeries[iEnvCount] = code;
		    
	temp = code;
	if (code & ON) code &= ~(ON);
	
	if ((code == F_SERIES_MEDIUM_3 || code == F_SERIES_BOLD_3) && !(temp & ON)) 
	{
	     fprintf(fRtf, "}");
	     return;
	}

	diagnostics(4, "Entering CmdSetFontSeries");
	
	switch (code) {
		case F_SERIES_MEDIUM:   CmdSetFontShape(F_SHAPE_UPRIGHT); 
		 						break;
		 						
		case F_SERIES_MEDIUM_1: 
		case F_SERIES_MEDIUM_2:
		case F_SERIES_MEDIUM_3: fprintf(fRtf, "{");   
								CmdSetFontShape(F_SHAPE_UPRIGHT); 
								break;
	
		case F_SERIES_BOLD:    	fprintf(fRtf, "\\b ");         
								break;
								
		case F_SERIES_BOLD_1:  	fprintf(fRtf, "{");
								CmdSetFontShape(F_SHAPE_UPRIGHT); 
								fprintf(fRtf, "\\b ");  
								break;
								
		case F_SERIES_BOLD_2:      
		case F_SERIES_BOLD_3:   fprintf(fRtf, "{\\b ");        
								break;
	}
	
	if (code == F_SERIES_BOLD_2 || code == F_SERIES_MEDIUM_2){
		char *s; 
		s = getParam();
		ConvertString(s);
		fprintf(fRtf, "}");
		FontSeries[iEnvCount] = old_series;
		free(s);
	} else if (code == F_SERIES_BOLD_2 || code == F_SERIES_MEDIUM_2){
		Convert();
		fprintf(fRtf,"}");
		FontSeries[iEnvCount] = old_series;
	} 

	diagnostics(4, "Exiting CmdSetFontSeries");
}

int 
CurrentFontSeries(void)
/******************************************************************************
  purpose: returns the current font style being used
 ******************************************************************************/
{
	int             iEnvCount = CurrentEnvironmentCount();
	return FontSeries[iEnvCount];
}

void 
CmdSetFontSize(int code)
/******************************************************************************
 purpose : sets the fontsize to the point-size given by the LaTex-\fs_size-command
 globals : fontsize : includes the actual fontsize in the document
           heavily modified from D Taupin who would probably not approve
           and certainly should not be blamed.
 ******************************************************************************/
{
	int             iEnvCount, old_size, scaled_code;
	scaled_code = code;
	iEnvCount = CurrentEnvironmentCount();
    old_size = 	FontSize[iEnvCount];

	diagnostics(4, "Entering CmdSetFontSize");
	LatexFontSize[iEnvCount] = sitiny;
	if (code >= 14)
		LatexFontSize[iEnvCount] = siscriptsize;
	if (code >= 16)
		LatexFontSize[iEnvCount] = sifootnotesize;
	if (code >= 18)
		LatexFontSize[iEnvCount] = sismall;
	if (code >= 20)
		LatexFontSize[iEnvCount] = sinormalsize;
	if (code >= 24)
		LatexFontSize[iEnvCount] = silarge;
	if (code >= 28)
		LatexFontSize[iEnvCount] = siLarge;
	if (code >= 34)
		LatexFontSize[iEnvCount] = siLARGE;
	if (code >= 40)
		LatexFontSize[iEnvCount] = sihuge;
	if (code >= 50)
		LatexFontSize[iEnvCount] = siHuge;
	if (code >= 60)
		LatexFontSize[iEnvCount] = siHUGE;

	scaled_code = (code * DefaultFontSize()) / 20;

	fprintf(fRtf, "\\fs%d ", scaled_code);
	FontSize[iEnvCount] = scaled_code;
	Convert();
	FontSize[iEnvCount] = old_size;

	diagnostics(4, "Exiting CmdSetFontSize");
}

int 
CurrentFontSize(void)
/******************************************************************************
  purpose: returns the current font size being used
 ******************************************************************************/
{
	int             iEnvCount = CurrentEnvironmentCount();
	return FontSize[iEnvCount];
}

void
InitializeDocumentFont(int size)
/******************************************************************************
  purpose: Initialize the basic font properties for a document
 ******************************************************************************/
{
	int             iEnvCount = CurrentEnvironmentCount();
	
	FontSize[iEnvCount] = size;
	LatexFontSize[iEnvCount] = sinormalsize;
	FontShape[iEnvCount] = F_SHAPE_UPRIGHT;
	FontSeries[iEnvCount] = F_SERIES_MEDIUM;
	FontFamily[iEnvCount] = TexFontNumber("Roman");

	default_document_font_size = FontSize[iEnvCount];
	default_document_font_family = FontFamily[iEnvCount];
}


void 
CmdEmphasize(int code)
/****************************************************************************
 purpose: handle \em, \emph, and \begin{em} ... \end{em}
 
 		  EMPAHSIZE_IMMEDIATE refers to the \emph{string} construction
 		  and is handled as \textit{string} or \textup{string}
 		  
 		  {\em string} should be properly localized by brace mechanisms
 		  
 		  \begin{em} ... \end{em} will be localized by environment mechanisms
 ******************************************************************************/
{
	int temp = code;
	
	if (code & ON) code &= ~(ON);  /* mask MSB */
	diagnostics(4,"Entering CmdEmphasize, curr shape=%d, code=%d", CurrentFontShape(), code);
	
	if (code == F_EMPHASIZE) {

		if (temp & ON) {
		
			fprintf(fRtf,"{");	
			switch (CurrentFontShape()) {
				case F_SHAPE_UPRIGHT:
				case F_SHAPE_UPRIGHT_1:
				case F_SHAPE_UPRIGHT_2:
					CmdSetFontShape(F_SHAPE_ITALIC);
					break;
		
				case F_SHAPE_ITALIC:
				case F_SHAPE_SLANTED:
				case F_SHAPE_ITALIC_1:
				case F_SHAPE_ITALIC_2:	
				case F_SHAPE_SLANTED_1:
				case F_SHAPE_SLANTED_2:
					CmdSetFontShape(F_SHAPE_UPRIGHT);
					break;	
			}
		} else
			fprintf(fRtf,"}");
	
	} else {

		switch (CurrentFontShape()) {
		
			case F_SHAPE_UPRIGHT:
			case F_SHAPE_UPRIGHT_1:
			case F_SHAPE_UPRIGHT_2:
				if (code == F_EMPHASIZE_IMMEDIATE)
					CmdSetFontShape(F_SHAPE_ITALIC_2);
				else
					CmdSetFontShape(F_SHAPE_ITALIC_1);
				break;
	
			case F_SHAPE_ITALIC:
			case F_SHAPE_SLANTED:
			case F_SHAPE_ITALIC_1:
			case F_SHAPE_ITALIC_2:	
			case F_SHAPE_SLANTED_1:
			case F_SHAPE_SLANTED_2:
				if (code == F_EMPHASIZE_IMMEDIATE)
					CmdSetFontShape(F_SHAPE_UPRIGHT_2);
				else 
					CmdSetFontShape(F_SHAPE_UPRIGHT_1);
				break;	
		}
	}
	diagnostics(4,"Exiting CmdEmphasize");
}

void 
CmdUnderline(int code)
/****************************************************************************
 purpose: handle \underline{text}
 ******************************************************************************/
{
	char *s;
	diagnostics(4,"Entering CmdUnderline");
	
	fprintf(fRtf, "{\\ul ");          
	s = getParam();
	ConvertString(s);
	free(s);
	fprintf(fRtf, "}");
	diagnostics(4,"Exiting CmdUnderline");
}

void 
CmdTextNormal(int code)
/****************************************************************************
 purpose: handle \rm, \textnormal{text}  {\normalfont ...}
 ******************************************************************************/
{
	char *s;
	diagnostics(4,"Entering CmdTextNormal");
	
	if (code==F_TEXT_NORMAL_2) {
		fprintf(fRtf, "{");
		CmdSetFontFamily(F_FAMILY_ROMAN);
		CmdSetFontSeries(F_SERIES_MEDIUM);
		s = getParam();
		ConvertString(s);
		free(s);
		fprintf(fRtf, "}");
	
	} else {
		CmdSetFontFamily(F_FAMILY_ROMAN);
		CmdSetFontSeries(F_SERIES_MEDIUM);
		CmdSetFontShape(F_SHAPE_UPRIGHT);
	}

	diagnostics(4,"Exiting CmdTextNormal");
}

void    
DupPrevFontEnvironment(void)
{
	int iEnvCount = CurrentEnvironmentCount();
	
	if (iEnvCount <= 1) {
		FontSize[iEnvCount]   = 22;
		FontSeries[iEnvCount] = F_SERIES_MEDIUM;
		FontShape[iEnvCount]  = F_SHAPE_UPRIGHT;
		FontFamily[iEnvCount] = F_FAMILY_ROMAN;
	} else {
		FontSize[iEnvCount]   = FontSize[iEnvCount - 1];
		FontSeries[iEnvCount] = FontSeries[iEnvCount - 1];
		FontShape[iEnvCount]  = FontShape[iEnvCount - 1];
		FontFamily[iEnvCount] = FontFamily[iEnvCount - 1];
	}
}

int
DefaultFontFamily(void)
{
	return default_document_font_family;
}

int
DefaultFontSize(void)
{
	return default_document_font_size;
}

void 
ResetFontSeries(void)
/****************************************************************************
     purpose : resets the current font series
 ****************************************************************************/
{
	switch (CurrentFontSeries()) {
		case F_SERIES_MEDIUM:   break;
		case F_SERIES_MEDIUM_1: break;
		case F_SERIES_MEDIUM_2: break;
	
		case F_SERIES_BOLD:
		case F_SERIES_BOLD_1:
		case F_SERIES_BOLD_2:  fprintf(fRtf, "\\b ");
	}
}

void 
RestoreFont(void)
/****************************************************************************
     purpose : resets the current font characteristics
 ****************************************************************************/
{
	int i;
	int iEnvCount = CurrentEnvironmentCount();
	for (i=iEnvCount; i>=0; i--)
		diagnostics(3," iEnv=%d Family=%d Size=%d Series=%d Shape=%d", i, FontFamily[i], FontSize[i], FontSeries[i], FontShape[i]);
	diagnostics(3,"");
	fprintf(fRtf, "\\f%d", CurrentFontFamily());          
	fprintf(fRtf, "\\fs%d", CurrentFontSize());          
	ResetFontSeries();
	ResetFontShape();
}

void 
ResetFontShape(void)
/****************************************************************************
     purpose : resets the font to italic or small caps
 ****************************************************************************/
{
	diagnostics(4, "Entering ResetFontShape shape=%d",CurrentFontShape());

	switch (CurrentFontShape()) {
		case F_SHAPE_UPRIGHT:   
		case F_SHAPE_UPRIGHT_1:
		case F_SHAPE_UPRIGHT_2: break;
	
		case F_SHAPE_ITALIC: 
		case F_SHAPE_ITALIC_1: 
		case F_SHAPE_ITALIC_2: 
	
		case F_SHAPE_SLANTED: 
		case F_SHAPE_SLANTED_1:
		case F_SHAPE_SLANTED_2: fprintf(fRtf, "\\i ");        break;
	
		case F_SHAPE_CAPS: 
		case F_SHAPE_CAPS_1:  
		case F_SHAPE_CAPS_2:    fprintf(fRtf, "\\scaps ");        break;
	}

	diagnostics(4, "Exiting ResetFontShape");
}

