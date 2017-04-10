/* preamble.c - Handles LaTeX commands that should only occur in the preamble.

Copyright (C) 2001-2002 The Free Software Foundation

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

This file is available from http://sourceforge.net/projects/latex2rtf/
 
Authors:
    2001-2002 Scott Prahl
*/

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "main.h"
#include "convert.h"
#include "utils.h"
#include "preamble.h"
#include "fonts.h"
#include "cfg.h"
#include "encodings.h"
#include "parser.h"
#include "funct1.h"
#include "lengths.h"
#include "ignore.h"
#include "commands.h"
#include "counters.h"
#include "xrefs.h"
#include "direct.h"
#include "styles.h"
#include "vertical.h"
#include "auxfile.h"
#include "acronyms.h"

extern char *Version;  /*storage and definition in version.h */

static int g_preambleTwoside = FALSE;
static int g_preambleTwocolumn = FALSE;
static int g_preambleTitlepage = FALSE;
static int g_preambleLandscape = FALSE;
static int g_preambleGeometry = FALSE;

static int g_geomMargl = 0;
static int g_geomMargr = 0;
static int g_geomMargt = 0;
static int g_geomMargb = 0;

static int gColorPackage = 0;

static char *g_preambleTitle = NULL;
static char *g_preambleAuthor = NULL;
static char *g_preambleDate = NULL;
static char *g_preambleAffiliation = NULL;
static char *g_preambleAbstract = NULL;
static char *g_preambleAck = NULL;

static char *g_preambleCFOOT = NULL;
static char *g_preambleLFOOT = NULL;
static char *g_preambleRFOOT = NULL;
static char *g_preambleCHEAD = NULL;
static char *g_preambleLHEAD = NULL;
static char *g_preambleRHEAD = NULL;

static void setPaperSize(char *size);
static void setDocumentOptions(char *optionlist);
static void WriteFontHeader(void);
static void WriteStyleHeader(void);
static void WritePageSize(void);
void ParseOptGeometry(char *options);
void ExecGeomOptions (char *option, char *value1, char *value2);

void setPackageBabel(char *option)
{
   char *replica,*language,*comma;

   if (option == NULL || *option == '\0') return;
   diagnostics(2, "setPackageBabel [%s]", option);

   replica = strdup_noblanks(option);
   language = replica;
   
   while (language && *language) {
       
       comma = strchr(language,',');
       if (comma != NULL) *comma = '\0';
           
	   if (strstr(language, "german")) { /* also catches ngerman */
			GermanMode = TRUE;
			PushEnvironment(GERMAN_MODE);
			ReadLanguage("german");
			return;
		}
	
		if (strstr(language, "french")) { /* also frenchb */
			FrenchMode = TRUE;
			PushEnvironment(FRENCH_MODE);
		}
	
		if (strstr(language, "russian")) {
			RussianMode = TRUE;
			PushEnvironment(RUSSIAN_MODE);
		}
	
		if (strstr(language, "czech")) {
			CzechMode = TRUE;
			PushEnvironment(CZECH_MODE);
			if (CurrentFontEncoding()==ENCODING_1252) /* has not been set already */
				CmdFontEncoding(ENCODING_LATIN_2);
		}
	
		ReadLanguage(language);
		language = (comma) ? comma+1 : NULL;
    }
    safe_free(replica);
}

void setPackageInputenc(char *option)
{
    if (strstr(option, "applemac"))
        CmdFontEncoding(ENCODING_APPLE);
    else if (strstr(option, "decmulti"))
        CmdFontEncoding(ENCODING_DEC);
    else if (strstr(option, "latin1"))
        CmdFontEncoding(ENCODING_1252);
    else if (strstr(option, "latin2"))
        CmdFontEncoding(ENCODING_LATIN_2);
    else if (strstr(option, "latin3"))
        CmdFontEncoding(ENCODING_LATIN_3);
    else if (strstr(option, "latin4"))
        CmdFontEncoding(ENCODING_LATIN_4);
    else if (strstr(option, "latin5"))
        CmdFontEncoding(ENCODING_LATIN_5);
    else if (strstr(option, "latin9"))
        CmdFontEncoding(ENCODING_LATIN_9);
    else if (strstr(option, "latin10"))
        CmdFontEncoding(ENCODING_LATIN_10);
    else if (strstr(option, "next"))
        CmdFontEncoding(ENCODING_NEXT);
    else if (strstr(option, "maccyr"))
        CmdFontEncoding(ENCODING_APPLE_CYRILLIC);
    else if (strstr(option, "macce"))
        CmdFontEncoding(ENCODING_APPLE_CE);
    else if (strstr(option, "koi8-r"))
        CmdFontEncoding(ENCODING_KOI8_R);
    else if (strstr(option, "koi8-u"))
        CmdFontEncoding(ENCODING_KOI8_U);
    else if (strstr(option, "437"))
        CmdFontEncoding(ENCODING_437);
    else if (strstr(option, "ansinew"))
        CmdFontEncoding(ENCODING_1252);
    else if (strstr(option, "1252"))
        CmdFontEncoding(ENCODING_1252);
    else if (strstr(option, "850"))
        CmdFontEncoding(ENCODING_850);
    else if (strstr(option, "852"))
        CmdFontEncoding(ENCODING_852);
    else if (strstr(option, "855"))
        CmdFontEncoding(ENCODING_852);
    else if (strstr(option, "865"))
        CmdFontEncoding(ENCODING_865);
    else if (strstr(option, "866"))
        CmdFontEncoding(ENCODING_866);
    else if (strstr(option, "1250"))
        CmdFontEncoding(ENCODING_1250);
    else if (strstr(option, "1251"))
        CmdFontEncoding(ENCODING_1251);
    else if (strstr(option, "1253"))
        CmdFontEncoding(ENCODING_1253);
    else if (strstr(option, "raw"))
        CmdFontEncoding(ENCODING_RAW);
    else if (strstr(option, "utf8"))
        CmdFontEncoding(ENCODING_UTF8);
    else if (strstr(option, "ot2enc"))
        setPackageBabel("russian");     
    else
        diagnostics(WARNING, "Input Encoding <%s> not supported", option);
}

static void setPackageFont(char *font)
{
    int fnumber = -1;

    if (strcmp(font, "palatino") == 0)
        fnumber = RtfFontNumber("Palatino");

    else if (strstr(font, "times"))
        fnumber = RtfFontNumber("Times");

    else if (strstr(font, "chancery"))
        fnumber = RtfFontNumber("Zapf Chancery");

    else if (strstr(font, "courier"))
        fnumber = RtfFontNumber("Courier");

    else if (strstr(font, "avant"))
        fnumber = RtfFontNumber("Avant Garde");

    else if (strstr(font, "helvet"))
        fnumber = RtfFontNumber("Helvetica");

    else if (strstr(font, "newcen"))
        fnumber = RtfFontNumber("New Century Schoolbook");

    else if (strstr(font, "book"))
        fnumber = RtfFontNumber("Bookman");

/*    InitializeDocumentFont(fnumber, -1, -1, -1); */
    if (fnumber == -1)
        diagnostics(WARNING, "Font Package <%s> not supported.", font);
}

static void setThree(char *s, int ten, int eleven, int twelve)
{
    int n = DefaultFontSize();

    if (n == 20)
        setLength(s, ten * 20);
    else if (n == 22)
        setLength(s, eleven * 20);
    else
        setLength(s, twelve * 20);
}

static void setPaperSize(char *option)

