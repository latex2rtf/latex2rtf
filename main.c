/*
 * $Id: main.c,v 1.3 2001/08/12 15:56:56 prahl Exp $
 * History:
 * $Log: main.c,v $
 * Revision 1.3  2001/08/12 15:56:56  prahl
 * latex2rtf version 1.5 by Ralf Schlatterbeck
 *
 * Revision 1.8  1995/05/24  17:12:11  ralf
 * Corrected bug with variable input being NULL
 *
 * Revision 1.7  1995/05/24  16:02:41  ralf
 * Changes by Vladimir Menkov for DOS port
 *
 * Revision 1.6  1995/05/24  15:09:10  ralf
 * Added support for spanish ligatures by John E. Davis (untested)
 *
 * Revision 1.5  1995/05/24  14:48:57  ralf
 * Corrected searching for citations in .aux
 *
 * Revision 1.4  1995/03/23  15:58:08  ralf
 * Reworked version by Friedrich Polzer and Gerhard Trisko
 *
 *
 * Revision 1.3  1994/06/21  08:14:11  ralf
 * Corrected some bugs
 *
 * Revision 1.2  1994/06/17  14:19:41  ralf
 * Corrected various bugs, for example interactive read of arguments
 *
 * Revision 1.1  1994/06/17  11:26:29  ralf
 * Initial revision
 */
/***************************************************************************
     name : main.c
   author : DORNER Fernando, GRANZER Andreas
            POLZER Friedrich,TRISKO Gerhard
            MS DOS changes by Vladimir MENKOV
 * removed RemoveFontList() in main()
 * blanklines can consist of blanks (bBlankLine in Convert)
 * new option -l for Latin-1
 * writing of reference-list included
  purpose : main convert-routine from LaTex2Rtf
 *****************************************************************************/


/****************************** includes *************************************/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "commands.h"
#include "funct1.h"
#include "fonts.h"
#include "stack.h"
#include "funct2.h"
#include "direct.h"
#include "ignore.h"
#include "version.h"
#include "cfg.h"
#include "encode.h"
/****************************************************************************/


/********************************* global variables *************************/
long linenumber=1;		      /* counts lines in the LaTex-document */
FILE *fTex = stdin;		/* file pointer to Latex file */
FILE *fRtf = stdout;		/* file pointer to RTF file */
char *input = NULL, *output = NULL;
char *AuxName = NULL;          /* file pointer to AUX file */
char *BblName = NULL;          /* file pointer to BBL file */

char *progname;			/* name of the executable file */
char *latexname= "stdin";	/* name of LaTex-File */
char alignment = JUSTIFIED;	/* default for justified: */
fpos_t pos_begin_kill;
BOOL bCite = UNDEFINED;         /* to produce citations */
/****************************************************************************/
/*** function prototypes ***/
BOOL PrepareTex(char *filename, FILE **f);  /* opens file for reading */
BOOL PrepareRtf(char * filename, FILE **f); /* creates file and writes */
					   /* RTF header */
BOOL CloseTex(FILE **f);
BOOL CloseRtf(FILE **f);

BOOL TranslateCommand();  /* converts commands */
BOOL TranslateSpecKey();  /* converts special keys */
void *GetCommandFunc(char *cCommand);
void numerror(int num);
BOOL ConvertFormula(char * command);

enum TexCharSetKind TexCharSet = SEVEN_BIT; /* default SEVEN_BIT for
					       converting special chars */



