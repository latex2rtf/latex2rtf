
/* chars.c - Handle special TeX characters and logos

Copyright (C) 2002 The Free Software Foundation

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

*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "commands.h"
#include "fonts.h"
#include "cfg.h"
#include "ignore.h"
#include "encode.h"
#include "parser.h"
#include "chars.h"
#include "funct1.h"
#include "convert.h"

void TeXlogo();
void LaTeXlogo();

/* create overstrike character using the \O (overstrike) field */
/* in Word.  Only do this when in a formula, or if a unicode variant */
/* (see above) is not available */
void putOverstrikeChar(const char *font, char *s, unsigned int overstrike)
{
	if (g_processing_fields==0) fprintRTF("{\\field{\\*\\fldinst EQ ");
		
	fprintRTF("\\\\O(");
    ConvertString(s);
	fprintRTF("%c", g_field_separator);
	fprintRTF("{\\field{\\*\\fldinst SYMBOL %u \\\\f ", overstrike);
	fprintRTF("\"%s\"}{\\fldrslt }}", font);
	fprintRTF(")");

	if (g_processing_fields==0) fprintRTF("}{\\fldrslt }}");
}

void CmdUmlauteChar(int code)

/*****************************************************************************
 purpose : converts characters with diaeresis (\"{a}) from LaTeX to RTF
 ******************************************************************************/
{
    int num;
    char *cParam = getBraceParam();

    if (cParam == NULL)
        return;

    switch (cParam[0]) {
        case 'o':
            fprintRTF("\\'f6");
            break;
        case 'O':
            fprintRTF("\\'d6");
            break;
        case 'a':
            fprintRTF("\\'e4");
            break;
        case 'A':
            fprintRTF("\\'c4");
            break;
        case 'u':
            fprintRTF("\\'fc");
            break;
        case 'U':
            fprintRTF("\\'dc");
            break;
        case 'E':
            fprintRTF("\\'cb");
            break;
        case 'I':
            fprintRTF("\\'cf");
            break;
        case 'e':
            fprintRTF("\\'eb");
            break;
        case 'y':
            fprintRTF("\\'ff");
            break;

        case 'Y':
            if (g_unicode) {
                fprintRTF("\\u376Y");
                break;
            }

        default:
            if (strcmp(cParam, "\\i") == 0) {
                fprintRTF("\\'ef");
                break;
            }

            num = RtfFontNumber("MT Extra");
            if (!g_processing_fields)
                fprintRTF("{\\field{\\*\\fldinst EQ ");
            fprintRTF("\\\\O(");
            ConvertString(cParam);
            fprintRTF("%c\\\\S({\\f%d\\'26\\'26}))", g_field_separator, num);
            if (!g_processing_fields)
                fprintRTF("}{\\fldrslt }}");
            break;

    }
    free(cParam);
}

void CmdLApostrophChar(int code)

/******************************************************************************
 purpose: converts symbols with grave accents (\`{a}) from LaTeX to RTF
 ******************************************************************************/
{
    int num;
    char *cParam = getBraceParam();

    if (cParam == NULL)
        return;
    switch (cParam[0]) {
        case 'A':
            fprintRTF("\\'c0");
            break;
        case 'E':
            fprintRTF("\\'c8");
            break;
        case 'I':
            fprintRTF("\\'cc");
            break;
        case 'O':
            fprintRTF("\\'d2");
            break;
        case 'U':
            fprintRTF("\\'d9");
            break;
        case 'a':
            fprintRTF("\\'e0");
            break;
        case 'e':
            fprintRTF("\\'e8");
            break;
        case 'i':
            fprintRTF("\\'ec");
            break;
        case 'o':
            fprintRTF("\\'f2");
            break;
        case 'u':
            fprintRTF("\\'f9");
            break;

        case 'N':
            if (g_unicode) {
                fprintRTF("\\u504N");
                break;
            }
        case 'n':
            if (g_unicode) {
                fprintRTF("\\u505n");
                break;
            }

        default:
            if (strcmp(cParam, "\\i") == 0) {
                fprintRTF("\\'ed");
                break;
            }

            num = RtfFontNumber("MT Extra");
            if (!g_processing_fields)
                fprintRTF("{\\field{\\*\\fldinst EQ ");
            fprintRTF("\\\\O(");
            ConvertString(cParam);
            fprintRTF("%c\\\\S({\\f%d\\'23}))", g_field_separator, num);
            if (!g_processing_fields)
                fprintRTF("}{\\fldrslt }}");
            break;
    }
    free(cParam);
}

void CmdRApostrophChar(int code)

