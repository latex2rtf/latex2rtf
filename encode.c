/* $Id: encode.c,v 1.5 2001/10/07 17:48:39 prahl Exp $ 
   Translate high bit characters into RTF assuming that
   the default codepage is ansi (1252)
   
   There are many non-translated characters, patches welcome.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "l2r_fonts.h"
#include "funct1.h"
#include "encode.h"
#include "encode_tables.h"

static void applemac_enc(int index)
{
char *s;
	s = applemac_2_ansi[index];
	if (s[0]!='0'){
		fprintRTF("\\'%s",s);
		return;
	}
	s = applemac_2_sym[index];
	if (s[0]!='0'){
		int sym=RtfFontNumber("Symbol");
		fprintRTF("{\\f%d\\'%s}",sym,s);
		return;
	}
	if (0xDE) { /* U+64257 LATIN SMALL LIGATURE FI */ 
		fprintRTF("fi");
		return;
	}
	if (index + 128 == 0xDF) { /* U+64258 LATIN SMALL LIGATURE FL */ 
		fprintRTF("fl");
		return;
	}
	if (index + 128 == 0xF0) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF5) { /* U+305 LATIN SMALL LETTER DOTLESS I */ 
		fprintRTF("i");
		return;
	}
	if (index + 128 == 0xF9) { /* U+728 BREVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFA) { /* U+729 DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFB) { /* U+730 RING ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFD) { /* U+733 DOUBLE ACUTE ACCENT */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFE) { /* U+731 OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFF) { /* U+711 CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
}

static void cp437_enc(int index)
{
char *s;
	s = cp437_2_ansi[index];
	if (s[0]!='0'){
		fprintRTF("\\'%s",s);
		return;
	}
	s = cp437_2_sym[index];
	if (s[0]!='0'){
		int sym=RtfFontNumber("Symbol");
		fprintRTF("{\\f%d\\'%s}",sym,s);
		return;
	}
	if (index + 128 == 0x9E) { /* U+8359 PESETA SIGN */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA9) { /* U+8976 REVERSED NOT SIGN */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB0) { /* U+9617 LIGHT SHADE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB1) { /* U+9618 MEDIUM SHADE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB2) { /* U+9619 DARK SHADE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDB) { /* U+9608 FULL BLOCK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDC) { /* U+9604 LOWER HALF BLOCK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDD) { /* U+9612 LEFT HALF BLOCK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDE) { /* U+9616 RIGHT HALF BLOCK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDF) { /* U+9600 UPPER HALF BLOCK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF8) { /* U+8728 RING OPERATOR */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFA) { /* U+8729 BULLET OPERATOR */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFC) { /* U+8319 SUPERSCRIPT LATIN SMALL LETTER N */ 
		fprintRTF("{\\up6 n}");
		return;
	}
	if (index + 128 == 0xFE) { /* U+9632 BLACK SQUARE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
}

static void cp850_enc(int index)
{
char *s;
	s = cp850_2_ansi[index];
	if (s[0]!='0'){
		fprintRTF("\\'%s",s);
		return;
	}
	s = cp850_2_sym[index];
	if (s[0]!='0'){
		int sym=RtfFontNumber("Symbol");
		fprintRTF("{\\f%d\\'%s}",sym,s);
		return;
	}
	if (index + 128 == 0xB0) { /* U+9617 LIGHT SHADE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB1) { /* U+9618 MEDIUM SHADE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB2) { /* U+9619 DARK SHADE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD5) { /* U+305 LATIN SMALL LETTER DOTLESS I */ 
		fprintRTF("i");
		return;
	}
	if (index + 128 == 0xDB) { /* U+9608 FULL BLOCK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDC) { /* U+9604 LOWER HALF BLOCK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDF) { /* U+9600 UPPER HALF BLOCK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF7) { /* U+731 OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFA) { /* U+729 DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFE) { /* U+9632 BLACK SQUARE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
}

static void cp852_enc(int index)
{
char *s;
	s = cp852_2_ansi[index];
	if (s[0]!='0'){
		fprintRTF("\\'%s",s);
		return;
	}
	s = cp852_2_sym[index];
	if (s[0]!='0'){
		int sym=RtfFontNumber("Symbol");
		fprintRTF("{\\f%d\\'%s}",sym,s);
		return;
	}
	if (index + 128 == 0x85) { /* U+367 LATIN SMALL LETTER U WITH RING ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x86) { /* U+263 LATIN SMALL LETTER C WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x88) { /* U+322 LATIN SMALL LETTER L WITH STROKE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x8A) { /* U+336 LATIN CAPITAL LETTER O WITH DOUBLE ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x8B) { /* U+337 LATIN SMALL LETTER O WITH DOUBLE ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x8D) { /* U+377 LATIN CAPITAL LETTER Z WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x8F) { /* U+262 LATIN CAPITAL LETTER C WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x91) { /* U+313 LATIN CAPITAL LETTER L WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x92) { /* U+314 LATIN SMALL LETTER L WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x95) { /* U+317 LATIN CAPITAL LETTER L WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x96) { /* U+318 LATIN SMALL LETTER L WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x97) { /* U+346 LATIN CAPITAL LETTER S WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x98) { /* U+347 LATIN SMALL LETTER S WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x9B) { /* U+356 LATIN CAPITAL LETTER T WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x9C) { /* U+357 LATIN SMALL LETTER T WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x9D) { /* U+321 LATIN CAPITAL LETTER L WITH STROKE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x9F) { /* U+269 LATIN SMALL LETTER C WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA4) { /* U+260 LATIN CAPITAL LETTER A WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA5) { /* U+261 LATIN SMALL LETTER A WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA6) { /* U+381 LATIN CAPITAL LETTER Z WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA7) { /* U+382 LATIN SMALL LETTER Z WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA8) { /* U+280 LATIN CAPITAL LETTER E WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA9) { /* U+281 LATIN SMALL LETTER E WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAA) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAB) { /* U+378 LATIN SMALL LETTER Z WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAC) { /* U+268 LATIN CAPITAL LETTER C WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAD) { /* U+351 LATIN SMALL LETTER S WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB0) { /* U+9617 LIGHT SHADE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB1) { /* U+9618 MEDIUM SHADE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB2) { /* U+9619 DARK SHADE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB7) { /* U+282 LATIN CAPITAL LETTER E WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB8) { /* U+350 LATIN CAPITAL LETTER S WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBD) { /* U+379 LATIN CAPITAL LETTER Z WITH DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBE) { /* U+380 LATIN SMALL LETTER Z WITH DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xC6) { /* U+258 LATIN CAPITAL LETTER A WITH BREVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xC7) { /* U+259 LATIN SMALL LETTER A WITH BREVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD0) { /* U+273 LATIN SMALL LETTER D WITH STROKE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD1) { /* U+272 LATIN CAPITAL LETTER D WITH STROKE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD2) { /* U+270 LATIN CAPITAL LETTER D WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD4) { /* U+271 LATIN SMALL LETTER D WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD5) { /* U+327 LATIN CAPITAL LETTER N WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD8) { /* U+283 LATIN SMALL LETTER E WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDB) { /* U+9608 FULL BLOCK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDC) { /* U+9604 LOWER HALF BLOCK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDD) { /* U+354 LATIN CAPITAL LETTER T WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDE) { /* U+366 LATIN CAPITAL LETTER U WITH RING ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDF) { /* U+9600 UPPER HALF BLOCK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE3) { /* U+323 LATIN CAPITAL LETTER N WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE4) { /* U+324 LATIN SMALL LETTER N WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE5) { /* U+328 LATIN SMALL LETTER N WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE8) { /* U+340 LATIN CAPITAL LETTER R WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xEA) { /* U+341 LATIN SMALL LETTER R WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xEB) { /* U+368 LATIN CAPITAL LETTER U WITH DOUBLE ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xEE) { /* U+355 LATIN SMALL LETTER T WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF1) { /* U+733 DOUBLE ACUTE ACCENT */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF3) { /* U+711 CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF4) { /* U+728 BREVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF7) { /* U+731 OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFA) { /* U+729 DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFB) { /* U+369 LATIN SMALL LETTER U WITH DOUBLE ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFC) { /* U+344 LATIN CAPITAL LETTER R WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFD) { /* U+345 LATIN SMALL LETTER R WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFE) { /* U+9632 BLACK SQUARE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
}

static void cp865_enc(int index)
{
char *s;
	s = cp865_2_ansi[index];
	if (s[0]!='0'){
		fprintRTF("\\'%s",s);
		return;
	}
	s = cp865_2_sym[index];
	if (s[0]!='0'){
		int sym=RtfFontNumber("Symbol");
		fprintRTF("{\\f%d\\'%s}",sym,s);
		return;
	}
	if (index + 128 == 0x9E) { /* U+8359 PESETA SIGN */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA9) { /* U+8976 REVERSED NOT SIGN */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB0) { /* U+9617 LIGHT SHADE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB1) { /* U+9618 MEDIUM SHADE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB2) { /* U+9619 DARK SHADE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDB) { /* U+9608 FULL BLOCK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDC) { /* U+9604 LOWER HALF BLOCK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDD) { /* U+9612 LEFT HALF BLOCK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDE) { /* U+9616 RIGHT HALF BLOCK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDF) { /* U+9600 UPPER HALF BLOCK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF8) { /* U+8728 RING OPERATOR */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFA) { /* U+8729 BULLET OPERATOR */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFC) { /* U+8319 SUPERSCRIPT LATIN SMALL LETTER N */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFE) { /* U+9632 BLACK SQUARE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
}

static void cp1250_enc(int index)
{
char *s;
	s = cp1250_2_ansi[index];
	if (s[0]!='0'){
		fprintRTF("\\'%s",s);
		return;
	}
	s = cp1250_2_sym[index];
	if (s[0]!='0'){
		int sym=RtfFontNumber("Symbol");
		fprintRTF("{\\f%d\\'%s}",sym,s);
		return;
	}
	if (index + 128 == 0x80) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x81) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x83) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x88) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x8C) { /* U+346 LATIN CAPITAL LETTER S WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x8D) { /* U+356 LATIN CAPITAL LETTER T WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x8E) { /* U+381 LATIN CAPITAL LETTER Z WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x8F) { /* U+377 LATIN CAPITAL LETTER Z WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x90) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x98) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x9C) { /* U+347 LATIN SMALL LETTER S WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x9D) { /* U+357 LATIN SMALL LETTER T WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x9E) { /* U+382 LATIN SMALL LETTER Z WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x9F) { /* U+378 LATIN SMALL LETTER Z WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA1) { /* U+711 CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA2) { /* U+728 BREVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA3) { /* U+321 LATIN CAPITAL LETTER L WITH STROKE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA5) { /* U+260 LATIN CAPITAL LETTER A WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAA) { /* U+350 LATIN CAPITAL LETTER S WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAC) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAF) { /* U+379 LATIN CAPITAL LETTER Z WITH DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB2) { /* U+731 OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB3) { /* U+322 LATIN SMALL LETTER L WITH STROKE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB9) { /* U+261 LATIN SMALL LETTER A WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBA) { /* U+351 LATIN SMALL LETTER S WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBC) { /* U+317 LATIN CAPITAL LETTER L WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBD) { /* U+733 DOUBLE ACUTE ACCENT */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBE) { /* U+318 LATIN SMALL LETTER L WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBF) { /* U+380 LATIN SMALL LETTER Z WITH DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xC0) { /* U+340 LATIN CAPITAL LETTER R WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xC3) { /* U+258 LATIN CAPITAL LETTER A WITH BREVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xC5) { /* U+313 LATIN CAPITAL LETTER L WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xC6) { /* U+262 LATIN CAPITAL LETTER C WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xC8) { /* U+268 LATIN CAPITAL LETTER C WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xCA) { /* U+280 LATIN CAPITAL LETTER E WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xCC) { /* U+282 LATIN CAPITAL LETTER E WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xCF) { /* U+270 LATIN CAPITAL LETTER D WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD0) { /* U+272 LATIN CAPITAL LETTER D WITH STROKE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD1) { /* U+323 LATIN CAPITAL LETTER N WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD2) { /* U+327 LATIN CAPITAL LETTER N WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD5) { /* U+336 LATIN CAPITAL LETTER O WITH DOUBLE ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD8) { /* U+344 LATIN CAPITAL LETTER R WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD9) { /* U+366 LATIN CAPITAL LETTER U WITH RING ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDB) { /* U+368 LATIN CAPITAL LETTER U WITH DOUBLE ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDE) { /* U+354 LATIN CAPITAL LETTER T WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE0) { /* U+341 LATIN SMALL LETTER R WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE3) { /* U+259 LATIN SMALL LETTER A WITH BREVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE5) { /* U+314 LATIN SMALL LETTER L WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE6) { /* U+263 LATIN SMALL LETTER C WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE8) { /* U+269 LATIN SMALL LETTER C WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xEA) { /* U+281 LATIN SMALL LETTER E WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xEC) { /* U+283 LATIN SMALL LETTER E WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xEF) { /* U+271 LATIN SMALL LETTER D WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF0) { /* U+273 LATIN SMALL LETTER D WITH STROKE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF1) { /* U+324 LATIN SMALL LETTER N WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF2) { /* U+328 LATIN SMALL LETTER N WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF5) { /* U+337 LATIN SMALL LETTER O WITH DOUBLE ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF8) { /* U+345 LATIN SMALL LETTER R WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF9) { /* U+367 LATIN SMALL LETTER U WITH RING ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFB) { /* U+369 LATIN SMALL LETTER U WITH DOUBLE ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFE) { /* U+355 LATIN SMALL LETTER T WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFF) { /* U+729 DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
}

static void cp1252_enc(int index)
{
char *s;
	s = cp1252_2_ansi[index];
	if (s[0]!='0'){
		fprintRTF("\\'%s",s);
		return;
	}
	s = cp1252_2_sym[index];
	if (s[0]!='0'){
		int sym=RtfFontNumber("Symbol");
		fprintRTF("{\\f%d\\'%s}",sym,s);
		return;
	}
	if (index + 128 == 0x80) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x81) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x8D) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x8E) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x8F) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x90) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x9D) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0x9E) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
}

static void decmulti_enc(int index)
{
char *s;
	s = decmulti_2_ansi[index];
	if (s[0]!='0'){
		fprintRTF("\\'%s",s);
		return;
	}
	s = decmulti_2_sym[index];
	if (s[0]!='0'){
		int sym=RtfFontNumber("Symbol");
		fprintRTF("{\\f%d\\'%s}",sym,s);
		return;
	}
	if (index + 128 == 0xA0) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA4) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA6) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAC) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAD) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAE) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAF) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB4) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB8) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBE) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD0) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDE) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF0) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFE) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFF) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
}

static void latin1_enc(int index)
{
char *s;
	s = latin1_2_ansi[index];
	if (s[0]!='0'){
		fprintRTF("\\'%s",s);
		return;
	}
	s = latin1_2_sym[index];
	if (s[0]!='0'){
		int sym=RtfFontNumber("Symbol");
		fprintRTF("{\\f%d\\'%s}",sym,s);
		return;
	}
}

static void latin2_enc(int index)
{
char *s;
	s = latin2_2_ansi[index];
	if (s[0]!='0'){
		fprintRTF("\\'%s",s);
		return;
	}
	s = latin2_2_sym[index];
	if (s[0]!='0'){
		int sym=RtfFontNumber("Symbol");
		fprintRTF("{\\f%d\\'%s}",sym,s);
		return;
	}
	if (index + 128 == 0xA1) { /* U+260 LATIN CAPITAL LETTER A WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA2) { /* U+728 BREVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA3) { /* U+321 LATIN CAPITAL LETTER L WITH STROKE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA5) { /* U+317 LATIN CAPITAL LETTER L WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA6) { /* U+346 LATIN CAPITAL LETTER S WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAA) { /* U+350 LATIN CAPITAL LETTER S WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAB) { /* U+356 LATIN CAPITAL LETTER T WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAC) { /* U+377 LATIN CAPITAL LETTER Z WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAE) { /* U+381 LATIN CAPITAL LETTER Z WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAF) { /* U+379 LATIN CAPITAL LETTER Z WITH DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB1) { /* U+261 LATIN SMALL LETTER A WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB2) { /* U+731 OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB3) { /* U+322 LATIN SMALL LETTER L WITH STROKE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB5) { /* U+318 LATIN SMALL LETTER L WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB6) { /* U+347 LATIN SMALL LETTER S WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB7) { /* U+711 CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBA) { /* U+351 LATIN SMALL LETTER S WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBB) { /* U+357 LATIN SMALL LETTER T WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBC) { /* U+378 LATIN SMALL LETTER Z WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBD) { /* U+733 DOUBLE ACUTE ACCENT */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBE) { /* U+382 LATIN SMALL LETTER Z WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBF) { /* U+380 LATIN SMALL LETTER Z WITH DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xC0) { /* U+340 LATIN CAPITAL LETTER R WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xC3) { /* U+258 LATIN CAPITAL LETTER A WITH BREVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xC5) { /* U+313 LATIN CAPITAL LETTER L WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xC6) { /* U+262 LATIN CAPITAL LETTER C WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xC8) { /* U+268 LATIN CAPITAL LETTER C WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xCA) { /* U+280 LATIN CAPITAL LETTER E WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xCC) { /* U+282 LATIN CAPITAL LETTER E WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xCF) { /* U+270 LATIN CAPITAL LETTER D WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD0) { /* U+272 LATIN CAPITAL LETTER D WITH STROKE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD1) { /* U+323 LATIN CAPITAL LETTER N WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD2) { /* U+327 LATIN CAPITAL LETTER N WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD5) { /* U+336 LATIN CAPITAL LETTER O WITH DOUBLE ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD8) { /* U+344 LATIN CAPITAL LETTER R WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD9) { /* U+366 LATIN CAPITAL LETTER U WITH RING ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDB) { /* U+368 LATIN CAPITAL LETTER U WITH DOUBLE ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDE) { /* U+354 LATIN CAPITAL LETTER T WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE0) { /* U+341 LATIN SMALL LETTER R WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE3) { /* U+259 LATIN SMALL LETTER A WITH BREVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE5) { /* U+314 LATIN SMALL LETTER L WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE6) { /* U+263 LATIN SMALL LETTER C WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE8) { /* U+269 LATIN SMALL LETTER C WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xEA) { /* U+281 LATIN SMALL LETTER E WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xEC) { /* U+283 LATIN SMALL LETTER E WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xEF) { /* U+271 LATIN SMALL LETTER D WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF0) { /* U+273 LATIN SMALL LETTER D WITH STROKE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF1) { /* U+324 LATIN SMALL LETTER N WITH ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF2) { /* U+328 LATIN SMALL LETTER N WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF5) { /* U+337 LATIN SMALL LETTER O WITH DOUBLE ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF8) { /* U+345 LATIN SMALL LETTER R WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF9) { /* U+367 LATIN SMALL LETTER U WITH RING ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFB) { /* U+369 LATIN SMALL LETTER U WITH DOUBLE ACUTE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFE) { /* U+355 LATIN SMALL LETTER T WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFF) { /* U+729 DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
}

static void latin3_enc(int index)
{
char *s;
	s = latin3_2_ansi[index];
	if (s[0]!='0'){
		fprintRTF("\\'%s",s);
		return;
	}
	s = latin3_2_sym[index];
	if (s[0]!='0'){
		int sym=RtfFontNumber("Symbol");
		fprintRTF("{\\f%d\\'%s}",sym,s);
		return;
	}
	if (index + 128 == 0xA1) { /* U+294 LATIN CAPITAL LETTER H WITH STROKE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA2) { /* U+728 BREVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA5) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA6) { /* U+292 LATIN CAPITAL LETTER H WITH CIRCUMFLEX */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA9) { /* U+304 LATIN CAPITAL LETTER I WITH DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAA) { /* U+350 LATIN CAPITAL LETTER S WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAB) { /* U+286 LATIN CAPITAL LETTER G WITH BREVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAC) { /* U+308 LATIN CAPITAL LETTER J WITH CIRCUMFLEX */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAE) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAF) { /* U+379 LATIN CAPITAL LETTER Z WITH DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB1) { /* U+295 LATIN SMALL LETTER H WITH STROKE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB6) { /* U+293 LATIN SMALL LETTER H WITH CIRCUMFLEX */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB9) { /* U+305 LATIN SMALL LETTER DOTLESS I */ 
		fprintRTF("i");
		return;
	}
	if (index + 128 == 0xBA) { /* U+351 LATIN SMALL LETTER S WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBB) { /* U+287 LATIN SMALL LETTER G WITH BREVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBC) { /* U+309 LATIN SMALL LETTER J WITH CIRCUMFLEX */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBE) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBF) { /* U+380 LATIN SMALL LETTER Z WITH DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xC3) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xC5) { /* U+266 LATIN CAPITAL LETTER C WITH DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xC6) { /* U+264 LATIN CAPITAL LETTER C WITH CIRCUMFLEX */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD0) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD5) { /* U+288 LATIN CAPITAL LETTER G WITH DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD8) { /* U+284 LATIN CAPITAL LETTER G WITH CIRCUMFLEX */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDD) { /* U+364 LATIN CAPITAL LETTER U WITH BREVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDE) { /* U+348 LATIN CAPITAL LETTER S WITH CIRCUMFLEX */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE3) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE5) { /* U+267 LATIN SMALL LETTER C WITH DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE6) { /* U+265 LATIN SMALL LETTER C WITH CIRCUMFLEX */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF0) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF5) { /* U+289 LATIN SMALL LETTER G WITH DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF8) { /* U+285 LATIN SMALL LETTER G WITH CIRCUMFLEX */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFD) { /* U+365 LATIN SMALL LETTER U WITH BREVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFE) { /* U+349 LATIN SMALL LETTER S WITH CIRCUMFLEX */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFF) { /* U+729 DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
}

static void latin4_enc(int index)
{
char *s;
	s = latin4_2_ansi[index];
	if (s[0]!='0'){
		fprintRTF("\\'%s",s);
		return;
	}
	s = latin4_2_sym[index];
	if (s[0]!='0'){
		int sym=RtfFontNumber("Symbol");
		fprintRTF("{\\f%d\\'%s}",sym,s);
		return;
	}
	if (index + 128 == 0xA1) { /* U+260 LATIN CAPITAL LETTER A WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA2) { /* U+312 LATIN SMALL LETTER KRA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA3) { /* U+342 LATIN CAPITAL LETTER R WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA5) { /* U+296 LATIN CAPITAL LETTER I WITH TILDE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xA6) { /* U+315 LATIN CAPITAL LETTER L WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAA) { /* U+274 LATIN CAPITAL LETTER E WITH MACRON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAB) { /* U+290 LATIN CAPITAL LETTER G WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAC) { /* U+358 LATIN CAPITAL LETTER T WITH STROKE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xAE) { /* U+381 LATIN CAPITAL LETTER Z WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB1) { /* U+261 LATIN SMALL LETTER A WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB2) { /* U+731 OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB3) { /* U+343 LATIN SMALL LETTER R WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB5) { /* U+297 LATIN SMALL LETTER I WITH TILDE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB6) { /* U+316 LATIN SMALL LETTER L WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB7) { /* U+711 CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBA) { /* U+275 LATIN SMALL LETTER E WITH MACRON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBB) { /* U+291 LATIN SMALL LETTER G WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBC) { /* U+359 LATIN SMALL LETTER T WITH STROKE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBD) { /* U+330 LATIN CAPITAL LETTER ENG */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBE) { /* U+382 LATIN SMALL LETTER Z WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xBF) { /* U+331 LATIN SMALL LETTER ENG */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xC0) { /* U+256 LATIN CAPITAL LETTER A WITH MACRON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xC7) { /* U+302 LATIN CAPITAL LETTER I WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xC8) { /* U+268 LATIN CAPITAL LETTER C WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xCA) { /* U+280 LATIN CAPITAL LETTER E WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xCC) { /* U+278 LATIN CAPITAL LETTER E WITH DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xCF) { /* U+298 LATIN CAPITAL LETTER I WITH MACRON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD0) { /* U+272 LATIN CAPITAL LETTER D WITH STROKE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD1) { /* U+325 LATIN CAPITAL LETTER N WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD2) { /* U+332 LATIN CAPITAL LETTER O WITH MACRON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD3) { /* U+310 LATIN CAPITAL LETTER K WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xD9) { /* U+370 LATIN CAPITAL LETTER U WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDD) { /* U+360 LATIN CAPITAL LETTER U WITH TILDE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDE) { /* U+362 LATIN CAPITAL LETTER U WITH MACRON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE0) { /* U+257 LATIN SMALL LETTER A WITH MACRON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE7) { /* U+303 LATIN SMALL LETTER I WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE8) { /* U+269 LATIN SMALL LETTER C WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xEA) { /* U+281 LATIN SMALL LETTER E WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xEC) { /* U+279 LATIN SMALL LETTER E WITH DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xEF) { /* U+299 LATIN SMALL LETTER I WITH MACRON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF0) { /* U+273 LATIN SMALL LETTER D WITH STROKE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF1) { /* U+326 LATIN SMALL LETTER N WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF2) { /* U+333 LATIN SMALL LETTER O WITH MACRON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF3) { /* U+311 LATIN SMALL LETTER K WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF9) { /* U+371 LATIN SMALL LETTER U WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFD) { /* U+361 LATIN SMALL LETTER U WITH TILDE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFE) { /* U+363 LATIN SMALL LETTER U WITH MACRON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFF) { /* U+729 DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
}

static void latin5_enc(int index)
{
char *s;
	s = latin5_2_ansi[index];
	if (s[0]!='0'){
		fprintRTF("\\'%s",s);
		return;
	}
	s = latin5_2_sym[index];
	if (s[0]!='0'){
		int sym=RtfFontNumber("Symbol");
		fprintRTF("{\\f%d\\'%s}",sym,s);
		return;
	}
	if (index + 128 == 0xD0) { /* U+286 LATIN CAPITAL LETTER G WITH BREVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDD) { /* U+304 LATIN CAPITAL LETTER I WITH DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xDE) { /* U+350 LATIN CAPITAL LETTER S WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xEA) { /* U+281 LATIN SMALL LETTER E WITH OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xEC) { /* U+279 LATIN SMALL LETTER E WITH DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xEF) { /* U+299 LATIN SMALL LETTER I WITH MACRON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF0) { /* U+287 LATIN SMALL LETTER G WITH BREVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFD) { /* U+305 LATIN SMALL LETTER DOTLESS I */ 
		fprintRTF("i");
		return;
	}
	if (index + 128 == 0xFE) { /* U+351 LATIN SMALL LETTER S WITH CEDILLA */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
}

