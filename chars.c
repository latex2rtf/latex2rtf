/* $Id: chars.c,v 1.8 2001/10/07 17:48:39 prahl Exp $

   purpose : handles special characters and logos
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "commands.h"
#include "l2r_fonts.h"
#include "cfg.h"
#include "ignore.h"
#include "util.h"
#include "encode.h"
#include "parser.h"
#include "chars.h"
#include "funct1.h"
#include "convert.h"

void            TeXlogo();
void            LaTeXlogo();

void 
CmdUmlauteChar(int code)
/*****************************************************************************
 purpose : converts characters with diaeresis from LaTeX to RTF
 ******************************************************************************/
{
	int            num;
	char           *cParam = getParam();
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
	case 'i':
		fprintRTF("\\'ef");
		break;
	case 'y':
		fprintRTF("\\'ff");
		break;

	default:
		num = RtfFontNumber("MT Extra");
		fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O");
		fprintRTF("(%c%c\\\\S(\\f%d\\'26\\'26))}", cParam[0], FORMULASEP, num);
		fprintRTF("{\\fldrslt }}");
		break;

	}
	free(cParam);
}

void 
CmdLApostrophChar( int code)
/******************************************************************************
 purpose: converts special symbols from LaTeX to RTF
 ******************************************************************************/
{
	int            num;
	char           *cParam = getParam();
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
	default:
		num = RtfFontNumber("MT Extra");
		fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O");
		fprintRTF("(%c%c\\\\S(\\f%d\\'23))}", cParam[0], FORMULASEP, num);
		fprintRTF("{\\fldrslt }}");
		break;
	}
	free(cParam);
}

void 
CmdRApostrophChar(int code)
/******************************************************************************
 purpose: converts special symbols from LaTeX to RTF
 ******************************************************************************/
{
	char           *cParam = getParam();
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
	default:
		fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O");
		fprintRTF("(%c%c\\\\S(\\'b4))}", cParam[0], FORMULASEP);
		fprintRTF("{\\fldrslt }}");
		break;

	}
	free(cParam);
}

void 
CmdMacronChar(int code)
/******************************************************************************
 purpose: converts special symbols from LaTeX to RTF
 ******************************************************************************/
{
	int             upsize;
	char           *cParam = getParam();
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
		fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O");
		fprintRTF("(%c%c\\\\S(\\'af))}", cParam[0], FORMULASEP);
		fprintRTF("{\\fldrslt }}");
		break;

	default:
		upsize = (0.05 * CurrentFontSize()) + 0.45;
		fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O");
		fprintRTF("(%c%c\\\\S\\\\up%d(\\'af))}", cParam[0], FORMULASEP, upsize);
		fprintRTF("{\\fldrslt }}");
	}

	free(cParam);
}

void 
CmdHatChar(int code)
/******************************************************************************
 purpose: \^{o} and \hat{o} symbols from LaTeX to RTF
 ******************************************************************************/
{
	int            num;
	char           *cParam = getParam();
	if (cParam == NULL)
		return;

	switch (cParam[0]) {
	case 'A':
		fprintRTF("\\'c2");
		break;
	case 'E':
		fprintRTF("\\'ca");
		break;
	case 'I':
		fprintRTF("\\'ce");
		break;
	case 'O':
		fprintRTF("\\'d4");
		break;
	case 'U':
		fprintRTF("\\'db");
		break;
	case 'a':
		fprintRTF("\\'e2");
		break;
	case 'e':
		fprintRTF("\\'ea");
		break;
	case 'i':
		fprintRTF("\\'ee");
		break;
	case 'o':
		fprintRTF("\\'f4");
		break;
	case 'u':
		fprintRTF("\\'fb");
		break;

	default:
		num = RtfFontNumber("MT Extra");
		fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O");
		fprintRTF("(%c%c\\\\S(\\f%d\\'24))}", cParam[0], FORMULASEP, num);
		fprintRTF("{\\fldrslt }}");
		break;
	}

	free(cParam);
}

void 
CmdOaccentChar(int code)
/******************************************************************************
 purpose: converts \r accents from LaTeX to RTF
 ******************************************************************************/
{
	char           *cParam;
	
	cParam = getParam();
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
			fprintf(stderr, "Cannot put \\r on '%s'", cParam);
		break;

	default:
		fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O");
		fprintRTF("(%c%c\\\\S(\\'b0))}", cParam[0], FORMULASEP);
		fprintRTF("{\\fldrslt }}");
		break;
	}

	free(cParam);
}