/****************************************************************************/
int main(int argc, char **argv)
/****************************************************************************
purpose: checks parameter and starts convert routine
params:  command line arguments argc, argv
globals: initializes in- and outputfile fTex, fRtf,
 ****************************************************************************/
{

  int c, errflag = 0;
  char tmpText[PATHMAX];
  extern int getopt(int, char *const[], const char *);
  extern char *optarg;
  extern int optind, opterr;


  PushEnvironment(HEADER);
  PushEnvironment(DOCUMENT);
  progname=argv[0];
  while((c = getopt(argc, argv, "Vlo:a:b:")) != EOF)
  {
    switch(c)
    {
      case 'V':
	printf("%s: %s\n", progname, Version);
	return(0);
      case 'a':
	AuxName = optarg;
	break;
      case 'b':
	BblName = optarg;
	break;
      case 'o':
	output = optarg;
	break;
      case 'l':
        TexCharSet = ISO_8859_1;
        fprintf(stderr,"Latin-1 (= ISO 8859-1) special characters will be ");
        fprintf(stderr,"converted into RTF-Commands!\n");
	break;
      default:
	errflag = 1;
    }
  }
  if(argc > optind + 1 || errflag)
  {
    fprintf(stderr,"%s: Usage: %s [-V] [-l] [-o outfile] [-a auxfile] [-b bblfile] inputfile\n",
      progname, progname);
    fprintf(stderr,"-l\t Latin-1 (= ISO 8859-1) special characters will be\n");
    fprintf(stderr,"\t converted into RTF-Commands!\n");
    return(1);
  }
  if(argc == optind + 1)
  {
    input = argv[optind];
    latexname = input;
  }
  if (AuxName == NULL)
  {
    char *s;
    if(input != NULL)
    {
	if((AuxName = malloc(strlen(input) + 5)) == NULL)
	    error(" malloc error -> out of memory!\n");
	strcpy (AuxName, input);
	if((s = strrchr(AuxName, '.')) == NULL || strcmp(s, ".tex") != 0)
	   strcat(AuxName, ".aux");
	else
	   strcpy (s, ".aux");
    }
    else
	AuxName = "";
  }

  ReadCfg();
  PrepareTex(input,&fTex);
  PrepareRtf(output,&fRtf);
  Push(1,0);
  Convert(fTex, fRtf);
  /* if citations were produced, now the reference-list will be created */
  PushEnvironment(DOCUMENT);
  if ( bCite != UNDEFINED)
     WriteRefList();

  CloseTex(&fTex);
  CloseRtf(&fRtf);
  return(0);
}


/****************************************************************************/
/* Global Flags for Convert Routine */
/****************************************************************************/
int RecursLevel = 0;
int BracketLevel = 1;
int ret = 0;
BOOL mbox = FALSE;
BOOL MathMode = FALSE;
BOOL bNewPar = FALSE;
int indent = 0;
BOOL NoNewLine = FALSE;
int ConvertFlag;
BOOL bInDocument = FALSE;
int tabcounter = 0;
int fontsize = 20;
BOOL twocolumn = FALSE;
BOOL titlepage = FALSE;
BOOL article = TRUE;
BOOL TABBING_ON = FALSE;
BOOL TABBING_RETURN = FALSE;
BOOL TABBING_ON_itself = FALSE;
BOOL TITLE_AUTHOR_ON = FALSE;
BOOL GermanMode = FALSE;  /* switches support for germanstyle on or off */
/* the Germand Mode supports most of the commands defined in GERMAN.STY file
   by H.Partl(TU Wien) 87-06-17 */
BOOL bTabular = FALSE;
BOOL bBlankLine=TRUE;  /* to handle pseudo-blank lines (contains spaces)
                          correctly					    */
int colCount;     /* number of columns in a tabular environment             */
int actCol;       /* actual column in the tabular environment               */
char* colFmt;
int DefFont = 0;  /* contains default font number, which is associated
		     with TeX's font "Roman"                                */



