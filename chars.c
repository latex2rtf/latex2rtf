
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
#include "math.h"
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
static void putOverstrikeChar(const char *font, char *s, 
                              unsigned int overstrike, double raise)
{
	if (g_processing_fields==0) fprintRTF("{\\field{\\*\\fldinst EQ ");
		
	fprintRTF("\\\\O(");
    ConvertString(s);
	fprintRTF("%c {", g_field_separator);
	
	if (raise != 0) {
		fprintRTF("\\up%d ", (int) floor(raise * CurrentFontSize()));
	}
	
	if (strcmp(font,"simple")==0 ) {
	
		fprintRTF("\\'%x",overstrike);	
		
	} else if (strcmp(font,"unicode")==0 ) {
	
		fprintRTF("\\u%u\\'%x",overstrike,overstrike);

	} else {  /* seems to only work for MT Extra */

		fprintRTF("{\\field{\\*\\fldinst SYMBOL %u \\\\f ", overstrike);
		fprintRTF("\"%s\" \\\\h}{\\fldrslt }}", font);

	}
	
	fprintRTF("})");

	if (g_processing_fields==0) fprintRTF("}{\\fldrslt }}");
}

/* this will emit a unicode character. 
   RTF requires that unicode values above 2^15 be negative
   The default character is what the RTF parser should use if it does not know
   the unicode equivalent.
*/
static void putUnicodeChar(unsigned char b1, unsigned char b2, char default_char)
{
	if (b1<128)
		fprintRTF("\\u%d%c",b1*256+b2,default_char);
	else
		fprintRTF("\\u%d%c",b1*256+b2-65536,default_char);
}

void CmdUmlauteChar(int code)

/*****************************************************************************
 purpose : converts characters with diaeresis, e.g., \"{a} or \ddot{a}
 ******************************************************************************/
{
    int done = 0;
    char *cParam = getBraceParam();

    if (cParam == NULL)
        return;

	/* These encodings will fail in equation field translation */
    if (g_processing_fields==0) {
		switch (cParam[0]) {
			case 'o':
				fprintRTF("\\'f6");
                done = 1;
				break;
			case 'O':
				fprintRTF("\\'d6");
                done = 1;
				break;
			case 'a':
				fprintRTF("\\'e4");
                done = 1;
				break;
			case 'A':
				fprintRTF("\\'c4");
                done = 1;
				break;
			case 'u':
				fprintRTF("\\'fc");
                done = 1;
				break;
			case 'U':
				fprintRTF("\\'dc");
                done = 1;
				break;
			case 'E':
				fprintRTF("\\'cb");
                done = 1;
				break;
			case 'I':
				fprintRTF("\\'cf");
                done = 1;
				break;
			case 'e':
				fprintRTF("\\'eb");
                done = 1;
				break;
			case 'y':
				fprintRTF("\\'ff");
                done = 1;
				break;
			case 'Y':
				fprintRTF("\\u376Y");
                done = 1;
				break;
		}

		if (strcmp(cParam, "\\i") == 0) {
			fprintRTF("\\'ef");
			done = 1;
		}
	}

	if (!done)
		putOverstrikeChar("unicode", cParam, 168, 0);

    free(cParam);
}

void CmdGraveChar(int code)

/******************************************************************************
 purpose: converts symbols with grave accents (\`{a}) from LaTeX to RTF
 ******************************************************************************/
{
    int done = 0;
    char *cParam = getBraceParam();

    if (cParam == NULL)
        return;

	/* These encodings will fail in equation field translation */
    if (g_processing_fields==0) {
		switch (cParam[0]) {
			case 'A':
				fprintRTF("\\'c0");
				done = 1;
				break;
			case 'E':
				fprintRTF("\\'c8");
				done = 1;
				break;
			case 'I':
				fprintRTF("\\'cc");
				done = 1;
				break;
			case 'O':
				fprintRTF("\\'d2");
				done = 1;
				break;
			case 'U':
				fprintRTF("\\'d9");
				done = 1;
				break;
			case 'a':
				fprintRTF("\\'e0");
				done = 1;
				break;
			case 'e':
				fprintRTF("\\'e8");
				done = 1;
				break;
			case 'i':
				fprintRTF("\\'ec");
				done = 1;
				break;
			case 'o':
				fprintRTF("\\'f2");
				done = 1;
				break;
			case 'u':
				fprintRTF("\\'f9");
				done = 1;
				break;
			case 'N':
				fprintRTF("\\u504N");
				done = 1;
				break;            
			case 'n':
				fprintRTF("\\u505n");
				done = 1;
				break;
		}
	
		if (strcmp(cParam, "\\i") == 0) {
			fprintRTF("\\'ed");
			done = 1;
		}

	}

	if (!done)
		putOverstrikeChar("MT Extra", cParam, 35, 0);

    free(cParam);
}

