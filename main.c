/*
 * $Id: main.c,v 1.8 2001/08/12 18:53:25 prahl Exp $
 * History:
 * $Log: main.c,v $
 * Revision 1.8  2001/08/12 18:53:25  prahl
 * 1.9d
 *         Rewrote the \cite code.
 *         No crashes when .aux missing.
 *         Inserts '?' for unknown citations
 *         Added cite.tex and cite.bib to for testing \cite commands
 *         hyperref not tested since I don't use it.
 *         A small hyperref test file would be nice
 *         Revised treatment of \oe and \OE per Wilfried Hennings suggestions
 *         Added support for MT Extra in direct.cfg and fonts.cfg so that
 *         more math characters will be translated e.g., \ell (see oddchars.tex)
 *         added and improved font changing commands e.g., \texttt, \it
 *
 * Revision 1.11  1998/10/28 06:10:25  glehner
 * Added "errno.h"
 * Moved stdin/stdout assignation from declaration to function body
 * of (main).
 * Trying to call (getLinenumber) only when not reading from stdin.
 * Implemented (WriteTemp) from Wilfried Hennings (and colleagues)
 * to provide for proper eol-handling across different platforms.
 * Changed all eol-rtf-output to "\r\n".
 * Added second closing brace end eol in (CloseRtf).
 *
 * Revision 1.10  1998/07/03 06:54:13  glehner
 * added `language' and -i option
 * `twoside' for pagestyle handling
 * function diagnostic and asterisk form of LaTeX commands
 *
 * Revision 1.9  1997/02/15 20:52:27  ralf
 * Corrected some bugs and variable declarations found by lclint
 * Moved config file opening to cfg.c
 *
 * Revision 1.8  1995/05/24 17:12:11  ralf
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
#include <stdarg.h>
#include <errno.h>
#include "main.h"
#include "commands.h"
#include "funct1.h"
#include "l2r_fonts.h"
#include "stack.h"
#include "funct2.h"
#include "direct.h"
#include "ignore.h"
#include "version.h"
#include "cfg.h"
#include "encode.h"
#include "util.h"

#ifdef __MWERKS__
#include "MainMain.h"
#endif

/****************************************************************************/


/********************************* global variables *************************/
long linenumber=1;		               /* lines in the LaTex-document */
/*@null@*//*@observer@*/        char *currfile;/* current file name */
FILE *fTex = (FILE *)NULL;                     /* file pointer to Latex file */
FILE *fRtf = (FILE *)NULL;	               /* file pointer to RTF file */
       /*@null@*//*@observer@*/ char *input   = NULL;
static /*@null@*//*@observer@*/ char *output  = NULL;
                                char *AuxName = NULL;
                                char *BblName = NULL;

/*@observer@*/ char *progname;		       /* name of the executable file */
char *latexname= "stdin";                      /* name of LaTex-File */
char alignment = JUSTIFIED;	               /* default for justified: */
fpos_t pos_begin_kill;
bool bCite = FALSE;                        /* to produce citations */
bool GermanMode = FALSE;  /* switches support for germanstyle on or off */
/* the Germand Mode supports most of the commands defined in GERMAN.STY file
   by H.Partl(TU Wien) 87-06-17 */

char *language = "english";   /* in \begin{document} "language".cfg is read in */
bool twoside = FALSE;
/* verbosity of diagnostics output. */
static int verbosity = WARNING;
/*static int verbosity = 4;*/

/* file to log diagnostics output to.  NULL = stderr */
/*@null@*/
static FILE *logfile = NULL;

/* flags indicating to which rtf-version output is restricted */
static int rtf_major = 1;
static int rtf_minor = 5;
/* Holds the last referenced "link" value, used by \Ref and \Pageref */
char *hyperref = NULL;
bool pagenumbering = TRUE;        /* by default use plain style */
int headings = FALSE;
bool pagestyledefined = FALSE;    /* flag, set to true by pagestylecommand
				    triggers PlainPagestyle in \begin{document} */

/*SAP Changes to produce count figures and tables separately*/
bool g_processing_figure = FALSE; /* flag, set in figures and not tables */
bool g_processing_include = FALSE; /* flag set when include file being processed */
bool g_processing_eqnarray = FALSE; /* flag set when include file being processed */
int g_equation_number = 0;
bool g_show_equation_number = FALSE;
int g_enumerate_depth = 0;
bool g_suppress_equation_number = FALSE;
bool g_aux_file_missing = FALSE; /* assume that it exists */
/*SAP end fix*/