/****************************************************************************/
BOOL Convert()
/****************************************************************************
purpose: convertes inputfile and writes result to outputfile
globals: fTex, fRtf and all global flags for convert (see above)
 ****************************************************************************/
{
  char cThis = '\n';
  char cLast = '\n';
  char cLast2 = '\n';
  char cLastNoSpace = 'a';
  char cNext;
  int PopLevel=0,PopBrack,PPopLevel,PPopBrack,size,retlevel;
  int count = 0;
  int i;
  char cThishilf;

  RecursLevel++;
  Push(RecursLevel,BracketLevel);
  ++ConvertFlag;
  while (fread(&cThis, 1,1,fTex) == 1)
  {
    switch(cThis)
    {
    case '\\':{
	      int SaveFlag;
	      fpos_t pos1,pos2;

              bBlankLine=FALSE;
	      fgetpos(fRtf,&pos1);
	      Push(RecursLevel,BracketLevel);
	      SaveFlag=ConvertFlag;
	      TranslateCommand();

	      /*if ((TABBING_ON_itself) ||
		  ((TABBING_ON) &&
		  (TABBING_RETURN)))
		  {
		  TABBING_RETURN = FALSE;
		  if (TABBING_ON_itself)
		     TABBING_ON = FALSE;
		  TABBING_ON_itself = FALSE;
		  return TRUE;
		  } */

	      /* erase double identic values on stack top */
	      for(;;)
	      {
		if ((size = Pop(&PPopLevel, &PPopBrack)) <= 0 )
		{
		  Push(PPopLevel,PPopBrack);
		  break;
		}
		if ((size = Pop(&PopLevel, &PopBrack)) <= 0 )
		{
		  Push(PopLevel,PopBrack);
		  break;
		}
		if ( (PPopLevel == PopLevel) && (PPopBrack == PopBrack) )
		{
		  Push(PopLevel,PopBrack);
		}
		else
		{
		  Push(PopLevel,PopBrack);
		  Push(PPopLevel,PPopBrack);
		  break;
		}
	      };

	      if (ret > 0)
	      {
		--ret;
		--RecursLevel;
		return TRUE;
	      }
	      /* remove SaveFlag */
	      /*if (ConvertFlag == SaveFlag)    Convert was not called
	      {
		//cThis = ' ';	 error
	      }
              */
	      cThis = '\\';
	      fgetpos(fRtf,&pos2);
	      if (pos1 == pos2)    /* no RTF output */
	      {
		if (cLast == '\n')
		  cThis = '\n';
		else
		  cThis = ' ';
	      }
	      break;
	      }
    case '%': bBlankLine=FALSE;
              IgnoreTo('\n');
	      cThis = ' ';
	      break;
    case '{': bBlankLine=FALSE;
	      Push(RecursLevel,BracketLevel);
	      ++BracketLevel;
	      break;
    case '}': bBlankLine=FALSE;
	      BracketLevel--;
	      PPopLevel = RecursLevel;
	      PPopBrack = BracketLevel;
	      size = Pop(&PopLevel, &PopBrack);
	      while ( (size = Pop(&PopLevel, &PopBrack)) >= 0 )
	      {
		if ( PopBrack < BracketLevel )
		{
		  break;
		}
		PPopLevel = PopLevel;
		PPopBrack = PopBrack;
	      } /* while */
	      Push(PopLevel,PopBrack);	/* push back */
	      retlevel = PPopLevel;
	      ret = RecursLevel-retlevel;
	      Push(retlevel,BracketLevel);

	      if (ret > 0)
	      {
		ret--;
		RecursLevel--;
		return TRUE;
	      }
	      else
		break;
    case '\r':fprintf(stderr,"\n%s: ERROR: error in input file: %s at linenumber: %ld\n",progname,latexname,getLinenumber());
	      fprintf(stderr,"\nprogram aborted\n");
	      exit(-1);
	      break;
    case ' ': if (!bInDocument) continue;
	      if ( (cLast != ' ') && (cLast != '\n'))
	      {
		if (mbox == FALSE)
	       /*   if (bNewPar == FALSE)*/
		    fprintf(fRtf," ");
		else
		  fprintf(fRtf,"\\~");
	      }
	      break;
    case '~': bBlankLine=FALSE;
              if (!bInDocument) numerror(ERR_WRONG_COMMAND);
	      fprintf(fRtf,"\\~");
	      break;
    case '\n':
	      tabcounter=0;
	      linenumber++;
	      if (!bInDocument) continue;
	      if (cLast != '\n')
	      {
		if (bNewPar == TRUE)
		{
                   bNewPar = FALSE;
                   cThis = ' ';
                   break;
                }
		if (cLast != ' ')
		  fprintf(fRtf," ");	 /* treat as 1 space */
                else if(bBlankLine==TRUE)
                {
		  fprintf(fRtf,"\n\\par\\fi0\\li%d ",indent);
		}
	      }
	      else
	      {
		if (cLast2 != '\n')
		{
		  fprintf(fRtf,"\n\\par\\fi0\\li%d ",indent);
		}
	      }
              bBlankLine=TRUE;
	      break;
    case '^': bBlankLine=FALSE;
              if (!bInDocument) numerror(ERR_WRONG_COMMAND);

	      fprintf(fRtf,"{\\up6 ");
              if (fTexRead (&cNext,1,1,fTex) == 1)
              {
                 if (cNext == '{')
                 {
	            fseek(fTex,-1L,SEEK_CUR); /* reread last character */
  	            Push(RecursLevel,BracketLevel);
	            Convert();
	            if (ret > 0)
	            {
		       --ret;
		       --RecursLevel;
	               return TRUE;
	            }
	            fprintf(fRtf,"}");
                 }
                 else
                    fprintf(fRtf,"%c}",cNext);
              }
              else
	         error("Error reading Latex-File");
	      break;
    case '_': bBlankLine=FALSE;
              if (!bInDocument) numerror(ERR_WRONG_COMMAND);
	      fprintf(fRtf,"{\\dn6 ");
              if (fTexRead (&cNext,1,1,fTex) == 1)
              {
                 if (cNext == '{')
                 {
	            fseek(fTex,-1L,SEEK_CUR); /* reread last character */
  	            Push(RecursLevel,BracketLevel);
	            Convert();
	            if (ret > 0)
	            {
		       --ret;
		       --RecursLevel;
	               return TRUE;
	            }
	            fprintf(fRtf,"}");
                 }
                 else
                    fprintf(fRtf,"%c}",cNext);
              }
              else
	         error("Error reading Latex-File");
	      break;
    case '$': bBlankLine=FALSE;
              CmdFormula(FORM_DOLLAR);
	      break;
    case '&': if (bTabular)
              {
		 fprintf(fRtf," \\cell \\pard \\intbl ");
                 actCol++;
                 fprintf (fRtf, "\\q%c ", colFmt[actCol]);
              }
	      else
	         fprintf(fRtf,"&");
	      break;
    case '-' : bBlankLine=FALSE;
               count++;
	       while ((fread(&cThishilf,1,1,fTex) >= 1) && (cThishilf == '-'))
		   count++;
	       switch (count)
	       {
		 case 1: fprintf(fRtf,"-");
			 break;
		 case 2: fprintf(fRtf,"\\endash ");
			 break;
		 case 3: fprintf(fRtf,"\\emdash ");
			 break;
		 default:
		 { for (i = count-1; i >= 0; i--)
		   fprintf(fRtf,"-");
		 }
	       }
	       fseek(fTex,-1L,SEEK_CUR); /* reread last character */
	       count = 0;
		break;
    case '\'' :bBlankLine=FALSE;
               count++;
	       while ((fread(&cThishilf,1,1,fTex) >= 1) && (cThishilf == '\''))
		   count++;
	       if (count != 2)
		 { for (i = count-1; i >= 0; i--)
		   fprintf(fRtf,"\\rquote ");
		 }
	       else
		   fprintf(fRtf,"\\rdblquote ");
	       fseek(fTex,-1L,SEEK_CUR); /* reread last character */
	       count = 0;
	       break;
    case '`' : bBlankLine=FALSE;
               count++;
	       while ((fread(&cThishilf,1,1,fTex) >= 1) && (cThishilf == '`'))
		   count++;
	       if (count != 2)
		 { for (i = count-1; i >= 0; i--)
		   fprintf(fRtf,"\\lquote ");
		 }
	       else
		   fprintf(fRtf,"\\ldblquote ");
	       fseek(fTex,-1L,SEEK_CUR); /* reread last character */
	       count = 0;
	       break;
    case '\"': bBlankLine=FALSE;
               if (GermanMode)
		 TranslateGerman();
	       else
		 fprintf(fRtf,"\"");
	       break;
    case '\t':bBlankLine=FALSE;
              break;
    /* Added by John E. Davis to take care of !` and ?`. */
    case '?':
    case '!':
	{
	    char ch;

	    bBlankLine=FALSE;
	    if (fread (&ch, 1, 1, fTex) == 1)
	    {
		if (ch == '`')
		{
		    fprintf (fRtf, "\\mac\\'c%d\\pc ", (cThis == '?') ? 0 : 1);
		}
		else
		{
		    putc (cThis, fRtf);
		    fseek(fTex, -1L, SEEK_CUR);
		}
	    }
	}
	break;
    default: bBlankLine=FALSE;
             if (bInDocument)
	     {
	      if ((isupper(cThis)) &&
		 ((cLast=='.') || (cLast=='!') || (cLast=='?') || (cLast==':')))
	  	 fprintf(fRtf," ");
              switch (TexCharSet)
              {
                 case ISO_8859_1:
                      Write_ISO_8859_1 (cThis);
                      break;
                 case SEVEN_BIT:
                 default:
	              fprintf(fRtf,"%c",cThis);
                      break;
              }
	      bNewPar = FALSE;
	    }
	  break;
    }

  tabcounter++;
  cLast2 = cLast;
  cLast = cThis;
  if (cThis != ' ')
    cLastNoSpace = cThis;
  }
  RecursLevel--;
  return TRUE;
}


