/* $Id: main.c,v 1.50 2002/02/18 05:43:20 prahl Exp $ */

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
#include "util.h"
#include "parser.h"
#include "lengths.h"
#include "counters.h"
#include "preamble.h"
#include "xref.h"

FILE           *fRtf = (FILE *) NULL;	/* file pointer to RTF file */
char           *TexName = NULL;
static char    *RtfName = NULL;
char           *AuxName = NULL;
char           *BblName = NULL;

char           *progname;	            /* name of the executable file */
bool            GermanMode = FALSE;	    /* support germanstyle */
bool            FrenchMode = FALSE;	    /* support frenchstyle */

char            g_encoding[20] = "cp1252";
bool            twoside = FALSE;
int      		g_verbosity_level = WARNING;
static FILE    *logfile = NULL;

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
bool            g_processing_eqnarray = FALSE;	/* flag set when in an eqnarry */

bool            g_show_equation_number = FALSE;
int             g_enumerate_depth = 0;
bool            g_suppress_equation_number = FALSE;
bool            g_aux_file_missing = FALSE;	/* assume that it exists */

int				g_safety_braces = 0;
bool            g_processing_equation = FALSE;
bool            g_document_type = FORMAT_ARTICLE;
bool            g_processing_tabular = FALSE;
int				g_processing_arrays = 0;
int 			g_processing_fields = 0;
bool			g_RTF_warnings = FALSE;
char           *g_config_path = NULL;
char		   *g_tmp_path = NULL;
char           *g_preamble = NULL;
char     		g_field_separator = ',';

bool 			g_equation_rtf = FALSE;
bool			g_equation_inline_bitmap = FALSE;
bool			g_equation_display_bitmap = FALSE;
bool			g_equation_comment = FALSE;

int             indent = 0;
char            alignment = JUSTIFIED;	/* default for justified: */

int             RecursionLevel = 0;
bool            twocolumn = FALSE;
bool            titlepage = FALSE;

static void     OpenRtfFile(char *filename, FILE ** f);
static void     CloseRtf(FILE ** f);
static void     ConvertLatexPreamble(void);
static void     InitializeLatexLengths(void);
static void		usage(void);

void           *GetCommandFunc(char *cCommand);
static void 	ConvertWholeDocument(void);

extern char *optarg;
extern int   optind;
extern int getopt(int ac, char *const av[], const char *optstring);

int main(int argc, char **argv)
{
	int             c,x;
	char           *basename = NULL;

	progname = argv[0];
	optind = 1;
	while ((c = getopt(argc, argv, "lhvSVWZ:o:a:b:d:i:C:M:P:T:")) != EOF) {
		switch (c) {
		case 'a':
			AuxName = optarg;
			break;
		case 'b':
			BblName = optarg;
			break;
		case 'd':
			g_verbosity_level = *optarg - '0';
			if (g_verbosity_level < 0 || g_verbosity_level > 7) {
				diagnostics(WARNING, "debug level (-d# option) must be 0-7");
				usage();
			}
			break;
		case 'i':
			setPackageBabel(optarg);
			break;
		case 'l':
			setPackageBabel("latin1");
			break;
		case 'o':
			RtfName = optarg;
			break;
		case 'v':
			fprintf(stderr, "%s: %s\n", progname, Version);
			return (0);
		case 'C':
			setPackageInputenc(optarg);
			break;
		case 'M':
			sscanf(optarg, "%d", &x);
			diagnostics(WARNING, "Math option = %s x=%d",optarg,x);
			g_equation_rtf            = x & 1;
			g_equation_inline_bitmap  = x & 2;
			g_equation_display_bitmap = x & 4;
			g_equation_comment        = x & 8;
			break;
		case 'P':
			g_config_path = strdup(optarg);
			break;
		case 'S':
			g_field_separator = ';';
			break;
		case 'T':
			g_tmp_path = strdup(optarg);
			break;
		case 'V':
			fprintf(stderr, "%s: %s\n", progname, Version);
			fprintf(stderr, "RTFPATH = '%s'\n", getenv("RTFPATH"));
			return (0);
		case 'W':
			g_RTF_warnings = TRUE;
			break;
		case 'Z':
			g_safety_braces = FALSE;
			g_safety_braces = *optarg - '0';
			if (g_safety_braces < 0 || g_safety_braces > 9) {
				diagnostics(WARNING, "Number of safety braces (-Z#) must be 0-9");
				usage();
			}
			break;
			
		case 'h':
		case '?':
		default:
			usage();
		}
	}

 	argc -= optind;
    argv += optind;
	
	if (argc > 1) {
		diagnostics(WARNING, "Only a single file can be processed at a time");
		usage();
	}
	
	if (argc == 1) {
		char           *s, *t;

		s = strrchr(*argv, PATHSEP);
		if (s == NULL) s = *argv;

		t = strstr(s, ".tex");			/* remove .tex if present */
		if (t != NULL) 
			*t = '\0';
			
		basename = strdup(s);			
		s = strchr(basename, '.');		/* check for dot in name */
		
		if (t == NULL && s != NULL)
			TexName = strdup(basename);
		else
			TexName = strdup_together(basename, ".tex");
			
		RtfName = strdup_together(basename, ".rtf");
	} 
	
	if (AuxName == NULL && basename != NULL) 
		AuxName = strdup_together(basename, ".aux");

	if (BblName == NULL && basename != NULL)
		BblName = strdup_together(basename, ".bbl");

	if (basename) {
		diagnostics(3, "latex filename is <%s>", TexName);
		diagnostics(3, "  rtf filename is <%s>", RtfName);
		diagnostics(3, "  aux filename is <%s>", AuxName);
		diagnostics(3, "  bbl filename is <%s>", BblName);
	}
	
	ReadCfg();
	if (PushSource(TexName, NULL)) {
		OpenRtfFile(RtfName, &fRtf);
		
		InitializeStack();
		InitializeLatexLengths();
		InitializeDocumentFont(TexFontNumber("Roman"), 20, F_SHAPE_UPRIGHT, F_SERIES_MEDIUM);
	
		ConvertWholeDocument();	
		PopSource();
		CloseRtf(&fRtf);
		printf("\n");
		return 0;
	} else
		return 1;
}