/****************************************************************************/
/*** function prototypes ***/

/* opens file for reading */
static void PrepareTex( /*@null@*/ const char  *filename
               ,                  FILE **f
	       );
/* creates file and writes RTF header */
static void PrepareRtf( /*@null@*/ char * filename
               ,            FILE **f
	       );
static void CloseTex(FILE **f);
static void CloseRtf(FILE **f);

static bool TranslateCommand();  /* converts commands */
bool TranslateSpecKey();  /* converts special keys */
void *GetCommandFunc(char *cCommand);
bool ConvertFormula(char * command);

enum TexCharSetKind TexCharSet = SEVEN_BIT; /* default SEVEN_BIT for
					       converting special chars */



extern /*@observer@*/ char *optarg;
extern int optind;
extern int getopt(int ac, char *const av[], const char *optstring)
	 /*@modifies optind, optarg@*/
	 ;


/****************************************************************************
purpose: checks parameter and starts convert routine
params:  command line arguments argc, argv
globals: initializes in- and outputfile fTex, fRtf,
 ****************************************************************************/
int
main(int argc, char **argv)
{
  int c;
  bool errflag = FALSE;

  fTex = stdin; /* default input/output */
  fRtf = stdout;
  
  PushEnvironment(HEADER);
  PushEnvironment(DOCUMENT);
  progname=argv[0];
  optind = 1;
  while((c = getopt(argc, argv, "Vlo:a:b:v:i:")) != EOF)
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
	case 'i':
	  language = optarg;
	  break;
	case 'v':
	  {
	    char *endptr;
	    verbosity = (int) strtol(optarg, &endptr, 10);
	    if((verbosity < 0) || verbosity > MAX_VERBOSITY)
	      diagnostics(ERROR, "verbosity %d not between 0 and %d",
			  verbosity, MAX_VERBOSITY);
	    if((*endptr != '\0') || (endptr == optarg))
	      diagnostics(ERROR, "argument to -v option malformed: %s",
			  optarg);
	  }
	  break;
	default:
	  errflag = TRUE;
	}
    }
  if(argc > optind + 1 || errflag)
  {
    fprintf(stderr,"%s: Usage: %s [-V] [-l] [-o outfile]"
	    " [-a auxfile] [-b bblfile] [ -i languagefile ] inputfile\n",
      progname, progname);
    fprintf(stderr,"-l\t Latin-1 (= ISO 8859-1) special characters will be\n");
    fprintf(stderr,"\t converted into RTF-Commands!\n");
    return(1);
  }
  if(argc == optind + 1)
  {
    char *s, *newrtf;
    input = argv[optind];
    latexname = input;

	if((newrtf = malloc(strlen(input) + 5)) == NULL)
	    error(" malloc error -> out of memory!\n");
  	strcpy(newrtf,input);
	if((s = strrchr(newrtf, '.')) == NULL || strcmp(s, ".tex") != 0)
	   strcat(newrtf, ".rtf");
	else
	   strcpy (s, ".rtf");
  	freopen(newrtf, "w", stdout);
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
    {
	if((AuxName = malloc(1)) == NULL)
	    error(" malloc error -> out of memory!\n");
	*AuxName = '\0';
    }
  }

  ReadCfg();
  PrepareTex(input,&fTex);
  WriteTemp(fTex);         /* Normalize eol of inputfile */
  PrepareRtf(output,&fRtf);
  (void)Push(1,0);
  Convert();
  /* if citations were produced, now the reference-list will be created */
  PushEnvironment(DOCUMENT);
  if (bCite)
     WriteRefList();

  CloseTex(&fTex);
  CloseRtf(&fRtf);
  printf("\n");
  return(0);
}


/****************************************************************************/
/* Global Flags for Convert Routine */
/****************************************************************************/
int RecursLevel = 0;
int BracketLevel = 1;
static int ret = 0;
bool mbox = FALSE;
bool g_processing_equation = FALSE;
bool bNewPar = FALSE;
int indent = 0;
bool NoNewLine = FALSE;
static int ConvertFlag;
bool bInDocument = FALSE;
int tabcounter = 0;
int fontsize = 20;
bool twocolumn = FALSE;
bool titlepage = FALSE;
bool article = TRUE;
bool tabbing_on = FALSE;
bool tabbing_return = FALSE;
bool tabbing_on_itself = FALSE;
bool TITLE_AUTHOR_ON = FALSE;
bool g_processing_tabular = FALSE;
bool bBlankLine=TRUE;  /* to handle pseudo-blank lines (contains spaces)
                          correctly					    */
