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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

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

extern char *Version;  /*storage and definition in version.h */

static bool g_preambleTwoside = FALSE;
static bool g_preambleTwocolumn = FALSE;
static bool g_preambleTitlepage = FALSE;
static bool g_preambleLandscape = FALSE;
static bool g_preambleGeometry = FALSE;

static int g_geomMargl = 0;
static int g_geomMargr = 0;
static int g_geomMargt = 0;
static int g_geomMargb = 0;

static char *g_preambleTitle = NULL;
static char *g_preambleAuthor = NULL;
static char *g_preambleDate = NULL;
static char *g_preambleEncoding = NULL;
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
    if (strcmp(option, "german") == 0 || strcmp(option, "ngerman") == 0) {
        GermanMode = TRUE;
        PushEnvironment(GERMAN_MODE);
        ReadLanguage("german");
    }

    if (strcmp(option, "french") == 0 || strcmp(option, "frenchb") == 0) {
        FrenchMode = TRUE;
        PushEnvironment(FRENCH_MODE);
        ReadLanguage("french");
    }

    if (strcmp(option, "russian") == 0) {
        RussianMode = TRUE;
        PushEnvironment(RUSSIAN_MODE);
        ReadLanguage("russian");
    }

    if (strcmp(option, "spanish") == 0) {
        ReadLanguage("spanish");
    }

    if (strcmp(option, "czech") == 0) {
        CzechMode = TRUE;
        PushEnvironment(CZECH_MODE);
        ReadLanguage("czech");
        g_fcharset_number = 238;    /* East European in RTF Specification */
        my_strlcpy(g_charset_encoding_name, "raw", 20);
    }

}

void setPackageInputenc(char *option)
{
    g_preambleEncoding = strdup_noblanks(option);

    if (strcmp(option, "ansinew") == 0)
        my_strlcpy(g_charset_encoding_name, "cp1252", 20);

    else if (strcmp(option, "applemac") == 0 ||
      strcmp(option, "decmulti") == 0 ||
      strcmp(option, "latin1") == 0 ||
      strcmp(option, "latin2") == 0 ||
      strcmp(option, "latin3") == 0 ||
      strcmp(option, "latin4") == 0 ||
      strcmp(option, "latin5") == 0 ||
      strcmp(option, "latin9") == 0 ||
      strcmp(option, "next") == 0 ||
      strcmp(option, "cp437") == 0 ||
      strcmp(option, "cp437de") == 0 ||
      strcmp(option, "cp850") == 0 ||
      strcmp(option, "cp852") == 0 ||
      strcmp(option, "cp855") == 0 ||
      strcmp(option, "cp865") == 0 ||
      strcmp(option, "cp866") == 0 ||
      strcmp(option, "cp1250") == 0 ||
      strcmp(option, "cp1251") == 0 ||
      strcmp(option, "cp1252") == 0 ||
      strcmp(option, "437") == 0 ||
      strcmp(option, "437de") == 0 ||
      strcmp(option, "850") == 0 ||
      strcmp(option, "852") == 0 ||
      strcmp(option, "855") == 0 ||
      strcmp(option, "865") == 0 ||
      strcmp(option, "866") == 0 ||
      strcmp(option, "1250") == 0 ||
      strcmp(option, "1251") == 0 ||
      strcmp(option, "1252") == 0 ||
      strcmp(option, "maccyr") == 0 ||
      strcmp(option, "macukr") == 0 || 
      strcmp(option, "koi8-r") == 0 || 
      strcmp(option, "koi8-u") == 0) {

        my_strlcpy(g_charset_encoding_name, option, 20);
        g_fcharset_number = 0;  /* ANSI in RTF Specification */

    } else if (strcmp(option, "raw") == 0) {
        my_strlcpy(g_charset_encoding_name, "raw", 20);
        g_fcharset_number = 255;    /* OEM in RTF Specification */

    } else if (strcmp(option, "raw437") == 0) {
        g_fcharset_number = 254;    /* IBM PC in RTF Specification */
        my_strlcpy(g_charset_encoding_name, "raw", 20);

    } else if (strcmp(option, "raw852") == 0) {
        g_fcharset_number = 255;    /* Microsoft bug ... */
        my_strlcpy(g_charset_encoding_name, "raw", 20);

    } else if (strcmp(option, "raw1250") == 0) {
        g_fcharset_number = 238;    /* East European in RTF Specification */
        my_strlcpy(g_charset_encoding_name, "raw", 20);

    } else if (strcmp(option, "raw1251") == 0) {
        g_fcharset_number = 204;    /* Cyrillic in RTF Specification */
        my_strlcpy(g_charset_encoding_name, "raw", 20);

    } else if (strcmp(option, "raw1253") == 0) {
        g_fcharset_number = 161;    /* Greek in RTF Specification */
        my_strlcpy(g_charset_encoding_name, "raw", 20);
        
    } else if (strcmp(option, "utf8") == 0) {
        diagnostics(WARNING, "Input Encoding utf8 - experimental support");
        my_strlcpy(g_charset_encoding_name, "utf8", 20);

    } else if (strcmp(option, "utf8x") == 0) {
        diagnostics(WARNING, "Input Encoding utf8x - experimental support");
        my_strlcpy(g_charset_encoding_name, "utf8", 20);

    } else
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
        setLength("marginparsep", 10 * 20);
        setLength("columnsep", 10 * 20);
    }
}