void CmdAcuteChar(int code)

/******************************************************************************
 purpose: converts symbols with acute accents (\'{a}) from LaTeX to RTF
 ******************************************************************************/
{
    int done = 0;
    char *cParam = getBraceParam();

    if (cParam == NULL)
        return;

	/* These encodings will fail in equation field translation */
    if (g_processing_fields==0) {
		switch (cParam[0]) {
			case 'A':
				fprintRTF("\\'c1");
				done=1;
				break;
			case 'E':
				fprintRTF("\\'c9");
				done=1;
				break;
			case 'I':
				fprintRTF("\\'cd");
				done=1;
				break;
			case 'O':
				fprintRTF("\\'d3");
				done=1;
				break;
			case 'U':
				fprintRTF("\\'da");
				done=1;
				break;
			case 'a':
				fprintRTF("\\'e1");
				done=1;
				break;
			case 'e':
				fprintRTF("\\'e9");
				done=1;
				break;
			case 'i':
				fprintRTF("\\'ed");
				done=1;
				break;
			case 'o':
				fprintRTF("\\'f3");
				done=1;
				break;
			case 'u':
				fprintRTF("\\'fa");
				done=1;
				break;
			case 'y':
				fprintRTF("\\'fd");
				done=1;
				break;
			case 'Y':
				fprintRTF("\\'dd");
				done=1;
				break;	
			case 'C':
				fprintRTF("\\u262C");
				done=1;
				break;
			case 'c':
				fprintRTF("\\u263c");
				done=1;
				break;
			case 'G':
				fprintRTF("\\u500G");
				done=1;
				break;
			case 'g':
				fprintRTF("\\u501g");
				done=1;
				break;
			case 'L':
				fprintRTF("\\u313L");
				done=1;
				break;
			case 'l':
				fprintRTF("\\u314l");
				done=1;
				break;
			case 'N':
				fprintRTF("\\u323N");
				done=1;
				break;
			case 'n':
				fprintRTF("\\u324n");
				done=1;
				break;
			case 'R':
				fprintRTF("\\u340R");
				done=1;
				break;
			case 'r':
				fprintRTF("\\u341r");
				done=1;
				break;
			case 'S':
				fprintRTF("\\u346S");
				done=1;
				break;
			case 's':
				fprintRTF("\\u347s");
				done=1;
				break;
			case 'Z':
				fprintRTF("\\u377Z");
				done=1;
				break;
			case 'z':
				fprintRTF("\\u378z");
				done=1;
				break;
		}
			
		if (strcmp(cParam, "\\i") == 0) {
			fprintRTF("\\'ec");
			done=1;
		}
	}
			
/*   Nothing seems to work when converting to an equation.  This
     is simple and at least shows up in Word                          */

	if (!done)
		putOverstrikeChar("unicode", cParam, 180, 0);

    free(cParam);
}

/******************************************************************************
 purpose: converts \H{c}  must
 ******************************************************************************/
void CmdDoubleAcuteChar(int code)
{
    int done = 0;
    char *cParam = getBraceParam();

    if (cParam == NULL)
        return;

	/* These encodings will fail in equation field translation */
    if (g_processing_fields==0) {
		switch (cParam[0]) {
			case 'O':
				putUnicodeChar(0x01,0x50,'O');
				done = 1;
				break;
			case 'o':
				putUnicodeChar(0x01,0x51,'o');
				done = 1;
				break;
			case 'U':
				putUnicodeChar(0x01,0x70,'U');
				done = 1;
				break;
			case 'u':
				putUnicodeChar(0x01,0x71,'u');
				done = 1;
				break;
		}
	}
	
	if (!done)
		putOverstrikeChar("MT Extra", cParam, 251, 0);
		
    free(cParam);
}

void CmdMacronChar(int code)