/******************************************************************************
   Should also try to reset some of the other sizes at this time
******************************************************************************/
{
    if (strcmp(option, "landscape") == 0) {
        g_preambleLandscape = TRUE;

    } else if (strcmp(option, "a4paper") == 0) {

        setLength("pageheight", 845 * 20);
        setLength("hoffset", 0 * 20);
        setThree("oddsidemargin", 53, 46, 31);
        setLength("headheight", 12 * 20);
        setThree("textheight", 598, 596, 592);
        setLength("footskip", 30 * 20);
        setLength("marginparpush", 5 * 20);

        setLength("pagewidth", 598 * 20);
        setLength("voffset", 0 * 20);
        setLength("topmargin", 17 * 20);
        setLength("headsep", 25 * 20);
        setThree("textwidth", 345, 360, 390);
        setThree("columnwidth", 345, 360, 390);
        setThree("linewidth", 345, 360, 390);
        setLength("marginparsep", 10 * 20);
        setLength("columnsep", 10 * 20);

    } else if (strcmp(option, "a4") == 0) {

        setLength("pageheight", 845 * 20);
        setLength("hoffset", 0 * 20);
        setThree("oddsidemargin", 40, 33, 14);
        setLength("headheight", 12 * 20);
        setThree("textheight", 646, 637, 621);
        setLength("footskip", 30 * 20);
        setLength("marginparpush", 5 * 20);

        setLength("pagewidth", 598 * 20);
        setLength("voffset", 0 * 20);
        setLength("topmargin", 0 * 20);
        setLength("headsep", 25 * 20);
        setThree("textwidth", 361, 376, 412);
        setThree("columnwidth", 361, 376, 412);
        setThree("linewidth", 361, 376, 412);
        setLength("marginparsep", 10 * 20);
        setLength("columnsep", 10 * 20);

    } else if (strcmp(option, "a4wide") == 0) {

        setLength("pageheight", 845 * 20);
        setLength("hoffset", 0 * 20);
        setThree("oddsidemargin", 18, 9, 0);
        setLength("headheight", 12 * 20);
        setThree("textheight", 621, 637, 621);
        setLength("footskip", 30 * 20);
        setLength("marginparpush", 5 * 20);

        setLength("pagewidth", 598 * 20);
        setLength("voffset", 0 * 20);
        setLength("topmargin", 0 * 20);
        setLength("headsep", 25 * 20);
        setThree("textwidth", 425, 443, 461);
        setThree("linewidth", 425, 443, 461);
        setThree("columnwidth", 425, 443, 461);
        setLength("marginparsep", 10 * 20);
        setLength("columnsep", 10 * 20);

    } else if (strcmp(option, "letterpaper") == 0) {

        setLength("pageheight", 795 * 20);
        setLength("hoffset", 0 * 20);
        setThree("oddsidemargin", 62, 54, 39);
        setLength("headheight", 12 * 20);
        setThree("textheight", 550, 541, 549);
        setLength("footskip", 30 * 20);
        setLength("marginparpush", 5 * 20);

        setLength("pagewidth", 614 * 20);
        setLength("voffset", 0 * 20);
        setLength("topmargin", 18 * 20);
        setLength("headsep", 25 * 20);
        setThree("textwidth", 345, 360, 390);
        setThree("linewidth", 345, 360, 390);
        setThree("columnwidth", 345, 360, 390);
        setLength("marginparsep", 10 * 20);
        setLength("columnsep", 10 * 20);

    } else if (strcmp(option, "legalpaper") == 0) {

        setLength("pageheight", 1012 * 20);
        setLength("hoffset", 0 * 20);
        setThree("oddsidemargin", 62, 54, 39);
        setLength("headheight", 12 * 20);
        setThree("textheight", 766, 759, 766);
        setLength("footskip", 30 * 20);
        setLength("marginparpush", 5 * 20);

        setLength("pagewidth", 614 * 20);
        setLength("voffset", 0 * 20);
        setLength("topmargin", 18 * 20);
        setLength("headsep", 25 * 20);
        setThree("textwidth", 345, 360, 390);
        setThree("columnwidth", 345, 360, 390);
        setThree("linewidth", 345, 360, 390);
        setLength("marginparsep", 10 * 20);
        setLength("columnsep", 10 * 20);

    } else if (strcmp(option, "a5paper") == 0) {

        setLength("pageheight", 598 * 20);
        setLength("hoffset", 0 * 20);
        setLength("oddsidemargin", 0 * 20);
        setLength("headheight", 12 * 20);
        setLength("textheight", 350 * 20);
        setLength("footskip", 30 * 20);
        setLength("marginparpush", 5 * 20);

        setLength("pagewidth", 421 * 20);
        setLength("voffset", 0 * 20);
        setLength("topmargin", 18 * 20);
        setLength("headsep", 25 * 20);
        setLength("textwidth", 276 * 20);
        setLength("columnwidth", 276 * 20);
        setLength("linewidth", 276 * 20);
        setLength("marginparsep", 10 * 20);
        setLength("columnsep", 10 * 20);

    } else if (strcmp(option, "b5paper") == 0) {

        setLength("pageheight", 711 * 20);
        setLength("hoffset", 0 * 20);
        setLength("oddsidemargin", 0 * 20);
        setLength("headheight", 12 * 20);
        setLength("textheight", 460 * 20);
        setLength("footskip", 30 * 20);
        setLength("marginparpush", 5 * 20);

        setLength("pagewidth", 501 * 20);
        setLength("voffset", 0 * 20);
        setLength("topmargin", 19 * 20);
        setLength("headsep", 25 * 20);
        setLength("textwidth", 350 * 20);
        setLength("columnwidth", 350 * 20);
        setLength("linewidth", 350 * 20);
        setLength("marginparsep", 10 * 20);
        setLength("columnsep", 10 * 20);
    }
}

static void setPointSize(char *option)
{
    if (strcmp(option, "10pt") == 0) {
        InitializeDocumentFont(-1, 20, -1, -1, ENCODING_1252);
        setLength("baselineskip", 12 * 20);
        setLength("parindent", 15 * 20);
        setLength("parskip", 0 * 20);

    } else if (strcmp(option, "11pt") == 0) {
        InitializeDocumentFont(-1, 22, -1, -1, ENCODING_1252);
        setLength("baselineskip", 14 * 20);
        setLength("parindent", 17 * 20);
        setLength("parskip", 0 * 20);

    } else {
        InitializeDocumentFont(-1, 24, -1, -1, ENCODING_1252);
        setLength("baselineskip", (int) 14.5 * 20);
        setLength("parindent", 18 * 20);
        setLength("parskip", 0 * 20);
    }
}


