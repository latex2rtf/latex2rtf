/*
 * $Id: l2r_fonts.c,v 1.9 2001/08/12 21:15:46 prahl Exp $
 * History:
 * $Log: l2r_fonts.c,v $
 * Revision 1.9  2001/08/12 21:15:46  prahl
 *         Removed last two // comments
 *         Explicitly cast char to int in isalpha() and isdigit()
 *         Began the process of supporting Babel better
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "l2r_fonts.h"
#include "funct1.h"
#include "commands.h"
#include "cfg.h"
#include "parser.h"

/******************************************************************************/
void            error(char *);

char            PointSize[10] = "10pt";
char            normalsize[20] = "\\normalsize";
int             document_font_size = 22;

static int      curr_fontsize[MAXENVIRONS] = {24};
static char    *LatexSize[MAXENVIRONS] = {normalsize};
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

extern int      DefFont;
extern enum     TexCharSetKind TexCharSet;
extern int      curr_fontbold[MAXENVIRONS];
extern int      curr_fontital[MAXENVIRONS];
extern int      curr_fontscap[MAXENVIRONS];
extern int      curr_fontnumb[MAXENVIRONS];
extern char    *LatexSize[MAXENVIRONS];
extern char     PointSize[10];

#define MAXLEN 80

int 
GetFontNumber(char *Fname)
/****************************************************************************
 *   purpose: gets the font number from an Rtf font name
 * parameter: Fname: fontname in Rtf
 *    return: font number
 ****************************************************************************/
{
	int          num = 0;
	ConfigEntryT **config_handle = CfgStartIterate(FONT_A);

	while ((config_handle = CfgNext(FONT_A, config_handle)) != NULL) {
		if (strcmp((*config_handle)->RtfCommand, Fname) == 0) {
			return num;
		}
		num++;		/* found by gerard Pénillault, added by
				 * W.Hennings March08,1999 */
	}
	return getTexFontNumber("Roman");	/* default font */
}


int 
getTexFontNumber(char *Fname)
/****************************************************************************
  purpose: gets the RTF font number from given LaTex font
parameter: Fname: fontname in LaTex
   return: RTF font number
 ****************************************************************************/
{
	return SearchRtfIndex(Fname, FONT_A);
}


void 
CmdSetFontStyle(int code)
/****************************************************************************
     purpose : sets the font to bold, italic, underlined...
   parameter : code includes the character-format-style
     globals : fRtf
 ****************************************************************************/
{
		diagnostics(4, "Entering CmdSetFontStyle");
		
		if (code == CMD_BOLD_1 || code == CMD_ITALIC_1 || code == CMD_CAPS_1)
			fprintf(fRtf, "{\\plain");
		else
			fprintf(fRtf, "{");

		switch (code) {
		case CMD_BOLD_2:
		case CMD_BOLD_1:
		case CMD_BOLD:
			fprintf(fRtf, "\\b ");
			break;

		case CMD_ITALIC_2:
		case CMD_ITALIC_1:
		case CMD_ITALIC:
			fprintf(fRtf, "\\i ");
			break;

		case CMD_CAPS_2:
		case CMD_CAPS_1:
		case CMD_CAPS:
			fprintf(fRtf, "\\scaps ");
			break;

		case CMD_UNDERLINE:
			fprintf(fRtf, "\\ul ");
			break;

		case CMD_CENTERED:
			fprintf(fRtf, "\\qc ");
			break;
		}

		if (code == CMD_BOLD_2 || code == CMD_ITALIC_2 || code == CMD_CAPS_2){
			char *s;
			s = getParam();
			ConvertString(s);
			free(s);
		} else
			Convert();

		fprintf(fRtf, "}");

	diagnostics(4, "Exiting CmdSetFontStyle");

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
	int             iEnvCount;
	int             scaled_code = code;
	iEnvCount = CurrentEnvironmentCount();

	diagnostics(4, "Entering CmdSetFontSize");
	LatexSize[iEnvCount] = sitiny;
	if (code >= 10)
		LatexSize[iEnvCount] = sitiny;
	if (code >= 14)
		LatexSize[iEnvCount] = siscriptsize;
	if (code >= 16)
		LatexSize[iEnvCount] = sifootnotesize;
	if (code >= 18)
		LatexSize[iEnvCount] = sismall;
	if (code >= 20)
		LatexSize[iEnvCount] = sinormalsize;
	if (code >= 24)
		LatexSize[iEnvCount] = silarge;
	if (code >= 28)
		LatexSize[iEnvCount] = siLarge;
	if (code >= 34)
		LatexSize[iEnvCount] = siLARGE;
	if (code >= 40)
		LatexSize[iEnvCount] = sihuge;
	if (code >= 50)
		LatexSize[iEnvCount] = siHuge;
	if (code >= 60)
		LatexSize[iEnvCount] = siHUGE;

	scaled_code = (code * document_font_size) / 20;

	fprintf(fRtf, "{\\fs%d ", scaled_code);
	curr_fontsize[iEnvCount] = scaled_code;
	sprintf(PointSize, "%dpt", code / 2);

	Convert();
	fprintf(fRtf, "}");

	diagnostics(4, "Exiting CmdSetFontSize");
	/*
	 * We need to continue processing to account for a case like {\Large
	 * Some text {\normalsize normal} yet more text} otherwise 'yet more
	 * text' will not get processed correctly
	 */
}