/******************************************************************************
 purpose: converts special symbols from LaTeX to RTF
 ******************************************************************************/
{
    int done = 0;
    char *cParam = getBraceParam();

    if (cParam == NULL)
        return;

	/* These encodings will fail in equation field translation */
    if (g_processing_fields==0) {
		switch (cParam[0]) {
			case 'A':
				putUnicodeChar(0x01,0x00,'A');
				done = 1;
				break;
			case 'a':
				putUnicodeChar(0x01,0x01,'a');
				done = 1;
				break;
			case 'E':
				putUnicodeChar(0x01,0x12,'E');
				done = 1;
				break;
			case 'e':
				putUnicodeChar(0x01,0x13,'e');
				done = 1;
				break;
			case 'I':
				putUnicodeChar(0x01,0x2A,'l');
				done = 1;
				break;
			case 'i':
				putUnicodeChar(0x01,0x2B,'i');
				done = 1;
				break;
			case 'O':
				putUnicodeChar(0x01,0x4C,'O');
				done = 1;
				break;
			case 'o':
				putUnicodeChar(0x01,0x4D,'o');
				done = 1;
				break;
			case 'U':
				putUnicodeChar(0x01,0x6A,'U');
				done = 1;
				break;
			case 'u':
				putUnicodeChar(0x01,0x6B,'u');
				done = 1;
				break;
			case 'Y':
				putUnicodeChar(0x02,0x32,'Y');
				done = 1;
				break;
			case 'y':
				putUnicodeChar(0x02,0x33,'y');
				done = 1;
				break;
		}
	}
	
	if (!done)
		putOverstrikeChar("unicode", cParam, 175, 0);

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
		putOverstrikeChar("MT Extra", cParam, 36, 0);
		
	free(cParam);
}

void CmdOaccentChar(int code)

/******************************************************************************
 purpose: converts \r accents from LaTeX to RTF
 ******************************************************************************/
{
    int done = 0;
    char *cParam = getBraceParam();
	
    if (cParam == NULL)
        return;

    diagnostics(4,"CmdOaccentChar letter='%s' in eq field=%d",cParam,
                g_processing_fields);
    
    if (g_processing_fields==0) {
		switch (cParam[0]) {
			case 'A':
				fprintRTF("\\'c5");
				done = 1;
				break;
	
			case 'a':
				fprintRTF("\\'e5");
				done = 1;
				break;
		}
		
		if (strcmp(cParam, "\\i") == 0) {
			fprintRTF("\\'ee");
			done = 1;
		}
	}

	if (!done)
		putOverstrikeChar("MT Extra", cParam, 254, 0);
		
    free(cParam);
}

void CmdTildeChar(int code)

/******************************************************************************
 purpose: converts \~{n} from LaTeX to RTF
 ******************************************************************************/
{
	int done = 0;
    char *cParam = getBraceParam();
	
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
		putOverstrikeChar("MT Extra", cParam, 37, 0);
		
	free(cParam);
}

void CmdCedillaChar(int code)

/*****************************************************************************
 purpose: converts \c{c} from LaTeX to RTF
 ******************************************************************************/
{
	int done = 0;
    char *cParam = getBraceParam();
	
    if (cParam == NULL) return;
	
    diagnostics(4,"CmdCedillaChar letter='%s' in eq field=%d",cParam,
                g_processing_fields);
    
	/* These fail in equation fields */
    if (g_processing_fields==0) {
		switch (cParam[0]) {
			case 'C':
				fprintRTF("\\'c7");
				done = 1;
				break;
			case 'c':
				fprintRTF("\\'e7");
				done = 1;
				break;
			case 'G':
				putUnicodeChar(0x01,0x22,'G');
				done = 1;
				break;
			case 'g':
				putUnicodeChar(0x01,0x23,'g');
				done = 1;
				break;
			case 'K':
				putUnicodeChar(0x01,0x36,'K');
				done = 1;
				break;
			case 'k':
				putUnicodeChar(0x01,0x37,'k');
				done = 1;
				break;
			case 'L':
				putUnicodeChar(0x01,0x3B,'L');
				done = 1;
				break;
			case 'l':
				putUnicodeChar(0x01,0x3C,'l');
				done = 1;
				break;
			case 'N':
				putUnicodeChar(0x01,0x45,'N');
				done = 1;
				break;
			case 'n':
				putUnicodeChar(0x01,0x46,'n');
				done = 1;
				break;
			case 'R':
				putUnicodeChar(0x01,0x56,'R');
				done = 1;
				break;
			case 'r':
				putUnicodeChar(0x01,0x57,'r');
				done = 1;
				break;
			case 'S':
				putUnicodeChar(0x01,0x5E,'S');
				done = 1;
				break;
			case 's':
				putUnicodeChar(0x01,0x5F,'s');
				done = 1;
				break;
			case 'T':
				putUnicodeChar(0x01,0x62,'T');
				done = 1;
				break;
			case 't':
				putUnicodeChar(0x01,0x63,'t');
				done = 1;
				break;
			case 'E':
				putUnicodeChar(0x02,0x28,'E');
				done = 1;
				break;
			case 'e':
				putUnicodeChar(0x02,0x29,'e');
				done = 1;
				break;
        }
    }
    
	if (!done) 
		putOverstrikeChar("MT Extra", cParam, 255, 0);


    free(cParam);
}

