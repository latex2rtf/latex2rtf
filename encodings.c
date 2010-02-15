
/* encodings.c - Translate high bit chars into RTF using codepage 1252

Copyright (C) 1995-2002 The Free Software Foundation

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
    diagnostics(6, "WriteEightBitChar char=%d index=%d encoding=%d", (unsigned int) cThis, eightbit_index, CurrentFontEncoding());

    if (CurrentFontEncoding() == ENCODING_RAW)
        fprintRTF("\\'%2X", (unsigned char) cThis);
    else if (CurrentFontEncoding() == ENCODING_1251)
        CmdUnicodeChar(eightbit_index+128);
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