static void setPointSize(char *option)
{
    if (strcmp(option, "10pt") == 0) {
        InitializeDocumentFont(-1, 20, -1, -1);
        setLength("baselineskip", 12 * 20);
        setLength("parindent", 15 * 20);
        setLength("parskip", 0 * 20);

    } else if (strcmp(option, "11pt") == 0) {
        InitializeDocumentFont(-1, 22, -1, -1);
        setLength("baselineskip", 14 * 20);
        setLength("parindent", 17 * 20);
        setLength("parskip", 0 * 20);

    } else {
        InitializeDocumentFont(-1, 24, -1, -1);
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

/*		while (*option == ' ') option++;  skip leading blanks */
        diagnostics(2, " (setDocumentOptions) option <%s>", option);
        if (strcmp(option, "10pt") == 0 || strcmp(option, "11pt") == 0 || strcmp(option, "12pt") == 0)
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
        else if (strcmp(option, "german" ) == 0 ||
                 strcmp(option, "ngerman") == 0 ||
                 strcmp(option, "spanish") == 0 ||
                 strcmp(option, "english") == 0 ||
                 strcmp(option, "russian") == 0 ||
                 strcmp(option, "czech"  ) == 0 || 
                 strcmp(option, "frenchb") == 0 || 
                 strcmp(option, "french") == 0)
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
                   strcmp(option, "pstricks-add") == 0 ||
                   strcmp(option, "fancyhdr"    ) == 0 ||
                   strcmp(option, "html"        ) == 0 ||
                   strcmp(option, "epsf"        ) == 0 ||
                   strcmp(option, "psfig"       ) == 0 ||
                   strcmp(option, "verbatim"    ) == 0 ||
                   strcmp(option, "paralist"    ) == 0 ) {
            diagnostics(WARNING, "Incomplete support for package/option '%s' ", option);

        } else if (strcmp(option, "textcomp"    ) == 0 || 
                   strcmp(option, "fontenc"     ) == 0 ||
                   strcmp(option, "eurosym"     ) == 0 ||
                   strcmp(option, "ucs"         ) == 0 ||
                   strcmp(option, "alltt"       ) == 0 ||
                   strcmp(option, "url"         ) == 0 ||
                   strcmp(option, "nameref"     ) == 0 ||
                   strcmp(option, "amssymb"     ) == 0) {
            /* do nothing ... but don't complain */
        } else if (strcmp(option, "color") == 0) {
            diagnostics(WARNING, "Color support limited to eight basic colors");
        } else if (strcmp(option, "man") == 0 ||
                   strcmp(option, "jou") == 0) {
            diagnostics(WARNING, "ignoring [%s], assuming [doc]", option);
        } else if (strcmp(option, "doc") == 0) {
            diagnostics(WARNING, "Some support for apa class");
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
     diagnostics(4, "CmdUseOnepackage \\usepackage[%s]{%s}", options, package);

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
		if (options)
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
			g_geomMargr = (getLength("pagewidth") * 0.15);
			g_geomMargl = g_geomMargr;
		} else {
			g_geomMargr = (getLength("pagewidth") * 0.3 * 0.4);
			g_geomMargl = (getLength("pagewidth") * 0.3 * 0.6);
		}
        g_geomMargt = (getLength("pageheight") * 0.3 * 0.4);
        g_geomMargb = (getLength("pageheight") * 0.3 * 0.6);
	
        if (options) {
	    	ParseOptGeometry(options);
	    }
	
    } else
		setDocumentOptions(package);
	  
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
    int dist1, dist2, dist3, dist4, ratio_sum, margin_sum;
    char *value1b = NULL, *value2b = NULL;

    if (strstr(key, "ratio")) {
	if (strchr(value1, ':')) { /* each value is a ratio */
	    value1 = strtok(value1, ": ");
	    value1b = strtok(NULL, ": ");
	    value2 = strtok(value2, ": ");
	    value2b = strtok(NULL, ": ");
	} else { /* each value is part of a single ratio */
	    dist1 = dist3 = atoi(value1);
	    dist2 = dist4 = atoi(value2);
	    diagnostics(3, "one ratio parameter, %d:%d", dist1, dist2);
	}
    } else if (strstr(key, "centering") == NULL) {
		dist1=getStringDimension(value1);
		dist2=getStringDimension(value2);
		diagnostics(3, "twips paramters, %d and %d", dist1, dist2);
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
    	if (comma) *comma = '\0';	/* replace ',' by '\0' */
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
	startParagraph("contents", SECTION_TITLE_PARAGRAPH);
	fprintRTF(" ");
	ConvertBabelName("CONTENTSNAME");
	CmdEndParagraph(0);
	
	g_tableofcontents = TRUE;
	startParagraph("Normal", GENERIC_PARAGRAPH);
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
	startParagraph("author", GENERIC_PARAGRAPH);
}


