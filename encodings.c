/* encodings.c - Translate high bit chars into RTF using codepage 1252

Copyright (C) 1995-2010 The Free Software Foundation

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
    1995-1997 Ralf Schlatterbeck
    1998-2000 Georg Lehner
    2001-2002 Scott Prahl
*/

#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "encoding_tables.h"
#include "encodings.h"
#include "fonts.h"
#include "chars.h"
#include "parser.h"
#include "vertical.h"

/*
Cyrillic letters with one-letter Roman equivalents can be entered `as
is': {\cyr a, b, v, g}, etc. This is also true for some letter
sequences that are treated as ligatures by the \LaTeX\ Cyrillic
fonts ({\cyr dj, zh, lj, nj, kh, ts, ch, sh, shch, yu} and {\cyr
ya}). Note that if you want to write the letter combination {\cyr
t{}s} you have to write \texttt{t\{\}s} in order not to produce a
{\cyr ts}.
*/

void CmdOT2Transliteration(int cThis)
{
    int cNext,cThird;
    switch (cThis) {
        case 'd':
            cNext = getTexChar();
            if (cNext == 'j')
                CmdUnicodeChar(0x0452); /* dj */
            else {
                ungetTexChar(cNext);
                CmdUnicodeChar(ot2Unicode[cThis]);
            }
            break;
        case 'z':
            cNext = getTexChar();
            if (cNext == 'h')
                CmdUnicodeChar(0x0436);  /*zh*/
            else {
                ungetTexChar(cNext);
                CmdUnicodeChar(ot2Unicode[cThis]);
            }
            break;
        case 'l':
            cNext = getTexChar();
            if (cNext == 'j')
                CmdUnicodeChar(0x0459);  /*lj*/
            else {
                ungetTexChar(cNext);
                CmdUnicodeChar(ot2Unicode[cThis]);
            }
            break;
        case 'n':
            cNext = getTexChar();
            if (cNext == 'j')
                CmdUnicodeChar(0x045A); /*nj */
            else {
                ungetTexChar(cNext);
                CmdUnicodeChar(ot2Unicode[cThis]);
            }
            break;
        case 'k':
            cNext = getTexChar();
            if (cNext == 'h')
                CmdUnicodeChar(0x0445);
            else {
                ungetTexChar(cNext);
                CmdUnicodeChar(ot2Unicode[cThis]);
            }
            break;
        case 't':
            cNext = getTexChar();
            if (cNext == 's')
                CmdUnicodeChar(0x0446); /* ts */
            else {
                ungetTexChar(cNext);
                CmdUnicodeChar(ot2Unicode[cThis]);
            }
            break;
        case 'c':
            cNext = getTexChar();
            if (cNext == 'h')
                CmdUnicodeChar(0x0447); /*ch*/
            else {
                ungetTexChar(cNext);
                CmdUnicodeChar(ot2Unicode[cThis]);
            }
            break;
        case 's':
            cNext = getTexChar();
            if (cNext != 'h') {
                CmdUnicodeChar(ot2Unicode[cThis]); /*s*/
                ungetTexChar(cNext);
                break;
            }
            cThird = getTexChar();
            if (cThird != 'c') {
                CmdUnicodeChar(0x0448);  /*sh*/
                ungetTexChar(cThird);
                break;
            }
            cNext = getTexChar();
            if (cNext != 'h') {
                CmdUnicodeChar(0x0448);  /*sh*/
                CmdUnicodeChar(ot2Unicode[cThird]); /*c*/
                ungetTexChar(cNext);
                break;
            }
            CmdUnicodeChar(0x0449);  /*shch*/
            break;
        case 'y':
            cNext = getTexChar();
            if (cNext == 'a') {
                CmdUnicodeChar(0x044F); /*ya*/
                ungetTexChar(cNext);
                break;
            }
            if (cNext == 'u') {
                CmdUnicodeChar(0x044E); /*yu*/
                ungetTexChar(cNext);
                break;
            }
            CmdUnicodeChar(ot2Unicode[cThis]); /*y*/
            ungetTexChar(cNext);
            break;

        case 'D':
            cNext = getTexChar();
            if (cNext == 'j' || cNext == 'J')
                CmdUnicodeChar(0x0402); /* dj */
            else {
                ungetTexChar(cNext);
                CmdUnicodeChar(ot2Unicode[cThis]);
            }
            break;
        case 'Z':
            cNext = getTexChar();
            if (cNext == 'h' || cNext == 'H')
                CmdUnicodeChar(0x0416);  /*zh*/
            else {
                ungetTexChar(cNext);
                CmdUnicodeChar(ot2Unicode[cThis]);
            }
            break;
        case 'L':
            cNext = getTexChar();
            if (cNext == 'j' || cNext == 'J')
                CmdUnicodeChar(0x0409);  /*lj*/
            else {
                ungetTexChar(cNext);
                CmdUnicodeChar(ot2Unicode[cThis]);
            }
            break;
        case 'N':
            cNext = getTexChar();
            if (cNext == 'j' || cNext == 'J')
                CmdUnicodeChar(0x040A); /*nj */
            else {
                ungetTexChar(cNext);
                CmdUnicodeChar(ot2Unicode[cThis]);
            }
            break;
        case 'K':
            cNext = getTexChar();
            if (cNext == 'h' || cNext == 'H')
                CmdUnicodeChar(0x0425);
            else {
                ungetTexChar(cNext);
                CmdUnicodeChar(ot2Unicode[cThis]);
            }
            break;
        case 'T':
            cNext = getTexChar();
            if (cNext == 's' || cNext == 'S')
                CmdUnicodeChar(0x0426); /* ts */
            else {
                ungetTexChar(cNext);
                CmdUnicodeChar(ot2Unicode[cThis]);
            }
            break;
        case 'C':
            cNext = getTexChar();
            if (cNext == 'h' || cNext == 'H')
                CmdUnicodeChar(0x0427); /*ch*/
            else {
                ungetTexChar(cNext);
                CmdUnicodeChar(ot2Unicode[cThis]);
            }
            break;
        case 'S':
            cNext = getTexChar();
            if (cNext != 'h' && cNext != 'H') {
                CmdUnicodeChar(ot2Unicode[cThis]); /*s*/
                ungetTexChar(cNext);
                break;
            }
            cThird = getTexChar();
            if (cThird != 'c' && cThird != 'c') {
                CmdUnicodeChar(0x0428);  /*sh*/
                ungetTexChar(cThird);
                break;
            }
            cNext = getTexChar();
            if (cNext != 'h' && cNext != 'H') {
                CmdUnicodeChar(0x0428);  /*sh*/
                CmdUnicodeChar(ot2Unicode[cThird]); /*c*/
                ungetTexChar(cNext);
                break;
            }
            CmdUnicodeChar(0x0429);  /*shch*/
            break;
        case 'Y':
            cNext = getTexChar();
            if (cNext == 'a' || cNext == 'A') {
                CmdUnicodeChar(0x042F); /*ya*/
                ungetTexChar(cNext);
                break;
            }
            if (cNext == 'u' || cNext == 'U') {
                CmdUnicodeChar(0x042E); /*yu*/
                ungetTexChar(cNext);
                break;
            }
            CmdUnicodeChar(ot2Unicode[cThis]); /*y*/
            ungetTexChar(cNext);
            break;

        default:
            CmdUnicodeChar(ot2Unicode[cThis]);
            break;
        }
                
}