int colCount;     /* number of columns in a tabular environment             */
int actCol;       /* actual column in the tabular environment               */
/*@only@*//*@null@*/ char* colFmt = NULL;
size_t DefFont = 0;  /* contains default font number, which is associated
		     with TeX's font "Roman"                                */



/****************************************************************************/
void Convert()
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
  (void)Push(RecursLevel,BracketLevel);
  ++ConvertFlag;
  while (fread(&cThis, 1,1,fTex) == 1)
  {
    switch(cThis)
    {
    case '\\':{
	      int SaveFlag;
	      fpos_t pos1,pos2;

              bBlankLine=FALSE;
	      if(fgetpos(fRtf,&pos1) != 0)
		diagnostics(ERROR,
			    "Failed fgetpos; main.c (Convert1): errno %d", errno);

	      (void)Push(RecursLevel,BracketLevel);
	      SaveFlag=ConvertFlag;
	      /* For now we ignore the return value of TranslateCommand */
	      (void)TranslateCommand();

	      /*if ((tabbing_on_itself) ||
		  ((tabbing_on) &&
		  (tabbing_return)))
		  {
		  tabbing_return = FALSE;
		  if (tabbing_on_itself)
		     tabbing_on = FALSE;
		  tabbing_on_itself = FALSE;
		  return;
		  } */

	      /* erase double identic values on stack top */
	      for(;;)
	      {
		if ((size = Pop(&PPopLevel, &PPopBrack)) <= 0 )
		{
		  (void)Push(PPopLevel,PPopBrack);
		  break;
		}
		if ((size = Pop(&PopLevel, &PopBrack)) <= 0 )
		{
		  (void)Push(PopLevel,PopBrack);
		  break;
		}
		if ( (PPopLevel == PopLevel) && (PPopBrack == PopBrack) )
		{
		  (void)Push(PopLevel,PopBrack);
		}
		else
		{
		  (void)Push(PopLevel,PopBrack);
		  (void)Push(PPopLevel,PPopBrack);
		  break;
		}
	      };

	      if (ret > 0)
	      {
		--ret;
		--RecursLevel;
		return;
	      }
	      /* remove SaveFlag */
	      /*if (ConvertFlag == SaveFlag)    Convert was not called
	      {
		//cThis = ' ';	 error
	      }
              */
	      cThis = '\\';
	      if(fgetpos(fRtf,&pos2) != 0)
		diagnostics(ERROR,
			    "Failed fgetpos; main.c (Convert2): errno %d", errno);

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
	      (void)Push(RecursLevel,BracketLevel);
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
	      (void)Push(PopLevel,PopBrack);	/* push back */
	      retlevel = PPopLevel;
	      ret = RecursLevel-retlevel;
	      (void)Push(retlevel,BracketLevel);

	      if (ret > 0)
	      {
		ret--;
		RecursLevel--;
		return;
	      }
	      else
		break;

    case ' ': if (!bInDocument) continue;
	      if ( (cLast != ' ') && (cLast != '\n') && (cLast != '\r'))
	      {
		if (!mbox)
	       /*   if (bNewPar == FALSE)*/
		    fprintf(fRtf," ");
		else
		  fprintf(fRtf,"\\~");
	      }
	      break;
    case '~': bBlankLine=FALSE;
              if (!bInDocument) numerror(ERR_NOT_IN_DOCUMENT);
	      fprintf(fRtf,"\\~");
	      break;
    case '\r':
	      fprintf(stderr,"\n%s: ERROR: error in input file: %s at linenumber: %ld\n",progname,latexname,getLinenumber());
	      fprintf(stderr,"\nprogram aborted\n");
	      exit(EXIT_FAILURE);
	      /*@notreached@*/
	      break;
    case '\n':
	      tabcounter=0;
	      linenumber++;
	      if (!bInDocument) continue;

/*SAP Fix to allow blank lines with spaces*/
	       cNext = '\0';
	       while((fread(&cNext,1,1,fTex) == 1) && (cNext==' '));
	       if (cNext != '\0') rewind_one();
/*SAP end fix */
		
	      if (cLast != '\n')
	      {
		if (bNewPar)
		{
                   bNewPar = FALSE;
                   cThis = ' ';
                   break;
                }
		if (cLast!=' ')
		  fprintf(fRtf," ");	 /* treat as 1 space */
                else if (bBlankLine)
		  fprintf(fRtf,"\n\\par\\fi0\\li%d ",indent);
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
              if (!bInDocument) numerror(ERR_NOT_IN_DOCUMENT);

	      fprintf(fRtf,"{\\up6 \\fs20 ");
              if (fTexRead (&cNext,1,1,fTex) == 1)
              {
                 if (cNext == '{')
                 {
	            rewind_one(); /* reread last character */
  	            (void)Push(RecursLevel,BracketLevel);
	            Convert();
	            if (ret > 0)
	            {
		       --ret;
		       --RecursLevel;
	               return;
	            }
	            fprintf(fRtf,"}");
                 }
/* SAP fix ^\alpha */
                 else if (cNext == '\\') 
                 {
	            TranslateCommand();
	            fprintf(fRtf,"}");
                 }
/* SAP end of fix */
                 else
                    fprintf(fRtf,"%c}",cNext);
              }
              else
	         error("Error reading Latex-File");
	      break;
    case '_': bBlankLine=FALSE;
              if (!bInDocument) numerror(ERR_NOT_IN_DOCUMENT);
	      fprintf(fRtf,"{\\dn6 \\fs20 ");
              if (fTexRead (&cNext,1,1,fTex) == 1)
              {
                 if (cNext == '{')
                 {
	            rewind_one(); /* reread last character */
  	            (void)Push(RecursLevel,BracketLevel);
	            Convert();
	            if (ret > 0)
	            {
		       --ret;
		       --RecursLevel;
	               return;
	            }
	            fprintf(fRtf,"}");
                 }
/* SAP fix _\alpha */
                 else if (cNext == '\\') 
                 {
	            TranslateCommand();
	            fprintf(fRtf,"}");
                 }
/* SAP end of fix */
                 else
                    fprintf(fRtf,"%c}",cNext);
              }
              else
	         error("Error reading Latex-File");
	      break;
    case '$': 
             bBlankLine=FALSE;
             if (fTexRead (&cNext,1,1,fTex) == 1)
              {
                 if (cNext == '$')
                 {
                 	CmdFormula2(FORM_DOLLAR);
                 	break;
                 }
	         rewind_one(); /* reread last character */
    	      }
              CmdFormula(FORM_DOLLAR);
	      break;
    case '&': 
    
    	      if (g_processing_tabular && g_processing_equation)   /* in an eqnarray */
	      {
			fprintf(fRtf,"\\tab ");
			break;
	      }

	      if (g_processing_tabular)				   /* in tabular */
	      {
			 fprintf(fRtf," \\cell \\pard \\intbl ");
	                 actCol++;
	                 fprintf (fRtf, "\\q%c ", colFmt[actCol]);
	                 break;
	      }

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
	       rewind_one(); /* reread last character */
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
	       rewind_one(); /* reread last character */
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
	       rewind_one(); /* reread last character */
	       count = 0;
	       break;
    case '\"': bBlankLine=FALSE;
               if (GermanMode)
		 TranslateGerman();
	       else
		 fprintf(fRtf,"\"");
	       break;
//    case '\t':bBlankLine=FALSE;
//              break;
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
		    if(putc (cThis, fRtf) == EOF)
		      diagnostics(ERROR, "Failed putc; main.c (Convert)");
		    rewind_one();
		}
	    }
	}
	break;
    default: bBlankLine=FALSE;
             if (bInDocument)
	     {
	      if ((isupper((unsigned char) cThis)) &&
		 ((cLast=='.') || (cLast=='!') || (cLast=='?') || (cLast==':')))
	  	 fprintf(fRtf," ");

	      if (TexCharSet==ISO_8859_1)
			Write_ISO_8859_1(cThis);
	      else
		    	Write_Default_Charset(cThis);

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
}

/***********************************************************************
 * not zero if major and minor are under the prefixed rtf-version to
 * generate (established by -rm.m option).  Purpose: check if you want
 * to generate the following rtf-tokens or if you have to output a
 * diagnostic message.
 **********************************************************************/
bool
rtf_restrict(int major, int minor)
{
  return((major <= rtf_major) && (minor <= rtf_minor));
}

/****************************************************************************/
void error(char * text)
/****************************************************************************
purpose: writes error message
globals: reads progname;
 ****************************************************************************/
{
  fprintf(stderr,"\nERROR at line %ld: %s",text,getLinenumber());
  fprintf(stderr,"\nprogram aborted\n");
  exit(EXIT_FAILURE);
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
      if (g_processing_include) return;  /*SAP - HACK end of file ok in include files */
      sprintf(text,"%s%s%s%ld%s","unexpected end of input file in: ",latexname," at linenumber: ",getLinenumber(),"\n");
      error(text);
      /*@notreached@*/
      break;
    case ERR_WRONG_COMMAND:
      sprintf(text,"%s%s%s%ld%s","unexpected command or character in: ",latexname," at linenumber: ",getLinenumber(),"\n");
      error(text);
      /*@notreached@*/
      break;
      
    case ERR_NOT_IN_DOCUMENT:
      sprintf(text,"%s%s%s%ld%s","No longer in document ",latexname," at linenumber: ",getLinenumber(),"\nCheck for missing \\begin{document} or the like\n");
      error(text);
      /*@notreached@*/
      break;
    case ERR_Param:
      error("wrong number of parameters\n");
      /*@notreached@*/
      break;
    case ERR_WRONG_COMMAND_IN_TABBING:
      sprintf(text,"%s%s%s%ld%s","wrong command in Tabbing-kill-command-line in: ",latexname," at linenumber: ",getLinenumber(),"\n");
      error(text);
      /*@notreached@*/
      break;
    default :
      error("internal error");
      /*@notreached@*/
      break;
  }
}