static void setDocumentOptions(char *optionlist)
{
    char *option;

    if (optionlist == NULL) return;
    
    option = strtok(optionlist, ",");

    while (option) {

/*      while (*option == ' ') option++;  skip leading blanks */
        diagnostics(2, " (setDocumentOptions) option <%s>", option);
        if (TryPackageIgnore(option)) {
            /* silently ignore */
        } else if (strcmp(option, "10pt") == 0 || strcmp(option, "11pt") == 0 || strcmp(option, "12pt") == 0)
            setPointSize(option);
        else if (strcmp(option, "a4"         ) == 0 ||
                 strcmp(option, "a4paper"    ) == 0 ||
                 strcmp(option, "a4wide"     ) == 0 ||
                 strcmp(option, "b5paper"    ) == 0 ||
                 strcmp(option, "a5paper"    ) == 0 ||
                 strcmp(option, "letterpaper") == 0 || 
                 strcmp(option, "landscape"  ) == 0 || 
                 strcmp(option, "legalpaper" ) == 0)
            setPaperSize(option);
        else if (  streq(option, "afrikaans")
                || streq(option, "bahasa")
                || streq(option, "basque")
                || streq(option, "brazil")
                || streq(option, "breton")
                || streq(option, "catalan")
                || streq(option, "croatian")
                || streq(option, "czech")
                || streq(option, "danish")
                || streq(option, "dutch")
                || streq(option, "english")
                || streq(option, "esperanto")
                || streq(option, "estonian")
                || streq(option, "finnish")
                || streq(option, "french")
                || streq(option, "galician")
                || streq(option, "german")
                || streq(option, "icelandic")
                || streq(option, "irish")
                || streq(option, "italian")
                || streq(option, "latin")
                || streq(option, "lsorbian")
                || streq(option, "magyar")
                || streq(option, "norsk")
                || streq(option, "nynorsk")
                || streq(option, "polish")
                || streq(option, "portuges")
                || streq(option, "romanian")
                || streq(option, "russian")
                || streq(option, "samin")
                || streq(option, "scottish")
                || streq(option, "serbian")
                || streq(option, "slovak")
                || streq(option, "slovene")
                || streq(option, "spanish")
                || streq(option, "swedish")
                || streq(option, "turkish")
                || streq(option, "ukrainian")
                || streq(option, "usorbian")
                || streq(option, "welsh") )
            setPackageBabel(option);
        else if (strcmp(option, "twoside") == 0)
            g_preambleTwoside = TRUE;
        else if (strcmp(option, "twocolumn") == 0)
            g_preambleTwocolumn = TRUE;
        else if (strcmp(option, "titlepage") == 0)
            g_preambleTitlepage = TRUE;
        else if (strcmp(option, "isolatin1") == 0)
            setPackageInputenc("latin1");
        else if (strcmp(option, "hyperlatex") == 0)
            PushEnvironment(HYPERLATEX_MODE);
        else if (strcmp(option, "apalike") == 0)
            g_document_bibstyle = BIBSTYLE_APALIKE;
        else if (strcmp(option, "apanat1b") == 0)
            PushEnvironment(APACITE_MODE);
        else if (strcmp(option, "endfloat") == 0)
            g_endfloat_figures = TRUE;
        else if (strcmp(option, "bibentry") == 0)
            /* do nothing */;
        else if (strcmp(option, "harvard") == 0) {
            PushEnvironment(HARVARD_MODE);
            g_document_bibstyle = BIBSTYLE_HARVARD;
            /* parse the .aux file for harvardcite's */
            /* if not already done so                */
            LoadAuxFile();
        } else if (strcmp(option, "natbib") == 0) {
            PushEnvironment(NATBIB_MODE);
            g_document_bibstyle = BIBSTYLE_NATBIB;
        } else if (strcmp(option, "authordate") == 0 ||
          strcmp(option, "authordate1") == 0 ||
          strcmp(option, "authordate2") == 0 ||
          strcmp(option, "authordate3") == 0 ||
          strcmp(option, "authordate4") == 0 || strcmp(option, "authordate1-4") == 0) {
            PushEnvironment(AUTHORDATE_MODE);
            g_document_bibstyle = BIBSTYLE_AUTHORDATE;
        } else if (strcmp(option, "apacite") == 0 || strcmp(option, "apacitex") == 0) {
            PushEnvironment(APACITE_MODE);
            g_document_bibstyle = BIBSTYLE_APACITE;
        } else if (strcmp(option, "hyperref") == 0) {
            PushEnvironment(HYPERREF_MODE);
        } else if (strcmp(option, "amsmath") == 0) {
            g_amsmath_package = TRUE;
            diagnostics(WARNING, "Incomplete support for package/option '%s' ", option);
        } else if (strcmp(option, "endnotes"    ) == 0 ||
                   strcmp(option, "bm"          ) == 0 ||
                   strcmp(option, "pstricks-add") == 0 ||
                   strcmp(option, "fancyhdr"    ) == 0 ||
                   strcmp(option, "html"        ) == 0 ||
                   strcmp(option, "epsf"        ) == 0 ||
                   strcmp(option, "psfig"       ) == 0 ||
                   strcmp(option, "verbatim"    ) == 0 ||
                   strcmp(option, "paralist"    ) == 0 ) {
            diagnostics(WARNING, "Incomplete support for package/option '%s' ", option);

        } 
        else if (strcmp(option, "color") == 0) {
            gColorPackage = 1;
        } else if (strcmp(option, "man") == 0 ||
                   strcmp(option, "jou") == 0) {
            diagnostics(WARNING, "ignoring [%s], assuming [doc]", option);
        } else if (strcmp(option, "doc") == 0) {
            diagnostics(WARNING, "Some support for apa class");
        } else if (strcmp(option, "setspace") == 0) {
            diagnostics(WARNING, "Incomplete support for setspace package");
        } else if (strcmp(option,"ifpdf") == 0) {
            diagnostics(WARNING, "Trivial support for the ifpdf package");
            ConvertString("\\newif\\ifpdf");
        } else if (strcmp(option,"tikz") == 0) {
            diagnostics(WARNING, "basic support for tikz");
        } else {
            diagnostics(WARNING, "Package/option '%s' unknown.", option);
        }
        option = strtok(NULL, ",");
    }
}

/******************************************************************************
 purpose: parse \documentstyle[options]{format} or \documentclass[options]{format}
 ******************************************************************************/
void CmdDocumentStyle(int code)
{
    char *format, *format_with_spaces;
    char *options, *options_with_spaces;

    options_with_spaces = getBracketParam();
    format_with_spaces = getBraceParam();

    format = strdup_noblanks(format_with_spaces);
    free(format_with_spaces);

    if (options_with_spaces)
        diagnostics(4, "Documentstyle/class[%s]{%s}", options_with_spaces, format);
    else
        diagnostics(4, "Documentstyle/class{%s}", format);

    g_document_type = FORMAT_ARTICLE;
    if (strcmp(format, "book") == 0)
        g_document_type = FORMAT_BOOK;

    else if (strcmp(format, "report") == 0)
        g_document_type = FORMAT_REPORT;

    else if (strcmp(format, "letter") == 0)
        g_document_type = FORMAT_LETTER;

    else if (strcmp(format, "article") == 0)
        g_document_type = FORMAT_ARTICLE;

    else if (strcmp(format, "slides") == 0)
        g_document_type = FORMAT_SLIDES;

    else if (strcmp(format, "apa") == 0) {
        g_document_type = FORMAT_APA;
        g_document_bibstyle = BIBSTYLE_APACITE;
        PushEnvironment(APACITE_MODE);
        diagnostics(WARNING, "Meager support for \\documentclass{apa}");
    } else
        diagnostics(WARNING, "Document format <%s> unknown, using article format", format);

    if (options_with_spaces) {
        options = strdup_noblanks(options_with_spaces);
        free(options_with_spaces);
        setDocumentOptions(options);
        free(options);
    }
    free(format);
}

