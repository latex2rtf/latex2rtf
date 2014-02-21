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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

This file is available from http://sourceforge.net/projects/latex2rtf/

*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "math.h"
#include "commands.h"
#include "fonts.h"
#include "cfg.h"
#include "ignore.h"
#include "encodings.h"
#include "parser.h"
#include "chars.h"
#include "funct1.h"
#include "convert.h"
#include "utils.h"
#include "vertical.h"
#include "fields.h"

/*****************************************************************************
 purpose: emit a unicode character.  values above 2^15 are negative
          the default_char should be a simple ascii 0-127 character
 ******************************************************************************/
static void putUnicodeChar(unsigned char b1, unsigned char b2, char default_char)
{
    if (getTexMode() == MODE_VERTICAL)
        changeTexMode(MODE_HORIZONTAL);

    if (b1<128)
        fprintRTF("\\u%d%c",b1*256+b2,default_char);
    else
        fprintRTF("\\u%d%c",b1*256+b2-65536,default_char);
}

/******************************************************************************
 * purpose : inserts a Unicode character
 *******************************************************************************/
void CmdUnicodeChar(int code)
{
    unsigned char a,b;
    uint16_t thechar;
    
    if (getTexMode() == MODE_VERTICAL)
        changeTexMode(MODE_HORIZONTAL);
    
    thechar = code;
    a = thechar >> 8;
    b = thechar - a * 256;
    putUnicodeChar(a,b,'?');
}

/*****************************************************************************
 purpose : converts characters with diaeresis, e.g., \"{a} or \ddot{a}
 ******************************************************************************/
void CmdUmlauteChar(int code)
{
    int n = RtfFontNumber("STIXGeneral");
    char *cParam = getBraceParam();

    if (strstr(cParam, "\\i")) {
        fprintRTF("\\'ef");
        free(cParam);
        return;
    }

    switch (cParam[0]) {
        case 'A':
            fprintRTF("\\'c4");
            break;
        case 'E':
            fprintRTF("\\'cb");
            break;
        case 'I':
            fprintRTF("\\'cf");
            break;
        case 'O':
            fprintRTF("\\'d6");
            break;
        case 'U':
            fprintRTF("\\'dc");
            break;
        case 'a':
            fprintRTF("\\'e4");
            break;
        case 'e':
            fprintRTF("\\'eb");
            break;
        case 'i':
            fprintRTF("\\'ef");
            break;
        case 'o':
            fprintRTF("\\'f6");
            break;
        case 'u':
            fprintRTF("\\'fc");
            break;
        case 'y':
            fprintRTF("\\'ff");
            break;
        case 'Y':
            putUnicodeChar(0x01,0x78,'Y');
            break;
        case 'H':
            putUnicodeChar(0x1E,0x26,'H');
            break;
        case 'h':
            putUnicodeChar(0x1E,0x27,'h');
            break;
        case 'W':
            putUnicodeChar(0x1E,0x83,'W');
            break;
        case 'w':
            putUnicodeChar(0x1E,0x84,'w');
            break;
        case 'X':
            putUnicodeChar(0x1E,0x8C,'X');
            break;
        case 'x':
            putUnicodeChar(0x1E,0x8D,'x');
            break;
        case 't':
            putUnicodeChar(0x1E,0x97,'t');
            break;
        case '\0':
            fprintRTF(" \\u776.");
            break;
        default:
            fprintRTF("{\\f%d",n);
            fprintRTF("\\u776.}");   /* unicode combining character 0x308*/
            ConvertString(cParam);
            break;  
    }

    free(cParam);
}

/******************************************************************************
 purpose: converts symbols with grave accents (\`{a}) from LaTeX to RTF
 ******************************************************************************/
void CmdGraveChar(int code)
{
    int n = RtfFontNumber("STIXGeneral");
    char *cParam = getBraceParam();

    if (strstr(cParam, "\\i")) {
        fprintRTF("\\'ec");
        free(cParam);
        return;
    }

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
            fprintRTF("\\u504N");
            break;            
        case 'n':
            fprintRTF("\\u505n");
            break;
        case '\0':
            fprintRTF(" \\u768\\'60");
            break;
        default:
            fprintRTF("{\\f%d",n);
            fprintRTF("\\u768\\'60}");  /* unicode combining character 0x0300 */
            ConvertString(cParam);
            break;  
    }

    free(cParam);
}