/* Writes the given warning message in format, ... if global verbosity
   is higher or equal then level.  If ??? option is given,
   i.e. logfile is not null, everything is logged to the logfile
   -vn Flag (verbosity)	0 ... only errors = -q
                        1 ... (default) Translation Warnings
			2 ... conditions on output e.g. (rtf1.5 options)
			3 ... complete logging of what's going on.
*/
void
diagnostics(int level, char *format, ...)
{
  va_list ap, apf; /*LEG240698 The GNU libc info says that after using
		    the vfprintf function on some systems the ap
		    pointer is destroyed. Well, let's use a second one
		    for safety */
  FILE *errfile;

  /* output always to stderr on level 0 and 1 but observe quiet
     option */
  va_start (ap, format);

  if((level <= 1) && (verbosity != 0)) 
    {
      switch(level)
	{
	case 0: 
	  fprintf(stderr, "\nError! ");
	  break;
	case 1:
	  fprintf(stderr, "\nWarning! ");
	  break;
	default:
	  fprintf(stderr, "\n   ");
	}
      fprintf(stderr, "\n%s\n   line: %ld\n", latexname, getLinenumber());
      vfprintf(stderr, format, ap);
    }

  if(logfile != NULL)
    errfile = logfile;
  else
    errfile = stderr;


  va_start(apf, format);

  if((level <= verbosity) && (level > 1))
    {
      fprintf(errfile, "\n%s:%d: ", latexname, getLinenumber());
      switch(level)
	{
	case 0: 
	  fprintf(errfile, "Error! ");
	  break;
	case 1:
	  fprintf(errfile, "Warning! ");
	  break;
	default:
	  fprintf(errfile, "   ");
	}
      vfprintf(errfile, format, apf);
    }

  va_end(apf);

  if(level == 0)
    {
      fprintf(stderr, "\n");
      exit(EXIT_FAILURE);
    }
}