/******************************************************************************
 purpose: handle \usepackage[option]{packagename}
******************************************************************************/
static void CmdUseOnepackage(char* package, char *options)
{
     diagnostics(2, "CmdUseOnepackage \\usepackage[%s]{%s}", options, package);

    if (TryPackageIgnore(package) == TRUE) 
        return;
    
    if (strcmp(package, "inputenc") == 0 && options)
        setPackageInputenc(options);

    else if (strcmp(package, "graphics") == 0)
        g_graphics_package = GRAPHICS_GRAPHICS;
    
    else if (strcmp(package, "graphicx") == 0)
        g_graphics_package = GRAPHICS_GRAPHICX;

    else if (strcmp(package, "isolatin1") == 0)
        setPackageInputenc("latin1");

    else if (strcmp(package, "spanish") == 0)
        setPackageBabel(package);

    else if (strcmp(package, "babel") == 0) {
        if (options && *options)
            setPackageBabel(options);

    } else if ( strcmp(package, "german")  == 0 ||
                strcmp(package, "ngerman") == 0 ||
                strcmp(package, "czech")   == 0 || 
                strcmp(package, "frenchb") == 0 || 
                strcmp(package, "french") == 0)
            setPackageBabel(package);

    else if (strcmp(package, "palatino") == 0 ||
         strcmp(package, "times") == 0 ||
         strcmp(package, "bookman") == 0 ||
         strcmp(package, "chancery") == 0 ||
         strcmp(package, "courier") == 0 ||
         strstr(package, "avant") || strstr(package, "newcen") || strstr(package, "helvet"))
        setPackageFont(package);

    else if (strcmp(package, "endfloat") == 0) {
        g_endfloat_figures = TRUE;
        g_endfloat_tables  = TRUE;
        if (options && strstr(options,"nomarkers")) g_endfloat_markers = FALSE;

    } else if (strcmp(package, "cite") == 0) {
        set_sorted_citations();
        set_compressed_citations();

    } else if (strcmp(package, "subfigure") == 0) {
        diagnostics(WARNING, "partial support for subfigure package");

    } else if (strcmp(package, "natbib") == 0) {
        if (options && strstr(options, "longnamesfirst"))
            set_longnamesfirst();
        if (options && strstr(options, "super"))
            set_bibpunct_style_super();
        if (options && strstr(options, "comma"))
            set_bibpunct_style_separator(",");
        if (options && strstr(options, "colon"))
            set_bibpunct_style_separator(":");
        if (options && strstr(options, "round"))
            set_bibpunct_style_paren("(",")");
        if (options && strstr(options, "square"))
            set_bibpunct_style_paren("[","]");
        if (options && strstr(options, "curly"))
            set_bibpunct_style_paren("{","}");
        if (options && strstr(options, "angle"))
            set_bibpunct_style_paren("<",">");
        if (options && strstr(options, "sort"))
            set_sorted_citations();
        if (options && strstr(options, "compress"))
            set_compressed_citations();
          
        PushEnvironment(NATBIB_MODE);
        g_document_bibstyle = BIBSTYLE_NATBIB;
    
    } else if (strcmp(package, "geometry") == 0) {

    /* Set default values for geometry package */        
        g_preambleGeometry = TRUE;
        if(g_preambleTwoside==FALSE) {
            g_geomMargr = (int) (getLength("pagewidth") * 0.15);
            g_geomMargl = g_geomMargr;
        } else {
            g_geomMargr = (int) (getLength("pagewidth") * 0.3 * 0.4);
            g_geomMargl = (int) (getLength("pagewidth") * 0.3 * 0.6);
        }
        g_geomMargt = (int) (getLength("pageheight") * 0.3 * 0.4);
        g_geomMargb = (int) (getLength("pageheight") * 0.3 * 0.6);
    
        if (options) {
            ParseOptGeometry(options);
        }
    } else if (strcmp(package,"acronym") == 0) {
        UsePackageAcronym(options);
    } else if (strcmp(package,"ifpdf") == 0) {
        ConvertString("\\newif\\ifpdf");
    } else {
        setDocumentOptions(package);
    }
}

/******************************************************************************
 purpose: collect options from \geometry{options} 
 ******************************************************************************/
void CmdGeometry(int code)
{
    char *options;
    options = getBraceParam();
    if (options) {
        diagnostics(2, "geometry command, argument %s\n", options);
        ParseOptGeometry(options);
        free(options);
    }
}



/**********************************************************************************
purpose: parse options from \usepackage[options]{geometry} and \geometry{options}
**********************************************************************************/
void ParseOptGeometry(char *options)
{
    char *key, *value1, *value2, *next, *comma = ", ", *colon = ": ";

    while (options) {
        next = keyvalue_pair(options,&key,&value1);
                
        if (value1 == NULL) {
            diagnostics(2, "geometry package, single option=[%s]", key);
            ExecGeomOptions (key, NULL, NULL);
        }
        else if (*value1 == '{') {
            PushSource(NULL, value1);
            free(value1);
            value1 = getBraceParam();
            PopSource();
            value1 = strtok(value1, comma);
            value2 = strtok(NULL, comma);
            diagnostics(2, "option=%s with values %s and %s", key, value1, value2);
            ExecGeomOptions (key, value1, value2);
            free(value1);
        }
        else if (strchr(value1, ':')) {
            value1 = strtok(value1, colon);
            value2 = strtok(NULL, colon);
            diagnostics(2, "option=%s with ratio '%s:%s'", key, value1, value2);
            ExecGeomOptions (key, value1, value2);
            free(value1);
        }
        else {
            diagnostics(2, "geometry package, option=[%s], value=%s", key, value1);
            value2=value1;
            ExecGeomOptions (key, value1, value2);
            free(value1);
        }
                
        if (key) free(key);
        options = next;
    }
}

/******************************************************************************
 purpose: implement geometry options 
******************************************************************************/

void ExecGeomOptions (char *key, char *value1, char *value2)
{
    int ratio_sum, margin_sum;
    char *value1b = NULL, *value2b = NULL;
    int dist1 = 0;
    int dist2 = 0;
    
    if (key == NULL) return;
    
    if (strstr(key, "ratio")) {
        if (strchr(value1, ':')) { /* each value is a ratio */
            value1 = strtok(value1, ": ");
            value1b = strtok(NULL, ": ");
            value2 = strtok(value2, ": ");
            value2b = strtok(NULL, ": ");
        } else { /* each value is part of a single ratio */
            dist1 = atoi(value1);
            dist2 = atoi(value2);
            diagnostics(3, "one ratio parameter, %d:%d", dist1, dist2);
        }
    } else if (strstr(key, "centering") == NULL) {
        dist1=getStringDimension(value1);
        dist2=getStringDimension(value2);
        diagnostics(3, "twips parameters, %d and %d", dist1, dist2);
    }

    if (strcmp(key, "vmargin") == 0) {
        diagnostics(3, "vmargin distance(top)=%d, distance (bottom)=%d twips", dist1, dist2);
        g_geomMargt = dist1;
        g_geomMargb = dist2;
    } else if (strcmp(key, "hmargin") == 0) {
        diagnostics(3, "hmargin distance(left)=%d, distance (right)=%d twips", dist1, dist2);
        g_geomMargl = dist1;
        g_geomMargr = dist2;
    } else if (strcmp(key, "margin") == 0) {
        ExecGeomOptions ("hmargin", value1, value2);
        ExecGeomOptions ("vmargin", value1, value2);
    } else if ((strcmp(key, "left") == 0) || (strcmp(key, "lmargin") == 0) || (strcmp(key, "inner") == 0) ) {
        g_geomMargl = dist1;
    } else if ((strcmp(key, "right") == 0) || (strcmp(key, "rmargin") == 0) || (strcmp(key, "outer") == 0) ) {
        g_geomMargr = dist1;
    } else if ((strcmp(key, "top") == 0) || (strcmp(key, "tmargin") == 0)) {
        g_geomMargt = dist1;
    } else if ((strcmp(key, "bottom") == 0) || (strcmp(key, "bmargin") == 0)) {
        g_geomMargb = dist1;
    } else if (strcmp(key, "hmarginratio") == 0) {
        ratio_sum = dist1 + dist2;
        margin_sum = g_geomMargl + g_geomMargr;
        g_geomMargl = (int) (((float) dist1 / (float) ratio_sum) * (float) margin_sum);
        diagnostics(3, "g_geomMargl %d", g_geomMargl);
        g_geomMargr = (int) (((float) dist2 / (float) ratio_sum) * (float) margin_sum);
        diagnostics(3, "g_geomMargr %d", g_geomMargr);
    } else if (strcmp(key, "vmarginratio") == 0) {
        ratio_sum = dist1 + dist2;
        margin_sum = g_geomMargt + g_geomMargb;
        g_geomMargt = (int) (((float) dist1 / (float) ratio_sum) * (float) margin_sum);
        g_geomMargb = (int) (((float) dist2 / (float) ratio_sum) * (float) margin_sum);
    } else if ((strcmp(key, "marginratio") == 0) || (strcmp(key, "ratio") == 0)) {
        ExecGeomOptions ("hmarginratio", value1, value1b);
        ExecGeomOptions ("vmarginratio", value2, value2b);
    } else if (strcmp(key, "hcentering") == 0) {
        ExecGeomOptions ("hmarginratio", "1", "1");
    } else if (strcmp(key, "vcentering") == 0) {
        ExecGeomOptions ("vmarginratio", "1", "1");
    } else if (strcmp(key, "centering") == 0) {
        ExecGeomOptions ("vmarginratio", "1", "1");
        ExecGeomOptions ("hmarginratio", "1", "1");
    }

}