/******************************************************************************
 purpose: converts symbols with acute accents (\'{a}) from LaTeX to RTF
 ******************************************************************************/
void CmdAcuteChar(int code)
{
    int n = RtfFontNumber("STIXGeneral");
    char *cParam = getBraceParam();

    if (strstr(cParam, "\\i")) {
        fprintRTF("\\'ed");
        free(cParam);
        return;
    }

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
            fprintRTF("\\u262C");
            break;
        case 'c':
            fprintRTF("\\u263c");
            break;
        case 'G':
            fprintRTF("\\u500G");
            break;
        case 'g':
            fprintRTF("\\u501g");
            break;
        case 'L':
            fprintRTF("\\u313L");
            break;
        case 'l':
            fprintRTF("\\u314l");
            break;
        case 'N':
            fprintRTF("\\u323N");
            break;
        case 'n':
            fprintRTF("\\u324n");
            break;
        case 'R':
            fprintRTF("\\u340R");
            break;
        case 'r':
            fprintRTF("\\u341r");
            break;
        case 'S':
            fprintRTF("\\u346S");
            break;
        case 's':
            fprintRTF("\\u347s");
            break;
        case 'Z':
            fprintRTF("\\u377Z");
            break;
        case 'z':
            fprintRTF("\\u378z");
            break;
        case '\0':
            fprintRTF("\\'b4");
            break;
        default:
            fprintRTF("{\\f%d",n);
            fprintRTF("\\u769\\'b4}");   /* unicode combining character 0x0301 */
            ConvertString(cParam);
            break;  
    }
            
    free(cParam);
}

/******************************************************************************
 purpose: converts \H{o}
 ******************************************************************************/
void CmdDoubleAcuteChar(int code)
{
    int n = RtfFontNumber("STIXGeneral");
    char *cParam = getBraceParam();

    switch (cParam[0]) {
        case 'O':
            putUnicodeChar(0x01,0x50,'O');
            break;
        case 'o':
            putUnicodeChar(0x01,0x51,'o');
            break;
        case 'U':
            putUnicodeChar(0x01,0x70,'U');
            break;
        case 'u':
            putUnicodeChar(0x01,0x71,'u');
            break;
        case '\0':
            putUnicodeChar(0x02,0xDD,'"');
            break;
        default:
            fprintRTF("{\\f%d",n);
            fprintRTF("\\u779\"}");  /* unicode combining character 0x030B */
            ConvertString(cParam);
            break;  
    }
    
    free(cParam);
}

/******************************************************************************
 purpose: converts special symbols from LaTeX to RTF
 ******************************************************************************/
void CmdMacronChar(int code)
{
    int n = RtfFontNumber("STIXGeneral");
    char *cParam = getBraceParam();

    if (strstr(cParam, "\\i")) {
        putUnicodeChar(0x01,0x2B,'i');
        free(cParam);
        return;
    }

    switch (cParam[0]) {
        case 'A':
            putUnicodeChar(0x01,0x00,'A');
            break;
        case 'a':
            putUnicodeChar(0x01,0x01,'a');
            break;
        case 'E':
            putUnicodeChar(0x01,0x12,'E');
            break;
        case 'e':
            putUnicodeChar(0x01,0x13,'e');
            break;
        case 'I':
            putUnicodeChar(0x01,0x2A,'l');
            break;
        case 'i':
            putUnicodeChar(0x01,0x2B,'i');
            break;
        case 'G':
            putUnicodeChar(0x1E,0x20,'G');
            break;
        case 'g':
            putUnicodeChar(0x1E,0x21,'g');
            break;
        case 'O':
            putUnicodeChar(0x01,0x4C,'O');
            break;
        case 'o':
            putUnicodeChar(0x01,0x4D,'o');
            break;
        case 'U':
            putUnicodeChar(0x01,0x6A,'U');
            break;
        case 'u':
            putUnicodeChar(0x01,0x6B,'u');
            break;
        case 'Y':
            putUnicodeChar(0x02,0x32,'Y');
            break;
        case 'y':
            putUnicodeChar(0x02,0x33,'y');
            break;
        case '\0':
            fprintRTF("\\'5f");
            break;
        default:
            fprintRTF("{\\f%d",n);
            fprintRTF("\\u772\\'5f}");  /* unicode combining character 0x0304 */
            ConvertString(cParam);
            break;  
    }

    free(cParam);
}