static void
ConvertWholeDocument(void)
{
char * body, *sec_head, *sec_head2, *label;
	char t[] = "\\begin{document}";

		PushEnvironment(PREAMBLE);
		SetTexMode(MODE_VERTICAL);
		ConvertLatexPreamble(); 
		WriteRtfHeader();
		ConvertString(t);

		g_processing_preamble = FALSE;
		getSection(&body,&sec_head,&label);
		
		diagnostics(2,"*******************\nbody=%s",body);
		diagnostics(2,"*******************\nsec_head=%s",sec_head);
		diagnostics(2,"*******************\nlabel=%s",g_section_label);
		ConvertString(body);
		free(body);
		if (label) free(label);
		
		while(sec_head) {
			getSection(&body,&sec_head2,&g_section_label);
			label = ExtractLabelTag(sec_head);
			if (label) {
				if (g_section_label) free(g_section_label);
				g_section_label = label;
			} 
		diagnostics(2,"\n========this section head==========\n%s",sec_head);
		diagnostics(2,"\n============ label ================\nlabel=%s",g_section_label);
		diagnostics(2,"\n==============body=================\n%s\n=========end  body=================",body);
		diagnostics(2,"\n========next section head==========\n%s",sec_head2);
			ConvertString(sec_head);
			ConvertString(body);
			free(body);
			free(sec_head);
			sec_head = sec_head2;
		}
}

static void
usage(void)
{
    	fprintf(stderr, "Usage:\n\t %s [options] input[.tex]\n", progname);
		fprintf(stderr, "Options:\n");
		fprintf(stderr, "\t -a auxfile       : use LaTeX auxfile rather than input.aux\n");
		fprintf(stderr, "\t -b bblfile       : use BibTex bblfile rather than input.bbl)\n");
		fprintf(stderr, "\t -d#              : debug level (# is 0-6)\n");
		fprintf(stderr, "\t -h               : display this help\n");
		fprintf(stderr, "\t -i language      : babel idiom (german, french)\n");
		fprintf(stderr, "\t -l               : use latin1 encoding (default)\n");
		fprintf(stderr, "\t -o outputfile    : RTF output other than input.rtf\n");
		fprintf(stderr, "\t -v               : version information\n");
		fprintf(stderr, "\t -C codepage      : input encoding (latin1, cp850, etc.)\n");
		fprintf(stderr, "\t -M#              : math equation handling\n");
		fprintf(stderr, "\t -P /path/to/cfg  : directory containing .cfg files\n");
		fprintf(stderr, "\t -V               : version information\n");
		fprintf(stderr, "\t -S               : use ';' to separate args in RTF fields\n");
		fprintf(stderr, "\t -T /path/to/tmp  : temporary directory\n");
		fprintf(stderr, "\t -W               : include warnings in RTF\n");
		fprintf(stderr, "\t -Z#              : add # of '}'s at end of rtf file (# is 0-9)\n\n");
		fprintf(stderr, "RTFPATH designates the directory for configuration files (*.cfg)\n");
		fprintf(stderr, "\t RTFPATH = '%s'\n\n", getenv("RTFPATH"));
		exit(1);
}

