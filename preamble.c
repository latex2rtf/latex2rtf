/***************************************************************************
purpose : Handles LaTeX commands that should only occur in the preamble.
          These are gathered together because the entire preamble must be
		  parsed before the RTF header can be written.
		  
		  When \begin{document} is encountered, then the RTF header is created.
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "l2r_fonts.h"
#include "cfg.h"
#include "util.h"
#include "encode.h"
#include "parser.h"
#include "funct1.h"
#include "funct2.h"
#include "preamble.h"
#include "lengths.h"
#include "ignore.h"
#include "commands.h"
#include "counters.h"

static int    g_preambleFormat   = FORMAT_ARTICLE;
static bool   g_preambleTwoside  = FALSE;
static bool   g_preambleTwocolumn= FALSE;
static bool   g_preambleTitlepage= FALSE;
static bool   g_preambleLandscape= TRUE;
extern bool   pagestyledefined;

static char * g_preambleTitle    = NULL;
static char * g_preambleAuthor   = NULL;
static char * g_preambleDate     = NULL;
static char * g_preambleLanguage = NULL;
static char * g_preambleEncoding = NULL;

static void setPackageBabel(char * option);
static void setPackageInputenc(char * option);
static void setPaperSize(char * size);
static void setDocumentOptions(char *optionlist);
static void WriteFontHeader(void);
static void WriteStyleHeader(void);
static void WritePageSize(void);

static void
setPackageBabel(char * option)
{
	g_preambleLanguage = strdup(option);
	if (strcmp(option, "german") == 0)
		GermanMode = TRUE;
}

static void
setPackageInputenc(char * option)
{
	g_preambleEncoding = strdup(option);
	fprintf(stderr,"\n Input Encoding <%s> not supported yet", option);
}

static void
setPackageFont(char * font)
{
	fprintf(stderr,"\n Font Package <%s> not supported yet", font);
}

static void
setPaperSize(char * option)
/******************************************************************************
   Should also try to reset some of the other sizes at this time
******************************************************************************/
{
	if (strcmp(option, "landscape") == 0) {
		g_preambleLandscape = TRUE;
		
	} else if (strcmp(option, "a4paper") == 0 || strcmp(option, "a4") == 0) {
		setLength("pagewidth", 21.0/2.54*72*20);
		setLength("pageheight", 29.7/2.54*72*20);
	
	} else if (strcmp(option, "a4wide") == 0 ) {
		setLength("pagewidth", 5.875*72*20);
		setLength("pageheight", 29.7/2.54*72*20);

	} else if (strcmp(option, "letterpaper") == 0) {
		setLength("pagewidth", 8.5*72*20);
		setLength("pageheight", 11*72*20);

	} else if (strcmp(option, "legalpaper") == 0) {
		setLength("pagewidth", 8.5*72*20);
		setLength("pageheight", 14*72*20);
	}
}

static void 
setDocumentOptions(char *optionlist)
/******************************************************************************
******************************************************************************/
{
	char           *option;

	option = strtok(optionlist, ",");

	while (option) {
		diagnostics(4, "                    option   %s", option);
		if (strcmp(option, "11pt") == 0)
			SetDocumentFontSize(22);
		else if (strcmp(option, "12pt") == 0)
			SetDocumentFontSize(24);
		else if (strcmp(option, "a4")  == 0 ||
			     strcmp(option, "a4paper") == 0 || 
			     strcmp(option, "a4wide") == 0 || 
			     strcmp(option, "letterpaper") == 0 || 
			     strcmp(option, "landscape") == 0 || 
				 strcmp(option, "legalpaper")  == 0) 
			setPaperSize(option);
		else if (strcmp(option, "german")  == 0 ||
			     strcmp(option, "spanish") == 0 || 
			     strcmp(option, "english") == 0 || 
				 strcmp(option, "french")  == 0) 
			setPackageBabel(option);
		else if (strcmp(option, "twoside") == 0) {
			g_preambleTwoside = TRUE;
//			fprintf(fRtf, "\\facingp");
		} else if (strcmp(option, "twocolumn") == 0) {
//			fprintf(fRtf, "\\cols2\\colsx709 ");	/* two columns -- space between columns 709 */
			g_preambleTwocolumn = TRUE;
		} else if (strcmp(option, "titlepage") == 0) {
			g_preambleTitlepage = TRUE;
		} else if (strcmp(option, "isolatin1") == 0) {
//			TexCharSet = ISO_8859_1;
			fprintf(stderr, "\nisolatin1 style option encountered.");
			fprintf(stderr, "\nLatin-1 (= ISO 8859-1) special characters will be ");
			fprintf(stderr, "converted into RTF-Commands!\n");
		} else if (strcmp(option, "hyperlatex") == 0) {
//			PushEnvironment(HYPERLATEX);
		} else if (!TryVariableIgnore(option, fTex)) {
			diagnostics(WARNING, "Unknown style option %s ignored", option);
		}
		option = strtok(NULL, ",");
	}
}

