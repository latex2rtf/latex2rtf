/*
 * $Id: cfg.c,v 1.1 2001/08/12 15:56:56 prahl Exp $
 * History:
 * $Log: cfg.c,v $
 * Revision 1.1  2001/08/12 15:56:56  prahl
 * latex2rtf version 1.5 by Ralf Schlatterbeck
 *
 * Revision 1.2  1995/05/24  11:54:03  ralf
 * Removed an off-by-one malloc error
 *
 * Revision 1.1  1995/03/23  16:09:01  ralf
 * Initial revision
 *
 */
/*****************************************************************************
     name : cfg.c
    autor : POLZER Friedrich und TRISKO Gerhard
  purpose : create sorted array for cfg-files, supports access with search()
 *****************************************************************************/


/****************************** includes *************************************/
#include "cfg.h"
/****************************************************************************/



/******************************* typedefs & structures **********************/
struct BinTreeElT
{
   char *theString;
   struct BinTreeElT *left, *right;
};
/****************************************************************************/


/********************************* global variables *************************/
struct BinTreeElT *DirectRoot = NULL, *IgnoreRoot = NULL, *FontRoot = NULL;

struct ArrayElementT *DirectArray, *FontArray, *IgnoreArray;
int DirectArraySize, FontArraySize, IgnoreArraySize;

char ConfigName[20];
int lineno;

extern char* progname;
/****************************************************************************/



/****************************************************************************/
/*** function prototypes ***/

int BuildBinTree (struct BinTreeElT **Root, FILE *fp);
void AddToTree (char *theCommand, struct BinTreeElT **Root);
BOOL ScanTree (struct BinTreeElT *Root, struct ArrayElementT *Array, int *index);
/****************************************************************************/






/****************************************************************************/
BOOL ReadCfg (void)
/****************************************************************************
purpose: opens config-files & starts reading them
params:  none
globals: Direct-, Font- IgnoreArray[Size/Root]
 ****************************************************************************/
{
   FILE *fp;
   int size;
   int index = 0;
   
   fp = open_cfg("direct.cfg");
   (void) strcpy (ConfigName, "direct.cfg");
   size = BuildBinTree (&DirectRoot, fp);
   DirectArray = malloc (size * sizeof(struct ArrayElementT) );
   if (DirectArray == NULL)
      error(" malloc error -> out of memory\n");
   (void) fclose (fp);

   ScanTree (DirectRoot, DirectArray, &index); 

   DirectArraySize = index;
   index = 0;

   fp = open_cfg("fonts.cfg");
   (void) strcpy (ConfigName, "fonts.cfg");
   size = BuildBinTree (&FontRoot, fp);
   FontArray = malloc (size * sizeof(struct ArrayElementT) );
   if (FontArray == NULL)
      error(" malloc error -> out of memory\n");
   (void) fclose (fp);

   ScanTree (FontRoot, FontArray, &index); 

   FontArraySize = index;
   index = 0;

   fp = open_cfg("ignore.cfg");
   (void) strcpy (ConfigName, "ignore.cfg");
   size = BuildBinTree (&IgnoreRoot, fp);
   IgnoreArray = malloc (size * sizeof(struct ArrayElementT) );
   if (IgnoreArray == NULL)
      error(" malloc error -> out of memory\n");
   (void) fclose (fp);

   ScanTree (IgnoreRoot, IgnoreArray, &index); 

   IgnoreArraySize = index;

}








/****************************************************************************/
int BuildBinTree (struct BinTreeElT **Root, FILE *fp)
/****************************************************************************
purpose:  reads line from cfg-file and AddToTree 
params:   Root for binary Tree, File-pointer for cfg-file
globals:  none
 ****************************************************************************/
{
   char buffer[512];
   char *theCommand;
   int size = 0;
   char *CommandEnd;
   
   lineno = 0;

   for (;;)
   {
     if (feof(fp))
        break;

     if ( (fgets( buffer, 512, fp )) == NULL)
        break;

     lineno++;

     if (strlen(buffer) > 500)
     {
        fprintf(stderr,"\n%s: ERROR: line too long in %s - only 500 characters", progname, ConfigName);
        fprintf(stderr, "\n%d: \"%s\"\n", lineno, buffer);
        fprintf(stderr,"\nprogram aborted\n");
        exit(-1);
     }


     if ((buffer[0] == '#') ||(buffer[0] == ' ') || (buffer[0] == '\0') || (buffer[0] == '\n'))
       continue;

     CommandEnd = strchr (buffer, '.');
     if (CommandEnd == NULL)
     {
        fprintf(stderr,"\n%s: ERROR: illegal format in %s - . expected",progname, ConfigName);
        fprintf(stderr, "\n%d: \"%s\"\n", lineno, buffer);
        fprintf(stderr,"\nprogram aborted\n");
        exit(-1);
     }

     CommandEnd[1] = '\0';
     
     theCommand = malloc (strlen(buffer) + 1);
     if (theCommand == NULL)
        error(" malloc error -> out of memory\n");

     strcpy (theCommand, buffer);

     AddToTree (theCommand, Root);
     size++;


   } /* for */

   return size;
}     