/****************************************************************************/
void error(char * text)
/****************************************************************************
purpose: writes error message
globals: reads progname;
 ****************************************************************************/
{
  fprintf(stderr,"\n%s: ERROR: %s",progname,text);
  fprintf(stderr,"\nprogram aborted\n");
  exit(-1);
}


/****************************************************************************/
void numerror(int num)
/****************************************************************************
purpose: writes error message identified by number - for messages on many
	 places in code. Calls function error.
globals: progname; latexname; linenumber;
 ****************************************************************************/
{

  char text[1024];

  switch(num)
  {
    case ERR_EOF_INPUT:
      sprintf(text,"%s%s%s%ld%s","unexpected end of input file in: ",latexname," at linenumber: ",getLinenumber(),"\n");
      error(text);
      break;
    case ERR_WRONG_COMMAND:
      sprintf(text,"%s%s%s%ld%s","unexpected command or character in: ",latexname," at linenumber: ",getLinenumber(),"\n");
      error(text);
      break;
    case ERR_Param:
      error("wrong number of parameters\n");
      break;
    case ERR_WRONG_COMMAND_IN_TABBING:
      sprintf(text,"%s%s%s%ld%s","wrong command in Tabbing-kill-command-line in: ",latexname," at linenumber: ",getLinenumber(),"\n");
      error(text);
      break;
    default :
      error("internal error");
      break;
  }
}


