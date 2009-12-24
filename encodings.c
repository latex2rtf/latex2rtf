
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
	
	if (CurrentFontFamily() ==TexFontNumber("Typewriter"))
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
    diagnostics(6, "WriteEightBitChar char=%d index=%d encoding=%s", (unsigned int) cThis, eightbit_index, g_charset_encoding_name);

    if (strcmp(g_charset_encoding_name, "raw") == 0)
        fprintRTF("\\'%2X", (unsigned char) cThis);
    else if (strcmp(g_charset_encoding_name, "latin1") == 0)
        CmdUnicodeChar(eightbit_index+128);
    else if (strcmp(g_charset_encoding_name, "applemac") == 0)
		CmdUnicodeChar(appleUnicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "cp437") == 0)
		CmdUnicodeChar(cp437Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "cp850") == 0)
		CmdUnicodeChar(cp850Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "cp852") == 0)
		CmdUnicodeChar(cp852Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "cp865") == 0)
		CmdUnicodeChar(cp865Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "437") == 0)
		CmdUnicodeChar(cp437Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "850") == 0)
		CmdUnicodeChar(cp850Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "852") == 0)
		CmdUnicodeChar(cp852Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "865") == 0)
		CmdUnicodeChar(cp865Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "cp1250") == 0)
		CmdUnicodeChar(cp1250Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "cp1252") == 0)
		CmdUnicodeChar(cp1252Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "1250") == 0)
		CmdUnicodeChar(cp1250Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "1252") == 0)
		CmdUnicodeChar(cp1252Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "latin2") == 0)
		CmdUnicodeChar(latin2Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "latin3") == 0)
		CmdUnicodeChar(latin3Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "latin4") == 0)
		CmdUnicodeChar(latin4Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "latin5") == 0)
		CmdUnicodeChar(latin5Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "latin9") == 0)
		CmdUnicodeChar(latin9Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "latin10") == 0)
		CmdUnicodeChar(latin10Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "next") == 0)
		CmdUnicodeChar(nextUnicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "cp1251") == 0)
		CmdUnicodeChar(cp1251Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "cp855") == 0)
		CmdUnicodeChar(cp855Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "cp866") == 0)
		CmdUnicodeChar(cp866Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "1251") == 0)
		CmdUnicodeChar(cp1251Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "855") == 0)
		CmdUnicodeChar(cp855Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "866") == 0)
		CmdUnicodeChar(cp866Unicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "koi8-r") == 0)
		CmdUnicodeChar(koi8rUnicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "koi8-u") == 0)
		CmdUnicodeChar(koi8uUnicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "decmulti") == 0)
		CmdUnicodeChar(decmultiUnicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "macce") == 0)
		CmdUnicodeChar(appleCEUnicode[eightbit_index]);
    else if (strcmp(g_charset_encoding_name, "maccyr") == 0)
		CmdUnicodeChar(appleCyrrilicUnicode[eightbit_index]);
}
