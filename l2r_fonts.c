/*
 * $Id: l2r_fonts.c,v 1.6 2001/08/12 19:32:24 prahl Exp $
 * History:
 * $Log: l2r_fonts.c,v $
 * Revision 1.6  2001/08/12 19:32:24  prahl
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
 * Revision 1.6  1998/10/28 06:27:56  glehner
 * Removed <malloc.h>
 *
 * Revision 1.5  1997/02/15 20:55:50  ralf
 * Some reformatting and changes suggested by lclint
 * Removed direct access to data structures in cfg.c
 *
 * Revision 1.4  1995/05/24 15:32:22  ralf
 * Changes by Vladimir Menkov for DOS port
 *
 * Revision 1.3  1995/03/23  15:58:08  ralf
 * Reworked version by Friedrich Polzer and Gerhard Trisko
 *
 * Revision 1.2  1994/06/17  14:19:41  ralf
 * Corrected various bugs, for example interactive read of arguments
 *
 * Revision 1.1  1994/06/17  11:26:29  ralf
 * Initial revision
 *
 */
/***************************************************************************
     name : fonts.c
   author : DORNER Fernando, GRANZER Andreas
            POLZER Friedrich,TRISKO Gerhard
 * uses now sorted array instead of fonts.cfg and chained list
 *
 purpose : The LaTeX font will be converted to the RTF font
 ******************************************************************************/

/****************************************** includes ************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "l2r_fonts.h"
#include "commands.h"
#include "cfg.h" 
/******************************************************************************/
void error(char *);

char PointSize[10] = "10pt";
char normalsize[20] = "\\normalsize";
int document_font_size = 22;

static int curr_fontsize[MAXENVIRONS] = {24};
static char *LatexSize[MAXENVIRONS] = {normalsize};
  static char sitiny[20] = "\\tiny";
  static char siscriptsize[20] = "\\scriptsize";
  static char sifootnotesize[20] = "\\footnotesize";
  static char sismall[20] = "\\small";
  static char sinormalsize[20] = "\\normalsize";
  static char silarge[20] = "\\large";
  static char siLarge[20] = "\\Large";
  static char siLARGE[20] = "\\LARGE";
  static char sihuge[20] = "\\huge";
  static char siHuge[20] = "\\Huge";
  static char siHUGE[20] = "\\HUGE";

/************************************* extern variables *********************/
extern size_t DefFont;
extern enum TexCharSetKind TexCharSet;
extern int curr_fontbold[MAXENVIRONS];
extern int curr_fontital[MAXENVIRONS];
extern int curr_fontscap[MAXENVIRONS];
extern int curr_fontnumb[MAXENVIRONS];
extern char *LatexSize[MAXENVIRONS];
extern char PointSize[10];


/******************************************************************************/


/********************* defines and structures ********************************/
#define MAXLEN 80
/****************************************************************************/


/***/
void WriteFontHeader(FILE* fRtf)
/****************************************************************************
 *   purpose: writes fontnumbers and styles for headers into Rtf-File 
 * parameter: fRtf: File-Pointer to Rtf-File
 *   globals: 
 *            DefFont (default font number)
 *   note;
 
 \fcharset0:    ANSI coding
 \fcharset1:    MAC coding
 \fcharset2:    PC coding (implies CodePage 437)
 \fcharset3:    PCA coding (implies CodePage 850) 
 ****************************************************************************/
{
    size_t num = 0;
    const ConfigEntryT **config_handle;

    fprintf(fRtf,"{\\fonttbl");

    config_handle = CfgStartIterate (FONT_A);
    while ((config_handle = CfgNext (FONT_A, config_handle)) != NULL)
    {
	if (strstr((*config_handle)->TexCommand,"MacRoman"))
		fprintf( fRtf
		       , "{\\f%u\\fnil\\fcharset1 %s;}"
		       , (unsigned int)num
		       , (*config_handle)->RtfCommand
		       );
	else
	if (strstr((*config_handle)->RtfCommand,"Symbol"))
		fprintf( fRtf
		       , "{\\f%u\\fnil\\fcharset2 %s;}"
		       , (unsigned int)num
		       , (*config_handle)->RtfCommand
		       );
	else
		fprintf( fRtf
		       , "{\\f%u\\fnil\\fcharset0 %s;}"
		       , (unsigned int)num
		       , (*config_handle)->RtfCommand
		       );
	++num;
    }

    fprintf(fRtf,"}\\f%u\n", (unsigned int)(DefFont = GetFontNumber("Roman")));
    fprintf(fRtf,"{\\stylesheet{\\fs%d\\lang1031\\snext0 Normal;}",CurrentFontSize());
    fprintf( fRtf,"{%s%u%s \\sbasedon0\\snext0 heading 1;}\n"
	   , HEADER11,(unsigned int)DefFont,HEADER12);
    fprintf( fRtf,"{%s%u%s \\sbasedon0\\snext0 heading 2;}\n"
           , HEADER21,(unsigned int)DefFont,HEADER22);
    fprintf( fRtf,"{%s%u%s \\sbasedon0\\snext0 heading 3;}\n"
           , HEADER31,(unsigned int)DefFont,HEADER32);
    fprintf( fRtf,"{%s%u%s \\sbasedon0\\snext0 heading 4;}\n"
           , HEADER41,(unsigned int)DefFont,HEADER42);

    fprintf(fRtf,"%s\n", HEADER03);
    fprintf(fRtf,"%s\n", HEADER13);
    fprintf(fRtf,"%s\n", HEADER23);
    fprintf(fRtf,"%s\n", HEADER33);
    fprintf(fRtf,"%s\n", HEADER43);
}