void 
CmdSetFont(int code)
/******************************************************************************
  purpose: sets an font for the actual character-style
parameter: code: includes the font-type
 ******************************************************************************/
{
	int          num;

	diagnostics(4, "Entering CmdSetFont");
	switch (code) {
	case F_ROMAN_2:
	case F_ROMAN_1:
	case F_ROMAN:
		num = getTexFontNumber("Roman");
		break;
	case F_SLANTED_2:
	case F_SLANTED_1:
	case F_SLANTED:
		num = getTexFontNumber("Slanted");
		break;
	case F_SANSSERIF_2:
	case F_SANSSERIF_1:
	case F_SANSSERIF:
		num = getTexFontNumber("Sans Serif");
		break;
	case F_TYPEWRITER_2:
	case F_TYPEWRITER_1:
	case F_TYPEWRITER:
		num = getTexFontNumber("Typewriter");
		break;
	default:
		num = DefFont;
	}
	
	fprintf(fRtf, "{");

	if (code == F_ROMAN        || code == F_ROMAN_1   || code == F_ROMAN_2 || 
		code == F_TYPEWRITER_1 || code == F_SLANTED_1 || code == F_SANSSERIF_1)
		fprintf(fRtf, "\\plain");
	
	if (code == F_ROMAN_2      || code == F_SANSSERIF_2 || 
	    code == F_TYPEWRITER_2 || code == F_SLANTED_2) {
		char *s;
		fprintf(fRtf, "\\f%d ", num);
		s = getParam();
		ConvertString(s);
		free(s);

	} else {
		fprintf(fRtf, "\\f%d ", num);
		Convert();
	}
	fprintf(fRtf, "}");

	diagnostics(4, "Exiting CmdSetFont");
}

int 
CurrentFontSize(void)
/******************************************************************************
  purpose: returns the current font size being used
 ******************************************************************************/
{
	int             iEnvCount = CurrentEnvironmentCount();
	return (curr_fontsize[iEnvCount]);
}

void 
SetDocumentFontSize(int code)
/******************************************************************************
  purpose: sets the default size for the document
 ******************************************************************************/
{
	int             iEnvCount = CurrentEnvironmentCount();
	document_font_size = code;
	curr_fontsize[iEnvCount] = code;
	LatexSize[iEnvCount] = sinormalsize;
}

void 
BasicSetFontSize(int code)
/******************************************************************************
  purpose: sets the default size for the document
 ******************************************************************************/
{
	int             iEnvCount = CurrentEnvironmentCount();
	curr_fontsize[iEnvCount] = code;
	LatexSize[iEnvCount] = sinormalsize;
}