/****************************************************************************/
BOOL PrepareRtf(char *filename, FILE **f)  /* creates file and writes */
					   /* RTF header */
/****************************************************************************
purpose: creates output file and writes RTF-header.
params: filename - name of outputfile
	f - pointer to filepointer to store file ID
globals: fontsize;
 ****************************************************************************/
{
  if(filename != NULL)
  {
      /* I have replaced "wb" with "w" in the following
         fopen, for correct operation under MS DOS (with the -o option).
         I believe this should make no difference under UNIX.
                                                   --V.Menkov
       */

      if ((*f = fopen(filename,"w")) == NULL)	 /* open file */
      {
	error("Error opening RTF-file");
	exit(1);
      }
  }
  /* write header */
  fprintf(*f,"{\\rtf1\\pc\\fs%d\\deff0\\deflang1024\n",fontsize);
  fprintf(*f,"{\\info{\\version1}}\\widowctrl\\ftnbj\\sectd\\linex0\\endnhere");
  fprintf(*f,"\\qj \n");
  WriteFontHeader(*f);
  return TRUE;
}


/****************************************************************************/
BOOL PrepareTex(char *filename, FILE **f)  /* opens file for reading */
/****************************************************************************
purpose: opens input file.
params: filename - name of inputfile
	f - pointer to filepointer to store file ID
 ****************************************************************************/
{
  if(filename != NULL)
  {
      if ((*f = fopen(filename,"r")) == NULL)	/* open file */
      {
	error("Error opening LATEX-file");
	exit(1);
      }
  }
  return TRUE;
}