/***/
size_t GetFontNumber(char * Fname)
/****************************************************************************
 *   purpose: gets the font number from an Rtf font name
 * parameter: Fname: fontname in Rtf
 *    return: font number
 ****************************************************************************/
{
    size_t num = 0;
    const ConfigEntryT **config_handle = CfgStartIterate (FONT_A);

    while ((config_handle = CfgNext (FONT_A, config_handle)) != NULL)
    {
	if (strcmp ((*config_handle)->RtfCommand, Fname) == 0)
	{
	    return num;  
	}
      num++; /* found by gerard Pénillault, added by W.Hennings March08,1999 */
    }
    return GetTexFontNumber ("Roman");  /* default font */
}

/***/
size_t GetTexFontNumber(char * Fname)
/****************************************************************************
  purpose: gets the RTF font number from given LaTex font
parameter: Fname: fontname in LaTex
   return: RTF font number
 ****************************************************************************/
{
    return SearchRtfIndex (Fname, FONT_A);
}

void CmdSetFontStyle(int code)
/****************************************************************************
     purpose : sets the font to bold, italic, underlined...
   parameter : code includes the character-format-style
     globals : fRtf: Rtf-File-Pointer
 ****************************************************************************/
{
  if (tabbing_on || 1)
    {
     if (code == CMD_BOLD_1 || code == CMD_ITALIC_1 || code == CMD_CAPS_1)
     	fprintf(fRtf, "{\\plain");
     else
        fprintf(fRtf,"{");

     switch(code)
     {
       case CMD_BOLD_1:
       case CMD_BOLD:      fprintf(fRtf,"\\b ");
		                   break;
		                   
       case CMD_ITALIC_1:
       case CMD_ITALIC:    fprintf(fRtf,"\\i ");
			               break;
			               
       case CMD_CAPS_1:
       case CMD_CAPS:      fprintf(fRtf,"\\scaps ");
		                   break;
		                   
       case CMD_UNDERLINE: fprintf(fRtf,"\\ul ");
			               break;
			               
       case CMD_CENTERED:  fprintf(fRtf,"\\qc ");
                           break;
     }
        
     Convert();
     fprintf(fRtf,"}");
   }
}

void CmdSetFontSize(int code)
/******************************************************************************
 purpose : sets the fontsize to the point-size given by the LaTex-\fs_size-command
 globals : fontsize : includes the actual fontsize in the document 
           heavily modified from D Taupin who would probably not approve
           and certainly should not be blamed.
 ******************************************************************************/
{ int iEnvCount;
  int scaled_code = code;
  iEnvCount = CurrentEnvironmentCount();
  
	LatexSize[iEnvCount] = sitiny;
	if (code >= 10) LatexSize[iEnvCount] = sitiny;
	if (code >= 14) LatexSize[iEnvCount] = siscriptsize;
	if (code >= 16) LatexSize[iEnvCount] = sifootnotesize;
	if (code >= 18) LatexSize[iEnvCount] = sismall;
    if (code >= 20) LatexSize[iEnvCount] = sinormalsize;
	if (code >= 24) LatexSize[iEnvCount] = silarge;
	if (code >= 28) LatexSize[iEnvCount] = siLarge;
	if (code >= 34) LatexSize[iEnvCount] = siLARGE;
	if (code >= 40) LatexSize[iEnvCount] = sihuge;
	if (code >= 50) LatexSize[iEnvCount] = siHuge;
	if (code >= 60) LatexSize[iEnvCount] = siHUGE;

  scaled_code = (code*document_font_size)/20;
  fprintf(fRtf,"{\\fs%d ",scaled_code);
  curr_fontsize[iEnvCount] = scaled_code;
  sprintf(PointSize,"%dpt",code/2);

  Convert();
  fprintf(fRtf,"} ");

/* We need to continue processing to account for a case like
             {\Large Some text {\normalsize normal} yet more text}
   otherwise 'yet more text' will not get processed correctly */
  Convert();
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
    case F_ROMAN_1:
    case F_ROMAN:         num = GetTexFontNumber("Roman");
		                  break;
    case F_SLANTED_1:
    case F_SLANTED:       num = GetTexFontNumber("Slanted");
		                  break;
    case F_SANSSERIF_1:
    case F_SANSSERIF:     num = GetTexFontNumber("Sans Serif");
		                  break;
    case F_TYPEWRITER_1:
    case F_TYPEWRITER:    num = GetTexFontNumber("Typewriter");
		                  break;
    default: num = 0;
  }
  if (code == F_ROMAN_1 || code == F_SANSSERIF_1 || code == F_TYPEWRITER_1 || code == F_SLANTED_1)
  	fprintf(fRtf, "{\\plain");
  else
    fprintf(fRtf, "{");
    
  fprintf(fRtf,"\\f%u ", (unsigned int)num);
  Convert();
  fprintf(fRtf,"} ");

  Convert();
}

int CurrentFontSize(void)
/******************************************************************************
  purpose: returns the current font size being used
 ******************************************************************************/
{
  int iEnvCount = CurrentEnvironmentCount();
  return (curr_fontsize[iEnvCount]);
}

void SetDocumentFontSize(int code)
/******************************************************************************
  purpose: sets the default size for the document
 ******************************************************************************/
{
  int iEnvCount = CurrentEnvironmentCount();
  document_font_size = code;
  curr_fontsize[iEnvCount] = code;
  LatexSize[iEnvCount] = sinormalsize;
}

void BasicSetFontSize(int code)
/******************************************************************************
  purpose: sets the default size for the document
 ******************************************************************************/
{
  int iEnvCount = CurrentEnvironmentCount();
  curr_fontsize[iEnvCount] = code;
  LatexSize[iEnvCount] = sinormalsize;
}