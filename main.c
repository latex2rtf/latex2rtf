/*
 * $Id: main.c,v 1.11 2001/08/12 19:40:25 prahl Exp $
 * History:
 * $Log: main.c,v $
 * Revision 1.11  2001/08/12 19:40:25  prahl
 * 1.9g
 *         Added commands to read and set TeX lengths
 *         Added commands to read and set TeX counters
 *         Fixed bug in handling of \item[text]
 *         Eliminated comparison of fpos_t variables
 *         Revised getLinenumber ... this is not perfect
 *         Fixed bug in getTexChar() routine
 *         Clearly separate preamble from the document in hopes that
 *           one day more appropriate values for page size, margins,
 *           paragraph spacing etc, will be used in the RTF header
 *         I have added initial support for page sizes still needs testing
 *         added two more test files misc3.tex and misc4.tex
 *         misc4.tex produces a bad rtf file currently
 *         separated all letter commands into letterformat.c
 *         cleaned up warning calls throughout code
 *         added \neq \leq \geq \mid commands to direct.cfg
 *         collected and added commands to write RTF header in preamble.c
 *         broke isolatin1 and hyperlatex support, these will be fixed next version
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
#include "chars.h"
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
#include "parser.h"
#include "lengths.h"
#include "counters.h"
#include "preamble.h"

#ifdef __MWERKS__
#include "MainMain.h"
#endif

/********************************* global variables *************************/
long            linenumber = 1;	/* lines in the LaTex-document */
 /* @null@ *//* @observer@ */ char *currfile;	/* current file name */
FILE           *fTex = (FILE *) NULL;	/* file pointer to Latex file */
FILE           *fRtf = (FILE *) NULL;	/* file pointer to RTF file */
 /* @null@ *//* @observer@ */ char *input = NULL;
static				/* @null@ */
 /* @observer@ */ char *output = NULL;
char           *AuxName = NULL;
char           *BblName = NULL;

 /* @observer@ */ char *progname;	/* name of the executable file */
char           *latexname = "stdin";	/* name of LaTex-File */
char            alignment = JUSTIFIED;	/* default for justified: */
fpos_t          pos_begin_kill;
bool            bCite = FALSE;	/* to produce citations */
bool            GermanMode = FALSE;	/* switches support for germanstyle
					 * on or off */
/*
 * the Germand Mode supports most of the commands defined in GERMAN.STY file
 * by H.Partl(TU Wien) 87-06-17
 */

char           *language = "english";	/* in \begin{document} "language".cfg
					 * is read in */
bool            twoside = FALSE;
/* verbosity of diagnostics output. */
static int      verbosity = WARNING;
/* static int verbosity = 4; */

/* file to log diagnostics output to.  NULL = stderr */
/* @null@ */
static FILE    *logfile = NULL;

/* flags indicating to which rtf-version output is restricted */
static int      rtf_major = 1;
static int      rtf_minor = 5;

/* Holds the last referenced "link" value, used by \Ref and \Pageref */
char           *hyperref = NULL;
bool            pagenumbering = TRUE;	/* by default use plain style */
int             headings = FALSE;
bool            pagestyledefined = FALSE;	/* flag, set to true by
						 * pagestylecommand triggers
						 * PlainPagestyle in
						 * \begin{document} */

bool            g_processing_figure = FALSE;	/* flag, set for figures and not tables */
bool            g_processing_include = FALSE;	/* flag set when include file is being processed */
bool            g_processing_eqnarray = FALSE;	/* flag set when in an eqnarry */

bool            g_show_equation_number = FALSE;
int             g_enumerate_depth = 0;
bool            g_suppress_equation_number = FALSE;
bool            g_aux_file_missing = FALSE;	/* assume that it exists */

/* present in preamble, inserted when \maketitle encountered */
char           *g_author = NULL;
char           *g_title  = NULL;
char           *g_date   = NULL;

int             curr_fontbold[MAXENVIRONS] = {0};
int             curr_fontital[MAXENVIRONS] = {0};
int             curr_fontscap[MAXENVIRONS] = {0};
int             curr_fontnumb[MAXENVIRONS] = {0};