void 
CmdDocumentStyle(int code)
/******************************************************************************
 purpose: parse \documentstyle[options]{format} or \documentclass[options]{format}
 ******************************************************************************/
{
	char            *format;
	char            optionlist[100];

	getBracketParam(optionlist, 99);
	format = getParam();

	diagnostics(4, "Documentstyle/class[%s]{%s}", optionlist,format);

	g_preambleFormat = FORMAT_ARTICLE;
	if (strcmp(format, "book") == 0)
		g_preambleFormat = FORMAT_BOOK;
		
	else if (strcmp(format, "report") == 0)
		g_preambleFormat = FORMAT_REPORT;

	else if (strcmp(format, "letter") == 0)
		g_preambleFormat = FORMAT_LETTER;

	else if (strcmp(format, "article") == 0)
		g_preambleFormat = FORMAT_ARTICLE;

	else if (strcmp(format, "slides") == 0)
		g_preambleFormat = FORMAT_SLIDES;

	else
		fprintf(stderr, "\nDocument format <%s> unknown, using article format", format);

	setDocumentOptions(optionlist);
	free(format);
}

void 
CmdUsepackage(int code)
/******************************************************************************
 purpose: handle \usepackage[option]{packagename}
******************************************************************************/
{
	char            *package;
	char            optionlist[100];

	getBracketParam(optionlist, 99);
	package=getParam();

	diagnostics(4, "Package {%s} with options [%s] encountered", package, optionlist);

	if (strcmp(package, "inputenc") == 0)
		setPackageInputenc(optionlist);
		
	else if (strcmp(package, "babel") == 0)
		setPackageBabel(optionlist);
		
	else if (strcmp(package, "palatino") == 0 ||
	         strcmp(package, "times") == 0    ||
	         strcmp(package, "helvetica") == 0 )
		setPackageFont(optionlist);
	else
		setDocumentOptions(package);
		
	free(package);
}

/******************************************************************************
  purpose: saves title, author or date information
 ******************************************************************************/
void
CmdTitle(int code)
{
	switch (code) {
	case TITLE_TITLE:
		g_preambleTitle = getParam();
		break;

	case TITLE_AUTHOR:
		g_preambleAuthor = getParam();
		break;

	case TITLE_DATE:
		g_preambleDate = getParam();
		break;

	case TITLE_TITLEPAGE:
		g_preambleTitlepage = TRUE;
		break;
	}
}

void
CmdMakeTitle(int code)
/******************************************************************************
  purpose: Creates a title page based on saved values for author, title, and date
 ******************************************************************************/
{
	char            title_begin[10];
	char            author_begin[10];
	char            date_begin[10];

	sprintf(title_begin, "%s%2d", "\\fs", (30 * CurrentFontSize()) / 20);
	sprintf(author_begin, "%s%2d", "\\fs", (24 * CurrentFontSize()) / 20);
	sprintf(date_begin, "%s%2d", "\\fs", (24 * CurrentFontSize()) / 20);

	fprintf(fRtf, "\n\\par\\pard\\qc {%s ", title_begin);
	if (g_preambleTitle != NULL && strcmp(g_preambleTitle, "") != 0)
		ConvertString(g_preambleTitle);
	fprintf(fRtf, "}");

	fprintf(fRtf, "\n\\par\\qc {%s ", author_begin);
	if (g_preambleAuthor != NULL && strcmp(g_preambleAuthor, "") != 0)
		ConvertString(g_preambleAuthor);
	fprintf(fRtf, "}");
	
	fprintf(fRtf, "\n\\par\\qc {%s ", date_begin);
	if (g_preambleDate != NULL && strcmp(g_preambleDate, "") != 0)
		ConvertString(g_preambleDate);
	fprintf(fRtf, "}");
	
	fprintf(fRtf, "\n\\par\n\\par\\pard\\q%c ", alignment);
	if (g_preambleTitlepage)
		fprintf(fRtf, "\\page ");
}