/******************************************************************************
 purpose: converts symbols with acute accents (\'{a}) from LaTeX to RTF
 ******************************************************************************/
{
    char *cParam = getBraceParam();

    if (cParam == NULL)
        return;

    switch (cParam[0]) {
        case 'A':
            fprintRTF("\\'c1");
            break;
        case 'E':
            fprintRTF("\\'c9");
            break;
        case 'I':
            fprintRTF("\\'cd");
            break;
        case 'O':
            fprintRTF("\\'d3");
            break;
        case 'U':
            fprintRTF("\\'da");
            break;
        case 'a':
            fprintRTF("\\'e1");
            break;
        case 'e':
            fprintRTF("\\'e9");
            break;
        case 'i':
            fprintRTF("\\'ed");
            break;
        case 'o':
            fprintRTF("\\'f3");
            break;
        case 'u':
            fprintRTF("\\'fa");
            break;
        case 'y':
            fprintRTF("\\'fd");
            break;
        case 'Y':
            fprintRTF("\\'dd");
            break;

        case 'C':
            if (g_unicode) {
                fprintRTF("\\u262C");
                break;
            }
        case 'c':
            if (g_unicode) {
                fprintRTF("\\u263c");
                break;
            }
        case 'G':
            if (g_unicode) {
                fprintRTF("\\u500G");
                break;
            }
        case 'g':
            if (g_unicode) {
                fprintRTF("\\u501g");
                break;
            }
        case 'L':
            if (g_unicode) {
                fprintRTF("\\u313L");
                break;
            }
        case 'l':
            if (g_unicode) {
                fprintRTF("\\u314l");
                break;
            }
        case 'N':
            if (g_unicode) {
                fprintRTF("\\u323N");
                break;
            }
        case 'n':
            if (g_unicode) {
                fprintRTF("\\u324n");
                break;
            }
        case 'R':
            if (g_unicode) {
                fprintRTF("\\u340R");
                break;
            }
        case 'r':
            if (g_unicode) {
                fprintRTF("\\u341r");
                break;
            }
        case 'S':
            if (g_unicode) {
                fprintRTF("\\u346S");
                break;
            }
        case 's':
            if (g_unicode) {
                fprintRTF("\\u347s");
                break;
            }
        case 'Z':
            if (g_unicode) {
                fprintRTF("\\u377Z");
                break;
            }
        case 'z':
            if (g_unicode) {
                fprintRTF("\\u378z");
                break;
            }

        default:
            if (strcmp(cParam, "\\i") == 0) {
                fprintRTF("\\'ec");
                break;
            }

            if (!g_processing_fields)
                fprintRTF("{\\field{\\*\\fldinst EQ ");
            fprintRTF("\\\\O(");
            ConvertString(cParam);
            fprintRTF("%c\\\\S(\\'b4))", g_field_separator);
            if (!g_processing_fields)
                fprintRTF("}{\\fldrslt }}");
            break;

    }
    free(cParam);
}

static void put_unicode_char(unsigned char b1, unsigned char b2)
{
		if (b1==0)
			fprintRTF("\\'%ux",b1);
		else if (b1<128)
			fprintRTF("\\u%d*",b1*256+b2);
		else
			fprintRTF("\\u%d*",b1*256+b2-65536);
}

/******************************************************************************
 purpose: converts \H{c}  must
 ******************************************************************************/
void CmdDoubleAcuteChar(int code)
{
    char *cParam = getBraceParam();

    if (cParam == NULL)
        return;

    switch (cParam[0]) {
        case 'O':
        	put_unicode_char(0x01,0x50);
            break;
        case 'o':
        	put_unicode_char(0x01,0x51);
            break;
        case 'U':
        	put_unicode_char(0x01,0x70);
            break;
        case 'u':
        	put_unicode_char(0x01,0x71);
            break;
        default:
            break;
    }
    free(cParam);
}

void CmdMacronChar(int code)

/******************************************************************************
 purpose: converts special symbols from LaTeX to RTF
 ******************************************************************************/
{
    char *cParam = getBraceParam();

    if (cParam == NULL)
        return;

    switch (cParam[0]) {
        case 'A':
        	put_unicode_char(0x01,0x00);
            break;
        case 'a':
        	put_unicode_char(0x01,0x01);
            break;
        case 'E':
        	put_unicode_char(0x01,0x12);
            break;
        case 'e':
        	put_unicode_char(0x01,0x13);
            break;
        case 'I':
        	put_unicode_char(0x01,0x2A);
            break;
        case 'i':
        	put_unicode_char(0x01,0x2B);
            break;
        case 'O':
        	put_unicode_char(0x01,0x4C);
            break;
        case 'o':
        	put_unicode_char(0x01,0x4D);
            break;
        case 'U':
        	put_unicode_char(0x01,0x6A);
            break;
        case 'u':
        	put_unicode_char(0x01,0x6B);
            break;
        case 'Y':
        	put_unicode_char(0x02,0x32);
            break;
        case 'y':
        	put_unicode_char(0x02,0x33);
            break;
        default:
			if (!g_processing_fields)
				fprintRTF("{\\field{\\*\\fldinst EQ ");
			fprintRTF("\\\\O(");
			ConvertString(cParam);
			fprintRTF("%c\\\\S(\\'af))", g_field_separator);
			if (!g_processing_fields)
				fprintRTF("}{\\fldrslt }}");
    }

    free(cParam);
}

void CmdHatChar(int code)