/****************************************************************************/
void PrepareRtf(char *filename, FILE **f)
/****************************************************************************
purpose: creates output file and writes RTF-header.
params: filename - name of outputfile, possibly NULL for already open file
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
      }
  }
  /* write header */
#ifdef DEFAULT_MAC_ENCODING
  fprintf(*f,"{\\rtf1\\mac\\fs%d\\deff0\\deflang1024\n",fontsize);
#else
  fprintf(*f,"{\\rtf1\\PC\\fs%d\\deff0\\deflang1024\n",fontsize);
#endif
  fprintf(*f,"{\\info{\\version1}}\\widowctrl\\ftnbj\\sectd\\linex0\\endnhere");
  fprintf(*f,"\\qj \n");
  WriteFontHeader(*f);
}


/****************************************************************************/
void PrepareTex(const char *filename, FILE **f)
/****************************************************************************
purpose: opens input file.
params: filename - name of inputfile, possibly NULL
	f - pointer to filepointer to store file ID
 ****************************************************************************/
{
  if(filename != NULL)
  {
      if ((*f = fopen(filename,"r")) == NULL)	/* open file */
      {
	error("Error opening LATEX-file");
      }
  }
}

/*************************************************************************
WriteTemp:    Reads the original inputfile, and normalizes all end-of-lines
              to \n writing it to a temporary file. Then feeds this tempfile
              to l2r.

	      also changes all tabs to a single space
	      
globals:f - pointer to inputfile
*************************************************************************/
void
WriteTemp(FILE *f)
{
  FILE *tout;
  int cnt = 0;
  char cThis;

  tout = tmpfile();
  
  while (fread(&cThis, 1, 1, f) == 1)
  {
      if (cThis =='\r')
      {
	  if (fread(&cThis, 1, 1, f) == 1)
	    {
	      if (cThis == '\n')
		fprintf(tout, "%c", '\n');
	      else
		{
		  fprintf(tout, "%c", '\n');
      		  if (cThis =='\r')
		  	fprintf(tout, "%c", '\n');
	          else if (cThis == '\t')
	      	        fprintf(tout, "%c", ' ');
		  else
		  	fprintf(tout, "%c", cThis);
		}
	    }
      }
      else if (cThis == '\t')
      	fprintf(tout, "%c", ' ');
      else
	fprintf(tout, "%c", cThis);
  }
  
  fclose(f);
  rewind(tout);
  fTex = tout;
}

