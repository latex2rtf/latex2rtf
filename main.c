/* $Id: main.c,v 1.27 2001/10/12 05:45:07 prahl Exp $ */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include "main.h"
#include "convert.h"
#include "commands.h"
#include "chars.h"
#include "l2r_fonts.h"
#include "stack.h"
#include "direct.h"
#include "ignore.h"
#include "version.h"
#include "funct1.h"
#include "cfg.h"
#include "encode.h"
#include "parser.h"
#include "lengths.h"
#include "counters.h"
#include "preamble.h"
#include "biblio.h"

long           linenumber = 1;	        /* lines in the LaTex-document */
char           *currfile;	            /* current file name */
FILE           *fTex = (FILE *) NULL;	/* file pointer to Latex file */
FILE           *fRtf = (FILE *) NULL;	/* file pointer to RTF file */
char           *input = NULL;
static char    *output = NULL;
char           *AuxName = NULL;
char           *BblName = NULL;

char           *progname;	            /* name of the executable file */
char           *latexname = "stdin";	/* name of LaTex-File */
bool            GermanMode = FALSE;	    /* support germanstyle */

char            g_language[20] = "english";	/* before \begin{document} "g_language".cfg is read in */
char            g_encoding[20] = "cp1252";
bool            twoside = FALSE;
int      		g_verbosity_level = WARNING;
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

bool            g_processing_preamble = TRUE;	/* flag set until \begin{document} */
bool            g_processing_figure = FALSE;	/* flag, set for figures and not tables */
bool            g_processing_include = FALSE;	/* flag set when include file is being processed */
bool            g_processing_eqnarray = FALSE;	/* flag set when in an eqnarry */

bool            g_show_equation_number = FALSE;
int             g_enumerate_depth = 0;
bool            g_suppress_equation_number = FALSE;
bool            g_aux_file_missing = FALSE;	/* assume that it exists */

bool            g_processing_equation = FALSE;
bool            g_document_type = FORMAT_ARTICLE;
bool            g_processing_tabular = FALSE;

int             indent = 0;
char            alignment = JUSTIFIED;	/* default for justified: */

int             RecursionLevel = 0;
int             tabcounter = 0;
bool            twocolumn = FALSE;
bool            titlepage = FALSE;
bool            tabbing_on = FALSE;
bool            tabbing_return = FALSE;
bool            tabbing_on_itself = FALSE;
long          pos_begin_kill;

int             colCount;			/* number of columns in a tabular environment */
int             actCol;				/* actual column in the tabular environment */
char           *colFmt = NULL;

static void     OpenTexFile(const char *filename ,FILE ** f);
static void     OpenRtfFile(char *filename, FILE ** f);
static void     CloseTex(FILE ** f);
static void     CloseRtf(FILE ** f);
static void     ConvertLatexPreamble(void);
static void     InitializeLatexLengths(void);

void           *GetCommandFunc(char *cCommand);