/******************************************************************************
 purpose: \^{o} and \hat{o} symbols from LaTeX to RTF
 ******************************************************************************/
{
    int done = 0;
    char *cParam = getBraceParam();
	
    if (cParam == NULL)
        return;

    diagnostics(4,"CmdHatChar letter='%s' in eq field=%d",cParam,
                g_processing_fields);
    
	/* These encodings will fail in equation field translation */
    if (g_processing_fields==0) {
		switch (cParam[0]) {
			case 'A':
					fprintRTF("\\'c2");
					done=1;
					break;
			case 'E':
					fprintRTF("\\'ca");
					done=1;
					break;
			case 'I':
					fprintRTF("\\'ce");
					done=1;
					break;
			case 'O':
					fprintRTF("\\'d4");
					done=1;
					break;
			case 'U':
					fprintRTF("\\'db");
					done=1;
					break;
			case 'a':
					fprintRTF("\\'e2");
					done=1;
					break;
			case 'e':
					fprintRTF("\\'ea");
					done=1;
					break;
			case 'i':
					fprintRTF("\\'ee");
					done=1;
					break;
			case 'o':
					fprintRTF("\\'f4");
					done=1;
					break;
			case 'u':
					fprintRTF("\\'fb");
					done=1;
					break;
			case 'C':
					fprintRTF("\\u264C");
					done=1;
					break;
			case 'c':
					fprintRTF("\\u265c");
					done=1;
					break;
			case 'G':
					fprintRTF("\\u284G");
					done=1;
					break;
			case 'g':
					fprintRTF("\\u285g");
					done=1;
					break;
			case 'H':
					fprintRTF("\\u292H");
					done=1;
					break;
			case 'h':
					fprintRTF("\\u293h");
					done=1;
					break;
			case 'J':
					fprintRTF("\\u308J");
					done=1;
					break;
			case 'S':
					fprintRTF("\\u348S");
					done=1;
					break;
			case 's':
					fprintRTF("\\u349s");
					done=1;
					break;
			case 'W':
					fprintRTF("\\u372W");
					done=1;
					break;
			case 'w':
					fprintRTF("\\u373w");
					done=1;
					break;
			case 'Y':
					fprintRTF("\\u374Y");
					done=1;
					break;
			case 'y':
					fprintRTF("\\u375y");
					done=1;
					break;
		}
	} 

	if (!done)
		putOverstrikeChar("MT Extra", cParam, 36);
		
	free(cParam);
}

void CmdOaccentChar(int code)

/******************************************************************************
 purpose: converts \r accents from LaTeX to RTF
 ******************************************************************************/
{
    char *cParam;
    int symfont = RtfFontNumber("Symbol");

    cParam = getBraceParam();
    if (cParam == NULL)
        return;

    switch (cParam[0]) {
        case 'A':
            fprintRTF("\\'c5");
            break;

        case 'a':
            fprintRTF("\\'e5");
            break;

        case '\\':
            if (strcmp(cParam, "\\i") == 0)
                fprintRTF("\\'ee");
            else
                diagnostics(WARNING, "Cannot put \\r on '%s'", cParam);
            break;

        default:
            if (!g_processing_fields)
                fprintRTF("{\\field{\\*\\fldinst EQ ");
            fprintRTF("\\\\O(");
            ConvertString(cParam);
            fprintRTF("%c\\\\S({\\f%d\\u-3920\\'b0}))", g_field_separator, symfont);
            if (!g_processing_fields)
                fprintRTF("}{\\fldrslt }}");
            break;
    }

    free(cParam);
}

void CmdTildeChar(int code)

/******************************************************************************
 purpose: converts \~{n} from LaTeX to RTF
 ******************************************************************************/
{
    char *cParam = getBraceParam();
	int done = 0;
	
    if (cParam == NULL) return;
	
    diagnostics(4,"CmdTildeChar letter='%s' in eq field=%d",cParam,
                g_processing_fields);
    
	/* These fail in equation fields */
    if (g_processing_fields==0) {
		switch (cParam[0]) {
			case 'A':
				fprintRTF("\\'c3");
				done = 1;
				break;
			case 'O':
				fprintRTF("\\'d5");
				done = 1;
				break;
			case 'a':
				fprintRTF("\\'e3");
				done = 1;
				break;
			case 'o':
				fprintRTF("\\'f5");
				done = 1;
				break;
			case 'n':
				fprintRTF("\\'f1");
				done = 1;
				break;
			case 'N':
				fprintRTF("\\'d1");
				done = 1;
				break;
			case 'I':
				fprintRTF("\\u296I");
				done = 1;
				break;
			case 'U':
				fprintRTF("\\u360U");
				done = 1;
				break;
			case 'u':
				fprintRTF("\\u361u");
				done = 1;
				break;
		}
		
		if (strcmp(cParam, "\\i") == 0) {
			fprintRTF("\\u297i");
			done = 1;
		}
	}

	if (!done) 
		putOverstrikeChar("MT Extra", cParam, 37);
	free(cParam);
}

void CmdCedillaChar(int code)