/****************************************************************************/
BOOL CloseTex(FILE **f)
/****************************************************************************
purpose: closes input file.
params: f - pointer to filepointer to invalidate
 ****************************************************************************/
{
  if(*f != stdin)
      fclose(*f);
  *f = NULL;
  return TRUE;
}


/****************************************************************************/
BOOL CloseRtf(FILE **f)
/****************************************************************************
purpose: closes output file.
params: f - pointer to filepointer to invalidate
globals: progname;
 ****************************************************************************/
{
  fprintf(*f,"}");
  if(*f != stdout)
  {
      if(fclose(*f) == EOF)
      {
	fprintf(stderr, "%s: Warning: Error closing RTF-File\n", progname);
      }
  }
  *f = NULL;
  return TRUE;
}


/****************************************************************************/
BOOL TranslateCommand()
/****************************************************************************
purpose: The function is called on a backslash in input file and
	 tries to call the command-function for the following command.
returns: sucess or not
globals: fTex, fRtf, command-functions have side effects or recursive calls;
         global flags for convert
 ****************************************************************************/
{
  char cCommand[MAXCOMMANDLEN];
  int i;
  char cThis;
  for (i = 0; ;i++)   /* get command from input stream */
  {
    if (fread(&cThis,1,1,fTex) < 1)
       break;
    if (i == 0) /* test for special characters */
    {
      switch(cThis)
      {
	case '}': fprintf(fRtf,"\\}"); return TRUE;
	case '{': fprintf(fRtf,"\\{"); return TRUE;
	case '#': fprintf(fRtf,"#"); return TRUE;
	case '$': fprintf(fRtf,"$"); return TRUE;
	case '&': fprintf(fRtf,"&"); return TRUE;
	case '%': fprintf(fRtf,"%%"); return TRUE;
	case '_': fprintf(fRtf,"_"); return TRUE;
	case '\\':    /* produces a newline in output */
		      /*	  no new paragraph    */
		  if (bTabular)
		  {
                     for (; actCol< colCount; actCol++)
                     {
                        fprintf (fRtf, " \\cell \\pard \\intbl ");
                     }
		     fprintf(fRtf," \\cell \\pard \\intbl \\row \n \\pard \\intbl \\q%c ", colFmt[1]);
                     actCol = 1;
		  }
		  else
		  {
	             if (fread(&cThis,1,1,fTex) < 1)
		        cThis='a';

		     if ((cThis == ' ') ||
		        (cThis == '*'))  /* ignore * after \\ -> it's the same command as \\ */
		     {
		        if (fread(&cThis,1,1,fTex) < 1)
		           cThis='a';
		        while(cThis == ' ') 		   /*space  ignore */
		        if (fread(&cThis,1,1,fTex) != 1)
		           break;
		       fseek(fTex,-1,SEEK_CUR);
		    }
		    else
		    {
		       fseek(fTex,-1,SEEK_CUR);
		    }

		     fprintf(fRtf,"\\par ");
		     bNewPar = TRUE;
		     tabcounter = 0;
		     if (TABBING_ON)
		        fgetpos(fRtf,&pos_begin_kill);
		  }
		  return TRUE;
	case ' ': fprintf(fRtf," ");	/* ordinary interword space */
		  while (cThis == ' ')	 /* all spaces after commands are ignored */
		  {
		     if (fread(&cThis,1,1,fTex) < 1)
			numerror(ERR_EOF_INPUT);
		  }

		  fseek(fTex,-1L,SEEK_CUR);
		  return TRUE;
		  break;
	case '-': /* hyphen */
		  /* caution: this character has another effect in the tabbing-environment */
		  if (TABBING_ON);
		  else
		     fprintf(fRtf,"\\-");
		  return TRUE;
		  break;
	case '+': /* see tabbing-environment */
		  /* no harm for RTF-output */
		  break;
	case '<': /* see tabbing-environment */
		  /* no harm for RTF-output */
		  break;
	case '~': CmdTildeChar(0);
		  return TRUE;
	case '^': CmdSpitzeChar(0);
		  return TRUE;
	case '\"':CmdUmlaute(0);
		  return TRUE;
	case '`': if (TABBING_ON);
		  else
		    CmdLApostrophChar(0);
		  return TRUE;
	case '\'':if (TABBING_ON);
		  else
		     CmdRApostrophChar(0);  /* char ' =?= \' */
		  return TRUE;
	case '(': CmdFormula(FORM_RND_OPEN);
		  return TRUE;
	case '[': CmdFormula(FORM_ECK_OPEN);
		  return TRUE;
	case ')': CmdFormula(FORM_RND_CLOSE);
		  return TRUE;
	case ']': CmdFormula(FORM_ECK_CLOSE);
		  return TRUE;
	case '/': CmdIgnore(0);
		  return TRUE;
	case ',': CmdIgnore(0);  /* \, produces a small space */
		  return TRUE;
	case '@': CmdIgnore(0);  /* \@ produces an "end of sentence" space */
		  return TRUE;
	case '>': CmdTabjump(0);
		  return TRUE;
	case '=': CmdTabset(0);
		  return TRUE;
	case '3': fprintf(fRtf, "\\ansi\\'df\\pc ");   /* german symbol 'á' */
		  return TRUE;
      }
    }

    if (!isalpha(cThis))
    {
      int found_nl = FALSE;
      /* all spaces after commands are ignored, a single \n may occur */
      while (cThis == ' ' || cThis == '\t' || cThis == '\n' && !found_nl)
	{
	if(cThis == '\n')
	{
           found_nl = TRUE;
        }
	if (fread(&cThis,1,1,fTex) < 1)
	    break;
	}

      fseek(fTex,-1L,SEEK_CUR); /* position of next character after command
				   except space */
      break;
    }
    cCommand[i] = cThis;
  }

  cCommand[i] = '\0';  /* mark end of string with zero */

  if ( i == 0)
    return FALSE;
  if (CallCommandFunc(cCommand)==TRUE) /*call handling function for command*/
    return TRUE;
  else
  {
    if (TryDirectConvert(cCommand, fRtf) == TRUE)
      return TRUE;
    else
      {
	if (TryVariableIgnore(cCommand, fTex) == TRUE)
	  return TRUE;
	else
	{
	  fprintf(stderr,"\n%s: WARNING: command: %s not found - ignored\n",progname,cCommand);
	  return FALSE;
	}
      }
  }
  return TRUE;
}
/****************************************************************************/