extern char *optarg;
extern int   optind;
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
			setPackageInputenc("latin1");
			break;
		case 'i':
			setPackageBabel(optarg);
			break;
		case 'v':
			{
				char           *endptr;
				g_verbosity_level = (int) strtol(optarg, &endptr, 10);
				if (g_verbosity_level < 0)
					diagnostics(ERROR, "g_verbosity_level should be a positive integer 0--6");
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

	if (BblName == NULL) {
		char           *s;
		if (input != NULL) {
			if ((BblName = malloc(strlen(input) + 5)) == NULL)
				error(" malloc error -> out of memory!\n");
			strcpy(BblName, input);
			if ((s = strrchr(BblName, '.')) == NULL || strcmp(s, ".tex") != 0)
				strcat(BblName, ".bbl");
			else
				strcpy(s, ".bbl");
		} else {
			if ((BblName = malloc(1)) == NULL)
				error(" malloc error -> out of memory!\n");
			BblName = '\0';
		}
	}

	ReadCfg();
	OpenTexFile(input, &fTex);
	OpenRtfFile(output, &fRtf);

	
	InitializeStack();
	InitializeLatexLengths();
	InitializeDocumentFont(TexFontNumber("Roman"), 20, F_SHAPE_UPRIGHT, F_SERIES_MEDIUM);

	PushEnvironment(PREAMBLE);
	SetTexMode(MODE_VERTICAL);
    ConvertLatexPreamble(); 
	WriteRtfHeader();
	
	ReadLanguage(g_language);

	g_processing_preamble = FALSE;
	diagnostics(4,"Entering Convert from main");
	Convert();
	diagnostics(4,"Exiting Convert from main");

	CloseTex(&fTex);
	CloseRtf(&fRtf);
	printf("\n");
	return (0);
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
	fprintf(stderr, "\nERROR at line %ld: %s", linenumber, text);
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
		if (g_processing_include)
			return;	/* SAP - HACK end of file ok in include files */
		sprintf(text, "%s%s%s%ld%s", "unexpected end of input file in: ", latexname, " at linenumber: ", linenumber, "\n");
		error(text);
		/* @notreached@ */
		break;
	case ERR_WRONG_COMMAND:
		sprintf(text, "%s%s%s%ld%s", "unexpected command or character in: ", latexname, " at linenumber: ", linenumber, "\n");
		error(text);
		/* @notreached@ */
		break;

	case ERR_NOT_IN_DOCUMENT:
		sprintf(text, "\nNot in document %s at line %ld.  Missing \\begin{document}?\n", latexname, linenumber);
		error(text);
		/* @notreached@ */
		break;
	case ERR_Param:
		error("wrong number of parameters\n");
		/* @notreached@ */
		break;
	case ERR_WRONG_COMMAND_IN_TABBING:
		sprintf(text, "%s%s%s%ld%s", "wrong command in Tabbing-kill-command-line in: ", latexname, " at linenumber: ", linenumber, "\n");
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
 * Writes the given warning message in format, ... if global g_verbosity_level is
 * higher or equal then level.  If ??? option is given, i.e. logfile is not
 * null, everything is logged to the logfile -vn Flag (g_verbosity_level)	0 ...
 * only errors = -q 1 ... (default) Translation Warnings 2 ... conditions on
 * output e.g. (rtf1.5 options) 3 ... complete logging of what's going on.
 */
void
diagnostics(int level, char *format,...)
{
	va_list        apf;
	FILE           *errfile;
	int            i;
	
	if (logfile != NULL)
		errfile = logfile;
	else
		errfile = stderr;


	va_start(apf, format);

	if (level <= g_verbosity_level) {
		int iEnvCount=CurrentEnvironmentCount();
		switch (level) {
		case 0:
			fprintf(errfile, "\nError! line=%ld ", linenumber);
			break;
		case 1:
			fprintf(errfile, "\nWarning line=%ld ", linenumber);
			break;
		case 4:
		case 5:
		    fprintf(errfile, "\n%s %4ld bra=%d rec=%d env=%d ", input, linenumber, BraceLevel, RecursionLevel, iEnvCount);
			for (i=0; i<RecursionLevel; i++)
				fprintf(errfile, " ");
			break;
		default:
			fprintf(errfile, "\nline=%ld ", linenumber);
			break;
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
	/* Default Page Sizes */
	setLength("pageheight",  795*20);
	setLength("hoffset",       0*20);
	setLength("oddsidemargin",62*20);
	setLength("headheight",   12*20);
	setLength("textheight",  550*20);
	setLength("footskip",     30*20);
	setLength("marginparpush", 5*20);

	setLength("pagewidth",   614*20);
	setLength("voffset",       0*20);
	setLength("topmargin",    18*20);
	setLength("headsep",      25*20);	
	setLength("textwidth",   345*20);
	setLength("marginparsep", 10*20);
	setLength("columnsep",    10*20);

	setLength("smallskipamount", 3*20);
	setLength("medskipamount", 6*20);
	setLength("bigskipamount", 12*20);
	setLength("evensidemargin",11*20);

	/* Default Paragraph Sizes */
	setLength("baselineskip",12*20);
	setLength("parindent",   15*20);
	setLength("parskip",      0*20);
				
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
	 
	s = getTexUntil("\\begin{document}",0);
	
	diagnostics(4, "Entering ConvertString() from ConvertLatexPreamble <%s>",s);
	ConvertString(s);
	diagnostics(4, "Exiting ConvertString() from ConvertLatexPreamble");
	
	fRtf = hidden;
	free(s);
}


void 
OpenRtfFile(char *filename, FILE ** f)
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
OpenTexFile(const char *filename, FILE ** f)
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
	CmdEndParagraph(0);
	fprintf(*f, "}\n}}}}}");
	if (*f != stdout) {
		if (fclose(*f) == EOF) {
			diagnostics(WARNING, "Error closing RTF-File");
		}
	}
	*f = NULL;
	diagnostics(4,"Closed RTF file");
}

void
putRtfChar(char cThis)
/****************************************************************************
purpose: output filter to escape characters written to an RTF file
         all output to the RTF file passes through this routine or the one below
 ****************************************************************************/
{
	if (cThis == '\\')
		fprintf(fRtf, "\\\\");
	else if (cThis == '{')
		fprintf(fRtf, "\\{");
	else if (cThis == '}')
		fprintf(fRtf, "\\}");
	else if (cThis == '\n') 
		fprintf(fRtf, "\n\\par ");
	else
		fputc(cThis, fRtf);
}

void
fprintRTF(char *format, ...)
/****************************************************************************
purpose: output filter to track of brace depth and font settings
         all output to the RTF file passes through this routine or the one above
 ****************************************************************************/
{
	char buffer[1024];
	char *text;
	va_list       apf;

	va_start(apf, format);
	vsprintf(buffer, format, apf);
	va_end(apf);
	text = buffer;
	
	while ( *text ) {
	
		if (*text < 0)
			
			WriteEightBitChar(text[0]);
			
		else {		
			fputc(*text, fRtf);
			
			if (*text == '{')
				PushFontSettings();
			
			if (*text == '}')
				PopFontSettings();
				
			if (*text == '\\')
				MonitorFontChanges(text);
		}
		text++;
	}			
}