static void latin9_enc(int index)
{
char *s;
	s = latin9_2_ansi[index];
	if (s[0]!='0'){
		fprintRTF("\\'%s",s);
		return;
	}
	s = latin9_2_sym[index];
	if (s[0]!='0'){
		int sym=RtfFontNumber("Symbol");
		fprintRTF("{\\f%d\\'%s}",sym,s);
		return;
	}
	if (index + 128 == 0xA4) { /* U+8364 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB4) { /* U+381 LATIN CAPITAL LETTER Z WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xB8) { /* U+382 LATIN SMALL LETTER Z WITH CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
}

static void next_enc(int index)
{
char *s;
	s = next_2_ansi[index];
	if (s[0]!='0'){
		fprintRTF("\\'%s",s);
		return;
	}
	s = next_2_sym[index];
	if (s[0]!='0'){
		int sym=RtfFontNumber("Symbol");
		fprintRTF("{\\f%d\\'%s}",sym,s);
		return;
	}
	if (index + 128 == 0xAE) { /* U+64257 LATIN SMALL LIGATURE FI */ 
		fprintRTF("fi");
		return;
	}
	if (index + 128 == 0xAF) { /* U+64258 LATIN SMALL LIGATURE FL */ 
		fprintRTF("fl");
		return;
	}
	if (index + 128 == 0xC1) { /* U+715 MODIFIER LETTER GRAVE ACCENT */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xC6) { /* U+728 BREVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xC7) { /* U+729 DOT ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xCA) { /* U+730 RING ABOVE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xCD) { /* U+733 DOUBLE ACUTE ACCENT */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xCE) { /* U+731 OGONEK */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xCF) { /* U+711 CARON */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xE8) { /* U+321 LATIN CAPITAL LETTER L WITH STROKE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xF5) { /* U+305 LATIN SMALL LETTER DOTLESS I */ 
		fprintRTF("i");
		return;
	}
	if (index + 128 == 0xF8) { /* U+322 LATIN SMALL LETTER L WITH STROKE */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFE) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
	if (index + 128 == 0xFF) { /* U+65535 unknown */ 
/*		fprintRTF("TRANSLATION");*/
		return;
	}
}