/******************************************************************************
 purpose: emits a character based on the TeX encoding
          code is assumed to be in base 10
 ******************************************************************************/
void CmdChar(int code)
{
    char c;
    
    if (CurrentFontFamily() == TexFontNumber("Typewriter"))
        CmdUnicodeChar(cmttUnicode[code]);
    else
        CmdUnicodeChar(cmrUnicode[code]);

    c = getNonBlank();
    ungetTexChar(c);    
}


void WriteEightBitChar(unsigned char cThis, FILE *f)
{
    int eightbit_index;

    if ( cThis <= 127) {
        fputc(cThis, f);
        return;
    }

    eightbit_index = cThis - 128;
    diagnostics(6, "  WriteEightBitChar '%c' char=%3d index=%d encoding=%2d", cThis, (unsigned int) cThis, eightbit_index, CurrentFontEncoding());

    if (CurrentFontEncoding() == ENCODING_RAW)
        fprintRTF("\\'%2X", (unsigned char) cThis);
    else if (CurrentFontEncoding() == ENCODING_1251)
        CmdUnicodeChar(cp1251Unicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_APPLE)
        CmdUnicodeChar(appleUnicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_437)
        CmdUnicodeChar(cp437Unicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_850)
        CmdUnicodeChar(cp850Unicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_852)
        CmdUnicodeChar(cp852Unicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_855)
        CmdUnicodeChar(cp855Unicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_865)
        CmdUnicodeChar(cp865Unicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_866)
        CmdUnicodeChar(cp866Unicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_1250)
        CmdUnicodeChar(cp1250Unicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_1252)
        CmdUnicodeChar(cp1252Unicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_LATIN_2)
        CmdUnicodeChar(latin2Unicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_LATIN_3)
        CmdUnicodeChar(latin3Unicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_LATIN_4)
        CmdUnicodeChar(latin4Unicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_LATIN_5)
        CmdUnicodeChar(latin5Unicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_LATIN_9)
        CmdUnicodeChar(latin9Unicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_LATIN_10)
        CmdUnicodeChar(latin10Unicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_NEXT)
        CmdUnicodeChar(nextUnicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_KOI8_R)
        CmdUnicodeChar(koi8rUnicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_KOI8_U)
        CmdUnicodeChar(koi8uUnicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_DEC)
        CmdUnicodeChar(decmultiUnicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_APPLE_CE)
        CmdUnicodeChar(appleCEUnicode[eightbit_index]);
    else if (CurrentFontEncoding() == ENCODING_APPLE_CYRILLIC)
        CmdUnicodeChar(appleCyrrilicUnicode[eightbit_index]);
}