/******************************************************************************
 purpose: \^{o} and \hat{o} symbols from LaTeX to RTF
 ******************************************************************************/
void CmdHatChar(int code)
{
    int n = RtfFontNumber("STIXGeneral");
    char *cParam = getBraceParam();
        
    switch (cParam[0]) {
        case 'a':
            fprintRTF("\\'e2");
            break;
        case 'A':
            fprintRTF("\\'c2");
            break;
        case 'e':
            fprintRTF("\\'ea");
            break;
        case 'E':
            fprintRTF("\\'ca");
            break;
        case 'i':
            fprintRTF("\\'ee");
            break;
        case 'I':
            fprintRTF("\\'ce");
            break;
        case 'o':
            fprintRTF("\\'f4");
            break;
        case 'O':
            fprintRTF("\\'d4");
            break;
        case 'u':
            fprintRTF("\\'fb");
            break;
        case 'U':
            fprintRTF("\\'db");
            break;
        case 'C':
            fprintRTF("\\u264C");
            break;
        case 'c':
            fprintRTF("\\u265c");
            break;
        case 'G':
            fprintRTF("\\u284G");
            break;
        case 'g':
            fprintRTF("\\u285g");
            break;
        case 'H':
            fprintRTF("\\u292H");
            break;
        case 'h':
            fprintRTF("\\u293h");
            break;
        case 'J':
            fprintRTF("\\u308J");
            break;
        case 'S':
            fprintRTF("\\u348S");
            break;
        case 's':
            fprintRTF("\\u349s");
            break;
        case 'W':
            fprintRTF("\\u372W");
            break;
        case 'w':
            fprintRTF("\\u373w");
            break;
        case 'Y':
            fprintRTF("\\u374Y");
            break;
        case 'y':
            fprintRTF("\\u375y");
            break;
        case '\0':
            fprintRTF("\\'5e");
            break;
        default:
            fprintRTF("{\\f%d",n);
            fprintRTF("\\u770\\'5e}");  /* unicode combining character 0x0302 */
            ConvertString(cParam);
            break;  
    }
        
    free(cParam);
}

/******************************************************************************
 purpose: converts \r accents from LaTeX to RTF
 ******************************************************************************/
void CmdRingChar(int code)
{
    int n = RtfFontNumber("STIXGeneral");
    char *cParam;

	if (code == 1) {
		char c, d;
		c = getTexChar();
		d = getTexChar();
		if (c!='2' || d != '3')	{
			fprintf(stderr, "sorry only \\accent23 is supported \n");
			return;
		}
	}

    cParam = getBraceParam();
	    
    switch (cParam[0]) {
        case 'A':
            fprintRTF("\\'c5");
            break;
        case 'a':
            fprintRTF("\\'e5");
            break;
        case 'U':
            putUnicodeChar(0x01,0x6E,'U');
            break;
        case 'u':
            putUnicodeChar(0x01,0x6F,'u');
            break;
        case 'w':
            putUnicodeChar(0x01,0x98,'w');
            break;
        case 'y':
            putUnicodeChar(0x01,0x99,'y');
            break;
        case '\0':
            fprintRTF("\\'b0");
            break;
        default:
            fprintRTF("{\\f%d",n);
            fprintRTF("\\u778\\'b0}");  /* unicode combining character 0x030A */
            ConvertString(cParam);
            break;  
    }
        
    free(cParam);
}

/******************************************************************************
 purpose: converts \~{n} from LaTeX to RTF
 ******************************************************************************/
void CmdTildeChar(int code)
{
    int n = RtfFontNumber("STIXGeneral");
    char *cParam = getBraceParam();
    
    if (strstr(cParam, "\\i")) {
        fprintRTF("\\u297i");
        free(cParam);
        return;
    }

    switch (cParam[0]) {
        case 'A':
            fprintRTF("\\'c3");
            break;
        case 'O':
            fprintRTF("\\'d5");
            break;
        case 'a':
            fprintRTF("\\'e3");
            break;
        case 'o':
            fprintRTF("\\'f5");
            break;
        case 'n':
            fprintRTF("\\'f1");
            break;
        case 'N':
            fprintRTF("\\'d1");
            break;
        case 'I':
            fprintRTF("\\u296I");
            break;
        case 'U':
            fprintRTF("\\u360U");
            break;
        case 'u':
            fprintRTF("\\u361u");
            break;
        case '\0':
            fprintRTF("\\'7e");
            break;
        default:
            fprintRTF("{\\f%d",n);
            fprintRTF("\\u771\\'7e}");  /* unicode combining character 0x0303 */
            ConvertString(cParam);
            break;  
    }
        
    free(cParam);
}