/******************************************************************************
  purpose: Creates a title page based on saved values for author, title, and date
 ******************************************************************************/
void CmdMakeTitle(int code)
{
    char title_begin[10];
    char author_begin[10];
    char date_begin[10];

    PushTrackLineNumber(FALSE);
    snprintf(title_begin, 10, "%s%2d", "\\fs", (30 * CurrentFontSize()) / 20);
    snprintf(author_begin, 10, "%s%2d", "\\fs", (24 * CurrentFontSize()) / 20);
    snprintf(date_begin, 10, "%s%2d", "\\fs", (24 * CurrentFontSize()) / 20);

    setAlignment(CENTERED);
    fprintRTF("\n\\par\\pard\\qc {%s ", title_begin);
    if (g_preambleTitle != NULL && strcmp(g_preambleTitle, "") != 0)
        ConvertString(g_preambleTitle);
    fprintRTF("}");

    fprintRTF("\n\\par\\pard\\qc {%s ", author_begin);
    if (g_preambleAuthor != NULL && strcmp(g_preambleAuthor, "") != 0)
        ConvertString(g_preambleAuthor);
    fprintRTF("}");

    fprintRTF("\n\\par\\pard\\qc {%s ", date_begin);
    if (g_preambleAffiliation != NULL && strcmp(g_preambleAffiliation, "") != 0)
        ConvertString(g_preambleAffiliation);
    fprintRTF("}");

    fprintRTF("\n\\par\\pard\\qc {%s ", date_begin);
    if (g_preambleDate != NULL && strcmp(g_preambleDate, "") != 0)
        ConvertString(g_preambleDate);
    fprintRTF("}");

    fprintRTF("\n\\par\\pard\\qc {%s ", date_begin);
    if (g_preambleAck != NULL && strcmp(g_preambleAck, "") != 0)
        ConvertString(g_preambleAck);
    fprintRTF("}");

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
    	startParagraph("Normal",FIRST_PARAGRAPH);

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
    int i;
    ConfigEntryT **config_handle;
    char *font_type, *font_name;
    int charset;

    fprintRTF("{\\fonttbl");

    config_handle = CfgStartIterate(FONT_A);
    i = 0;
    while ((config_handle = CfgNextByInsertion(FONT_A, config_handle)) != NULL) {

        font_type = (char *) (*config_handle)->TexCommand;
        font_name = (char *) (*config_handle)->RtfCommand;
        charset = g_fcharset_number;

        if (strncmp(font_name, "Symbol", 6) == 0)
            charset = 2;

        if (strncmp(font_type, "Cyrillic", 8) == 0)
            charset = 204;

        if (strncmp(font_type, "Latin2", 6) == 0)
            charset = 238;

        fprintRTF("{\\f%d\\fnil\\fcharset%d %s;}\n", i, charset, font_name);

        i++;
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

    style = CfgStartIterate(STYLE_A);
    while ((style = CfgNextByInsertion(STYLE_A, style)) != NULL) {
        rtf = (*style)->RtfCommand;
        diagnostics(5, "style <%s>=<%s>", (*style)->TexCommand, rtf);
        fprintRTF("{");
        InsertBasicStyle(rtf, TRUE);
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

/*	fprintRTF("\\ftnbj\\sectd\\linex0\\endnhere\\qj\n"); */

    int family = DefaultFontFamily();
	int size   = DefaultFontSize(); 
    int textwidth = getLength("textwidth");

    if (g_preambleLFOOT || g_preambleCFOOT || g_preambleRFOOT) {
        fprintRTF("{\\footer\\pard\\plain\\tqc\\tx%d\\tqr\\tx%d ", textwidth / 2, textwidth);

        if (g_preambleLFOOT)
            ConvertString(g_preambleLFOOT);

        fprintRTF("\\tab ");
        if (g_preambleCFOOT)
            ConvertString(g_preambleCFOOT);

        if (g_preambleRFOOT) {
            fprintRTF("\\tab ");
            ConvertString(g_preambleRFOOT);
        }

        fprintRTF("\\par}\n");
    } else {
		fprintRTF("{\\footer\\pard\\plain\\f%d\\fs%d\\qc\\chpgn\\par}\n",family,size);
    }

    if (g_preambleLHEAD || g_preambleCHEAD || g_preambleRHEAD) {
        fprintRTF("{\\header\\pard\\plain\\tqc\\tx%d\\tqr\\tx%d ", textwidth / 2, textwidth);

        if (g_preambleLHEAD)
            ConvertString(g_preambleLHEAD);

        fprintRTF("\\tab ");
        if (g_preambleCHEAD)
            ConvertString(g_preambleCHEAD);

        if (g_preambleRHEAD) {
            fprintRTF("\\tab ");
            ConvertString(g_preambleRHEAD);
        }

        fprintRTF("\\par}\n");
    }
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
    fprintRTF("\\red0\\green0\\blue128;\n");    /* dark blue */
    fprintRTF("\\red0\\green128\\blue128;\n");  /* dark cyan */
    fprintRTF("\\red0\\green128\\blue0;\n");    /* dark green */
    fprintRTF("\\red128\\green0\\blue128;\n");  /* dark magenta */
    fprintRTF("\\red128\\green0\\blue0;\n");    /* dark red */
    fprintRTF("\\red128\\green128\\blue0;\n");  /* dark yellow */
    fprintRTF("\\red128\\green128\\blue128;\n");    /* dark gray */
    fprintRTF("\\red192\\green192\\blue192;\n");    /* light gray */
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
/*	struct stat sb;*/
	time_t tm;

	fprintRTF("{\\info\n{\\title Original file was %s}\n",CurrentFileName());
	tm = time(NULL);
	fprintRTF("{\\doccomm Created using latex2rtf %s on %s}\n", Version, ctime(&tm));
/*	if (fstat(CurrentFileDescriptor(),&sb)) {} */
/*	fprintRTF("{\\creatim %s}\n", ctime(&tm)); */
	fprintRTF("}\n");
}

void WriteRtfHeader(void)

/****************************************************************************
purpose: writes header info for the RTF file

\rtf1 <charset> \deff? <fonttbl> <filetbl>? <colortbl>? <stylesheet>? <listtables>? <revtbl>?
 ****************************************************************************/
{
    int family = DefaultFontFamily();

/*	int size   = DefaultFontSize(); */

    diagnostics(4, "Writing header for RTF file");

/*	fprintRTF("{\\rtf1\\ansi\\fs%d\\deff%d\\deflang1024\n", size, family); */
    fprintRTF("{\\rtf1\\ansi\\uc1\\deff%d\\deflang1024\n", family);
    WriteFontHeader();
    WriteColorTable();
    WriteStyleHeader();
    WriteInfo();
    WriteHeadFoot();
    WritePageSize();
}