void CmdBreveChar(int code)

/*****************************************************************************
 purpose: converts \u{o} and \breve{o} 
 ******************************************************************************/
{
	int done = 0;
    char *cParam = getBraceParam();
	
    if (cParam == NULL) return;
	
    diagnostics(4,"CmdBreveChar letter='%s' in eq field=%d",cParam,
                g_processing_fields);
    
	/* These fail in equation fields */
    if (g_processing_fields==0) {
		switch (cParam[0]) {
			case 'A':
				putUnicodeChar(0x01,0x02,'A');
				done = 1;
				break;
			case 'a':
				putUnicodeChar(0x01,0x03,'a');
				done = 1;
				break;
			case 'E':
				putUnicodeChar(0x01,0x14,'E');
				done = 1;
				break;
			case 'e':
				putUnicodeChar(0x01,0x15,'e');
				done = 1;
				break;
			case 'G':
				putUnicodeChar(0x01,0x1e,'G');
				done = 1;
				break;
			case 'g':
				putUnicodeChar(0x01,0x1f,'g');
				done = 1;
				break;
			case 'I':
				putUnicodeChar(0x01,0x2c,'I');
				done = 1;
				break;
			case 'i':
				putUnicodeChar(0x01,0x2d,'i');
				done = 1;
				break;
			case 'O':
				putUnicodeChar(0x01,0x4e,'O');
				done = 1;
				break;
			case 'o':
				putUnicodeChar(0x01,0x4f,'o');
				done = 1;
				break;
			case 'U':
				putUnicodeChar(0x01,0x6c,'U');
				done = 1;
				break;
			case 'c':
				putUnicodeChar(0x01,0x0D,'c');
				done = 1;
				break;
		}
	}
	
	if (!done) 
		putOverstrikeChar("MT Extra", cParam, 252, 0);

    free(cParam);
}


void CmdHacekChar(int code)