/*****************************************************************************
 purpose: converts \c{c} from LaTeX to RTF
 ******************************************************************************/
void CmdCedillaChar(int code)
{
    int n = RtfFontNumber("STIXGeneral");
    char *cParam = getBraceParam();
    
    switch (cParam[0]) {
        case 'C':
            fprintRTF("\\'c7");
            break;
        case 'c':
            fprintRTF("\\'e7");
            break;
        case 'G':
            putUnicodeChar(0x01,0x22,'G');
            break;
        case 'g':
            putUnicodeChar(0x01,0x23,'g');
            break;
        case 'K':
            putUnicodeChar(0x01,0x36,'K');
            break;
        case 'k':
            putUnicodeChar(0x01,0x37,'k');
            break;
        case 'L':
            putUnicodeChar(0x01,0x3B,'L');
            break;
        case 'l':
            putUnicodeChar(0x01,0x3C,'l');
            break;
        case 'N':
            putUnicodeChar(0x01,0x45,'N');
            break;
        case 'n':
            putUnicodeChar(0x01,0x46,'n');
            break;
        case 'R':
            putUnicodeChar(0x01,0x56,'R');
            break;
        case 'r':
            putUnicodeChar(0x01,0x57,'r');
            break;
        case 'S':
            putUnicodeChar(0x01,0x5E,'S');
            break;
        case 's':
            putUnicodeChar(0x01,0x5F,'s');
            break;
        case 'T':
            putUnicodeChar(0x01,0x62,'T');
            break;
        case 't':
            putUnicodeChar(0x01,0x63,'t');
            break;
        case 'E':
            putUnicodeChar(0x02,0x28,'E');
            break;
        case 'e':
            putUnicodeChar(0x02,0x29,'e');
            break;
        case '\0':
            fprintRTF("\\'b8");
            break;
        default:
            fprintRTF("{\\f%d",n);
            fprintRTF("\\u807\\'b8}");  /* unicode combining character 0x0327 */
            ConvertString(cParam);
            break;  
    }

    free(cParam);
}

/*****************************************************************************
 purpose: converts \u{o} and \breve{o} 
 ******************************************************************************/
void CmdBreveChar(int code)
{
    int n = RtfFontNumber("STIXGeneral");
    char *cParam = getBraceParam();
    
    switch (cParam[0]) {
        case 'A':
            putUnicodeChar(0x01,0x02,'A');
            break;
        case 'a':
            putUnicodeChar(0x01,0x03,'a');
            break;
        case 'E':
            putUnicodeChar(0x01,0x14,'E');
            break;
        case 'e':
            putUnicodeChar(0x01,0x15,'e');
            break;
        case 'G':
            putUnicodeChar(0x01,0x1e,'G');
            break;
        case 'g':
            putUnicodeChar(0x01,0x1f,'g');
            break;
        case 'I':
            putUnicodeChar(0x01,0x2c,'I');
            break;
        case 'i':
            putUnicodeChar(0x01,0x2d,'i');
            break;
        case 'O':
            putUnicodeChar(0x01,0x4e,'O');
            break;
        case 'o':
            putUnicodeChar(0x01,0x4f,'o');
            break;
        case 'U':
            putUnicodeChar(0x01,0x6c,'U');
            break;
        case 'c':
            putUnicodeChar(0x01,0x0D,'c');
            break;
        case '\0':
            putUnicodeChar(0x02,0xD8,'u');
            break;
        default:
            fprintRTF("{\\f%d",n);
            fprintRTF("\\u774u}");  /* unicode combining character 0x0306 */
            ConvertString(cParam);
            break;  
    }

    free(cParam);
}

/*****************************************************************************
 purpose: converts \U{i} ... probably specific to ot2
 ******************************************************************************/
void CmdWideBreveChar(int code)
{
    int n = RtfFontNumber("STIXGeneral");
    char *cParam = getBraceParam();
    
    switch (cParam[0]) {
        case 'U':
            CmdUnicodeChar(0x040E);
            break;
        case 'I':
            CmdUnicodeChar(0x0419);
            break;
        case 'i':
            CmdUnicodeChar(0x0439);
            break;
        case 'u':
            CmdUnicodeChar(0x0439);
            break;
        default:
            fprintRTF("{\\f%d",n);
            fprintRTF("\\u774u}");  /* unicode combining character 0x0306 */
            ConvertString(cParam);
            break;
    }
        
    free(cParam);
}