/*****************************************************************************
 purpose: converts \c{c} from LaTeX to RTF
 ******************************************************************************/
{
    int down;
    char *cParam = getBraceParam();

    if (cParam == NULL)
        return;

    switch (cParam[0]) {
        case 'C':
            fprintRTF("\\'c7");
            break;
        case 'c':
            fprintRTF("\\'e7");
            break;
        case 'G':
        	put_unicode_char(0x01,0x22);
            break;
        case 'g':
        	put_unicode_char(0x01,0x23);
            break;
        case 'K':
        	put_unicode_char(0x01,0x36);
            break;
        case 'k':
        	put_unicode_char(0x01,0x37);
            break;
        case 'L':
        	put_unicode_char(0x01,0x3C);
            break;
        case 'l':
        	put_unicode_char(0x01,0x3D);
            break;
        case 'N':
        	put_unicode_char(0x01,0x46);
            break;
        case 'n':
        	put_unicode_char(0x01,0x47);
            break;
        case 'R':
        	put_unicode_char(0x01,0x58);
            break;
        case 'r':
        	put_unicode_char(0x01,0x59);
            break;
        case 'S':
        	put_unicode_char(0x01,0x5E);
            break;
        case 's':
        	put_unicode_char(0x01,0x5F);
            break;
        case 'T':
        	put_unicode_char(0x01,0x62);
            break;
        case 't':
        	put_unicode_char(0x01,0x63);
            break;
        case 'E':
        	put_unicode_char(0x02,0x28);
            break;
        case 'e':
        	put_unicode_char(0x02,0x29);
            break;
            
        default:
            down = CurrentFontSize() / 4;
            if (!g_processing_fields)
                fprintRTF("{\\field{\\*\\fldinst EQ ");
            fprintRTF("\\\\O(");
            ConvertString(cParam);
            fprintRTF("%c\\dn%d\\'b8)", g_field_separator, down);
            if (!g_processing_fields)
                fprintRTF("}{\\fldrslt }}");
            break;
    }

    free(cParam);
}

void CmdVecChar(int code)

/*****************************************************************************
 purpose: converts \vec{o} from LaTeX to RTF
 ******************************************************************************/
{
    int num;
    int upsize;
    char *cParam;

    cParam = getBraceParam();
    if (cParam == NULL)
        return;

    switch (cParam[0]) {
        case 'a':
        case 'c':
        case 'e':
        case 'g':
        case 'i':
        case 'j':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
            upsize = (3 * CurrentFontSize()) / 8;
            break;
        default:
            upsize = (CurrentFontSize() * 3) / 4;
    }

    num = RtfFontNumber("MT Extra");

    if (!g_processing_fields)
        fprintRTF("{\\field{\\*\\fldinst EQ ");
    fprintRTF("\\\\O(");
    ConvertString(cParam);
    fprintRTF("%c\\\\S({\\up%d\\f%d\\'72}))", g_field_separator, upsize, num);
    if (!g_processing_fields)
        fprintRTF("}{\\fldrslt }}");
    free(cParam);
}

void CmdBreveChar(int code)

/*****************************************************************************
 purpose: converts \u{o} and \breve{o} from LaTeX to RTF
 		  there is no breve in codepage 1252
 		  there is one \'f9 in the MacRoman, but that is not so portable
		  there is one in MT Extra, but the RTF parser for word mistakes
		  \'28 as a '(' and goes bananas.  Therefore need the extra \\\\
		  the only solution is to encode with unicode --- perhaps later
		  Now we just fake it with a u
 ******************************************************************************/
{
    int upsize, num;
    char *cParam = getBraceParam();

    if (cParam == NULL)
        return;

    switch (cParam[0]) {
        case 'A':
        	put_unicode_char(0x01,0x02);
            break;
        case 'a':
        	put_unicode_char(0x01,0x03);
            break;
        case 'E':
        	put_unicode_char(0x01,0x14);
            break;
        case 'e':
        	put_unicode_char(0x01,0x15);
            break;
        case 'G':
        	put_unicode_char(0x01,0x1e);
        	break;
        case 'g':
        	put_unicode_char(0x01,0x1f);
        	break;
        case 'I':
        	put_unicode_char(0x01,0x2c);
            break;
        case 'i':
        	put_unicode_char(0x01,0x2d);
            break;
        case 'O':
        	put_unicode_char(0x01,0x4e);
            break;
        case 'o':
        	put_unicode_char(0x01,0x4f);
            break;
        case 'U':
        	put_unicode_char(0x01,0x6c);
            break;
        case 'u':
        	put_unicode_char(0x01,0x6d);
            break;
        default:
			num = RtfFontNumber("MT Extra");
			upsize = CurrentFontSize() / 2;
			if (!g_processing_fields)
				fprintRTF("{\\field{\\*\\fldinst EQ ");
			fprintRTF("\\\\O(");
			ConvertString(cParam);
			fprintRTF("%c\\\\S({\\up%d\\f%d \\\\(}))", g_field_separator, upsize, num);
			if (!g_processing_fields)
				fprintRTF("}{\\fldrslt }}");
            break;
    }
    free(cParam);
}

void CmdUnderdotChar(int code)

/******************************************************************************
 purpose: converts chars with dots under from LaTeX to RTF
 ******************************************************************************/
{
    int num;
    int upsize;
    char *cParam;

    cParam = getBraceParam();
    if (cParam == NULL)
        return;
		
	switch (cParam[0]) {
		case 'A':
				fprintRTF("\\u258A");
				break;
				
		case 'a':
				fprintRTF("\\u259a");
				break;
				
		case 'E':
				fprintRTF("\\u276E");
				break;
				
		case 'e':
				fprintRTF("\\u277e");
				break;
				
		case 'G':
				fprintRTF("\\u286G");
				break;
				
		case 'g':
				fprintRTF("\\u287g");
				break;
				
		case 'I':
				fprintRTF("\\u300I");
				break;
				
		case 'O':
				fprintRTF("\\u334O");
				break;
				
		case 'o':
				fprintRTF("\\u335o");
				break;
				
		case 'U':
				fprintRTF("\\u364U");
				break;
				
		case 'u':
				fprintRTF("\\u365u");
				break;
	
		default:
			if (g_unicode && strcmp(cParam, "\\i") == 0) {
				fprintRTF("\\u301i");
				break;
			}
	
			upsize = CurrentFontSize() / 2;
			if (!g_processing_fields)
				fprintRTF("{\\field{\\*\\fldinst EQ ");
			fprintRTF("\\\\O(");
			ConvertString(cParam);
			fprintRTF("%c\\\\S({\\up%d\\f%d \\\\(}))", g_field_separator, upsize, num);
			if (!g_processing_fields)
				fprintRTF("}{\\fldrslt }}");
		   
	 }
	 free(cParam);
}