/******************************************************************************
 purpose: handle \usepackage[option]{pack1,pack2,pack3}
******************************************************************************/
void CmdUsepackage(int code)
{
    char *package, *package_with_spaces;
    char *options, *options_with_spaces;
    char *p,*comma;
    
    options = NULL;
    options_with_spaces = getBracketParam();
    package_with_spaces = getBraceParam();
    package = strdup_noblanks(package_with_spaces);
    free(package_with_spaces);

    if (options_with_spaces) {
        options = strdup_noblanks(options_with_spaces);
        free(options_with_spaces);
        diagnostics(2, "Package {%s} with options [%s]", package, options);
    } else
        diagnostics(2, "Package {%s} with no options", package);
        
    /* process package names one at a time */
    p = package;
    do {
        comma = strchr(p,',');
        if (comma) *comma = '\0';   /* replace ',' by '\0' */
        CmdUseOnepackage(p,options);
        if (comma) p = comma+1;
    } while (comma != NULL);
    
    if (options) free(options);
    free(package);
}


void CmdTitle(int code)

/******************************************************************************
  purpose: saves title, author or date information
 ******************************************************************************/
{
    switch (code) {
        case TITLE_TITLE:
            g_preambleTitle = getBraceParam();
            UpdateLineNumber(g_preambleTitle);
            break;

        case TITLE_AUTHOR:
            g_preambleAuthor = getBraceParam();
            UpdateLineNumber(g_preambleAuthor);
            break;

        case TITLE_DATE:
            g_preambleDate = getBraceParam();
            UpdateLineNumber(g_preambleDate);
            break;

        case TITLE_AFFILIATION:
            g_preambleAffiliation = getBraceParam();
            UpdateLineNumber(g_preambleAffiliation);
            break;

        case TITLE_ABSTRACT:
            g_preambleAbstract = getBraceParam();
            UpdateLineNumber(g_preambleAbstract);
            break;

        case TITLE_ACKNOWLEDGE:
            g_preambleAck = getBraceParam();
            UpdateLineNumber(g_preambleAck);
            break;

        case TITLE_TITLEPAGE:
            g_preambleTitlepage = TRUE;
            break;
    }
}

void CmdTableOfContents(int code)
{
    startParagraph("contents", PARAGRAPH_SECTION_TITLE);
    fprintRTF(" ");
    ConvertBabelName("CONTENTSNAME");
    CmdEndParagraph(0);
    
    g_tableofcontents = TRUE;
    startParagraph("Normal", PARAGRAPH_GENERIC);
    CmdVspace(VSPACE_SMALL_SKIP);
    fprintRTF("{\\field{\\*\\fldinst TOC \\\\o \"1-3\" }{\\fldrslt }}\n");  
    CmdNewPage(NewPage);
    CmdEndParagraph(0);
}

/******************************************************************************
  purpose: Handle \and in \author{A \and B}
 ******************************************************************************/
void CmdAnd(int code)
{
    startParagraph("author", PARAGRAPH_GENERIC);
}


/******************************************************************************
  purpose: Creates a title page based on saved values for author, title, and date
 ******************************************************************************/
void CmdMakeTitle(int code)
{
    char title_begin[10];
    char author_begin[10];
    char date_begin[10];

    diagnostics(4,"CmdMakeTitle with  title ='%s'",g_preambleTitle);
    diagnostics(4,"CmdMakeTitle with author ='%s'",g_preambleAuthor);
    PushTrackLineNumber(FALSE);
    snprintf(title_begin, 10, "%s%2d", "\\fs", (30 * CurrentFontSize()) / 20);
    snprintf(author_begin, 10, "%s%2d", "\\fs", (24 * CurrentFontSize()) / 20);
    snprintf(date_begin, 10, "%s%2d", "\\fs", (24 * CurrentFontSize()) / 20);

    setAlignment(CENTERED);
    
    if (g_preambleTitle != NULL && strcmp(g_preambleTitle, "") != 0) {
        startParagraph("title", PARAGRAPH_GENERIC);
        ConvertString(g_preambleTitle);
    }
    
    if (g_preambleAuthor != NULL && strcmp(g_preambleAuthor, "") != 0) {
        startParagraph("author", PARAGRAPH_GENERIC);
        ConvertString(g_preambleAuthor);
    }

    if (g_preambleAffiliation != NULL && strcmp(g_preambleAffiliation, "") != 0) {
        startParagraph("author", PARAGRAPH_GENERIC);
        ConvertString(g_preambleAffiliation);
    }

    startParagraph("author", PARAGRAPH_GENERIC);
    if (g_preambleDate == NULL || strcmp(g_preambleDate, "") == 0) 
        fprintRTF("\\chdate ");
    else
        ConvertString(g_preambleDate);

    if (g_preambleAck != NULL && strcmp(g_preambleAck, "") != 0) {
        startParagraph("author", PARAGRAPH_GENERIC);
        ConvertString(g_preambleAck);
    }

    CmdEndParagraph(0);
    setAlignment(JUSTIFIED);

    if (g_preambleAbstract != NULL && strcmp(g_preambleAbstract, "") != 0) {
        char *s = strdup_together3("{",g_preambleAbstract,"}");
        CmdAbstract(ABSTRACT_PRELUDE_BEGIN);
        ConvertString("\\noindent");
        ConvertString(s);
        CmdAbstract(ABSTRACT_PRELUDE_END);
        free(s);
    }

    if (g_preambleTitlepage)
        fprintRTF("\\page ");

    if (g_document_type == FORMAT_APA)
        startParagraph("Normal", PARAGRAPH_FIRST);

    PopTrackLineNumber();
}

void CmdPreambleBeginEnd(int code)

/***************************************************************************
   purpose: catch missed \begin{document} command 
***************************************************************************/
{
    char *cParam = getBraceParam();

    if (strcmp(cParam, "document"))
        diagnostics(ERROR, "\\begin{%s} found before \\begin{document}.  Giving up.  Sorry", cParam);

    CallParamFunc(cParam, ON);
    free(cParam);
}

void PlainPagestyle(void)

/******************************************************************************
  LEG030598
  purpose: sets centered page numbering at bottom in rtf-output

  globals : pagenumbering set to TRUE if pagenumbering is to occur, default
 ******************************************************************************/
{
    int fn = DefaultFontFamily();

    pagenumbering = TRUE;

    if (g_preambleTwoside) {
        fprintRTF("\n{\\footerr");
        fprintRTF("\\pard\\plain\\f%d\\qc", fn);
        fprintRTF("{\\field{\\*\\fldinst PAGE}{\\fldrslt ?}}\\par}");
        fprintRTF("\n{\\footerl");
        fprintRTF("\\pard\\plain\\f%d\\qc", fn);
        fprintRTF("{\\field{\\*\\fldinst PAGE}{\\fldrslt ?}}\\par}");
    } else {
        fprintRTF("\n{\\footer");
        fprintRTF("\\pard\\plain\\f%d\\qc", fn);
        fprintRTF("{\\field{\\*\\fldinst PAGE}{\\fldrslt ?}}\\par}");
    }
}

void CmdPagestyle( /* @unused@ */ int code)

/******************************************************************************
 * LEG030598
 purpose: sets page numbering in rtf-output
 parameter:

 globals : headings  set to TRUE if the pagenumber is to go into the header
           pagenumbering set to TRUE if pagenumbering is to occur- default

Produces latex-like headers and footers.
Needs to be terminated for:
- headings chapter, section information and page numbering
- myheadings page nunmbering, combined with markboth, markright.
 ******************************************************************************/
{
    static char *style = "";

    style = getBraceParam();
    if (strcmp(style, "empty") == 0) {
        if (pagenumbering) {
            fprintRTF("{\\footer}");
            pagenumbering = FALSE;
        }
    } else if (strcmp(style, "plain") == 0)
        PlainPagestyle();
    else if (strcmp(style, "headings") == 0) {
        headings = TRUE;
        /* insert code to put section information in header, pagenumbering in header */
    } else if (strcmp(style, "myheadings") == 0) {
        headings = TRUE;

        /*--- insert code to put empty section information in header, will be
              provided by markboth, markright
              pagenumbering in header */
    } else {
        diagnostics(WARNING, "\\pagestyle{%s} unknown", style);
    }
}