void 
CmdTildeChar( int code)
/******************************************************************************
 purpose: converts \~{n} from LaTeX to RTF
 ******************************************************************************/
{
	int             num;
	char           *cParam;
	
	cParam = getParam();
	if (cParam == NULL)
		return;

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
	default:
		num = RtfFontNumber("MT Extra");
		fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O");
		fprintRTF("(%c%c\\\\S(\\f%d\\'25))}", cParam[0], FORMULASEP, num);
		fprintRTF("{\\fldrslt }}");
		break;
	}
	free(cParam);
}

void 
CmdCedillaChar(int code)
/*****************************************************************************
 purpose: converts \c{c} from LaTeX to RTF
 ******************************************************************************/
{
	char           *cParam = getParam();
	if (cParam == NULL)
		return;

	switch (cParam[0]) {
	case 'C':
		fprintRTF("\\'c7");
		break;
	case 'c':
		fprintRTF("\\'e7");
		break;

	default:
		fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O");
		fprintRTF("(%c%c\\'b8)}", cParam[0], FORMULASEP);
		fprintRTF("{\\fldrslt }}");
		break;
	}

	free(cParam);
}

void 
CmdVecChar(int code)
/*****************************************************************************
 purpose: converts \vec{o} from LaTeX to RTF
 ******************************************************************************/
{
	int             num;
	int             upsize;
	char           *cParam;
	
	cParam = getParam();
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

	fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O");
	fprintRTF("(%c%c\\\\S(\\up%d\\f%d\\'72))}", cParam[0], FORMULASEP, upsize, num);
	fprintRTF("{\\fldrslt }}");
	free(cParam);
}

void 
CmdBreveChar(int code)
/*****************************************************************************
 purpose: converts \u{o} and \breve{o} from LaTeX to RTF
 		  there is no breve in codepage 1252
 		  there is one \'f9 in the MacRoman, but that is not so portable
		  there is one in MT Extra, but the RTF parser for word mistakes
		  \'28 as a '(' and goes bananas.
		  the only solution is to encode with unicode --- perhaps later
		  Now we just fake it with a u
 ******************************************************************************/
{
	int             upsize;
	char           *cParam;
	
	cParam = getParam();
	if (cParam == NULL)
		return;

	upsize = (CurrentFontSize() * 3) / 4;
	fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O");
	fprintRTF("(%c%c\\\\S(\\up%d u))}", cParam[0], FORMULASEP, upsize);
	fprintRTF("{\\fldrslt }}");
	free(cParam);
}

void 
CmdUnderdotChar(int code)
/******************************************************************************
 purpose: converts chars with dots under from LaTeX to RTF
 ******************************************************************************/
{
	int             dnsize;
	char           *cParam = getParam();
	if (cParam == NULL)
		return;

	dnsize = (0.2 * CurrentFontSize()) + 0.45;

	fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O");
	fprintRTF("(%c%c\\\\S\\\\do%d(\\'2e))}", cParam[0], FORMULASEP, dnsize);
	fprintRTF("{\\fldrslt }}");

	free(cParam);
}

void 
CmdHacekChar(int code)
/******************************************************************************
 purpose: converts \v from LaTeX to RTF
          need something that looks like \\O(a,\\S(\f1\'da)) in RTF file
 ******************************************************************************/
{
	int             num;
	int             upsize;
	char           *cParam;
	
	cParam = getParam();
	if (cParam == NULL)
		return;

	upsize = (0.4 * CurrentFontSize()) + 0.45;
	num = RtfFontNumber("Symbol");

	fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O");
	fprintRTF("(%c%c\\\\S(\\up%d\\f%d\\'da))}", cParam[0], FORMULASEP, upsize, num);
	fprintRTF("{\\fldrslt }}");

	free(cParam);
}

void 
CmdDotChar(int code)
/******************************************************************************
 purpose: converts \.{o} and \dot{o} from LaTeX to RTF
          need something that looks like \\O(a,\\S(\f2\'26)) in RTF file
 ******************************************************************************/
{
	int             num;
	char           *cParam;
	
	cParam = getParam();
	if (cParam == NULL)
		return;

	num = RtfFontNumber("MT Extra");

	fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O");
	fprintRTF("(%c%c\\\\S(\\f%d\\'26))}", cParam[0], FORMULASEP, num);
	fprintRTF("{\\fldrslt }}");

	free(cParam);
}