/****************************************************************************/
void CloseTex(FILE **f)
/****************************************************************************
purpose: closes input file.
params: f - pointer to filepointer to invalidate
 ****************************************************************************/
{
  if(*f != stdin)
      (void)fclose(*f);
  *f = NULL;
}


/****************************************************************************
purpose: closes output file.
params: f - pointer to filepointer to invalidate
globals: progname;
 ****************************************************************************/
void
CloseRtf(FILE **f)
{
  fprintf(*f,"}}\n");
  if(*f != stdout)
  {
      if(fclose(*f) == EOF)
      {
	fprintf(stderr, "%s: Warning: Error closing RTF-File\n", progname);
      }
  }
  *f = NULL;
}


/****************************************************************************/
bool TranslateCommand()
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
  char cThis, cNext;
  char option_string[100];
  
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
	case '\\':    	/* \\[1mm] and \\*[1mm]		/* produces a newline in output */
		      					/* no new paragraph    */

/* skip one * and any spaces that follow */
		if (fread(&cThis,1,1,fTex) == 1)
		{
		     if ((cThis == ' ') || (cThis == '*'))  /* ignore initial * and spaces */
		     {
		        while((fread(&cThis,1,1,fTex) == 1) && (cThis == ' ')); /*space  ignore */
		     }
		     rewind_one();
		}

		GetBracketParam(option_string,99);       /* skip spacing info e.g., \\[1mm] */
		
		if (g_processing_eqnarray) /* eqnarray */
		{
			fprintf(fRtf,"}");   /* close italics */
			if (g_show_equation_number  && !g_suppress_equation_number)
			{
				g_equation_number++;
				fprintf(fRtf,"\\tab (%d)",g_equation_number);
			}
			
			fprintf(fRtf,"\n\\par\\tab {\\i ");
	 		g_suppress_equation_number = FALSE;
			actCol = 1;
		  	return TRUE;
		}
		
		if (g_processing_tabular) /* tabular or array environment */
		{
			if (g_processing_equation) 	/* array */
			{
				fprintf(fRtf,"\n\\par\\tab ");			
				return TRUE;
			}
				
			for (; actCol< colCount; actCol++)
			{
				fprintf (fRtf, "\\cell\\pard\\intbl");
                     	}
			actCol = 1;
			fprintf(fRtf,"\\cell\\pard\\intbl\\row\n\\pard\\intbl\\q%c ", colFmt[1]);
		  	return TRUE;
		}
			