void CmdHeader(int code)

/******************************************************************************
 purpose: converts the \markboth and \markright Command in Header information
 parameter: code: BOTH_SIDES, RIGHT_SIDE

 globals : twoside,
 ******************************************************************************/
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

void CmdThePage(int code)

/******************************************************************************
 purpose: handles \thepage in headers and footers
 ******************************************************************************/
{
    diagnostics(4, "CmdThePage");

    fprintRTF("\\chpgn ");
}

void RtfHeader(int where, char *what)

/******************************************************************************
  purpose: generates the header command in the rtf-output
  parameter: where: RIGHT_SIDE, LEFT_SIDE -handed page, BOTH_SIDES
           what:  NULL - Convert from LaTeX input, else put "what" into rtf
                  output
 ******************************************************************************/
{
    int fn = TexFontNumber("Roman");

    switch (where) {
        case RIGHT_SIDE:
            fprintRTF("\n{\\headerr \\pard\\plain\\f%d ", fn);
            break;
        case LEFT_SIDE:
            fprintRTF("\n{\\headerl \\pard\\plain\\f%d ", fn);
            break;
        case BOTH_SIDES:
            fprintRTF("\n{\\header \\pard\\plain\\f%d ", fn);
            break;
        default:
            diagnostics(ERROR, "\n error -> called RtfHeader with illegal parameter\n ");
    }
    if (what == NULL) {
        diagnostics(4, "Entering Convert() from RtfHeader");
        Convert();
        diagnostics(4, "Exiting Convert() from RtfHeader");
        fprintRTF("}");
    } else
        fprintRTF("%s}", what);
}


void CmdHyphenation(int code)

/******************************************************************************
 purpose: discard all hyphenation hints since they really only make sense when
          used with TeX's hyphenation algorithms 
 ******************************************************************************/
{
    char *hyphenparameter = getBraceParam();

    free(hyphenparameter);
}

static void WriteFontHeader(void)

/****************************************************************************
 *   purpose: writes fontnumbers and styles for headers into Rtf-File
 
 \fcharset0:    ANSI coding (codepage 1252)
 \fcharset1:    MAC coding
 \fcharset2:    PC coding   (codepage 437)
 \fcharset3:    PCA coding  (codepage 850)
 \fcharset204:  Cyrillic    (codepage 1251)
 \fcharset238:  E. European (codepage 852, 1250)
 ****************************************************************************/
{
    int num=0;
    ConfigEntryT **config_handle;
    char *font_name;

    fprintRTF("{\\fonttbl");

    config_handle = CfgStartIterate();
    while ((config_handle = CfgNextByInsertion(FONT_A, config_handle)) != NULL) {
        font_name = (char *) (*config_handle)->RtfCommand;
        fprintRTF("{\\f%d\\fnil\\fcharset0 %s;}\n", num, font_name);
        num++;
    }

    fprintRTF("}\n");
}

static void WriteStyleHeader(void)

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
    ConfigEntryT **style;
    const char *rtf;

    fprintRTF("{\\stylesheet\n");

    style = CfgStartIterate();
    while ((style = CfgNextByInsertion(STYLE_A, style)) != NULL) {
        rtf = (*style)->RtfCommand;
        diagnostics(5, "style <%s>=<%s>", (*style)->TexCommand, rtf);
        fprintRTF("{");
        InsertBasicStyle(rtf, INSERT_STYLE_FOR_HEADER);
        fprintRTF(";}\n");
    }
    fprintRTF("}\n");
}

static void WritePageSize(void)

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
  \ftnbj          Footnotes at bottom of page (bj = bottom justified)
  \aftnnar        Endnotes numbered in arabic numerals

  \headery<N>     The header is <N> twips from the top of the page (the default is 720).
  \footery<N>     The footer is <N> twips from the bottom of the page (the default is 720).
****************************************************************************/
{
    int n;
    int family = DefaultFontFamily();

    fprintRTF("\\paperw%d", getLength("pagewidth"));
    fprintRTF("\\paperh%d", getLength("pageheight"));
    if (g_preambleTwoside)
        fprintRTF("\\facingp");
    if (g_preambleLandscape)
        fprintRTF("\\landscape");
    if (g_preambleTwocolumn)
        fprintRTF("\\cols2\\colsx709"); /* two columns -- space between columns 709 */

    if(!g_preambleGeometry){
      n = getLength("hoffset") + 72 * 20 + getLength("oddsidemargin");
      fprintRTF("\\margl%d", n);
      diagnostics(4, "Writepagesize left margin   =%d pt", n / 20);
      n = getLength("pagewidth") - (n + getLength("textwidth"));
      fprintRTF("\\margr%d", n);
      diagnostics(4, "Writepagesize right margin  =%d pt", n / 20);
      n = getLength("voffset") + 72 * 20 + getLength("topmargin") + getLength("headheight") + getLength("headsep");
      fprintRTF("\\margt%d", n);
      diagnostics(4, "Writepagesize top    margin =%d pt", n / 20);
      n = getLength("pageheight") - (n + getLength("textheight") + getLength("footskip"));
      fprintRTF("\\margb%d", n);
      diagnostics(4, "Writepagesize bottom margin =%d pt", n / 20);
    } else {
      /* Insert geometry dimensions here */
      diagnostics(2, "Using geometry package");
      diagnostics(2, "[l,r,t,b] = [%d,%d,%d,%d]",g_geomMargl,g_geomMargr,
                                                       g_geomMargt,g_geomMargb);
      fprintRTF("\\margl%d", g_geomMargl);
      fprintRTF("\\margr%d", g_geomMargr);
      fprintRTF("\\margt%d", g_geomMargt);
      fprintRTF("\\margb%d", g_geomMargb);
    }

    fprintRTF("\\pgnstart%d", getCounter("page"));
    fprintRTF("\\widowctrl\\qj\\ftnbj\\f%d\\aftnnar\n", family);
}

static void WriteHeadFoot(void)
/****************************************************************************
  \headerl        The header is on left pages only.
  \headerr        The header is on right pages only.
  \headerf        The header is on the first page only.
  \footerl        The footer is on left pages only.
  \footerr        The footer is on right pages only.
  \footerf        The footer is on the first page only.
****************************************************************************/
{
    int family = DefaultFontFamily();
    int size   = DefaultFontSize(); 
    int textwidth = getLength("textwidth");

    if (g_preambleLFOOT || g_preambleCFOOT || g_preambleRFOOT) {
        fprintRTF("{\\footer\\pard\\plain\\tqc\\tx%d\\tqr\\tx%d ", textwidth / 2, textwidth);
        setTexMode(MODE_HORIZONTAL); 

        if (g_preambleLFOOT)
            ConvertString(g_preambleLFOOT);

        fprintRTF("\\tab\n");
        if (g_preambleCFOOT)
            ConvertString(g_preambleCFOOT);

        if (g_preambleRFOOT) {
            fprintRTF("\\tab\n");
            ConvertString(g_preambleRFOOT);
        }

        fprintRTF("\\par}\n");
    } else {
        fprintRTF("{\\footer\\pard\\plain\\f%d\\fs%d\\qc\\chpgn\\par}\n",family,size);
    }

    if (g_preambleLHEAD || g_preambleCHEAD || g_preambleRHEAD) {
        fprintRTF("{\\header\\pard\\plain\\tqc\\tx%d\\tqr\\tx%d ", textwidth / 2, textwidth);
        setTexMode(MODE_HORIZONTAL); 

        if (g_preambleLHEAD)
            ConvertString(g_preambleLHEAD);

        fprintRTF("\\tab\n");
        if (g_preambleCHEAD)
            ConvertString(g_preambleCHEAD);

        if (g_preambleRHEAD) {
            fprintRTF("\\tab\n");
            ConvertString(g_preambleRHEAD);
        }

        fprintRTF("\\par}\n");
    }
    setTexMode(MODE_VERTICAL);
}