static void     PrepareTex(const char *filename ,FILE ** f);
static void     PrepareRtf(char *filename, FILE ** f);
static void     CloseTex(FILE ** f);
static void     CloseRtf(FILE ** f);
static void     ConvertLatexPreamble(void);
static void     InitializeLatexLengths(void);

static bool     TranslateCommand();	/* converts commands */
bool            TranslateSpecKey();	/* converts special keys */
void           *GetCommandFunc(char *cCommand);

enum TexCharSetKind TexCharSet = SEVEN_BIT;	/* default SEVEN_BIT for converting special chars */



extern /* @observer@ */ char *optarg;
extern int      optind;
extern int getopt(int ac, char *const av[], const char *optstring);

/****************************************************************************
purpose: checks parameter and starts convert routine
params:  command line arguments argc, argv
globals: initializes in- and outputfile fTex, fRtf,
 ****************************************************************************/
	int
	                main(int argc, char **argv)
{
	int             c;
	bool            errflag = FALSE;

	fTex = stdin;		/* default input/output */
	fRtf = stdout;

	SetDocumentFontSize(20);
	PushEnvironment(HEADER);
	PushEnvironment(DOCUMENT);
	progname = argv[0];
	optind = 1;
	while ((c = getopt(argc, argv, "Vlo:a:b:v:i:")) != EOF) {
		switch (c) {
		case 'V':
			printf("%s: %s\n", progname, Version);
			return (0);
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
			fprintf(stderr, "Latin-1 (= ISO 8859-1) special characters will be ");
			fprintf(stderr, "converted into RTF-Commands!\n");
			break;
		case 'i':
			language = optarg;
			break;
		case 'v':
			{
				char           *endptr;
				verbosity = (int) strtol(optarg, &endptr, 10);
				if ((verbosity < 0) || verbosity > MAX_VERBOSITY)
					diagnostics(ERROR, "verbosity %d not between 0 and %d",
						  verbosity, MAX_VERBOSITY);
				if ((*endptr != '\0') || (endptr == optarg))
					diagnostics(ERROR, "argument to -v option malformed: %s",
						    optarg);
			}
			break;
		default:
			errflag = TRUE;
		}
	}

	if (argc > optind + 1 || errflag) {
		fprintf(stderr, "%s: Usage: %s [-V] [-l] [-o outfile]"
		" [-a auxfile] [-b bblfile] [ -i languagefile ] inputfile\n",
			progname, progname);
		fprintf(stderr, "-l\t Latin-1 (= ISO 8859-1) special characters will be\n");
		fprintf(stderr, "\t converted into RTF-Commands!\n");
		return (1);
	}
	if (argc == optind + 1) {
		char           *s, *newrtf;

		/* if filename does not end in .tex then append .tex */
		if ((s = strrchr(argv[optind], PATHSEP)) == NULL)
			s = argv[optind];

		if ((s = strrchr(s, '.')) == NULL) {
			if ((input = malloc(strlen(argv[optind]) + 4)) == NULL)
				error(" malloc error -> out of memory!\n");
			strcpy(input, argv[optind]);
			strcat(input, ".tex");
		} else if (strcmp(s, ".tex"))
			error("latex files must end with .tex");
		else
			input = argv[optind];

		latexname = input;
		diagnostics(3, "latex filename is %s\n", input);

		/* create the .rtf filename */
		if ((newrtf = malloc(strlen(input) + 5)) == NULL)
			error(" malloc error -> out of memory!\n");
		strcpy(newrtf, input);
		if ((s = strrchr(newrtf, '.')) == NULL || strcmp(s, ".tex") != 0)
			strcat(newrtf, ".rtf");
		else
			strcpy(s, ".rtf");
		output = newrtf;
		diagnostics(3, "rtf filename is %s\n", output);
	}
	
	if (AuxName == NULL) {
		char           *s;
		if (input != NULL) {
			if ((AuxName = malloc(strlen(input) + 5)) == NULL)
				error(" malloc error -> out of memory!\n");
			strcpy(AuxName, input);
			if ((s = strrchr(AuxName, '.')) == NULL || strcmp(s, ".tex") != 0)
				strcat(AuxName, ".aux");
			else
				strcpy(s, ".aux");
		} else {
			if ((AuxName = malloc(1)) == NULL)
				error(" malloc error -> out of memory!\n");
			*AuxName = '\0';
		}
	}
	ReadCfg();
	PrepareTex(input, &fTex);
	PrepareRtf(output, &fRtf);

	Push(1, 0);
	InitializeLatexLengths();
	ConvertLatexPreamble();
	WriteRtfHeader();
	
	diagnostics(4,"Entering Convert from main");
	Convert();
	diagnostics(4,"Exiting Convert from main");

	if (bCite){
		diagnostics(4,"Starting bibliography");
		PushEnvironment(DOCUMENT);
		WriteRefList();
	}

	CloseTex(&fTex);
	CloseRtf(&fRtf);
	printf("\n");
	return (0);
}