/******************************************************************************
 purpose: converts \v{a}
          need something that looks like \\O(a,\\S(\f1\'da)) in RTF file
 ******************************************************************************/
void CmdCaronChar(int code)
{
    int n = RtfFontNumber("STIXGeneral");
    char *cParam = getBraceParam();

    switch (cParam[0]) {
        case 'C':
            putUnicodeChar(0x01,0x0C,'C');
            break;
        case 'c':
            putUnicodeChar(0x01,0x0D,'c');
            break;
        case 'D':
            putUnicodeChar(0x01,0x0E,'D');
            break;
        case 'd':
            putUnicodeChar(0x01,0x0F,'d');
            break;
        case 'E':
            putUnicodeChar(0x01,0x1A,'E');
            break;
        case 'e':
            putUnicodeChar(0x01,0x1B,'e');
            break;
        case 'L':
            putUnicodeChar(0x01,0x3D,'L');
            break;
        case 'l':
            putUnicodeChar(0x01,0x3E,'l');
            break;
        case 'N':
            putUnicodeChar(0x01,0x47,'N');
            break;
        case 'n':
            putUnicodeChar(0x01,0x48,'n');
            break;
        case 'R':
            putUnicodeChar(0x01,0x58,'R');
            break;
        case 'r':
            putUnicodeChar(0x01,0x59,'r');
            break;
        case 'S':
            putUnicodeChar(0x01,0x60,'S');
            break;
        case 's':
            putUnicodeChar(0x01,0x61,'s');
            break;
        case 'T':
            putUnicodeChar(0x01,0x64,'T');
            break;
        case 't':
            putUnicodeChar(0x01,0x65,'t');
            break;
        case 'Z':
            putUnicodeChar(0x01,0x7D,'Z');
            break;
        case 'z':
            putUnicodeChar(0x01,0x7E,'z');
            break;
        case 'A':
            putUnicodeChar(0x01,0xCD,'A');
            break;
        case 'a':
            putUnicodeChar(0x01,0xCE,'a');
            break;
        case 'I':
            putUnicodeChar(0x01,0xCF,'I');
            break;
        case 'i':
            putUnicodeChar(0x01,0xD0,'i');
            break;
        case 'O':
            putUnicodeChar(0x01,0xD1,'O');
            break;
        case 'o':
            putUnicodeChar(0x01,0xD2,'o');
            break;
        case 'U':
            putUnicodeChar(0x01,0xD3,'U');
            break;
        case 'u':
            putUnicodeChar(0x01,0xD4,'u');
            break;
        case 'G':
            putUnicodeChar(0x01,0xE6,'G');
            break;
        case 'g':
            putUnicodeChar(0x01,0xE7,'g');
            break;
        case 'K':
            putUnicodeChar(0x01,0xE8,'K');
            break;
        case 'k':
            putUnicodeChar(0x01,0xE9,'k');
            break;
        case 'j':
            putUnicodeChar(0x01,0xF0,'j');
            break;
        case 'H':
            putUnicodeChar(0x02,0x1E,'H');
            break;
        case 'h':
            putUnicodeChar(0x02,0x1F,'h');
            break;
        case '\0':
            putUnicodeChar(0x02,0xC7,'-');
            break;
        default:
            fprintRTF("{\\f%d",n);
            fprintRTF("\\u780-}");  /* unicode combining character 0x030C */
            ConvertString(cParam);
            break;  
    }

    free(cParam);
}

/******************************************************************************
 purpose: converts \.{o} and \dot{o} from LaTeX to RTF
          need something that looks like \\O(a,\\S(\f2\'26)) in RTF file
 ******************************************************************************/
