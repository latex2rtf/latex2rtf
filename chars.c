/* $Id: chars.c,v 1.22 2002/02/18 05:54:03 prahl Exp $

   purpose : handles special characters and logos
*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "commands.h"
#include "l2r_fonts.h"
#include "cfg.h"
#include "ignore.h"
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
	char           *cParam = getBraceParam();
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

	default:
		if (strcmp(cParam,"\\i")==0) {
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

void 
CmdLApostrophChar( int code)
/******************************************************************************
 purpose: converts special symbols from LaTeX to RTF
 ******************************************************************************/
{
	int            num;
	char           *cParam = getBraceParam();
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
		if (strcmp(cParam,"\\i")==0) {
			fprintRTF("\\'ec");
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

void 
CmdRApostrophChar(int code)
/******************************************************************************
 purpose: converts special symbols from LaTeX to RTF
 ******************************************************************************/
{
	char           *cParam = getBraceParam();
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
		if (strcmp(cParam,"\\i")==0) {
			fprintRTF("\\'ed");
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

void 
CmdMacronChar(int code)
/******************************************************************************
 purpose: converts special symbols from LaTeX to RTF
 ******************************************************************************/
{
	char           *cParam = getBraceParam();
	if (cParam == NULL)
		return;

	if (!g_processing_fields)
		fprintRTF("{\\field{\\*\\fldinst EQ ");
	fprintRTF("\\\\O(");
	ConvertString(cParam);
	fprintRTF("%c\\\\S(\\'af))", g_field_separator);
	if (!g_processing_fields)
		fprintRTF("}{\\fldrslt }}");

	free(cParam);
}

void 
CmdHatChar(int code)
/******************************************************************************
 purpose: \^{o} and \hat{o} symbols from LaTeX to RTF
 ******************************************************************************/
{
	int            num;
	char           *cParam = getBraceParam();
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
		if (strcmp(cParam,"\\i")==0) {
			fprintRTF("\\'ee");
			break;
		}
		num = RtfFontNumber("MT Extra");
		if (!g_processing_fields)
			fprintRTF("{\\field{\\*\\fldinst EQ ");
		fprintRTF("\\\\O(");
		ConvertString(cParam);
		fprintRTF("%c\\\\S({\\f%d\\'24}))", g_field_separator, num);
		if (!g_processing_fields)
			fprintRTF("}{\\fldrslt }}");
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
		fprintRTF("%c\\\\S(\\'b0))", g_field_separator);
		if (!g_processing_fields)
			fprintRTF("}{\\fldrslt }}");
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
	
	cParam = getBraceParam();
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
		if (!g_processing_fields) fprintRTF("{\\field{\\*\\fldinst EQ ");
		fprintRTF("\\\\O(");
		ConvertString(cParam);
		fprintRTF("%c\\\\S({\\f%d\\'25}))", g_field_separator, num);
		if (!g_processing_fields) fprintRTF("}{\\fldrslt }}");
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
	int down;
	char           *cParam = getBraceParam();
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
		down = CurrentFontSize() / 4;
		if (!g_processing_fields)
			fprintRTF("{\\field{\\*\\fldinst EQ ");
		fprintRTF("\\\\O(");
		ConvertString(cParam);
		fprintRTF("%c\\dn%d\\'b8)", g_field_separator,down);
		if (!g_processing_fields)
			fprintRTF("}{\\fldrslt }}");
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

void 
CmdBreveChar(int code)
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
	int             upsize,num;
	char           *cParam;
	
	num = RtfFontNumber("MT Extra");
	cParam = getBraceParam();
	if (cParam == NULL)
		return;

	upsize = CurrentFontSize()/2;
	if (!g_processing_fields)
		fprintRTF("{\\field{\\*\\fldinst EQ ");
	fprintRTF("\\\\O(");
	ConvertString(cParam);
	fprintRTF("%c\\\\S({\\up%d\\f%d \\\\(}))", g_field_separator, upsize, num);
	if (!g_processing_fields)
		fprintRTF("}{\\fldrslt }}");
	free(cParam);
}

void 
CmdUnderdotChar(int code)
/******************************************************************************
 purpose: converts chars with dots under from LaTeX to RTF
 ******************************************************************************/
{
	int             dnsize;
	char           *cParam = getBraceParam();
	if (cParam == NULL)
		return;

	dnsize = (0.4 * CurrentFontSize()) + 0.45;

	if (!g_processing_fields)
		fprintRTF("{\\field{\\*\\fldinst EQ ");
	fprintRTF("\\\\O(");
	ConvertString(cParam);
	fprintRTF("%c\\\\S(\\dn%d\\'2e))", g_field_separator, dnsize);
	if (!g_processing_fields)
		fprintRTF("}{\\fldrslt }}");

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
	
	cParam = getBraceParam();
	if (cParam == NULL)
		return;

	upsize = (0.4 * CurrentFontSize()) + 0.45;
	num = RtfFontNumber("Symbol");

	if (!g_processing_fields)
		fprintRTF("{\\field{\\*\\fldinst EQ ");
	fprintRTF("\\\\O(");
	ConvertString(cParam);
	fprintRTF("%c\\\\S({\\up%d\\f%d\\'da}))", g_field_separator, upsize, num);
		if (!g_processing_fields)
			fprintRTF("}{\\fldrslt }}");

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
	
	cParam = getBraceParam();
	if (cParam == NULL)
		return;

	num = RtfFontNumber("MT Extra");

	if (!g_processing_fields)
		fprintRTF("{\\field{\\*\\fldinst EQ ");
	fprintRTF("\\\\O(");
	ConvertString(cParam);
	fprintRTF("%c\\\\S({\\f%d\\'26}))", g_field_separator, num);
	if (!g_processing_fields)
		fprintRTF("}{\\fldrslt }}");

	free(cParam);
}

void 
CmdUnderbarChar(int code)
/******************************************************************************
 purpose: converts \.{o} and \dot{o} from LaTeX to RTF
          need something that looks like \\O(a,\\S(\f2\'26)) in RTF file
 ******************************************************************************/
 {
	char           *cParam;
	
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

void 
CmdDotlessChar(int code)
/******************************************************************************
 purpose: converts \i and \j to 'i' and 'j'
 ******************************************************************************/
{
	if (code==0)
		fprintRTF("i");
	else
		fprintRTF("j");
}

void
CmdChar(int code)
{
	char cThis;
	int num;
	int symfont = RtfFontNumber("Symbol");
	
	cThis = getNonSpace();
	if (cThis != '\'') { ungetTexChar(cThis); return; }
	
	num  = 64 * ((int) getTexChar() - (int) '0');
	num += 8  * ((int) getTexChar() - (int) '0');
	num +=      ((int) getTexChar() - (int) '0');
	
	switch (num) {
		case 0:   fprintRTF("{\\f%d G}", symfont); /* Gamma */
				  break;
				  
		case 1:   fprintRTF("{\\f%d D}", symfont); /* Delta */
				  break;
	
		case 2:   fprintRTF("{\\f%d Q}", symfont); /* Theta */
				  break;
	
		case 3:   fprintRTF("{\\f%d L}", symfont); /* Lambda */
				  break;
	
		case 4:   fprintRTF("{\\f%d X}", symfont); /* Xi */
				  break;
	
		case 5:   fprintRTF("{\\f%d P}", symfont); /* Pi */
				  break;
	
		case 6:   fprintRTF("{\\f%d S}", symfont); /* Sigma */
				  break;
	
		case 7:   fprintRTF("{\\f%d U}", symfont); /* Upsilon */
				  break;
	
		case 8:   fprintRTF("{\\f%d F}", symfont); /* Phi */
				  break;
	
		case 9:   fprintRTF("{\\f%d Y}", symfont); /* Psi */
				  break;
	
		case 10:  fprintRTF("{\\f%d W}", symfont); /* Omega */
				  break;
	
		case 11:  fprintRTF("ff");
				  break;
	
		case 12:  fprintRTF("fi");
				  break;
	
		case 13:  fprintRTF("fl");
				  break;
	
		case 14:  fprintRTF("ffi");
				  break;
	
		case 15:  fprintRTF("ffl");
				  break;
	
		case 16:  fprintRTF("i");  /* Dotless i */
				  break;
	
		case 17:  fprintRTF("j");  /* Dotless j */
				  break;
	
		case 18:  fprintRTF("`");
				  break;
	
		case 19:  fprintRTF("'");
				  break;
	
		case 20:  fprintRTF("v");
				  break;
	
		case 21:  fprintRTF("u");
				  break;
	
		case 22:  fprintRTF("-"); /* overbar */
				  break;
	
		case 23:  fprintRTF("{\\f%d \\'b0}", symfont); /* degree */
				  break;
	
		case 24:  fprintRTF("\\'b8"); /*cedilla*/
				  break;
	
		case 25:  fprintRTF("\\'df"); /*§*/
				  break;
	
		case 26:  fprintRTF("\\'e6"); /* ae */
				  break;
	
		case 27:  fprintRTF("\\'8c"); /* oe */
				  break;
	
		case 28:  fprintRTF("\\'f8"); /*oslash*/
				  break;
	
		case 29:  fprintRTF("\\'c6"); /*AE*/
				  break;
	
		case 30:  fprintRTF("\\'8c"); /*OE*/
				  break;
	
		case 31:  fprintRTF("\\'d8"); /* capital O with stroke*/
				  break;
	
		case 32:  fprintRTF(" "); /*space differs with font*/
				  break;
	
		case 60:  fprintRTF("<"); /* less than differs with font*/
				  break;
				  
		case 62:  fprintRTF(">"); /* greater than differs with font*/
				  break;
				  
		case 123: fprintRTF("\\{"); /* open brace differs with font*/
				  break;
				  
		case 124: fprintRTF("\\\\"); /* backslash differs with font*/
				  break;
				  
		case 125: fprintRTF("\\}"); /*close brace differs with font*/
				  break;
				  
		case 127: fprintRTF("\\'a8"); /*diaeresis differs with font*/
				  break;
		
		default:
				  putRtfChar((char) num);
				  break;
	}
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
    			fuptext=getBraceParam();
        		ConvertString(fuptext); 
        		free(fuptext);
        		break;  	
  }
  
  fprintRTF("}");
}
