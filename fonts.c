/*
 * $Id: fonts.c,v 1.2 2001/08/12 15:47:04 prahl Exp $
 * History:
 * $Log: fonts.c,v $
 * Revision 1.2  2001/08/12 15:47:04  prahl
 * latex2rtf version 1.1 by Ralf Schlatterbeck
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
  purpose : The LaTeX font will be converted to the RTF font
 ******************************************************************************/

/****************************************** includes ************************/
#include <stdio.h>
#include "main.h"
#include "fonts.h"
#include <malloc.h>
#include <stdlib.h>
/******************************************************************************/
void error(char *);

/************************************* extern variables *********************/
extern char *progname;
/******************************************************************************/


/********************* defines and structures ********************************/
#define MAXLEN 80
struct FontListTag { char texName[MAXLEN];
		     char rtfName[MAXLEN];
		     struct FontListTag *next;
		   } *FontList;
/****************************************************************************/

/****************************** global variables ****************************/
int fontcount = 0;
/****************************************************************************/

/****************************************************************************/
BOOL WriteFontHeader(FILE* fRtf)
/****************************************************************************
  purpose: reads from the file fonts.cfg how some easy LaTex-commands can be
	   converted into Rtf-commands by text exchange
	   especially font-number-type-convertions
parameter: fRtf: File-Pointer to Rtf-File
 ****************************************************************************/
{
  FILE *fp;
  char buffer[512];
  char *buffpoint;
  char *mempoint;
  struct FontListTag *ListElem, *TmpElem;
  int count;

  fprintf(fRtf,"{\\fonttbl");

  FontList = (struct FontListTag*)malloc(sizeof (struct FontListTag));
  if (FontList == NULL)
    error(" malloc error -> out of memory\n");
  FontList->next = NULL;
  ListElem = FontList;

  fp = open_cfg("fonts.cfg");
  for(;;)
  {
    if ( (fgets(buffer, 512, fp )) == NULL)
      break;
    if (strlen(buffer) > 500)
    {
      fprintf(stderr, "\n%s: ERROR:line too long in FONTS.CFG - only 500 characters",progname);
      fprintf(stderr,"\nprogram aborted\n");
      fclose(fp);
      exit(-1);
    }
    buffpoint = buffer;
    /* on empty lines the value of *buffpoint is 10 (LF) therfore < 32 */
    if ((*buffpoint == '#') || (*buffpoint < 32))
      continue; 		   /* read next line */

    mempoint = ListElem->texName;
    count = 0;
    while (*buffpoint != ',')
    {
      *mempoint = *buffpoint;
      mempoint++;
      buffpoint++;
      count++;
      if (count >= MAXLEN)
      {
	fprintf(stderr, "\n%s: ERROR: error in file FONTS.CFG - missing comma at font %d",
	    progname,fontcount+1);
	fprintf(stderr,"\nprogram aborted\n");
	fclose(fp);
	exit(-1);
      }
    }
    *mempoint = '\0';
    buffpoint++;

    mempoint = ListElem->rtfName;
    count = 0;
    while (*buffpoint != '.')
    {
      *mempoint = *buffpoint;
      mempoint++;
      buffpoint++;
      count++;
      if (count >= MAXLEN)
      {
	fprintf(stderr, "\n%s: ERROR: error in file FONTS.CFG - missing '.' at font %d",
	    progname,fontcount+1);
	fclose(fp);
	fprintf(stderr,"\nprogram aborted\n");
	exit(-1);
      }
    }
    *mempoint = '\0';
    ++fontcount;

    fprintf(fRtf,"{\\f%d\\fnil %s;}",fontcount-1,ListElem->rtfName);

    TmpElem = (struct FontListTag*)malloc(sizeof (struct FontListTag));
    TmpElem->next = NULL;
    TmpElem->rtfName[0] = '\0';
    TmpElem->texName[0] = '\0';
    ListElem->next = TmpElem;
    ListElem = TmpElem;


  }; /* end while */
  fprintf(fRtf,"}");
  return TRUE;
}


/****************************************************************************/
int GetFontNumber(char * Fname)
/****************************************************************************
  purpose: gets the font number Rtf
parameter: Fname: fontname in Rtf
 return: fontnumber from Rtf
 ****************************************************************************/
{
  int num = 0;
  struct FontListTag *list = FontList;


  while (list != NULL)
  {

    if (strcmp(list->rtfName,Fname)==0)
      return num;
    num++;
    list = list->next;
  }

  return 0;
}

/****************************************************************************/
int GetTexFontNumber(char * Fname)
/****************************************************************************
  purpose: gets the font number LaTex
parameter: Fname: fontname in LaTex
 return: fontnumber from LaTex
 ****************************************************************************/
{
  int num = 0;
  struct FontListTag *list = FontList;

  while (list != NULL)
  {

    if (strcmp(list->texName,Fname)==0)
      return num;
    num++;
    list = list->next;
  }

  return 0;
}

/****************************************************************************/
void RemoveFontlist(void)
/****************************************************************************
 purpose: free all memory resources needed for font-exchange (LaTex-Rtf)
 ****************************************************************************/
{
  struct FontListTag *oldelem = FontList, *list;

  if (oldelem == NULL) return;
  FontList = NULL;
  list = oldelem->next;
  while (oldelem != NULL)
  {
    free(oldelem);
    oldelem = list;
    if (list != NULL)
      list = list->next;
  }
}