/****************************************************************************/
/* Global Flags for Convert Routine */
/****************************************************************************/
int             RecursLevel = 0;
int             BracketLevel = 1;
static int      ret = 0;
bool            mbox = FALSE;
bool            g_processing_equation = FALSE;
bool            bNewPar = FALSE;
int             indent = 0;
bool            NoNewLine = FALSE;
static int      ConvertFlag;
bool            bInDocument = FALSE;
int             tabcounter = 0;
bool            twocolumn = FALSE;
bool            titlepage = FALSE;
bool            article = TRUE;
bool            tabbing_on = FALSE;
bool            tabbing_return = FALSE;
bool            tabbing_on_itself = FALSE;
bool            TITLE_AUTHOR_ON = FALSE;
bool            g_processing_tabular = FALSE;
bool            g_preprocessing = FALSE;
bool            bBlankLine = TRUE;	/* handle pseudo-blank lines (with spaces) correctly */
int             colCount;			/* number of columns in a tabular environment */
int             actCol;				/* actual column in the tabular environment */
char           *colFmt = NULL;
int             DefFont = 0;		/* contains default TeX Roman font number*/

void 
Convert()
/****************************************************************************
purpose: convertes inputfile and writes result to outputfile
globals: fTex, fRtf and all global flags for convert (see above)
 ****************************************************************************/
{
	char            cThis = '\n';
	char            cLast = '\n';
	char            cLast2 = '\n';
	char            cNext;
	int             retlevel;
	int             count = 0;
	int             i;

	RecursLevel++;
	(void) Push(RecursLevel, BracketLevel);
	++ConvertFlag;
	while ((cThis = getTexChar()) && cThis != EOF && cThis != '\0') {
		diagnostics(5, "Current character in Convert() is %c", cThis);
		switch (cThis) {

		case '\\':
/*				fpos_t          pos1, pos2;

				bBlankLine = FALSE;
				if (fgetpos(fRtf, &pos1) != 0)
					diagnostics(ERROR, "Problem accessing rtf file");
*/
			(void) Push(RecursLevel, BracketLevel);
			
			/* For now we ignore the return value of TranslateCommand */
			(void) TranslateCommand();

			/* erase multiple identical values on top of stack */
			CleanStack();

			if (ret > 0) {
				--ret;
				--RecursLevel;
				return;
			}

/*				cThis = '\\';
				if (fgetpos(fRtf, &pos2) != 0)
					diagnostics(ERROR, "Problem accessing rtf file");

				if (pos1 == pos2) {
					if (cLast == '\n')
						cThis = '\n';
					else
						cThis = ' ';
				}
*/
				break;
			
			
		case '%':
			diagnostics(WARNING, "Ignoring %% in %s at line %ld\n", latexname, getLinenumber());
			cThis = ' ';
			break;
			
		case '{':
			bBlankLine = FALSE;
			(void) Push(RecursLevel, BracketLevel);
			++BracketLevel;
			break;
			
		case '}':
			bBlankLine = FALSE;
			BracketLevel--;
			retlevel = getStackRecursionLevel();
			ret = RecursLevel - retlevel;
			(void) Push(retlevel, BracketLevel);

			if (ret > 0) {
				ret--;
				RecursLevel--;
				return;
			} else
				break;

		case ' ':
			if (!bInDocument)
				continue;
				
			if ((cLast != ' ') && (cLast != '\n') ) {
				if (!mbox)
					/* if (bNewPar == FALSE) */
					fprintf(fRtf, " ");
				else
					fprintf(fRtf, "\\~");
			}
			break;
			
		case '~':
			bBlankLine = FALSE;
			if (!bInDocument)
				numerror(ERR_NOT_IN_DOCUMENT);
			fprintf(fRtf, "\\~");
			break;
			
		case '\r':
			diagnostics(WARNING, "Ignoring \\r in %s at line %ld", latexname, getLinenumber());
			cThis = ' ';
			break;
			
		case '\n':
			tabcounter = 0;
			if (!bInDocument)
				continue;

			while ((cNext = getTexChar()) == ' ');	/* blank line with
								 * spaces */
			ungetTexChar(cNext);

			if (cLast != '\n') {
				if (bNewPar) {
					bNewPar = FALSE;
					cThis = ' ';
					break;
				}
				if (cLast != ' ')
					fprintf(fRtf, " ");	/* treat as 1 space */
				else if (bBlankLine)
					fprintf(fRtf, "\n\\par\\fi0\\li%d ", indent);
			} else {
				if (cLast2 != '\n') {
					fprintf(fRtf, "\n\\par\\fi0\\li%d ", indent);
				}
			}
			bBlankLine = TRUE;
			break;

		case '^':
			bBlankLine = FALSE;
			if (!bInDocument)
				numerror(ERR_NOT_IN_DOCUMENT);

			{
				char           *s = NULL;
				if ((s = getMathParam())) {
					fprintf(fRtf, "{\\up6 \\fs20 ");
					ConvertString(s);
					fprintf(fRtf, "}");
					free(s);
				}
			}
			break;

		case '_':
			bBlankLine = FALSE;
			if (!bInDocument)
				numerror(ERR_NOT_IN_DOCUMENT);
			{
				char           *s = NULL;
				if ((s = getMathParam())) {
					fprintf(fRtf, "{\\dn6 \\fs20 ");
					ConvertString(s);
					fprintf(fRtf, "}");
					free(s);
				}
			}
			break;

		case '$':
			bBlankLine = FALSE;
			cNext = getTexChar();
			if (cNext == '$')	/* check for $$ */
				CmdFormula2(FORM_DOLLAR);
			else {
				ungetTexChar(cNext);
				CmdFormula(FORM_DOLLAR);
			}
			break;

		case '&':
			if (g_processing_tabular && g_processing_equation) {	/* in an eqnarray */
				fprintf(fRtf, "\\tab ");
				break;
			}
			if (g_processing_tabular) {	/* in tabular */
				fprintf(fRtf, " \\cell \\pard \\intbl ");
				actCol++;
				fprintf(fRtf, "\\q%c ", colFmt[actCol]);
				break;
			}
			fprintf(fRtf, "&");
			break;

		case '-':
			bBlankLine = FALSE;
			count++;
			while ((cNext = getTexChar()) && cNext == '-')
				count++;
			ungetTexChar(cNext);	/* reread last character */
			switch (count) {
			case 1:
				fprintf(fRtf, "-");
				break;
			case 2:
				fprintf(fRtf, "\\endash ");
				break;
			case 3:
				fprintf(fRtf, "\\emdash ");
				break;
			default:
				{
					for (i = count - 1; i >= 0; i--)
						fprintf(fRtf, "-");
				}
			}
			count = 0;
			break;
			
		case '\'':
			bBlankLine = FALSE;
			count++;
			while ((cNext = getTexChar()) && cNext == '\'')
				count++;
			ungetTexChar(cNext);
			if (count != 2) {
				for (i = count - 1; i >= 0; i--)
					fprintf(fRtf, "\\rquote ");
			} else
				fprintf(fRtf, "\\rdblquote ");
			count = 0;
			break;
			
		case '`':
			bBlankLine = FALSE;
			count++;
			while ((cNext = getTexChar()) && cNext == '`')
				count++;
			ungetTexChar(cNext);
			if (count != 2) {
				for (i = count - 1; i >= 0; i--)
					fprintf(fRtf, "\\lquote ");
			} else
				fprintf(fRtf, "\\ldblquote ");
			count = 0;
			break;
			
		case '\"':
			bBlankLine = FALSE;
			if (GermanMode)
				TranslateGerman();
			else
				fprintf(fRtf, "\"");
			break;

		case '?':
		case '!':
			{
				char            ch;
				bBlankLine = FALSE;
				if ((ch = getTexChar()) && ch == '`') {
					if (cThis == '?')
						fprintf(fRtf, "{\\ansi\\'bf}");
					else
						fprintf(fRtf, "{\\ansi\\'a1}");
				} else {
						fprintf(fRtf, "%c", cThis);
						ungetTexChar(ch);
				}
			}
			break;
			
		default:
			bBlankLine = FALSE;
			if (bInDocument) {
				if (isupper(cThis) && ((cLast == '.') || (cLast == '!') || (cLast == '?') || (cLast == ':')))
					fprintf(fRtf, " ");

				if (TexCharSet == ISO_8859_1)
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
	return ((major <= rtf_major) && (minor <= rtf_minor));
}

void 
error(char *text)
/****************************************************************************
purpose: writes error message
globals: reads progname;
 ****************************************************************************/
{
	fprintf(stderr, "\nERROR at line %ld: %s", getLinenumber(), text);
	fprintf(stderr, "\nprogram aborted\n");
	exit(EXIT_FAILURE);
}

void 
numerror(int num)
/****************************************************************************
purpose: writes error message identified by number - for messages on many
	 places in code. Calls function error.
globals: progname; latexname; linenumber;
 ****************************************************************************/
{

	char            text[1024];

	switch (num) {
	case ERR_EOF_INPUT:
		if (g_processing_include || g_preprocessing)
			return;	/* SAP - HACK end of file ok in include files */
		sprintf(text, "%s%s%s%ld%s", "unexpected end of input file in: ", latexname, " at linenumber: ", getLinenumber(), "\n");
		error(text);
		/* @notreached@ */
		break;
	case ERR_WRONG_COMMAND:
		sprintf(text, "%s%s%s%ld%s", "unexpected command or character in: ", latexname, " at linenumber: ", getLinenumber(), "\n");
		error(text);
		/* @notreached@ */
		break;

	case ERR_NOT_IN_DOCUMENT:
		sprintf(text, "%s%s%s%ld%s", "No longer in document ", latexname, " at linenumber: ", getLinenumber(), "\nCheck for missing \\begin{document} or the like\n");
		error(text);
		/* @notreached@ */
		break;
	case ERR_Param:
		error("wrong number of parameters\n");
		/* @notreached@ */
		break;
	case ERR_WRONG_COMMAND_IN_TABBING:
		sprintf(text, "%s%s%s%ld%s", "wrong command in Tabbing-kill-command-line in: ", latexname, " at linenumber: ", getLinenumber(), "\n");
		error(text);
		/* @notreached@ */
		break;
	default:
		error("internal error");
		/* @notreached@ */
		break;
	}
}

/*
 * Writes the given warning message in format, ... if global verbosity is
 * higher or equal then level.  If ??? option is given, i.e. logfile is not
 * null, everything is logged to the logfile -vn Flag (verbosity)	0 ...
 * only errors = -q 1 ... (default) Translation Warnings 2 ... conditions on
 * output e.g. (rtf1.5 options) 3 ... complete logging of what's going on.
 */
void
diagnostics(int level, char *format,...)
{
	va_list         ap, apf;/* LEG240698 The GNU libc info says that
				 * after using the vfprintf function on some
				 * systems the ap pointer is destroyed. Well,
				 * let's use a second one for safety */
	FILE           *errfile;

	/*
	 * output always to stderr on level 0 and 1 but observe quiet option
	 */
	va_start(ap, format);

	if ((level <= 1) && (verbosity != 0)) {
		switch (level) {
		case 0:
			fprintf(stderr, "\nError! ");
			break;
		case 1:
			fprintf(stderr, "\nWarning! ");
			break;
		default:
			fprintf(stderr, "\n   ");
		}
		fprintf(stderr, "%s (%ld) ", input, getLinenumber());
		
		vfprintf(stderr, format, ap);
	}
	if (logfile != NULL)
		errfile = logfile;
	else
		errfile = stderr;


	va_start(apf, format);

	if ((level <= verbosity) && (level > 1)) {
		int iEnvCount=CurrentEnvironmentCount();
		fprintf(stderr, "\n%s %4ld=%4ld bra=%d rec=%d env=%d ", input, getLinenumber(), linenumber, BracketLevel, RecursLevel, iEnvCount);
		//fprintf(errfile, "\n%s:%ld: ", latexname, getLinenumber());
		switch (level) {
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

	if (level == 0) {
		fprintf(stderr, "\n");
		exit(EXIT_FAILURE);
	}
}

static void
InitializeLatexLengths(void)
{
	
	setLength("textheight", 550*20);
	setLength("pageheight", 795*20);
	setLength("headheight",  12*20);
	setLength("voffset",      0*20);
	setLength("footskip",    30*20);
	setLength("topmargin",   16*20);
	setLength("headsep",     25*20);	
	setLength("textwidth",  345*20);
	setLength("pagewidth",  614*20);
	setLength("textwidth",  345*20);
	setLength("hoffset",      0*20);
	setLength("marginparsep",11*20);

	setLength("baselineskip",11*20);
	setLength("parindent",   11*20);
	setLength("parskip",     11*20);
	setLength("evensidemargin",     11*20);
	setLength("oddsidemargin",     11*20);
				
	setCounter("page",          0);
	setCounter("chapter",       0);
	setCounter("section",       0);
	setCounter("subsection",    0);
	setCounter("subsubsection", 0);
	setCounter("paragraph",     0);
	setCounter("subparagraph",  0);
	setCounter("figure",        0);
	setCounter("table",         0);
	setCounter("equation",      0);
	setCounter("footnote",      0);
	setCounter("mpfootnote",    0);
 }


static void 
ConvertLatexPreamble(void)
/****************************************************************************
purpose: reads the LaTeX preamble (to \begin{document} ) for the file
 ****************************************************************************/
{
	FILE * hidden;
	char * s;
	
	diagnostics(4, "Reading LaTeX Preamble");
	hidden = fRtf;
	fRtf = stderr;
	 
	s = getTexUntil("\\begin{document}");
	
	diagnostics(4, "Entering ConvertString() from ConvertLatexPreamble <%s>",s);
	ConvertString(s);
	diagnostics(4, "Exiting ConvertString() from ConvertLatexPreamble");
	
	fRtf = hidden;
	free(s);
}


void 
PrepareRtf(char *filename, FILE ** f)
/****************************************************************************
purpose: creates output file and writes RTF-header.
params: filename - name of outputfile, possibly NULL for already open file
	f - pointer to filepointer to store file ID
 ****************************************************************************/
{
	diagnostics(4, "Opening RTF file %s", filename);
	if (filename != NULL) {
		/*
		 * I have replaced "wb" with "w" in the following fopen, for
		 * correct operation under MS DOS (with the -o option). I
		 * believe this should make no difference under UNIX.
		 * --V.Menkov
		 */

		if ((*f = fopen(filename, "w")) == NULL) {	/* open file */
			fprintf(stderr, "Error opening RTF file %s\n", filename);
			exit(EXIT_FAILURE);
		}
	}
}


void 
PrepareTex(const char *filename, FILE ** f)
/****************************************************************************
purpose: opens input file.
params: filename - name of inputfile, possibly NULL
	f - pointer to filepointer to store file ID
 ****************************************************************************/
{
	if (filename != NULL) {
		if ((*f = fopen(filename, "r")) == NULL) {	/* open file */
			fprintf(stderr, "Error opening LaTeX file %s\n", filename);
			exit(EXIT_FAILURE);
		}
	}

	diagnostics(4,"Opened LaTeX file %s, (%p)",filename, *f);

}

void 
CloseTex(FILE ** f)
/****************************************************************************
purpose: closes input file.
params: f - pointer to filepointer to invalidate
 ****************************************************************************/
{
	if (*f != stdin  && *f != NULL)
		fclose(*f);
	*f = NULL;
	diagnostics(4,"Closed LaTeX file");
}


void
CloseRtf(FILE ** f)
/****************************************************************************
purpose: closes output file.
params: f - pointer to filepointer to invalidate
globals: progname;
 ****************************************************************************/
{
	fprintf(*f, "}}\n");
	if (*f != stdout) {
		if (fclose(*f) == EOF) {
			diagnostics(WARNING, "Error closing RTF-File");
		}
	}
	*f = NULL;
	diagnostics(4,"Closed RTF file");
}


bool 
TranslateCommand()
/****************************************************************************
purpose: The function is called on a backslash in input file and
	 tries to call the command-function for the following command.
returns: sucess or not
globals: fTex, fRtf, command-functions have side effects or recursive calls;
         global flags for convert
 ****************************************************************************/
{
	char            cCommand[MAXCOMMANDLEN];
	int             i;
	char            cThis;
	char            option_string[100];

	diagnostics(5, "Beginning TranslateCommand()");

	cThis = getTexChar();

	switch (cThis) {
	case '}':
		fprintf(fRtf, "\\}");
		return TRUE;
	case '{':
		fprintf(fRtf, "\\{");
		return TRUE;
	case '#':
		fprintf(fRtf, "#");
		return TRUE;
	case '$':
		fprintf(fRtf, "$");
		return TRUE;
	case '&':
		fprintf(fRtf, "&");
		return TRUE;
	case '%':
		fprintf(fRtf, "%%");
		return TRUE;
	case '_':
		fprintf(fRtf, "_");
		return TRUE;
		
	case '\\':		/* \\[1mm] or \\*[1mm] possible */

		if ((cThis = getTexChar()) != '*')
			ungetTexChar(cThis);

		getBracketParam(option_string, 99);	

		if (g_processing_eqnarray) {	/* eqnarray */
			fprintf(fRtf, "}");	/* close italics */
			if (g_show_equation_number && !g_suppress_equation_number) {
				incrementCounter("equation");
				fprintf(fRtf, "\\tab (%d)", getCounter("equation"));
			}
			fprintf(fRtf, "\n\\par\\tab {\\i ");
			g_suppress_equation_number = FALSE;
			actCol = 1;
			return TRUE;
		}
		
		if (g_processing_tabular) {	/* tabular or array environment */
			if (g_processing_equation) {	/* array */
				fprintf(fRtf, "\n\\par\\tab ");
				return TRUE;
			}
			for (; actCol < colCount; actCol++) {
				fprintf(fRtf, "\\cell\\pard\\intbl");
			}
			actCol = 1;
			fprintf(fRtf, "\\cell\\pard\\intbl\\row\n\\pard\\intbl\\q%c ", colFmt[1]);
			return TRUE;
		}

		if (tabbing_on){
			ConvertString("}");
			ConvertString("{");
		}

		/* simple end of line ... */
		fprintf(fRtf, "\\par ");
		bNewPar = TRUE;
		tabcounter = 0;
		if (tabbing_on && (fgetpos(fRtf, &pos_begin_kill) != 0))
				diagnostics(ERROR, "File access problem in tabbing environment");
		return TRUE;

	case ' ':
		fprintf(fRtf, " ");	/* ordinary interword space */
		skipSpaces();
		return TRUE;

/* \= \> \< \+ \- \' \` all have different meanings in a tabbing environment */

	case '-':
		if (tabbing_on){
			ConvertString("}");
			ConvertString("{");
		} else
			fprintf(fRtf, "\\-");
		return TRUE;

	case '+':
		if (tabbing_on){
			ConvertString("}");
			ConvertString("{");
		}
		return TRUE;
		
	case '<':
		if (tabbing_on){
			ConvertString("}");
			ConvertString("{");
		}
		return TRUE;

	case '>':
		if (tabbing_on){
			ConvertString("}");
			CmdTabjump();
			ConvertString("{");
		}
		return TRUE;
		
	case '`':
		if (tabbing_on){
			ConvertString("}");
			ConvertString("{");
		} else
			CmdLApostrophChar(0);
		return TRUE;
		
	case '\'':
		if (tabbing_on){
			ConvertString("}");
			ConvertString("{");
			return TRUE;
		} else
			CmdRApostrophChar(0);	/* char ' =?= \' */
		return TRUE;

	case '=':
		if (tabbing_on){
			ConvertString("}");
			CmdTabset();
			ConvertString("{");
		}
		else
			CmdMacronChar(0);
		return TRUE;
		
	case '~':
		CmdTildeChar(0);
		return TRUE;
	case '^':
		CmdHatChar(0);
		return TRUE;
	case '.':
		CmdDotChar(0);
		return TRUE;
	case '\"':
		CmdUmlauteChar(0);
		return TRUE;
	case '(':
		CmdFormula(FORM_RND_OPEN);
		return TRUE;
	case '[':
		CmdFormula2(FORM_DOLLAR);
		return TRUE;
	case ')':
		CmdFormula(FORM_RND_CLOSE);
		return TRUE;
	case ']':
		CmdFormula2(FORM_DOLLAR);
		return TRUE;
	case '/':
		CmdIgnore(0);
		return TRUE;
	case ',':
		CmdIgnore(0);	/* \, produces a small space */
		return TRUE;
	case '@':
		CmdIgnore(0);	/* \@ produces an "end of sentence" space */
		return TRUE;
	case '3':
		fprintf(fRtf, "{\\ansi\\'df}");	/* german symbol '�' */
		return TRUE;
	}


	/*
	 * LEG180498 Commands consist of letters and can have an optional *
	 * at the end
	 */
	for (i = 0; i < MAXCOMMANDLEN; i++) {
		if (!isalpha(cThis) && (cThis != '*')) {
			bool            found_nl = FALSE;
			/*
			 * all spaces after commands are ignored, a single \n
			 * may occur
			 */
			while (cThis == ' ' || (cThis == '\n' && !found_nl)) {
				if (cThis == '\n')
					found_nl = TRUE;
				cThis = getTexChar();
			}

			ungetTexChar(cThis);	/* position of next character
						 * after command and optional
						 * space */
			break;
		} else
			cCommand[i] = cThis;

		cThis = getTexChar();
	}

	cCommand[i] = '\0';	/* mark end of string with zero */
	diagnostics(5, "TranslateCommand() <%s>", cCommand);

	if (i == 0)
		return FALSE;
	if (CallCommandFunc(cCommand))	/* call handling function for command */
		return TRUE;
	if (TryDirectConvert(cCommand, fRtf))
		return TRUE;
	if (TryVariableIgnore(cCommand, fTex))
		return TRUE;
	diagnostics(WARNING, "Command \\%s not found - ignored", cCommand);
	return FALSE;
}


/****************************************************************************
purpose: get number of actual line (do not use global linenumber, because
         it does not work correctly!)
         this function is not very efficient, but it will be used only
	 once when
         printing an error-message + program abort
params:  none
 ****************************************************************************/
long
getLinenumber(void)
{
	char            buffer[1024];
	long            oldpos;
	long            pos;
	int             linenum = 0;

	if (fTex == NULL || fTex == stdin)
		return 0;

	oldpos=ftell(fTex);
	if (oldpos == -1)
		error("ftell: can\'t get linenumber");
	if (fseek(fTex, 0L, SEEK_SET) == -1)
		error("fseek: can\'t get linenumber");

	do {
		if (fgets(buffer, 1023, fTex) == NULL)
			error("fgets: can\'t get linenumber");
		linenum++;
		pos = ftell(fTex);
		if (pos == -1)
			error("ftell: can\'t get linenumber");
	}
	while (pos < oldpos);

	if (fseek(fTex, oldpos, SEEK_SET) == -1)
		error("fseek: can\'t get linenumber");

	return linenum;
}

char           *
EmptyString(void)
{
	char           *s = malloc(1);
	if (s == NULL) {
		error(" malloc error -> out of memory!\n");
	}
	*s = '\0';
	return s;
}