void 
CmdPreambleBeginEnd(int code)
/***************************************************************************
   purpose: catch missed \begin{document} command ***************************************************************************/
{
	char           *cParam = getParam();
	
	if (!strcmp(cParam,"document"))
		diagnostics(ERROR, "\\begin{%s} found before \\begin{document}.  Giving up.  Sorry", cParam);
		
	CallParamFunc(cParam, ON);
	free(cParam);
}

/******************************************************************************
  LEG030598
  purpose: sets centered page numbering at bottom in rtf-output

  globals : pagenumbering set to TRUE if pagenumbering is to occur, default
 ******************************************************************************/
void
PlainPagestyle(void)
{
	int fn = getTexFontNumber("Roman");
	pagenumbering = TRUE;
	
	if (g_preambleTwoside) {
		fprintf(fRtf, "\n{\\footerr");
		fprintf(fRtf, "\\pard\\plain\\f%d\\qc",fn);
		fprintf(fRtf, "{\\field{\\*\\fldinst PAGE}{\\fldrslt ?}}\\par}");
		fprintf(fRtf, "\n{\\footerl");
		fprintf(fRtf, "\\pard\\plain\\f%d\\qc",fn);
		fprintf(fRtf, "{\\field{\\*\\fldinst PAGE}{\\fldrslt ?}}\\par}");
	} else {
		fprintf(fRtf, "\n{\\footer");
		fprintf(fRtf, "\\pard\\plain\\f%d\\qc",fn);
		fprintf(fRtf, "{\\field{\\*\\fldinst PAGE}{\\fldrslt ?}}\\par}");
	}
}

/******************************************************************************
 * LEG030598
 purpose: sets page numbering in rtf-output
 parameter:

 globals : headings  set to TRUE if the pagenumber is to go into the header
           pagenumbering set to TRUE if pagenumbering is to occur- default
	   pagestyledefined, flag, set to true

Produces latex-like headers and footers.
Needs to be terminated for:
- headings chapter, section informations and page numbering
- myheadings page nunmbering, combined with markboth, markright.

 ******************************************************************************/
void
CmdPagestyle( /* @unused@ */ int code)
{
	static char    *style = "";

	pagestyledefined = TRUE;
	style = getParam();
	if (strcmp(style, "empty") == 0) {
		if (pagenumbering) {
			fprintf(fRtf, "{\\footer}");
			pagenumbering = FALSE;
		}
	} else if (strcmp(style, "plain") == 0)
		PlainPagestyle();
	else if (strcmp(style, "headings") == 0) {
		headings = TRUE;
		/*--- but here code to put section information in header, pagenumbering
		      in header */
	} else if (strcmp(style, "myheadings") == 0) {
		headings = TRUE;
		/*--- but here code to put empty section information in header, will be
		      provided by markboth, markright
		      pagenumbering in header */
	} else {
		diagnostics(WARNING, "Unknown \\pagestyle{%s} ignored", style);
	}
}



/******************************************************************************
 * LEG030598
 purpose: converts the \markboth and \markright Command in Header information
 parameter: code: BOTH_SIDES, RIGHT_SIDE

 globals : twoside,
 ******************************************************************************/
void
CmdHeader(int code)
{
	if (code == BOTH_SIDES) {
		if (g_preambleTwoside) {
			RtfHeader(LEFT_SIDE, NULL);
			RtfHeader(RIGHT_SIDE, NULL);
		} else
			diagnostics(WARNING, "\\markboth used in onesided documentstyle");
	} else
		RtfHeader(BOTH_SIDES, NULL);
}

/******************************************************************************
  LEG030598
  purpose: generates the header command in the rtf-output
  parameter: where: RIGHT_SIDE, LEFT_SIDE -handed page, BOTH_SIDES
           what:  NULL - Convert from LaTeX input, else put "what" into rtf
                  output
 ******************************************************************************/