void CmdHacekChar(int code)

/******************************************************************************
 purpose: converts \v from LaTeX to RTF
          need something that looks like \\O(a,\\S(\f1\'da)) in RTF file
 ******************************************************************************/
{
    int num;
    int upsize;
    char *cParam;

    cParam = getBraceParam();
    if (cParam == NULL)
        return;

    switch (cParam[0]) {
        case 'C':
        	put_unicode_char(0x01,0x0C);
            break;
        case 'c':
        	put_unicode_char(0x01,0x0D);
            break;
        case 'D':
        	put_unicode_char(0x01,0x0E);
            break;
        case 'd':
        	put_unicode_char(0x01,0x0F);
            break;
        case 'E':
        	put_unicode_char(0x01,0x1A);
            break;
        case 'e':
        	put_unicode_char(0x01,0x1B);
            break;
        case 'L':
        	put_unicode_char(0x01,0x3D);
            break;
        case 'l':
        	put_unicode_char(0x01,0x3E);
            break;
        case 'N':
        	put_unicode_char(0x01,0x47);
            break;
        case 'n':
        	put_unicode_char(0x01,0x48);
            break;
        case 'R':
        	put_unicode_char(0x01,0x58);
            break;
        case 'r':
        	put_unicode_char(0x01,0x59);
            break;
        case 'S':
        	put_unicode_char(0x01,0x60);
            break;
        case 's':
        	put_unicode_char(0x01,0x61);
            break;
        case 'T':
        	put_unicode_char(0x01,0x64);
            break;
        case 't':
        	put_unicode_char(0x01,0x65);
            break;
        case 'Z':
        	put_unicode_char(0x01,0x7D);
            break;
        case 'z':
        	put_unicode_char(0x01,0x7E);
            break;
        case 'A':
        	put_unicode_char(0x01,0xCD);
            break;
        case 'a':
        	put_unicode_char(0x01,0xCE);
            break;
        case 'I':
        	put_unicode_char(0x01,0xCF);
            break;
        case 'i':
        	put_unicode_char(0x01,0xD0);
            break;
        case 'O':
        	put_unicode_char(0x01,0xD1);
            break;
        case 'o':
        	put_unicode_char(0x01,0xD2);
            break;
        case 'U':
        	put_unicode_char(0x01,0xD3);
            break;
        case 'u':
        	put_unicode_char(0x01,0xD4);
            break;
        case 'G':
        	put_unicode_char(0x01,0xE6);
            break;
        case 'g':
        	put_unicode_char(0x01,0xE7);
            break;
        case 'K':
        	put_unicode_char(0x01,0xE8);
            break;
        case 'k':
        	put_unicode_char(0x01,0xE9);
            break;
        case 'j':
        	put_unicode_char(0x01,0xF0);
            break;
        case 'H':
        	put_unicode_char(0x02,0x1E);
            break;
        case 'h':
        	put_unicode_char(0x02,0x1F);
            break;

        default:
			upsize = (int) ((0.4 * CurrentFontSize()) + 0.45);
			num = RtfFontNumber("Symbol");
		
			if (!g_processing_fields)
				fprintRTF("{\\field{\\*\\fldinst EQ ");
			fprintRTF("\\\\O(");
			ConvertString(cParam);
			fprintRTF("%c\\\\S({\\up%d\\f%d\\'da}))", g_field_separator, upsize, num);
			if (!g_processing_fields)
				fprintRTF("}{\\fldrslt }}");
	}
    free(cParam);
}

void CmdDotChar(int code)

/******************************************************************************
 purpose: converts \.{o} and \dot{o} from LaTeX to RTF
          need something that looks like \\O(a,\\S(\f2\'26)) in RTF file
 ******************************************************************************/
{
    char *cParam = getBraceParam();
	int done = 0;
	
    if (cParam == NULL) return;
	
    diagnostics(4,"CmdDotChar letter='%s' in eq field=%d",cParam,
                g_processing_fields);
    
	/* These encodings will fail in equation field translation */
    if (g_processing_fields==0) {
		switch (cParam[0]) {
			case 'A':
					fprintRTF("\\u550A");
					done=1;
					break;
	
			case 'a':
					fprintRTF("\\u551a");
					done=1;
					break;
	
			case 'C':
					fprintRTF("\\u266C");
					done=1;
					break;
				
			case 'c':
					fprintRTF("\\u267c");
					done=1;
					break;
				
			case 'E':
					fprintRTF("\\u278E");
					done=1;
					break;
				
			case 'e':
					fprintRTF("\\u279e");
					done=1;
					break;
				
			case 'G':
					fprintRTF("\\u288G");
					done=1;
					break;
				
			case 'g':
					fprintRTF("\\u289g");
					done=1;
					break;
				
			case 'I':
					fprintRTF("\\u304I");
					done=1;
					break;
				
			case 'O':
					fprintRTF("\\u558O");
					done=1;
					break;
				
			case 'o':
					fprintRTF("\\u559o");
					done=1;
					break;
				
			case 'Z':
					fprintRTF("\\u379Z");
					done=1;
					break;
				
			case 'z':
					fprintRTF("\\u380z");
					done=1;
					break;
		}
	}

	if (!done) 
		putOverstrikeChar("MT Extra", cParam, 38);

	free(cParam);
}