/****************************************************************************/
void IgnoreTo(char cEnd)
/****************************************************************************
purpose: ignores anything from inputfile till character cEnd
params:  charcter to stop ignoring
globals: changes inputfile fTex
         linenumber;
 ****************************************************************************/
{
  char c;
  while (fread(&c, 1,1,fTex) >= 1)
  {
    if (c == cEnd)
    {
      if (cEnd == '\n')
	fseek(fTex,-1L,SEEK_CUR);   /*linenumber is set in convert-routine */
	/*linenumber++;*/
      return;
    }
    else
    {
     if (c == '\n')
       linenumber++;
    }
  }
  numerror(ERR_EOF_INPUT);
}
/****************************************************************************/




/****************************************************************************/
FILE *open_cfg(const char *name)
/****************************************************************************
purpose: open config files specified in name
params:  name: config-file-name
 ****************************************************************************/
{
    char *cfg_path = getenv("RTFPATH");
    static char *path = NULL;
    static int size = 1024;
    int len;
    FILE *fp;

    if(path == NULL && (path = (char*)malloc(size)) == NULL)
    {
	fprintf(stderr, "%s: Fatal Error: Cannot allocate memory\n", progname);
	exit(23);
    }
    if(cfg_path != NULL)
    {
	char *s, *t;
	s = cfg_path;
	while(s && *s)
	{
	    t = s;
	    s = strchr(s, ':');
	    if(s)
	    {
		*s = '\0';
		s++;
	    }
	    if((len = (strlen(t) + strlen(name) + 2)) > size)
	    {
		size = len;
		if((path = (char*)realloc(path, size)) == NULL)
		{
		    fprintf(stderr, "%s: Fatal Error: Cannot allocate memory\n",
			progname);
		    exit(23);
		}
	    }
	    strcpy(path, t);
	    strcat(path, "/");
	    strcat(path, name);
	    if((fp = fopen(path,"r")) != NULL)
		return(fp);
	}
    }
    if((len = (strlen(LIBDIR) + strlen(name) + 2)) > size)
    {
	size = len;
	if((path = (char*)realloc(path, size)) == NULL)
	{
	    fprintf(stderr, "%s: Fatal Error: Cannot allocate memory\n",
		progname);
	    exit(23);
	}
    }
    strcpy(path, LIBDIR);
    strcat(path, "/");
    strcat(path, name);
    if((fp = fopen(path,"r")) == NULL)
    {
	fprintf(stderr, "\n%s: ERROR: cannot open file '%s'.",progname,name);
	fprintf(stderr,"\nprogram aborted\n");
	fclose(fp);
	exit(1);
    }
    return(fp);
}