void CmdDotChar(int code)
{
    int n = RtfFontNumber("STIXGeneral");
    char *cParam = getBraceParam();
        
    switch (cParam[0]) {
        case 'A':
            fprintRTF("\\u550A");
            break;  
        case 'a':
            fprintRTF("\\u551a");
            break;  
        case 'C':
            fprintRTF("\\u266C");
            break;              
        case 'c':
            fprintRTF("\\u267c");
            break;              
        case 'E':
            fprintRTF("\\u278E");
            break;              
        case 'e':
            fprintRTF("\\u279e");
            break;              
        case 'G':
            fprintRTF("\\u288G");
            break;              
        case 'g':
            fprintRTF("\\u289g");
            break;              
        case 'I':
            fprintRTF("\\u304I");
            break;              
        case 'O':
            fprintRTF("\\u558O");
            break;              
        case 'o':
            fprintRTF("\\u559o");
            break;              
        case 'Z':
            fprintRTF("\\u379Z");
            break;              
        case 'z':
            fprintRTF("\\u380z");
            break;
        case '\0':
            putUnicodeChar(0x02,0xD9,'.');
            break;
        default:
            fprintRTF("{\\f%d",n);
            fprintRTF("\\u775.}"); /* unicode combining character 0x0307 */
            ConvertString(cParam);
            break;  
    }

    free(cParam);
}

/******************************************************************************
 purpose: converts chars with dots underneath  \d{o}
 ******************************************************************************/
void CmdUnderdotChar(int code)
{
    int n = RtfFontNumber("STIXGeneral");
    char *cParam = getBraceParam();

    if (strstr(cParam, "\\i")) {
        fprintRTF("\\u301i");
        free(cParam);
    }
    
    switch (cParam[0]) {
        case 'B':
            putUnicodeChar(0x1E,0x04,'B');
            break;
        case 'b':
            putUnicodeChar(0x1E,0x05,'b');
            break;
        case 'D':
            putUnicodeChar(0x1E,0x0C,'D');
            break;
        case 'd':
            putUnicodeChar(0x1E,0x0D,'d');
            break;
        case 'H':
            putUnicodeChar(0x1E,0x24,'H');
            break;
        case 'h':
            putUnicodeChar(0x1E,0x25,'h');
            break;
        case 'K':
            putUnicodeChar(0x1E,0x32,'K');
            break;
        case 'k':
            putUnicodeChar(0x1E,0x33,'k');
            break;
        case 'L':
            putUnicodeChar(0x1E,0x36,'L');
            break;
        case 'l':
            putUnicodeChar(0x1E,0x37,'l');
            break;
        case 'M':
            putUnicodeChar(0x1E,0x42,'M');
            break;
        case 'm':
            putUnicodeChar(0x1E,0x43,'m');
            break;
        case 'N':
            putUnicodeChar(0x1E,0x46,'N');
            break;
        case 'n':
            putUnicodeChar(0x1E,0x47,'n');
            break;
        case 'R':
            putUnicodeChar(0x1E,0x5A,'R');
            break;
        case 'r':
            putUnicodeChar(0x1E,0x5B,'r');
            break;
        case 'S':
            putUnicodeChar(0x1E,0x62,'S');
            break;
        case 's':
            putUnicodeChar(0x1E,0x63,'s');
            break;
        case 'V':
            putUnicodeChar(0x1E,0x7E,'V');
            break;
        case 'v':
            putUnicodeChar(0x1E,0x7F,'v');
            break;
        case 'W':
            putUnicodeChar(0x1E,0x88,'W');
            break;
        case 'w':
            putUnicodeChar(0x1E,0x89,'w');
            break;
        case 'Z':
            putUnicodeChar(0x1E,0x92,'Z');
            break;
        case 'z':
            putUnicodeChar(0x1E,0x93,'z');
            break;
        case 'A':
            putUnicodeChar(0x1E,0xA0,'A');
            break;
        case 'a':
            putUnicodeChar(0x1E,0xA1,'a');
            break;
        case 'E':
            putUnicodeChar(0x1E,0xB8,'E');
            break;
        case 'e':
            putUnicodeChar(0x1E,0xB9,'e');
            break;
        case 'I':
            putUnicodeChar(0x1E,0xCA,'I');
            break;
        case 'i':
            putUnicodeChar(0x1E,0xCB,'i');
            break;
        case 'O':
            putUnicodeChar(0x1E,0xCC,'O');
            break;
        case 'o':
            putUnicodeChar(0x1E,0xCD,'o');
            break;
        case 'U':
            putUnicodeChar(0x1E,0xE4,'U');
            break;
        case 'u':
            putUnicodeChar(0x1E,0xE5,'u');
            break;
        case '\0':
            fprintRTF(" \\u803.");
            break;
        default:
            fprintRTF("{\\f%d",n);
            fprintRTF("\\u803.}"); /* unicode combining character 0x0323 */
            ConvertString(cParam);
            break;  
    }
    
    free(cParam);
}