void CmdHeadFoot(int code)

/******************************************************************************
 purpose: performs \cfoot, \lfoot, \rfoot, \chead, \lhead, \rhead commands (fancyhdr)
 adapted from code by Taupin in ltx2rtf
 ******************************************************************************/
{
    char *HeaderText = NULL;
    char *AlternText = NULL;

    AlternText = getBracketParam();
    if (AlternText)
        free(AlternText);

    HeaderText = getBraceParam();

    diagnostics(4, "CmdHeadFoot code=%d <%s>", code, HeaderText);
    switch (code) {
        case CFOOT:
            g_preambleCFOOT = HeaderText;
            break;
        case LFOOT:
            g_preambleLFOOT = HeaderText;
            break;
        case RFOOT:
            g_preambleRFOOT = HeaderText;
            break;
        case CHEAD:
            g_preambleCHEAD = HeaderText;
            break;
        case LHEAD:
            g_preambleLHEAD = HeaderText;
            break;
        case RHEAD:
            g_preambleRHEAD = HeaderText;
            break;
    }

    if (!g_processing_preamble)
        WriteHeadFoot();
}

static void WriteColorTable(void)

/****************************************************************************
     <colortbl>          '{' \colortbl <colordef>+ '}'
     <colordef>          \red ? & \green ? & \blue ? ';'
 ***************************************************************************/
{
    fprintRTF("{\\colortbl;\n");
    fprintRTF("\\red0\\green0\\blue0;\n");  /* black */
    fprintRTF("\\red0\\green0\\blue255;\n");    /* blue */
    fprintRTF("\\red0\\green255\\blue255;\n");  /* cyan */
    fprintRTF("\\red0\\green255\\blue0;\n");    /* green */
    fprintRTF("\\red255\\green0\\blue255;\n");  /* magenta */
    fprintRTF("\\red255\\green0\\blue0;\n");    /* red */
    fprintRTF("\\red255\\green255\\blue0;\n");  /* yellow */
    fprintRTF("\\red255\\green255\\blue255;\n");    /* white */
    
    if (!gColorPackage) {
        fprintRTF("}\n");
        return;
    }

    fprintRTF("\\red0\\green0\\blue128;\n");    /* dark blue */
    fprintRTF("\\red0\\green128\\blue128;\n");  /* dark cyan */
    fprintRTF("\\red0\\green128\\blue0;\n");    /* dark green */
    fprintRTF("\\red128\\green0\\blue128;\n");  /* dark magenta */
    fprintRTF("\\red128\\green0\\blue0;\n");    /* dark red */
    fprintRTF("\\red128\\green128\\blue0;\n");  /* dark yellow */
    fprintRTF("\\red128\\green128\\blue128;\n");    /* dark gray */
    fprintRTF("\\red192\\green192\\blue192;\n");    /* light gray */
    fprintRTF("\\red239\\green219\\blue197;\n");    /* Almond */    
    fprintRTF("\\red205\\green149\\blue117;\n");    /* Antique Brass */ 
    fprintRTF("\\red253\\green217\\blue181;\n");    /* Apricot */   
    fprintRTF("\\red120\\green219\\blue226;\n");    /* Aquamarine */    
    fprintRTF("\\red135\\green169\\blue107;\n");    /* Asparagus */ 
    fprintRTF("\\red255\\green164\\blue116;\n");    /* Atomic Tangerine */  
    fprintRTF("\\red250\\green231\\blue181;\n");    /* Banana Mania */  
    fprintRTF("\\red159\\green129\\blue112;\n");    /* Beaver */    
    fprintRTF("\\red253\\green124\\blue110;\n");    /* Bittersweet */   
    fprintRTF("\\red35\\green35\\blue35;\n");   /* Black */ 
    fprintRTF("\\red31\\green117\\blue254;\n"); /* Blue */  
    fprintRTF("\\red173\\green173\\blue214;\n");    /* Blue Bell */ 
    fprintRTF("\\red25\\green158\\blue189;\n"); /* Blue Green */    
    fprintRTF("\\red115\\green102\\blue189;\n");    /* Blue Violet */   
    fprintRTF("\\red222\\green93\\blue131;\n"); /* Blush */ 
    fprintRTF("\\red203\\green65\\blue84;\n");  /* Brick Red */ 
    fprintRTF("\\red180\\green103\\blue77;\n"); /* Brown */ 
    fprintRTF("\\red255\\green127\\blue73;\n"); /* Burnt Orange */  
    fprintRTF("\\red234\\green126\\blue93;\n"); /* Burnt Sienna */  
    fprintRTF("\\red176\\green183\\blue198;\n");    /* Cadet Blue */    
    fprintRTF("\\red255\\green255\\blue153;\n");    /* Canary */    
    fprintRTF("\\red28\\green211\\blue162;\n"); /* Caribbean Green */   
    fprintRTF("\\red255\\green170\\blue204;\n");    /* Carnation Pink */    
    fprintRTF("\\red221\\green68\\blue146;\n"); /* Cerise */    
    fprintRTF("\\red29\\green172\\blue214;\n"); /* Cerulean */  
    fprintRTF("\\red188\\green93\\blue88;\n");  /* Chestnut */  
    fprintRTF("\\red221\\green148\\blue117;\n");    /* Copper */    
    fprintRTF("\\red154\\green206\\blue235;\n");    /* Cornflower */    
    fprintRTF("\\red255\\green188\\blue217;\n");    /* Cotton Candy */  
    fprintRTF("\\red253\\green219\\blue109;\n");    /* Dandelion */ 
    fprintRTF("\\red43\\green108\\blue196;\n"); /* Denim */ 
    fprintRTF("\\red239\\green205\\blue184;\n");    /* Desert Sand */   
    fprintRTF("\\red110\\green81\\blue96;\n");  /* Eggplant */  
    fprintRTF("\\red29\\green249\\blue20;\n");  /* Electric Lime */ 
    fprintRTF("\\red113\\green188\\blue120;\n");    /* Fern */  
    fprintRTF("\\red109\\green174\\blue129;\n");    /* Forest Green */  
    fprintRTF("\\red195\\green100\\blue197;\n");    /* Fuchsia */   
    fprintRTF("\\red204\\green102\\blue102;\n");    /* Fuzzy Wuzzy Brown */ 
    fprintRTF("\\red231\\green198\\blue151;\n");    /* Gold */  
    fprintRTF("\\red252\\green217\\blue117;\n");    /* Goldenrod */ 
    fprintRTF("\\red168\\green228\\blue160;\n");    /* Granny Smith Apple */    
    fprintRTF("\\red149\\green145\\blue140;\n");    /* Gray */  
    fprintRTF("\\red28\\green172\\blue120;\n"); /* Green */ 
    fprintRTF("\\red240\\green232\\blue145;\n");    /* Green Yellow */  
    fprintRTF("\\red255\\green29\\blue206;\n"); /* Hot Magenta */   
    fprintRTF("\\red178\\green236\\blue93;\n"); /* Inch Worm */ 
    fprintRTF("\\red93\\green118\\blue203;\n"); /* Indigo */    
    fprintRTF("\\red202\\green55\\blue103;\n"); /* Jazzberry Jam */ 
    fprintRTF("\\red59\\green176\\blue143;\n"); /* Jungle Green */  
    fprintRTF("\\red253\\green252\\blue116;\n");    /* Laser Lemon */   
    fprintRTF("\\red252\\green180\\blue213;\n");    /* Lavender */  
    fprintRTF("\\red255\\green189\\blue136;\n");    /* Macaroni and Cheese */   
    fprintRTF("\\red246\\green100\\blue175;\n");    /* Magenta */   
    fprintRTF("\\red205\\green74\\blue74;\n");  /* Mahogany */  
    fprintRTF("\\red151\\green154\\blue170;\n");    /* Manatee */   
    fprintRTF("\\red255\\green130\\blue67;\n"); /* Mango Tango */   
    fprintRTF("\\red200\\green56\\blue90;\n");  /* Maroon */    
    fprintRTF("\\red239\\green152\\blue170;\n");    /* Mauvelous */ 
    fprintRTF("\\red253\\green188\\blue180;\n");    /* Melon */ 
    fprintRTF("\\red26\\green72\\blue118;\n");  /* Midnight Blue */ 
    fprintRTF("\\red48\\green186\\blue143;\n"); /* Mountain Meadow */   
    fprintRTF("\\red25\\green116\\blue210;\n"); /* Navy Blue */ 
    fprintRTF("\\red255\\green163\\blue67;\n"); /* Neon Carrot */   
    fprintRTF("\\red186\\green184\\blue108;\n");    /* Olive Green */   
    fprintRTF("\\red255\\green117\\blue56;\n"); /* Orange */    
    fprintRTF("\\red230\\green168\\blue215;\n");    /* Orchid */    
    fprintRTF("\\red65\\green74\\blue76;\n");   /* Outer Space */   
    fprintRTF("\\red255\\green110\\blue74;\n"); /* Outrageous Orange */ 
    fprintRTF("\\red28\\green169\\blue201;\n"); /* Pacific Blue */  
    fprintRTF("\\red255\\green207\\blue171;\n");    /* Peach */ 
    fprintRTF("\\red197\\green208\\blue230;\n");    /* Periwinkle */    
    fprintRTF("\\red253\\green215\\blue228;\n");    /* Piggy Pink */    
    fprintRTF("\\red21\\green128\\blue120;\n"); /* Pine Green */    
    fprintRTF("\\red252\\green116\\blue253;\n");    /* Pink Flamingo */ 
    fprintRTF("\\red247\\green128\\blue161;\n");    /* Pink Sherbet */  
    fprintRTF("\\red142\\green69\\blue133;\n"); /* Plum */  
    fprintRTF("\\red116\\green66\\blue200;\n"); /* Purple Heart */  
    fprintRTF("\\red157\\green129\\blue186;\n");    /* Purple Mountains’ Majesty */ 
    fprintRTF("\\red255\\green29\\blue206;\n"); /* Purple Pizzazz */    
    fprintRTF("\\red255\\green73\\blue108;\n"); /* Radical Red */   
    fprintRTF("\\red214\\green138\\blue89;\n"); /* Raw Sienna */    
    fprintRTF("\\red255\\green72\\blue208;\n"); /* Razzle Dazzle Rose */    
    fprintRTF("\\red227\\green37\\blue107;\n"); /* Razzmatazz */    
    fprintRTF("\\red238\\green32\\blue77;\n");  /* Red */   
    fprintRTF("\\red255\\green83\\blue73;\n");  /* Red Orange */    
    fprintRTF("\\red192\\green68\\blue143;\n"); /* Red Violet */    
    fprintRTF("\\red31\\green206\\blue203;\n"); /* Robin Egg Blue */    
    fprintRTF("\\red120\\green81\\blue169;\n"); /* Royal Purple */  
    fprintRTF("\\red255\\green155\\blue170;\n");    /* Salmon */    
    fprintRTF("\\red252\\green40\\blue71;\n");  /* Scarlet */   
    fprintRTF("\\red118\\green255\\blue122;\n");    /* Screamin Green */    
    fprintRTF("\\red159\\green226\\blue191;\n");    /* Sea Green */ 
    fprintRTF("\\red165\\green105\\blue79;\n"); /* Sepia */ 
    fprintRTF("\\red138\\green121\\blue93;\n"); /* Shadow */    
    fprintRTF("\\red69\\green206\\blue162;\n"); /* Shamrock */  
    fprintRTF("\\red251\\green126\\blue253;\n");    /* Shocking Pink */ 
    fprintRTF("\\red205\\green197\\blue194;\n");    /* Silver */    
    fprintRTF("\\red128\\green218\\blue235;\n");    /* Sky Blue */  
    fprintRTF("\\red236\\green234\\blue190;\n");    /* Spring Green */  
    fprintRTF("\\red255\\green207\\blue72;\n"); /* Sunglow */   
    fprintRTF("\\red253\\green94\\blue83;\n");  /* Sunset Orange */ 
    fprintRTF("\\red250\\green167\\blue108;\n");    /* Tan */   
    fprintRTF("\\red252\\green137\\blue172;\n");    /* Tickle Me Pink */    
    fprintRTF("\\red219\\green215\\blue210;\n");    /* Timberwolf */    
    fprintRTF("\\red23\\green128\\blue109;\n"); /* Tropical Rain Forest */  
    fprintRTF("\\red222\\green170\\blue136;\n");    /* Tumbleweed */    
    fprintRTF("\\red119\\green221\\blue231;\n");    /* Turquoise Blue */    
    fprintRTF("\\red253\\green252\\blue116;\n");    /* Unmellow Yellow */   
    fprintRTF("\\red146\\green110\\blue174;\n");    /* Violet (Purple) */   
    fprintRTF("\\red247\\green83\\blue148;\n"); /* Violet Red */    
    fprintRTF("\\red255\\green160\\blue137;\n");    /* Vivid Tangerine */   
    fprintRTF("\\red143\\green80\\blue157;\n"); /* Vivid Violet */  
    fprintRTF("\\red237\\green237\\blue237;\n");    /* White */ 
    fprintRTF("\\red162\\green173\\blue208;\n");    /* Wild Blue Yonder */  
    fprintRTF("\\red255\\green67\\blue164;\n"); /* Wild Strawberry */   
    fprintRTF("\\red252\\green108\\blue133;\n");    /* Wild Watermelon */   
    fprintRTF("\\red205\\green164\\blue222;\n");    /* Wisteria */  
    fprintRTF("\\red252\\green232\\blue131;\n");    /* Yellow */    
    fprintRTF("\\red197\\green227\\blue132;\n");    /* Yellow Green */  
    fprintRTF("\\red255\\green182\\blue83;\n"); /* Yellow Orange */   
    
    fprintRTF("}\n");
}