void CmdUnderbarChar(int code)

/******************************************************************************
 purpose: converts \.{o} and \dot{o} from LaTeX to RTF
          need something that looks like \\O(a,\\S(\f2\'26)) in RTF file
 ******************************************************************************/
{
    char *cParam;

    cParam = getBraceParam();
    if (cParam == NULL)
        return;

    if (cParam[0]) {
        if (!g_processing_fields)
            fprintRTF("{\\field{\\*\\fldinst EQ ");
        fprintRTF("\\\\O(");
        ConvertString(cParam);
        fprintRTF("%c_)", g_field_separator);
        if (!g_processing_fields)
            fprintRTF("}{\\fldrslt }}");
    }
    free(cParam);
}

void CmdHungarianChar(int code)

/******************************************************************************
 purpose: converts \H{o} from LaTeX to RTF
 ******************************************************************************/
{
    char *cParam;

    cParam = getBraceParam();
    if (cParam == NULL)
        return;

    switch (cParam[0]) {
        case 'O':
            if (g_unicode) {
                fprintRTF("\\u336O");
                break;
            }
        case 'o':
            if (g_unicode) {
                fprintRTF("\\u337o");
                break;
            }
        case 'U':
            if (g_unicode) {
                fprintRTF("\\u368U");
                break;
            }
        case 'u':
            if (g_unicode) {
                fprintRTF("\\u369u");
                break;
            }

        default:
            /* should really do some simulation like other accents */
            fprintRTF("%c", cParam[0]);
    }

    free(cParam);
}

void CmdDotlessChar(int code)

/******************************************************************************
 purpose: converts \i and \j to 'i' and 'j' or Unicode equivalent
 ******************************************************************************/
{
    if (code == 0)
        if (g_unicode)
            fprintRTF("\\u305i");
        else
            fprintRTF("i");
    else
        fprintRTF("j");
}

void CmdPolishL(int code)

/******************************************************************************
 purpose: converts \l and \L to 'l' and 'L' or Unicode equivalent
 ******************************************************************************/
{
    if (code == 1)
        if (g_unicode)
            fprintRTF("\\u322L");
        else
            fprintRTF("L");
    else
        if (g_unicode)
            fprintRTF("\\u323l");
        else
            fprintRTF("l");
}

/******************************************************************************
 purpose: converts \euro{amount} and \EUR{amount}
 ******************************************************************************/
void CmdEuro(int code)
{
	char *s=getBraceParam();
    fprintRTF("\\'80");
	ConvertString(s);
	free(s);
}

void CmdChar(int code)
{
    char cThis;
    int num;
    int symfont = RtfFontNumber("Symbol");

    cThis = getNonSpace();
    if (cThis != '\'') {
        ungetTexChar(cThis);
        return;
    }

    num = 64 * ((int) getTexChar() - (int) '0');
    num += 8 * ((int) getTexChar() - (int) '0');
    num += ((int) getTexChar() - (int) '0');

    switch (num) {
        case 0:
            fprintRTF("{\\f%d G}", symfont);    /* Gamma */
            break;

        case 1:
            fprintRTF("{\\f%d D}", symfont);    /* Delta */
            break;

        case 2:
            fprintRTF("{\\f%d Q}", symfont);    /* Theta */
            break;

        case 3:
            fprintRTF("{\\f%d L}", symfont);    /* Lambda */
            break;

        case 4:
            fprintRTF("{\\f%d X}", symfont);    /* Xi */
            break;

        case 5:
            fprintRTF("{\\f%d P}", symfont);    /* Pi */
            break;

        case 6:
            fprintRTF("{\\f%d S}", symfont);    /* Sigma */
            break;

        case 7:
            fprintRTF("{\\f%d U}", symfont);    /* Upsilon */
            break;

        case 8:
            fprintRTF("{\\f%d F}", symfont);    /* Phi */
            break;

        case 9:
            fprintRTF("{\\f%d Y}", symfont);    /* Psi */
            break;

        case 10:
            fprintRTF("{\\f%d W}", symfont);    /* Omega */
            break;

        case 11:
            fprintRTF("ff");
            break;

        case 12:
            fprintRTF("fi");
            break;

        case 13:
            fprintRTF("fl");
            break;

        case 14:
            fprintRTF("ffi");
            break;

        case 15:
            fprintRTF("ffl");
            break;

        case 16:
            fprintRTF("i");     /* Dotless i */
            break;

        case 17:
            fprintRTF("j");     /* Dotless j */
            break;

        case 18:
            fprintRTF("`");
            break;

        case 19:
            fprintRTF("'");
            break;

        case 20:
            fprintRTF("v");
            break;

        case 21:
            fprintRTF("u");
            break;

        case 22:
            fprintRTF("-");     /* overbar */
            break;

        case 23:
            fprintRTF("{\\f%d \\'b0}", symfont);    /* degree */
            break;

        case 24:
            fprintRTF("\\'b8"); /* cedilla */
            break;

        case 25:
            fprintRTF("\\'df"); /* § */
            break;

        case 26:
            fprintRTF("\\'e6"); /* ae */
            break;

        case 27:
            fprintRTF("\\'8c"); /* oe */
            break;

        case 28:
            fprintRTF("\\'f8"); /* oslash */
            break;

        case 29:
            fprintRTF("\\'c6");
            /*AE*/ break;

        case 30:
            fprintRTF("\\'8c");
            /*OE*/ break;

        case 31:
            fprintRTF("\\'d8"); /* capital O with stroke */
            break;

        case 32:
            fprintRTF(" ");     /* space differs with font */
            break;

        case 60:
            fprintRTF("<");     /* less than differs with font */
            break;

        case 62:
            fprintRTF(">");     /* greater than differs with font */
            break;

        case 123:
            fprintRTF("\\{");   /* open brace differs with font */
            break;

        case 124:
            fprintRTF("\\\\");  /* backslash differs with font */
            break;

        case 125:
            fprintRTF("\\}");   /* close brace differs with font */
            break;

        case 127:
            fprintRTF("\\'a8"); /* diaeresis differs with font */
            break;

        default:
            putRtfCharEscaped((char) num);
            break;
    }
}