void 
CmdUnderbarChar(int code)
/******************************************************************************
 purpose: converts \.{o} and \dot{o} from LaTeX to RTF
          need something that looks like \\O(a,\\S(\f2\'26)) in RTF file
 ******************************************************************************/
 {
	int             num;
	char           *cParam;
	
	cParam = getParam();
	if (cParam == NULL)
		return;

	num = RtfFontNumber("MT Extra");

	if (cParam[0]) {
		fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O");
		fprintRTF("(%c%c_)}", cParam[0], FORMULASEP);
		fprintRTF("{\\fldrslt }}");
	}
	free(cParam);
}

void 
TeXlogo()
/******************************************************************************
 purpose : prints the Tex logo in the RTF-File (D Taupin)
 ******************************************************************************/
{
	float           DnSize;
	int             dnsize;

	DnSize = 0.3 * CurrentFontSize();
	dnsize = DnSize + 0.45;
	fprintRTF("T{\\dn%d E}X", dnsize);
}

void 
LaTeXlogo()
/******************************************************************************
 purpose : prints the LaTeX logo in the RTF-File (D Taupin)
 ******************************************************************************/
{
	float           UpSize;
	float           FloatFsize;
	int             upsize, Asize;

	if (CurrentFontSize() > 14) 
		FloatFsize = 0.8 * CurrentFontSize();
	else
		FloatFsize = 0.9 * CurrentFontSize();
	Asize = FloatFsize + 0.45;

	UpSize = 0.25 * CurrentFontSize();
	upsize = UpSize + 0.45;
	fprintRTF("L{\\up%d\\fs%d A}", upsize, Asize);
	TeXlogo();
}

void 
CmdLogo(int code)
/******************************************************************************
 purpose : converts the LaTeX, TeX, SLiTex, etc logos to RTF 
 ******************************************************************************/
{
	int             font_num, dnsize;
	float           FloatFsize;
	float           DnSize;

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
			FloatFsize = CurrentFontSize();
		};
		DnSize = 0.3 * CurrentFontSize();
		dnsize = DnSize + 0.45;
		font_num = RtfFontNumber("Symbol");
		fprintRTF("2{\\dn%d\\f%d e}", dnsize, font_num);
		break;
	case CMD_AMSTEX:
		fprintRTF("{\\i AmS}-"); /* should be calligraphic */
		TeXlogo();
		break;
		
	case CMD_AMSLATEX:
		fprintRTF("{\\i AmS}-");  /* should be calligraphic */ 
		LaTeXlogo();
		break;
	}
	fprintRTF("}");
}

void 
CmdFrenchAbbrev(int code)
/******************************************************************************
  purpose: makes \\ier, \\ieme, etc
 ******************************************************************************/
{
  float FloatFsize;
  int up, size;
  char *fuptext;

  if (code == NUMERO) fprintRTF("n");
  if (code == NUMEROS) fprintRTF("n");
  if (code == CNUMERO) fprintRTF("N");
  if (code == CNUMEROS) fprintRTF("N");
  if (code == PRIMO) fprintRTF("1");
  if (code == SECUNDO) fprintRTF("2");
  if (code == TERTIO) fprintRTF("3");
  if (code == QUARTO) fprintRTF("4");
  
  FloatFsize = CurrentFontSize();
  
  if(FloatFsize > 14) FloatFsize *= 0.75;

  up = 0.3*FloatFsize+0.45;
  size = FloatFsize+0.45;
  
  fprintRTF("{\\fs%d\\up%d ",size ,up);
  switch(code)
  {
    case NUMERO : fprintRTF("o"); break;  	
    case CNUMERO : fprintRTF("o"); break;  	
    case NUMEROS : fprintRTF("os"); break;  	
    case CNUMEROS : fprintRTF("os"); break;  	
    case PRIMO : fprintRTF("o"); break;  	
    case SECUNDO : fprintRTF("o"); break;  	
    case TERTIO : fprintRTF("o"); break;  	
    case QUARTO : fprintRTF("o"); break;  	
    case IERF: fprintRTF("er"); break;  	
    case IERSF: fprintRTF("ers"); break;  	
    case IEMEF: fprintRTF("e"); break;  	
    case IEMESF: fprintRTF("es"); break;  	
    case IEREF: fprintRTF("re"); break;  	
    case IERESF: fprintRTF("res"); break;  	
    case FUP: 
    			fuptext=getParam();
        		ConvertString(fuptext); 
        		free(fuptext);
        		break;  	
  }
  
  fprintRTF("}");
}