/****************************************************************************/
void AddToTree (char *theCommand, struct BinTreeElT **Root)
/****************************************************************************
purpose:  inserts theCommand to the B-Tree
params:   theCommand (from cfg-file), Root is root for B-tree
globals:  none
 ****************************************************************************/
{
   struct BinTreeElT *theBinTreeEl;

   if ((*Root) == NULL)
   {
      theBinTreeEl = malloc (sizeof (struct BinTreeElT));
      if (theBinTreeEl == NULL)
         error(" malloc error -> out of memory\n");
     
      theBinTreeEl->theString = theCommand;
      theBinTreeEl->right = NULL;
      theBinTreeEl->left  = NULL;

      (*Root) = theBinTreeEl;
   }
   else
   {
      if (strcmp (theCommand, (*Root)->theString) > 0)
         AddToTree (theCommand, &((*Root)->right));
      else
         AddToTree (theCommand, &((*Root)->left));
   }
}
       
       
     


/****************************************************************************/
BOOL ScanTree (struct BinTreeElT *Root, struct ArrayElementT *Array, int *index)
/****************************************************************************
purpose:  scans recursivly the whole B-tree infix and fill the sorted array
params:   Root to the B-tree, Array will contain the sorted commands, index is current index 
globals:  none 
 ****************************************************************************/
{
   char *RtfBegin;
   
   if (Root->left != NULL)
      ScanTree (Root->left, Array, index); 
   

   RtfBegin = strchr (Root->theString, ',');
   if (RtfBegin == NULL)
   {
      fprintf(stderr,"\n%s: ERROR: illegal format in %s - , expected", progname, ConfigName);
      fprintf(stderr, "\n%d: \"%s\"\n", lineno, Root->theString);
      fprintf(stderr,"\nprogram aborted\n");
      exit(-1);
   }
 
   RtfBegin[0] = '\0';
   Array[*index].RtfCommand = &(RtfBegin[1]);
   Array[*index].TexCommand = Root->theString;
   Array[*index].number = *index;

   (*index)++;

   if (Root->right != NULL)
      ScanTree (Root->right, Array, index); 
}



  




 
/****************************************************************************/
int compare (char *el1, struct ArrayElementT *el2)
/****************************************************************************
purpose:  compare-function for bsearch
params:   el1 is the TexCommand to be searched, el2 is a component of the array
globals:  none (nona!) 
 ****************************************************************************/
{
   return strcmp (el1, el2->TexCommand);
}


  
/****************************************************************************/
char *search (char *theTexCommand, enum ArrayKind WhichArray)
/****************************************************************************
purpose:  search theTexCommand in a specified array
params:   theTexCommand is the Tex-Command to be searched, WhichArray defines which array to use
globals:  none
 ****************************************************************************/
{
   struct ArrayElementT *help;
   char *RtfResult;
   

   switch (WhichArray)
   {
      case DIRECT_A:
           help = (struct ArrayElementT *) bsearch (theTexCommand, DirectArray, 
                               DirectArraySize, sizeof(struct ArrayElementT), (fptr)compare); 
           if (help == NULL)
              RtfResult = NULL;
           else
              RtfResult = help->RtfCommand;
           break; 
      case IGNORE_A:
           help = (struct ArrayElementT *) bsearch (theTexCommand, IgnoreArray, 
                               IgnoreArraySize, sizeof(struct ArrayElementT), (fptr)compare); 
           if (help == NULL)
              RtfResult = NULL;
           else
              RtfResult = help->RtfCommand;
           break; 
      case FONT_A:
           help = (struct ArrayElementT *) bsearch (theTexCommand, FontArray, 
                               FontArraySize, sizeof(struct ArrayElementT), (fptr)compare); 
           if (help == NULL)
              RtfResult = NULL;
           else
              RtfResult = help->RtfCommand;
           break; 
      default:
           assert(0);
   } /* switch */

   return RtfResult;
}