/*****************************************************************************
 purpose: converts \vec{o} from LaTeX to RTF
 ******************************************************************************/
void CmdVecChar(int code)
{
    int n = RtfFontNumber("STIXGeneral");
    char *cParam = getBraceParam();
    fprintRTF("{\\f%d ",n);
    putUnicodeChar(0x20,0xD7,'-');  /* COMBINING RIGHT ARROW ABOVE */
    fprintRTF("}");
    ConvertString(cParam);
    free(cParam);
}

/******************************************************************************
 purpose: converts \b{o} 
 ******************************************************************************/
void CmdUnderbarChar(int code)
{
    char *cParam= getBraceParam();

    switch (cParam[0]) {
        case 'B':
            putUnicodeChar(0x1E,0x06,'B');
            break;
        case 'b':
            putUnicodeChar(0x1E,0x07,'b');
            break;
        case 'D':
            putUnicodeChar(0x1E,0x0E,'D');
            break;
        case 'd':
            putUnicodeChar(0x1E,0x0F,'d');
            break;
        case 'K':
            putUnicodeChar(0x1E,0x34,'K');
            break;
        case 'k':
            putUnicodeChar(0x1E,0x35,'k');
            break;
        case 'L':
            putUnicodeChar(0x1E,0x3A,'L');
            break;
        case 'l':
            putUnicodeChar(0x1E,0x3B,'l');
            break;
        case 'N':
            putUnicodeChar(0x1E,0x48,'N');
            break;
        case 'n':
            putUnicodeChar(0x1E,0x49,'n');
            break;
        case 'R':
            putUnicodeChar(0x1E,0x5E,'R');
            break;
        case 'r':
            putUnicodeChar(0x1E,0x5F,'r');
            break;
        case 'T':
            putUnicodeChar(0x1E,0x6E,'T');
            break;
        case 't':
            putUnicodeChar(0x1E,0x6F,'t');
            break;
        case 'Z':
            putUnicodeChar(0x1E,0x94,'Z');
            break;
        case 'z':
            putUnicodeChar(0x1E,0x95,'z');
            break;
        case 'h':
            putUnicodeChar(0x1E,0x96,'h');
            break;
        case '\0':
            fprintRTF("_");
            break;
        default:
            fprintRTF("\\u817_"); /* unicode combining character 0x0331 */
            ConvertString(cParam);
            break;  
    }
        
    free(cParam);
}

/******************************************************************************
 purpose: converts \i and \j to 'i' and 'j' or Unicode equivalent
 ******************************************************************************/