/****************************************************************************/
size_t fTexRead (void *ptr, size_t size, size_t nitems, FILE *stream)
/****************************************************************************
purpose: performs fread and increments linenumber if '\n' is read...
         errors are checked after calling fTexRead!
params:  same as needed  for fread(3)
 ****************************************************************************/
{
   size_t val;

   val = fread (ptr, size, nitems, stream);

   if ( *((char*)ptr) == '\n' )
      linenumber++;

   return val;
}



/****************************************************************************/
int getLinenumber (void)
/****************************************************************************
purpose: get number of actual line (do not use global linenumber, because
         it does not work correctly!)
         this function is not very efficient, but it will be used only once when
         printing an error-message + program abort
params:  none
 ****************************************************************************/
{
  char buffer[1024];
  fpos_t oldpos;
  fpos_t pos;
  int linenum = 0;

  if ( fgetpos (fTex, &oldpos) != 0)
     error ("fgetpos: can\'t get linenumber");
  if ( fseek (fTex, 0L, SEEK_SET) == -1)
     error ("fseek: can\'t get linenumber");

  do
  {
    if (fgets(buffer, 1023, fTex) == NULL)
       error ("fgets: can\'t get linenumber");
    linenum++;
    if ( fgetpos (fTex, &pos) != 0)
       error ("fgetpos: can\'t get linenumber");
  }
  while (pos < oldpos);

  if ( fsetpos (fTex, &oldpos) != 0)
     error ("fgetpos: can\'t get linenumber");

  return linenum;

}



