/*
 * $Id: fonts.c,v 1.3 2001/08/12 15:56:56 prahl Exp $
 * History:
 * $Log: fonts.c,v $
 * Revision 1.3  2001/08/12 15:56:56  prahl
 * latex2rtf version 1.5 by Ralf Schlatterbeck
 *
 * Revision 1.4  1995/05/24  15:32:22  ralf
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
#include <malloc.h>
#include <stdlib.h>
#include "main.h"
#include "fonts.h"
#include "cfg.h" 
/******************************************************************************/
void error(char *);

/************************************* extern variables *********************/
extern char *progname;

extern struct ArrayElementT *FontArray;
extern int FontArraySize;
extern int fontsize;
extern int DefFont;

/******************************************************************************/


/********************* defines and structures ********************************/
#define MAXLEN 80
/****************************************************************************/


/****************************************************************************/
BOOL WriteFontHeader(FILE* fRtf)
/****************************************************************************
  purpose: writes fontnumbers and styles for headers into Rtf-File 
parameter: fRtf: File-Pointer to Rtf-File
  globals: fontsize
           DefFont (default font number)
           FontArray (contains list of fonts defined in fonts.cfg)
           FontArraySize (number of elements in FontArray)
 ****************************************************************************/
{
  int i;

  fprintf(fRtf,"{\\fonttbl");

  for(i = 0; i < FontArraySize; i++)
  {
    fprintf(fRtf,"{\\f%d\\fnil %s;}", i, FontArray[i].RtfCommand);
  }; /* end for */

  fprintf(fRtf,"}\\f%d\n", DefFont = GetFontNumber("Roman"));

  fprintf(fRtf,"{\\stylesheet{\\fs%d\\lang1031\\snext0 Normal;}",fontsize);

  fprintf(fRtf,"{%s%d%s \\sbasedon0\\snext0 heading 1;}\n", HEADER11,DefFont,HEADER12);
  fprintf(fRtf,"{%s%d%s \\sbasedon0\\snext0 heading 2;}\n", HEADER21,DefFont,HEADER22);
  fprintf(fRtf,"{%s%d%s \\sbasedon0\\snext0 heading 3;}\n", HEADER31,DefFont,HEADER32);
  fprintf(fRtf,"{%s%d%s \\sbasedon0\\snext0 heading 4;}\n", HEADER41,DefFont,HEADER42);

  fprintf(fRtf,"%s\n", HEADER03);
  fprintf(fRtf,"%s\n", HEADER13);
  fprintf(fRtf,"%s\n", HEADER23);
  fprintf(fRtf,"%s\n", HEADER33);
  fprintf(fRtf,"%s\n", HEADER43);

  return TRUE;
}


/****************************************************************************/
int GetFontNumber(char * Fname)
/****************************************************************************
  purpose: gets the font number Rtf
parameter: Fname: fontname in Rtf
  globals: FontArray (contains list of fonts defined in fonts.cfg)
           FontArraySize (number of elements in FontArray)
   return: fontnumber from Rtf
 ****************************************************************************/
{
  int num = 0;

  for (num=0; num < FontArraySize; num++)
  {
     if (strcmp (FontArray[num].RtfCommand, Fname) == 0)
        return num;  
  }

  return GetTexFontNumber ("Roman");  /* default font */
}




/****************************************************************************/
int GetTexFontNumber(char * Fname)
/****************************************************************************
  purpose: gets the font number LaTex
parameter: Fname: fontname in LaTex
  globals: FontArray (contains list of fonts defined in fonts.cfg)
           FontArraySize (number of elements in FontArray)
   return: fontnumber from LaTex
 ****************************************************************************/
{
  struct ArrayElementT *help;


  help = (struct ArrayElementT *) bsearch (Fname, FontArray, FontArraySize,
                                   sizeof(struct ArrayElementT), (fptr)compare);
  if (help == NULL)
     return 0;
  else
     return help->number;

}