/* simple end of line ... */
		fprintf(fRtf,"\\par ");
		bNewPar = TRUE;
		tabcounter = 0;
		if (tabbing_on)
		{
			 if(fgetpos(fRtf,&pos_begin_kill) != 0)
			    diagnostics(ERROR,
					"Failed fgetpos;"
					" main.c (TranslateCommand): errno %d", errno);
		}
		return TRUE;

	case ' ': fprintf(fRtf," ");	/* ordinary interword space */
		  while (cThis == ' ') /* all spaces after commands are ignored */
		  {
		     if (fread(&cThis,1,1,fTex) < 1)
			numerror(ERR_EOF_INPUT);
		  }

		  rewind_one();
		  return TRUE;
		  /*@notreached@*/
		  break;
	case '-': /* hyphen */
		  /* caution: this character has another effect in the tabbing-environment */
		  if (!tabbing_on)
		     fprintf(fRtf,"\\-");
		  return TRUE;
		  /*@notreached@*/
		  break;
	case '+': /* see tabbing-environment */
		  /* no harm for RTF-output */
		  break;
	case '<': /* see tabbing-environment */
		  /* no harm for RTF-output */
		  break;
	case '~': CmdTildeChar(0);
		  return TRUE;
	case '^': CmdSpitzeChar();
		  return TRUE;
	case '\"':CmdUmlaute();
		  return TRUE;
	case '`': if (!tabbing_on)
		    CmdLApostrophChar(0);
		  return TRUE;
	case '\'':if (!tabbing_on)
		     CmdRApostrophChar(0);  /* char ' =?= \' */
		  return TRUE;
	case '(': CmdFormula(FORM_RND_OPEN);
		  return TRUE;
	case '[': CmdFormula2(FORM_DOLLAR);
		  return TRUE;
	case ')': CmdFormula(FORM_RND_CLOSE);
		  return TRUE;
	case ']': CmdFormula2(FORM_DOLLAR);
		  return TRUE;
	case '/': CmdIgnore(0);
		  return TRUE;
	case ',': CmdIgnore(0);  /* \, produces a small space */
		  return TRUE;
	case '@': CmdIgnore(0);  /* \@ produces an "end of sentence" space */
		  return TRUE;
	case '>': CmdTabjump();
		  return TRUE;
	case '=': CmdTabset();
		  return TRUE;
	case '3': fprintf(fRtf, "{\\ansi\\'df}");   /* german symbol 'á' */
		  return TRUE;
      }
    }
    /*LEG180498 Commands consist of letters and can have an optional * at the end*/
    if (!isalpha((unsigned char) cThis) && (cThis != '*'))
    {
      bool found_nl = FALSE;
      /* all spaces after commands are ignored, a single \n may occur */
//      while (cThis == ' ' || cThis == '\t' || cThis == '\n' && !found_nl)
      while (cThis == ' ' || cThis == '\n' && !found_nl)
	{
	if(cThis == '\n')
	{
           found_nl = TRUE;
        }
	if (fread(&cThis,1,1,fTex) < 1)
	    break;
	}

      rewind_one(); /* position of next character after command
				   except space */
      break;
    }
    cCommand[i] = cThis;
  }

  cCommand[i] = '\0';  /* mark end of string with zero */

  if ( i == 0)
    return FALSE;
  if (CallCommandFunc(cCommand)) /*call handling function for command*/
    return TRUE;
  if (TryDirectConvert(cCommand, fRtf))
    return TRUE;
  if (TryVariableIgnore(cCommand, fTex))
    return TRUE;
  fprintf(stderr,"\n%s: WARNING: command: %s not found - ignored\n",progname,cCommand);
  return FALSE;
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
	rewind_one();   /*linenumber is set in convert-routine */
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



/****************************************************************************
purpose: get number of actual line (do not use global linenumber, because
         it does not work correctly!)
         this function is not very efficient, but it will be used only 
	 once when
         printing an error-message + program abort
params:  none
 ****************************************************************************/
int 
getLinenumber (void)
{
  char buffer[1024];
  fpos_t oldpos;
  fpos_t pos;
  int linenum = 0;

  if (fTex == stdin) return 0;

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

char *EmptyString(void)
{
    char *s = malloc(1);
    if (s == NULL)
    {
	error(" malloc error -> out of memory!\n");
    }
    *s = '\0';
    return s;
}