void
RtfHeader(int where, char *what)
{
	int fn = getTexFontNumber("Roman");
	switch (where) {
		case RIGHT_SIDE:
		fprintf(fRtf, "\n{\\headerr \\pard\\plain\\f%d ",fn);
		break;
	case LEFT_SIDE:
		fprintf(fRtf, "\n{\\headerl \\pard\\plain\\f%d ",fn);
		break;
	case BOTH_SIDES:
		fprintf(fRtf, "\n{\\header \\pard\\plain\\f%d ",fn);
		break;
	default:
		diagnostics(ERROR, "\n error -> called RtfHeader with illegal parameter\n ");
	}
	if (what == NULL) {
		diagnostics(4, "Entering Convert() from RtfHeader");
		Convert();
		diagnostics(4, "Exiting Convert() from RtfHeader");
		fprintf(fRtf, "}");
	} else
		fprintf(fRtf, "%s}", what);
}


void 
CmdHyphenation(int code)
/******************************************************************************
 purpose: discard all hyphenation hints since they really only make sense when
          used with TeX's hyphenation algorithms 
 ******************************************************************************/
{
	char           *hyphenparameter = getParam();
	free(hyphenparameter);
}

static void 
WriteFontHeader(void)
/****************************************************************************
 *   purpose: writes fontnumbers and styles for headers into Rtf-File
 * parameter: fRtf: File-Pointer to Rtf-File
 *   globals:
 *            DefFont (default font number)
 *   note;

 \fcharset0:    ANSI coding
 \fcharset1:    MAC coding
 \fcharset2:    PC coding (implies CodePage 437)
 \fcharset3:    PCA coding (implies CodePage 850)
 ****************************************************************************/
{
	int                  num = 0;
	ConfigEntryT       **config_handle;

	fprintf(fRtf, "{\\fonttbl");

	config_handle = CfgStartIterate(FONT_A);
	while ((config_handle = CfgNext(FONT_A, config_handle)) != NULL) {
		if (strstr((*config_handle)->TexCommand, "MacRoman"))
			fprintf(fRtf
				,"{\\f%u\\fnil\\fcharset1 %s;}"
				,(unsigned int) num
				,(*config_handle)->RtfCommand
				);
		else if (strstr((*config_handle)->RtfCommand, "Symbol"))
			fprintf(fRtf
				,"{\\f%u\\fnil\\fcharset2 %s;}"
				,(unsigned int) num
				,(*config_handle)->RtfCommand
				);
		else
			fprintf(fRtf
				,"{\\f%u\\fnil\\fcharset0 %s;}"
				,(unsigned int) num
				,(*config_handle)->RtfCommand
				);
		++num;
	}

	fprintf(fRtf, "}");
}

static void
WriteStyleHeader(void)
/****************************************************************************
       --
      |   {\stylesheet{\fs20 \sbasedon222\snext10{keycode \shift...}
  A---|   {\s1 \ar \fs20 \sbasedon0\snext1 FLUSHRIGHT}{\s2\fi...}
      |   \sbasedon0snext2 IND:}}
       --
          ...
       --
      |  \widowctrl\ftnbj\ftnrestart \sectd \linex0\endnhere
      |  \pard\plain \fs20 This is Normal style.
  B---|  \par \pard\plain \s1
      |  This is right justified. I call this style FLUSHRIGHT.
      |  \par \pard\plain \s2
      |  This is an indented paragraph. I call this style IND...
       --
         \par}
 ****************************************************************************/
{
	fprintf(fRtf, "{\\stylesheet{\\fs%d\\lang1031\\snext0 Normal;}", CurrentFontSize());
	fprintf(fRtf, "{%s%u%s \\sbasedon0\\snext0 heading 1;}\n", HEADER11, DefFont, HEADER12);
	fprintf(fRtf, "{%s%u%s \\sbasedon0\\snext0 heading 2;}\n", HEADER21, DefFont, HEADER22);
	fprintf(fRtf, "{%s%u%s \\sbasedon0\\snext0 heading 3;}\n", HEADER31, DefFont, HEADER32);
	fprintf(fRtf, "{%s%u%s \\sbasedon0\\snext0 heading 4;}\n", HEADER41, DefFont, HEADER42);

	fprintf(fRtf, "%s\n", HEADER03);
	fprintf(fRtf, "%s\n", HEADER13);
	fprintf(fRtf, "%s\n", HEADER23);
	fprintf(fRtf, "%s\n", HEADER33);
	fprintf(fRtf, "%s\n", HEADER43);
}