static void WriteInfo(void)

/****************************************************************************
  \title          The title of the document
  \subject        The subject of the document
  \author         The author of the document
  \operator       The person who last made changes to the document
  \keywords       Selected keywords for the document
  \comment        Comments; text is ignored
  \version<N>     The version number of the document
  \doccomm        Comments displayed in Word's Summary Info dialog
  
{\info {\title This is a page} {\doccomm Converted using}}
 ***************************************************************************/
{
/*  struct stat sb;*/
    time_t tm;

    fprintRTF("{\\info\n{\\title Original file was %s}\n",CurrentFileName());
    tm = time(NULL);
    fprintRTF("{\\doccomm Created using latex2rtf %s on %s}\n", Version, ctime(&tm));
/*  if (fstat(CurrentFileDescriptor(),&sb)) {} */
/*  fprintRTF("{\\creatim %s}\n", ctime(&tm)); */
    fprintRTF("}\n");
}

void WriteRtfHeader(void)

/****************************************************************************
purpose: writes header info for the RTF file

\rtf1 <charset> \deff? <fonttbl> <filetbl>? <colortbl>? <stylesheet>? <listtables>? <revtbl>?
 ****************************************************************************/
{
    int family = DefaultFontFamily();

/*  int size   = DefaultFontSize(); */

    diagnostics(4, "Writing header for RTF file");

/*  fprintRTF("{\\rtf1\\ansi\\fs%d\\deff%d\\deflang1024\n", size, family); */
    fprintRTF("{\\rtf1\\ansi\\uc1\\deff%d\\deflang1024\n", family);
    WriteFontHeader();
    WriteColorTable();
    WriteStyleHeader();
    WriteInfo();
    WriteHeadFoot();
    WritePageSize();
}