void TeXlogo()

/******************************************************************************
 purpose : prints the Tex logo in the RTF-File (D Taupin)
 ******************************************************************************/
{
    float DnSize;
    int dnsize;

    DnSize = 0.3 * CurrentFontSize();
    dnsize = (int) (DnSize + 0.45);
    fprintRTF("T{\\dn%d E}X", dnsize);
}

void LaTeXlogo()

/******************************************************************************
 purpose : prints the LaTeX logo in the RTF-File (D Taupin)
 ******************************************************************************/
{
    float FloatFsize;
    int upsize, Asize;

    if (CurrentFontSize() > 14)
        FloatFsize = 0.8 * CurrentFontSize();
    else
        FloatFsize = 0.9 * CurrentFontSize();
    Asize = (int) (FloatFsize + 0.45);

    upsize = (int) (0.25 * CurrentFontSize() + 0.45);
    fprintRTF("L{\\up%d\\fs%d A}", upsize, Asize);
    TeXlogo();
}

void CmdLogo(int code)

/******************************************************************************
 purpose : converts the LaTeX, TeX, SLiTex, etc logos to RTF 
 ******************************************************************************/
{
    int font_num, dnsize;
    float FloatFsize;

    SetTexMode(MODE_HORIZONTAL);
    fprintRTF("{\\plain ");

    switch (code) {
        case CMD_TEX:
            TeXlogo();
            break;

        case CMD_LATEX:
            LaTeXlogo();
            break;

        case CMD_SLITEX:
            fprintRTF("{\\scaps Sli}");
            TeXlogo();
            break;

        case CMD_BIBTEX:
            fprintRTF("{\\scaps Bib}");
            TeXlogo();
            break;

        case CMD_LATEXE:
            LaTeXlogo();
            if (CurrentFontSize() > 14) {
                FloatFsize = 0.75 * CurrentFontSize();
            } else {
                FloatFsize = (float) CurrentFontSize();
            };
            dnsize = (int) (0.3 * CurrentFontSize() + 0.45);
            font_num = RtfFontNumber("Symbol");
            fprintRTF("2{\\dn%d\\f%d e}", dnsize, font_num);
            break;

        case CMD_AMSTEX:
            fprintRTF("{\\i AmS}-");    /* should be calligraphic */
            TeXlogo();
            break;

        case CMD_AMSLATEX:
            fprintRTF("{\\i AmS}-");    /* should be calligraphic */
            LaTeXlogo();
            break;

        case CMD_LYX:
            dnsize = (int) (0.3 * CurrentFontSize() + 0.45);
            fprintRTF("L{\\dn%d Y}X", dnsize);
            break;
    }
    fprintRTF("}");
}

void CmdCzechAbbrev(int code)

/******************************************************************************
  purpose: only handles \uv{quote} at the moment
 ******************************************************************************/
{
    char *quote;

    quote = getBraceParam();
    fprintRTF(" \\'84");
    ConvertString(quote);
    free(quote);
    fprintRTF("\\ldblquote ");
    return;
}

void CmdFrenchAbbrev(int code)