void
diagnostics(int level, char *format,...)
/*
purpose : Writes the message to errfile depending on debugging level
*/
{
	char           buffer[512], *buff_ptr;
	va_list        apf;
	FILE           *errfile;
	int            i,linenumber, iEnvCount;
	char          *input;
	
	buff_ptr = buffer;
	if (logfile != NULL)
		errfile = logfile;
	else
		errfile = stderr;

	va_start(apf, format);

	if (level <= g_verbosity_level) {

		linenumber = CurrentLineNumber();
		input      = CurrentFileName();
		iEnvCount  = CurrentEnvironmentCount();
		
		switch (level) {
		case 0:
			fprintf(errfile, "\nError! line=%d ", linenumber);
			CloseRtf(&fRtf);
			exit(0);
			break;
		case 1:
			fprintf(errfile, "\nWarning line=%d ", linenumber);
			if (g_RTF_warnings) {
				vsprintf(buffer, format, apf);
				fprintRTF("{\\plain\\cf2 [latex2rtf:");
				while (*buff_ptr){putRtfChar(*buff_ptr);buff_ptr++;}
				fprintRTF("]}");
			}
			break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		    fprintf(errfile, "\n%s %4d rec=%d ", input, linenumber, RecursionLevel);
			for (i=0; i<BraceLevel; i++)
				fprintf(errfile, "{");
			for (i=8; i>BraceLevel; i--)
				fprintf(errfile, " ");
			
			for (i=0; i<RecursionLevel; i++)
				fprintf(errfile, "  ");
			break;
		default:
			fprintf(errfile, "\nline=%d ", linenumber);
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
	setCounter("secnumdepth",   2);
	
 }


static void 
ConvertLatexPreamble(void)
/****************************************************************************
purpose: reads the LaTeX preamble (to \begin{document} ) for the file
 ****************************************************************************/
{
	FILE * hidden;
	char t[] = "\\begin{document}";
	
	diagnostics(4, "Reading LaTeX Preamble");
	hidden = fRtf;
	fRtf = stderr;
	 
	g_preamble = getTexUntil(t,0);
	
	diagnostics(4, "Entering ConvertString() from ConvertLatexPreamble <%s>",g_preamble);
	ConvertString(g_preamble);
	diagnostics(4, "Exiting ConvertString() from ConvertLatexPreamble");
	
	fRtf = hidden;
}


void 
OpenRtfFile(char *filename, FILE ** f)
/****************************************************************************
purpose: creates output file and writes RTF-header.
params: filename - name of outputfile, possibly NULL for already open file
	f - pointer to filepointer to store file ID
 ****************************************************************************/
{
	if (filename == NULL) {
		diagnostics(4, "Writing RTF to stdout");
		*f = stdout;

	} else {
		diagnostics(4, "Opening RTF file %s", filename);
		*f = fopen(filename, "wb");

		if (*f == NULL) 
			diagnostics(ERROR,  "Error opening RTF file %s\n", filename);
	}
}

void
CloseRtf(FILE ** f)
/****************************************************************************
purpose: closes output file.
params: f - pointer to filepointer to invalidate
globals: TexName;
 ****************************************************************************/
{
	int i;
	CmdEndParagraph(0);
	if (BraceLevel>1) 
		diagnostics(WARNING,"Mismatched '{' in RTF file, Conversion may cause problems.");

	if (BraceLevel-1>g_safety_braces) 
		diagnostics(WARNING,"Try translating with 'latex2rtf -Z%d %s'", BraceLevel-1, TexName);
	
	fprintf(*f, "}\n");
	for (i=0; i<g_safety_braces; i++)
		fprintf(*f, "}");
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