void WriteEightBitChar(char cThis)
{
	int index = (int) cThis + 128;
	diagnostics(5, "WriteEightBitChar char=%d index=%d encoding=%s",(int) cThis, index, g_encoding);
	if (strcmp(g_encoding, "applemac") == 0)
		applemac_enc(index);
	else if (strcmp(g_encoding, "cp437") == 0)
		cp437_enc(index);
	else if (strcmp(g_encoding, "cp850") == 0)
		cp850_enc(index);
	else if (strcmp(g_encoding, "cp852") == 0)
		cp852_enc(index);
	else if (strcmp(g_encoding, "cp865") == 0)
		cp865_enc(index);
	else if (strcmp(g_encoding, "decmulti") == 0)
		decmulti_enc(index);
	else if (strcmp(g_encoding, "cp1250") == 0)
		cp1250_enc(index);
	else if (strcmp(g_encoding, "cp1252") == 0)
		cp1252_enc(index);
	else if (strcmp(g_encoding, "latin1") == 0)
		latin1_enc(index);
	else if (strcmp(g_encoding, "latin2") == 0)
		latin2_enc(index);
	else if (strcmp(g_encoding, "latin3") == 0)
		latin3_enc(index);
	else if (strcmp(g_encoding, "latin4") == 0)
		latin4_enc(index);
	else if (strcmp(g_encoding, "latin5") == 0)
		latin5_enc(index);
	else if (strcmp(g_encoding, "latin9") == 0)
		latin9_enc(index);
	else if (strcmp(g_encoding, "next") == 0 ) 
		next_enc(index);
}