/******************************************************************************
 purpose: converts \v from LaTeX to RTF
          need something that looks like \\O(a,\\S(\f1\'da)) in RTF file
 ******************************************************************************/
{
	int done = 0;
    char *cParam = getBraceParam();
	
    if (cParam == NULL) return;
	
    diagnostics(4,"CmdHacekChar letter='%s' in eq field=%d",cParam,
                g_processing_fields);
    
	/* These fail in equation fields */
    if (g_processing_fields==0) {
		switch (cParam[0]) {
			case 'C':
				putUnicodeChar(0x01,0x0C,'C');
				done = 1;
				break;
			case 'c':
				putUnicodeChar(0x01,0x0D,'c');
				done = 1;
				break;
			case 'D':
				putUnicodeChar(0x01,0x0E,'D');
				done = 1;
				break;
			case 'd':
				putUnicodeChar(0x01,0x0F,'d');
				done = 1;
				break;
			case 'E':
				putUnicodeChar(0x01,0x1A,'E');
				done = 1;
				break;
			case 'e':
				putUnicodeChar(0x01,0x1B,'e');
				done = 1;
				break;
			case 'L':
				putUnicodeChar(0x01,0x3D,'L');
				done = 1;
				break;
			case 'l':
				putUnicodeChar(0x01,0x3E,'l');
				done = 1;
				break;
			case 'N':
				putUnicodeChar(0x01,0x47,'N');
				done = 1;
				break;
			case 'n':
				putUnicodeChar(0x01,0x48,'n');
				done = 1;
				break;
			case 'R':
				putUnicodeChar(0x01,0x58,'R');
				done = 1;
				break;
			case 'r':
				putUnicodeChar(0x01,0x59,'r');
				done = 1;
				break;
			case 'S':
				putUnicodeChar(0x01,0x60,'S');
				done = 1;
				break;
			case 's':
				putUnicodeChar(0x01,0x61,'s');
				done = 1;
				break;
			case 'T':
				putUnicodeChar(0x01,0x64,'T');
				done = 1;
				break;
			case 't':
				putUnicodeChar(0x01,0x65,'t');
				done = 1;
				break;
			case 'Z':
				putUnicodeChar(0x01,0x7D,'Z');
				done = 1;
				break;
			case 'z':
				putUnicodeChar(0x01,0x7E,'z');
				done = 1;
				break;
			case 'A':
				putUnicodeChar(0x01,0xCD,'A');
				done = 1;
				break;
			case 'a':
				putUnicodeChar(0x01,0xCE,'a');
				done = 1;
				break;
			case 'I':
				putUnicodeChar(0x01,0xCF,'I');
				done = 1;
				break;
			case 'i':
				putUnicodeChar(0x01,0xD0,'i');
				done = 1;
				break;
			case 'O':
				putUnicodeChar(0x01,0xD1,'O');
				done = 1;
				break;
			case 'o':
				putUnicodeChar(0x01,0xD2,'o');
				done = 1;
				break;
			case 'U':
				putUnicodeChar(0x01,0xD3,'U');
				done = 1;
				break;
			case 'u':
				putUnicodeChar(0x01,0xD4,'u');
				done = 1;
				break;
			case 'G':
				putUnicodeChar(0x01,0xE6,'G');
				done = 1;
				break;
			case 'g':
				putUnicodeChar(0x01,0xE7,'g');
				done = 1;
				break;
			case 'K':
				putUnicodeChar(0x01,0xE8,'K');
				done = 1;
				break;
			case 'k':
				putUnicodeChar(0x01,0xE9,'k');
				done = 1;
				break;
			case 'j':
				putUnicodeChar(0x01,0xF0,'j');
				done = 1;
				break;
			case 'H':
				putUnicodeChar(0x02,0x1E,'H');
				done = 1;
				break;
			case 'h':
				putUnicodeChar(0x02,0x1F,'h');
				done = 1;
				break;
		}
	}
	
	if (!done)
		putOverstrikeChar("MT Extra", cParam, 253, 0);

    free(cParam);
}

void CmdDotChar(int code)

/******************************************************************************
 purpose: converts \.{o} and \dot{o} from LaTeX to RTF
          need something that looks like \\O(a,\\S(\f2\'26)) in RTF file
 ******************************************************************************/
{
	int done = 0;
    char *cParam = getBraceParam();
	
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
		putOverstrikeChar("MT Extra", cParam, 38, 0);

	free(cParam);
}

void CmdUnderdotChar(int code)