/******************************************************************************
  purpose: makes \\ier, \\ieme, etc
 ******************************************************************************/
{
    float FloatFsize;
    int up, size;
    char *fuptext;

    if (code == INFERIEURA) {
        fprintRTF("<");
        return;
    }
    if (code == SUPERIEURA) {
        fprintRTF(">");
        return;
    }
    if (code == FRENCH_LQ) {
        fprintRTF("\\lquote");
        return;
    }
    if (code == FRENCH_RQ) {
        fprintRTF("\\rquote");
        return;
    }
    if (code == FRENCH_OG) {
        fprintRTF("\\'AB\\'A0");
        return;
    }                           /* guillemotleft */
    if (code == FRENCH_FG) {
        fprintRTF("\\'BB");
        return;
    }                           /* guillemotright */
    if (code == FRENCH_LQQ) {
        fprintRTF("\\ldblquote");
        return;
    }
    if (code == FRENCH_RQQ) {
        fprintRTF("\\rdblquote");
        return;
    }
    if (code == POINT_VIRGULE) {
        fprintRTF(";");
        return;
    }
    if (code == POINT_EXCLAMATION) {
        fprintRTF("!");
        return;
    }
    if (code == POINT_INTERROGATION) {
        fprintRTF("?");
        return;
    }
    if (code == DITTO_MARK) {
        fprintRTF("\"");
        return;
    }
    if (code == DEUX_POINTS) {
        fprintRTF(":");
        return;
    }
    if (code == LCS || code == FCS) {
        char *abbev = getBraceParam();

        fprintRTF("{\\scaps ");
        ConvertString(abbev);
        free(abbev);
        fprintRTF("}");
        return;
    }

    if (code == NUMERO)
        fprintRTF("n");
    if (code == NUMEROS)
        fprintRTF("n");
    if (code == CNUMERO)
        fprintRTF("N");
    if (code == CNUMEROS)
        fprintRTF("N");
    if (code == PRIMO)
        fprintRTF("1");
    if (code == SECUNDO)
        fprintRTF("2");
    if (code == TERTIO)
        fprintRTF("3");
    if (code == QUARTO)
        fprintRTF("4");

    FloatFsize = (float) CurrentFontSize();

    if (FloatFsize > 14)
        FloatFsize *= 0.75;

    up = (int) (0.3 * FloatFsize + 0.45);
    size = (int) (FloatFsize + 0.45);

    fprintRTF("{\\fs%d\\up%d ", size, up);
    switch (code) {
        case NUMERO:
            fprintRTF("o");
            break;
        case CNUMERO:
            fprintRTF("o");
            break;
        case NUMEROS:
            fprintRTF("os");
            break;
        case CNUMEROS:
            fprintRTF("os");
            break;
        case PRIMO:
            fprintRTF("o");
            break;
        case SECUNDO:
            fprintRTF("o");
            break;
        case TERTIO:
            fprintRTF("o");
            break;
        case QUARTO:
            fprintRTF("o");
            break;
        case IERF:
            fprintRTF("er");
            break;
        case IERSF:
            fprintRTF("ers");
            break;
        case IEMEF:
            fprintRTF("e");
            break;
        case IEMESF:
            fprintRTF("es");
            break;
        case IEREF:
            fprintRTF("re");
            break;
        case IERESF:
            fprintRTF("res");
            break;
        case FUP:
            fuptext = getBraceParam();
            ConvertString(fuptext);
            free(fuptext);
            break;
    }

    fprintRTF("}");
}

void CmdLatin1Char(int code)

/******************************************************************************
 * purpose : insert Latin1 character into RTF stream
 * ******************************************************************************/
{
    int n;

    if (code <= 0 || code >= 255)
        return;

    n = CurrentLatin1FontFamily();

    if (n >= 0)
        fprintRTF("{\\f%d\\\'%.2X}", n, code);
    else                        /* already using Latin1 Font */
        fprintRTF("\\\'%.2X", code);
}

void CmdLatin2Char(int code)

/******************************************************************************
 * purpose : insert Latin2 character into RTF stream
 * ******************************************************************************/
{
    int n;

    if (code <= 0 || code >= 255)
        return;

    n = CurrentLatin2FontFamily();

    if (n >= 0)
        fprintRTF("{\\f%d\\\'%.2X}", n, code);
    else                        /* already using Latin2 Font */
        fprintRTF("\\\'%.2X", code);
}

void CmdCyrillicChar(int code)

/******************************************************************************
 * purpose : insert cyrillic character into RTF stream
 * ******************************************************************************/
{
    int n;

    if (code <= 0 || code >= 255)
        return;

    n = CurrentCyrillicFontFamily();

    if (n >= 0)
        fprintRTF("{\\f%d\\\'%.2X}", n, code);
    else                        /* already using Cyrillic Font */
        fprintRTF("\\\'%.2X", code);
}

/******************************************************************************
 * purpose : insert cyrillic character into RTF stream
 * ******************************************************************************/
void CmdCyrillicStrChar(char *s)
{
    int n;

    if (s == NULL || strlen(s) != 2)
        return;

    n = CurrentCyrillicFontFamily();

    if (n >= 0)
        fprintRTF("{\\f%d\\\'%s}", n, s);
    else                        /* already using Cyrillic Font */
        fprintRTF("\\\'%s", s);
}

/******************************************************************************
 * purpose : \celcius and \degreecelsius from SIUnits.sty
 * ******************************************************************************/
void CmdDegreeCelsius(int code) 
{
  fprintRTF("\\'b0C");
}

void CmdMTExtraChar(int code)
{
    if (g_processing_fields == 0) {
    	int num = RtfFontNumber("MT Extra");
    	fprintRTF("{\\f%d\\'%.2X}",num,code);

    } else {
    
		fprintRTF("{\\field{\\*\\fldinst SYMBOL %u ", (unsigned int) code);
		fprintRTF("\\\\f \"MT Extra\"}{\\fldrslt }}");
	}
}