static void
WritePageSize(void)
/****************************************************************************
  \paperw<N>      The paper width (the default is 12,240).
  \paperh<N>      The paper height (the default is 15,840).
  \margl<N>       The left margin (the default is 1,800).
  \margr<N>       The right margin (the default is 1,800).
  \margt<N>       The top margin (the default is 1,440).
  \margb<N>       The bottom margin (the default is 1,440).

  \facingp        Facing pages (activates odd/even headers and gutters).
  \gutter<N>      The gutter width (the default is 0).
  \margmirror     Switches margin definitions on left and right pages.
  \landscape      Landscape format.
  \pgnstart<N>    The beginning page number (the default is 1).
  \widowctrl      Widow control.

  \headery<N>     The header is <N> twips from the top of the page (the default is 720).
  \footery<N>     The footer is <N> twips from the bottom of the page (the default is 720).
****************************************************************************/
{
	int n;

	fprintf(fRtf,"\\paperw%d", getLength("pagewidth"));
	fprintf(fRtf,"\\paperh%d", getLength("pageheight"));
	if (g_preambleTwoside)
		fprintf(fRtf,"\\facingp");
	if (g_preambleLandscape)
		fprintf(fRtf,"\\landscape");
		
	n = getLength("hoffset") + 72*20 + getLength("marginparsep");
	fprintf(fRtf, "\\margl%d", n);
	n = getLength("pagewidth") - (n + getLength("textwidth"));
	fprintf(fRtf, "\\margr%d", n);
	n = getLength("voffset") + 72*20 + getLength("topmargin") + getLength("headheight")+getLength("headsep");
	fprintf(fRtf, "\\margt%d", n);
	n = getLength("pageheight") - (n + getLength("textheight") + getLength("footskip"));
	fprintf(fRtf, "\\margb%d", n);
	
	fprintf(fRtf,"\\pgnstart%d", getCounter("page"));
	fprintf(fRtf,"\\widowctrl\n");
}

static void
WriteHeadFoot(void)
/****************************************************************************
  \headerl        The header is on left pages only.
  \headerr        The header is on right pages only.
  \headerf        The header is on the first page only.
  \footerl        The footer is on left pages only.
  \footerr        The footer is on right pages only.
  \footerf        The footer is on the first page only.
****************************************************************************/
{
	int n;

	fprintf(fRtf,"\\paperw%d", getLength("pagewidth"));
	fprintf(fRtf,"\\paperh%d", getLength("pageheight"));
	if (g_preambleTwoside)
		fprintf(fRtf,"\\facingp");
	if (g_preambleLandscape)
		fprintf(fRtf,"\\landscape");
		
	n = getLength("hoffset") + 72*20 + getLength("marginparsep");
	fprintf(fRtf, "\\margl%d", n);
	n = getLength("pagewidth") - (n + getLength("textwidth"));
	fprintf(fRtf, "\\margr%d", n);
	n = getLength("voffset") + 72*20 + getLength("topmargin") + getLength("headheight")+getLength("headsep");
	fprintf(fRtf, "\\margt%d", n);
	n = getLength("pageheight") - (n + getLength("textheight") + getLength("footskip"));
	fprintf(fRtf, "\\margb%d", n);
	
	fprintf(fRtf,"\\pgnstart%d", getCounter("page"));
	fprintf(fRtf, "\\widowctrl\\ftnbj\\sectd\\linex0\\endnhere\\qj \n");
}

static void
WriteInfo(void)
/****************************************************************************
  \title          The title of the document
  \subject        The subject of the document
  \author         The author of the document
  \operator       The person who last made changes to the document
  \keywords       Selected keywords for the document
  \comment        Comments; text is ignored
  \version<N>     The version number of the document
  \doccomm        Comments displayed in Word’s Summary Info dialog
  
{\info {\title This is a page} {\author \'ca}}
 ***************************************************************************/
{
}

void 
WriteRtfHeader(void )
/****************************************************************************
purpose: writes header info for the RTF file

\rtf1 <charset> \deff? <fonttbl> <filetbl>? <colortbl>? <stylesheet>? <listtables>? <revtbl>?
 ****************************************************************************/
{
	diagnostics(4, "Writing header for RTF file");

	fprintf(fRtf, "{\\rtf1\\PC\\fs%d\\deff0\\deflang1024\n", CurrentFontSize());
	WriteFontHeader();
	WriteStyleHeader();
	WriteInfo();
	WritePageSize();
	WriteHeadFoot();
}


