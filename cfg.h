/*
 * $Id: cfg.h,v 1.1 2001/08/12 15:56:56 prahl Exp $
 * History:
 * $Log: cfg.h,v $
 * Revision 1.1  2001/08/12 15:56:56  prahl
 * latex2rtf version 1.5 by Ralf Schlatterbeck
 *
 * Revision 1.1  1995/03/23  16:09:01  ralf
 * Initial revision
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <malloc.h>
#include "main.h"

  
/*** global definitons used in many(!) files ***/

typedef int (*fptr) (const void*, const void*);
  
struct ArrayElementT
{
   char *TexCommand;
   char *RtfCommand;
   int number;
};


enum ArrayKind {DIRECT_A, IGNORE_A, FONT_A};


BOOL ReadCfg (void);
char *search (char *theCommand, enum ArrayKind WhichArray);
int compare (char *el1, struct ArrayElementT *el2);

  