void CmdDotlessChar(int code)
{
    if (code == 0)
        fprintRTF("\\u305i");  /*LATIN SMALL LETTER DOTLESS I 0x0131 */
    else
        fprintRTF("\\u567j");  /*LATIN SMALL LETTER DOTLESS J 0x0237 */
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

static int identifyBase(char c)
{
    if (c == '\'')
        return 8;
    else if (c == '"')
        return 16;
    else if (c == '`')     /* next character is treated differently */
        return -1;
    else if (isdigit(c))
        return 10;
    else 
        return 0;
}

static int isOctal(int c)
{
    if ((int) '0' <= (int) c && (int) c <= (int) '7') return TRUE;
    return FALSE;
}

static int isHex(int c)
{
    if (isdigit(c)) return TRUE;
    if ((int) 'A' <= (int) c && (int) c <= (int) 'F') return TRUE;
    if ((int) 'a' <= (int) c && (int) c <= (int) 'f') return TRUE;
    return FALSE;
}

/******************************************************************************
 purpose: 
        code = 0, handles \char'35 or \char"35 or \char35 or \char`b
        code = 1, handles \symbol{\'22} or \symbol{\"22}
 ******************************************************************************/
void CmdSymbol(int code)
{
    char c, *s, *t;
    int n, base;

    if (code == 0) {
    
        char num[4];
        int i;
    
        c = getNonSpace();
        base = identifyBase(c);
        
        if (base == 0) {
            diagnostics(1,"malformed \\char construction");
            fprintRTF("%c",c);
            return;
        }
            
        if (base == -1) {
            c = getTexChar();   /* \char`b case */
            CmdChar((int) c);
            return;
        }
        
        if (base == 10) 
            ungetTexChar(c);
        
        /* read sequence of digits */
        for (i=0; i<4; i++) {
            num[i] = getTexChar();
            if (base == 10 && ! isdigit(num[i]) ) break;
            if (base == 8  && ! isOctal(num[i]) ) break;
            if (base == 16 && ! isHex  (num[i]) ) break;
         }
        ungetTexChar(num[i]);
        num[i] = '\0';
        
        n = (int) strtol(num,&s,base);
        CmdChar(n);
        
    } else {

        s = getBraceParam();
        t = strdup_noendblanks(s);
        free(s);
        
        base = identifyBase(*t);
        
        if (base == 0)
            return;
            
        if (base == -1) {
            CmdChar((int) *(t+1));   /* \char`b case */
            return;
        }

        n = (int) strtol(t+1,&s,base);
        CmdChar(n);
        free(t);
    }
    
}

static void TeXlogo(void )

/******************************************************************************
 purpose : prints the Tex logo in the RTF-File (D Taupin)
 ******************************************************************************/
{
    int dnsize;

    dnsize = (int) (0.3 * CurrentFontSize() + 0.45);
    fprintRTF("T{\\dn%d E}X", dnsize);
}

static void LaTeXlogo(void)

/******************************************************************************
 purpose : prints the LaTeX logo in the RTF-File (D Taupin)
 ******************************************************************************/
{
    float FloatFsize;
    int upsize, Asize;

    if (CurrentFontSize() > 14)
        FloatFsize = (float) (0.8 * CurrentFontSize());
    else
        FloatFsize = (float) (0.9 * CurrentFontSize());
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
    int dnsize;

    changeTexMode(MODE_HORIZONTAL);
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
            dnsize = (int) (0.3 * CurrentFontSize() + 0.45);
            fprintRTF("2{\\dn%d", dnsize);
            putUnicodeChar(0x03,0xF5,'e');
            fprintRTF("}");
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
        FloatFsize = (float) (FloatFsize * 0.75);

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

/******************************************************************************
 * purpose : \celcius and \degreecelsius from SIUnits.sty
 * ******************************************************************************/
void CmdDegreeCelsius(int code) 
{
  fprintRTF("\\'b0C");
}

void CmdNot(int code)
{
    char c, *s;
    c = getTexChar();
    
    switch (c) {
    
        case '=':
            CmdUnicodeChar(8800);
            break;
            
        case '<':
            CmdUnicodeChar(8814);
            break;
        
        case '>':
            CmdUnicodeChar(8815);
            break;
            
        case '\\':
            ungetTexChar(c);
            s = getSimpleCommand();
            
            if (strcmp(s,"\\leq")==0) 
                CmdUnicodeChar(8816);
            else if (strcmp(s,"\\geq")==0) 
                CmdUnicodeChar(8817);
            else if (strcmp(s,"\\equiv")==0) 
                CmdUnicodeChar(8802);
            else if (strcmp(s,"\\prec")==0) 
                CmdUnicodeChar(8832);
            else if (strcmp(s,"\\succ")==0) 
                CmdUnicodeChar(8833);
            else if (strcmp(s,"\\sim")==0) 
                CmdUnicodeChar(8769);
            else if (strcmp(s,"\\preceq")==0) 
                CmdUnicodeChar(8928);
            else if (strcmp(s,"\\succeq")==0) 
                CmdUnicodeChar(8929);
            else if (strcmp(s,"\\simeq")==0) 
                CmdUnicodeChar(8772);
            else if (strcmp(s,"\\subset")==0) 
                CmdUnicodeChar(8836);
            else if (strcmp(s,"\\supset")==0) 
                CmdUnicodeChar(8837);
            else if (strcmp(s,"\\approx")==0) 
                CmdUnicodeChar(8777);
            else if (strcmp(s,"\\subseteq")==0) 
                CmdUnicodeChar(8840);
            else if (strcmp(s,"\\supseteq")==0) 
                CmdUnicodeChar(8841);
            else if (strcmp(s,"\\cong")==0) 
                CmdUnicodeChar(8775);
            else if (strcmp(s,"\\sqsubseteq")==0) 
                CmdUnicodeChar(8930);
            else if (strcmp(s,"\\sqsupseteq")==0) 
                CmdUnicodeChar(8931);
            else if (strcmp(s,"\\asymp")==0) 
                CmdUnicodeChar(8813);
            else {
                fprintRTF("/");
                ConvertString(s);
            }
            if (s) free(s);
            break;
        default:
            fprintRTF("/");
            ungetTexChar(c);
            break;
    }
}

