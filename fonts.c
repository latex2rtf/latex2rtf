/*
 * $Id: fonts.c,v 1.4 2001/08/12 17:29:00 prahl Exp $
 * History:
 * $Log: fonts.c,v $
 * Revision 1.4  2001/08/12 17:29:00  prahl
 * latex2rtf version 1.8aa by Georg Lehner
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
#include "main.h"
#include "fonts.h"
#include "cfg.h" 
/******************************************************************************/
void error(char *);

/************************************* extern variables *********************/
extern int fontsize;
extern size_t DefFont;

/******************************************************************************/


/********************* defines and structures ********************************/
#define MAXLEN 80
/****************************************************************************/


/***/
void WriteFontHeader(FILE* fRtf)
/****************************************************************************
 *   purpose: writes fontnumbers and styles for headers into Rtf-File 
 * parameter: fRtf: File-Pointer to Rtf-File
 *   globals: fontsize
 *            DefFont (default font number)
 ****************************************************************************/
{
    size_t num = 0;
    const ConfigEntryT **config_handle;

    fprintf(fRtf,"{\\fonttbl");

    config_handle = CfgStartIterate (FONT_A);
    while ((config_handle = CfgNext (FONT_A, config_handle)) != NULL)
    {
	fprintf( fRtf
	       , "{\\f%u\\fnil %s;}"
	       , (unsigned int)num
	       , (*config_handle)->RtfCommand
	       );
	++num;
    }

    fprintf(fRtf,"}\\f%u\n", (unsigned int)(DefFont = GetFontNumber("Roman")));
    fprintf(fRtf,"{\\stylesheet{\\fs%d\\lang1031\\snext0 Normal;}",fontsize);
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