/******************************************************************************
 purpose: converts chars with dots underneath  \d{o}
 ******************************************************************************/
{
	int done = 0;
    char *cParam = getBraceParam();

    if (cParam == NULL) return;
	
    diagnostics(4,"CmdUnderdotChar letter='%s' in eq field=%d",cParam,
                g_processing_fields);
    
	/* These encodings will fail in equation field translation */
    if (g_processing_fields==0) {
		switch (cParam[0]) {
			case 'B':
					putUnicodeChar(0x1E,0x04,'B');
					done = 1;
					break;
					
			case 'b':
					putUnicodeChar(0x1E,0x05,'b');
					done = 1;
					break;
					
			case 'D':
					putUnicodeChar(0x1E,0x0C,'D');
					done = 1;
					break;
					
			case 'd':
					putUnicodeChar(0x1E,0x0D,'d');
					done = 1;
					break;
					
			case 'H':
					putUnicodeChar(0x1E,0x24,'H');
					done = 1;
					break;
					
			case 'h':
					putUnicodeChar(0x1E,0x25,'h');
					done = 1;
					break;
					
			case 'K':
					putUnicodeChar(0x1E,0x32,'K');
					done = 1;
					break;
					
			case 'k':
					putUnicodeChar(0x1E,0x33,'k');
					done = 1;
					break;
					
			case 'L':
					putUnicodeChar(0x1E,0x36,'L');
					done = 1;
					break;
					
			case 'l':
					putUnicodeChar(0x1E,0x37,'l');
					done = 1;
					break;
					
			case 'M':
					putUnicodeChar(0x1E,0x42,'M');
					done = 1;
					break;
					
			case 'm':
					putUnicodeChar(0x1E,0x43,'m');
					done = 1;
					break;

			case 'N':
					putUnicodeChar(0x1E,0x46,'N');
					done = 1;
					break;
					
			case 'n':
					putUnicodeChar(0x1E,0x47,'n');
					done = 1;
					break;

			case 'R':
					putUnicodeChar(0x1E,0x5A,'R');
					done = 1;
					break;
					
			case 'r':
					putUnicodeChar(0x1E,0x5B,'r');
					done = 1;
					break;

			case 'S':
					putUnicodeChar(0x1E,0x61,'S');
					done = 1;
					break;
					
			case 's':
					putUnicodeChar(0x1E,0x62,'s');
					done = 1;
					break;

			case 'V':
					putUnicodeChar(0x1E,0x7E,'V');
					done = 1;
					break;
					
			case 'v':
					putUnicodeChar(0x1E,0x7F,'v');
					done = 1;
					break;

			case 'W':
					putUnicodeChar(0x1E,0x88,'W');
					done = 1;
					break;
					
			case 'w':
					putUnicodeChar(0x1E,0x89,'w');
					done = 1;
					break;

			case 'Z':
					putUnicodeChar(0x1E,0x92,'Z');
					done = 1;
					break;
					
			case 'z':
					putUnicodeChar(0x1E,0x93,'z');
					done = 1;
					break;

			case 'A':
					putUnicodeChar(0x1E,0xA0,'A');
					done = 1;
					break;
					
			case 'a':
					putUnicodeChar(0x1E,0xA1,'a');
					done = 1;
					break;

			case 'E':
					putUnicodeChar(0x1E,0xB8,'E');
					done = 1;
					break;
					
			case 'e':
					putUnicodeChar(0x1E,0xB9,'e');
					done = 1;
					break;

			case 'I':
					putUnicodeChar(0x1E,0xCA,'I');
					done = 1;
					break;
					
			case 'i':
					putUnicodeChar(0x1E,0xCB,'i');
					done = 1;
					break;

			case 'O':
					putUnicodeChar(0x1E,0xCC,'O');
					done = 1;
					break;
					
			case 'o':
					putUnicodeChar(0x1E,0xCD,'o');
					done = 1;
					break;

			case 'U':
					putUnicodeChar(0x1E,0xE5,'U');
					done = 1;
					break;
					
			case 'u':
					putUnicodeChar(0x1E,0xE6,'u');
					done = 1;
					break;
		}
		
		if (strcmp(cParam, "\\i") == 0) {
			fprintRTF("\\u301i");
			done = 1;
		}
	}
	
	if (!done)
		putOverstrikeChar("simple", cParam, 46, -0.25);
	
	 free(cParam);
}

/*****************************************************************************
 purpose: half-height or full-height character
 ******************************************************************************/
int isShort(char c) 
{

    switch (c) {
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
			return 1;
	}
	
	return 0;
}

void CmdVecChar(int code)

/*****************************************************************************
 purpose: converts \vec{o} from LaTeX to RTF
 ******************************************************************************/
{
    char *cParam = getBraceParam();

    if (cParam == NULL) return;
	
    diagnostics(4,"CmdVecChar letter='%s' in eq field=%d",cParam,
                g_processing_fields);
    
	if (isShort(cParam[0]))
		putOverstrikeChar("MT Extra", cParam, 114, 0.750);
	else
		putOverstrikeChar("MT Extra", cParam, 114, 1.000);

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

void CmdDotlessChar(int code)

/******************************************************************************
 purpose: converts \i and \j to 'i' and 'j' or Unicode equivalent
 ******************************************************************************/
{
    if (code == 0)
        fprintRTF("\\u305i");
    else
        fprintRTF("j");
}

void CmdPolishL(int code)

/******************************************************************************
 purpose: converts \l and \L to 'l' and 'L' or Unicode equivalent
 ******************************************************************************/
{
    if (code == 1)
       fprintRTF("\\u322L");
    else
       fprintRTF("\\u323l");
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
