/*
 * $Id: encode.c,v 1.2 2001/09/16 05:11:19 prahl Exp $
 * History:
 * $Log: encode.c,v $
 * Revision 1.2  2001/09/16 05:11:19  prahl
 * Gave up and completely revised font handling.  latex2rtf now uses an output
 * filter to keep track of the brace level and font changes in the RTF file.
 * This allows \emph to be handled properly.  This will also allow digits to
 * be typeset upright in math mode, but this has not been implemented yet.
 *
 * Revision 1.1  2001/08/12 21:15:46  prahl
 *         Removed last two // comments
 *         Explicitly cast char to int in isalpha() and isdigit()
 *         Began the process of supporting Babel better
 *
 * Revision 1.4  1998/07/03 06:53:17  glehner
 * fixed missing spaces in Write_ISO8859_1
 *
 * Revision 1.3  1997/02/15 20:48:22  ralf
 * Removed separate declaration of globals.
 *
 * Revision 1.2  1995/05/10 06:37:43  ralf
 * Added own includefile (for consistency checking of decls)
 *
 * Revision 1.1  1995/03/23  16:09:01  ralf
 * Initial revision
 *
 */
/***************************************************************************
   name : encode.c
 author : POLZER Friedrich, TRISKO Gerhard
purpose : to convert Latin-1
 ****************************************************************************/

/********************************* includes *********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "l2r_fonts.h"
#include "funct1.h"
#include "encode.h"
/*****************************************************************************/

/******************************************************************************/
void 
Write_ISO_8859_1(char theChar)
/******************************************************************************
     purpose : converts theChar (ISO 8859-1) to an equivalent Rtf-Command
   parameter : theChar  will be converted, result is written to Rtf-file
     globals : fRtf
 ******************************************************************************/
{
	switch ((unsigned char) theChar) {
		/*
		 * LEG140498Begin The first "8" conversions had no space at
		 * the end and therefore did not translate to the correct
		 * character.
		 * 
		 * Also I changed the very first case value to octal notation,
		 * because in decimal notation it did not convert. Ask me
		 * why????
		 */
		/* case 246:fprintRTF("\\ansi\\'f6\\pc"); */
		case 0366:fprintRTF("{\ansi\\'f6}");
		break;		/* "o */
	case 214:
		fprintRTF("{\ansi\\'d6}");
		break;		/* "O */
	case 228:
		fprintRTF("{\ansi\\'e4}");
		break;		/* "a */
	case 196:
		fprintRTF("{\ansi\\'c4}");
		break;		/* "A */
	case 252:
		fprintRTF("{\ansi\\'fc}");
		break;		/* "u */
	case 220:
		fprintRTF("{\ansi\\'dc}");
		break;		/* "U */
	case 203:
		fprintRTF("{\ansi\\'cb}");
		break;		/* "E */
	case 207:
		fprintRTF("{\ansi\\'cf}");
		break;		/* "I */
	case 235:
		fprintRTF("{\ansi\\'eb}");
		break;		/* "e */
	case 239:
		fprintRTF("{\ansi\\'ef}");
		break;		/* "i */
	case 255:
		fprintRTF("{\ansi\\'ff}");
		break;		/* "y */
		/* LEG140498End */

	case 192:
		fprintRTF("{\ansi\\'c0}");
		break;		/* `A */
	case 200:
		fprintRTF("{\ansi\\'c8}");
		break;		/* `E */
	case 204:
		fprintRTF("{\ansi\\'cc}");
		break;		/* `I */
	case 210:
		fprintRTF("{\ansi\\'d2}");
		break;		/* `O */
	case 217:
		fprintRTF("{\ansi\\'d9}");
		break;		/* `U */
	case 224:
		fprintRTF("{\ansi\\'e0}");
		break;		/* `a */
	case 232:
		fprintRTF("{\ansi\\'e8}");
		break;		/* `e */
	case 236:
		fprintRTF("{\ansi\\'ec}");
		break;		/* `i */
	case 242:
		fprintRTF("{\ansi\\'f2}");
		break;		/* `o */
	case 249:
		fprintRTF("{\ansi\\'f9}");
		break;		/* `u */

	case 193:
		fprintRTF("{\ansi\\'c1}");
		break;		/* 'A */
	case 201:
		fprintRTF("{\ansi\\'c9}");
		break;		/* 'E */
	case 205:
		fprintRTF("{\ansi\\'cd}");
		break;		/* 'I */
	case 211:
		fprintRTF("{\ansi\\'d3}");
		break;		/* 'O */
	case 218:
		fprintRTF("{\ansi\\'da}");
		break;		/* 'U */
	case 225:
		fprintRTF("{\ansi\\'e1}");
		break;		/* 'a */
	case 233:
		fprintRTF("{\ansi\\'e9}");
		break;		/* 'e */
	case 237:
		fprintRTF("{\ansi\\'ed}");
		break;		/* 'i */
	case 243:
		fprintRTF("{\ansi\\'f3}");
		break;		/* 'o */
	case 250:
		fprintRTF("{\ansi\\'fa}");
		break;		/* 'u */
	case 221:
		fprintRTF("{\ansi\\'dd}");
		break;		/* 'Y */
	case 253:
		fprintRTF("{\ansi\\'fd}");
		break;		/* 'y */

	case 194:
		fprintRTF("{\ansi\\'c2}");
		break;		/* ^A */
	case 202:
		fprintRTF("{\ansi\\'ca}");
		break;		/* ^E */
	case 206:
		fprintRTF("{\ansi\\'ce}");
		break;		/* ^I */
	case 212:
		fprintRTF("{\ansi\\'d4}");
		break;		/* ^O */
	case 219:
		fprintRTF("{\ansi\\'db}");
		break;		/* ^U */
	case 226:
		fprintRTF("{\ansi\\'e2}");
		break;		/* ^a */
	case 234:
		fprintRTF("{\ansi\\'ea}");
		break;		/* ^e */
	case 238:
		fprintRTF("{\ansi\\'ee}");
		break;		/* ^i */
	case 244:
		fprintRTF("{\ansi\\'f4}");
		break;		/* ^o */
	case 251:
		fprintRTF("{\ansi\\'fb}");
		break;		/* ^u */

	case 195:
		fprintRTF("{\ansi\\'c3}");
		break;		/* ~A */
	case 213:
		fprintRTF("{\ansi\\'d5}");
		break;		/* ~O */
	case 227:
		fprintRTF("{\ansi\\'e3}");
		break;		/* ~a */
	case 245:
		fprintRTF("{\ansi\\'f5}");
		break;		/* ~o */
	case 209:
		fprintRTF("{\ansi\\'d1}");
		break;		/* ~N */
	case 241:
		fprintRTF("{\ansi\\'f1}");
		break;		/* ~n */

	case 223:
		fprintRTF("{\ansi\\'df}");
		break;		/* sz */
	case 161:
		fprintRTF("{\ansi\\'a1}");
		break;		/* ! */
	case 162:
		fprintRTF("{\ansi\\'a2}");
		break;		/* cent */
	case 163:
		fprintRTF("{\ansi\\'a3}");
		break;		/* pound */
	case 164:
		fprintRTF("{\ansi\\'a4}");
		break;		/* */
	case 165:
		fprintRTF("{\ansi\\'a5}");
		break;		/* Yen */
	case 166:
		fprintRTF("{\ansi\\'a6}");
		break;		/* pipe */
	case 167:
		fprintRTF("{\ansi\\'a7}");
		break;		/* paragraph */
	case 168:
		fprintRTF("{\ansi\\'a8}");
		break;		/* dots */
	case 169:
		fprintRTF("{\ansi\\'a9}");
		break;		/* copyright */
	case 170:
		fprintRTF("{\ansi\\'aa}");
		break;		/* a_ */
	case 171:
		fprintRTF("{\ansi\\'ab}");
		break;		/* << */
	case 172:
		fprintRTF("{\ansi\\'ac}");
		break;		/* -| */
	case 173:
		fprintRTF("{\ansi\\'ad}");
		break;		/* - */
	case 174:
		fprintRTF("{\ansi\\'ae}");
		break;		/* registered */
	case 175:
		fprintRTF("{\ansi\\'af}");
		break;		/* highscore */
	case 176:
		fprintRTF("{\ansi\\'b0}");
		break;		/* degree */
	case 177:
		fprintRTF("{\ansi\\'b1}");
		break;		/* +- */
	case 178:
		fprintRTF("{\ansi\\'b2}");
		break;		/* ^2 */
	case 179:
		fprintRTF("{\ansi\\'b3}");
		break;		/* ^3 */
	case 180:
		fprintRTF("{\ansi\\'b4}");
		break;		/* ' */
	case 181:
		fprintRTF("{\ansi\\'b5}");
		break;		/* my */
	case 182:
		fprintRTF("{\ansi\\'b6}");
		break;		/* pi */
	case 183:
		fprintRTF("{\ansi\\'b7}");
		break;		/* bullet */
	case 184:
		fprintRTF("{\ansi\\'b8}");
		break;		/* dot */
	case 185:
		fprintRTF("{\ansi\\'b9}");
		break;		/* ^1 */
	case 186:
		fprintRTF("{\ansi\\'ba}");
		break;		/* ^0_ */
	case 187:
		fprintRTF("{\ansi\\'bb}");
		break;		/* >> */
	case 188:
		fprintRTF("{\ansi\\'bc}");
		break;		/* 1/4 */
	case 189:
		fprintRTF("{\ansi\\'bd}");
		break;		/* 1/2 */
	case 190:
		fprintRTF("{\ansi\\'be}");
		break;		/* 3/4 */
	case 191:
		fprintRTF("{\ansi\\'bf}");
		break;		/* ? */

	case 197:
		fprintRTF("{\ansi\\'c5}");
		break;		/* oA */
	case 198:
		fprintRTF("{\ansi\\'c6}");
		break;		/* AE */
	case 199:
		fprintRTF("{\ansi\\'c7}");
		break;		/* french C */

	case 208:
		fprintRTF("{\ansi\\'d0}");
		break;		/* D */

	case 215:
		fprintRTF("{\ansi\\'d7}");
		break;		/* x */
	case 216:
		fprintRTF("{\ansi\\'d8}");
		break;		/* /O */

	case 222:
		fprintRTF("{\ansi\\'de}");
		break;		/* */

	case 229:
		fprintRTF("{\ansi\\'e5}");
		break;		/* oa */
	case 230:
		fprintRTF("{\ansi\\'e6}");
		break;		/* ae */
	case 231:
		fprintRTF("{\ansi\\'e7}");
		break;		/* french c */

	case 240:
		fprintRTF("{\ansi\\'f0}");
		break;		/* */

	case 247:
		fprintRTF("{\ansi\\'f7}");
		break;		/* / */
	case 248:
		fprintRTF("{\ansi\\'f8}");
		break;		/* /o */

	case 254:
		fprintRTF("{\ansi\\'fe}");
		break;		/* */

	case 127:
		fprintRTF(" ");	/* tilde should be translated to a
					 * space */
		break;		/* ~ */

	default:
		fprintRTF("%c", theChar);	/* other characters are
						 * written out unchanged */
	}
}

/******************************************************************************/
void 
Write_Default_Charset(char theChar)
/******************************************************************************
     purpose : writes theChar as an equivalent Rtf-Command
   parameter : theChar  will be converted, result is written to Rtf-file
     globals : fRtf
 ******************************************************************************/
{
	switch ((unsigned char) theChar) {
		case '~':fprintRTF(" ");	/* tilde should be translated
						 * to a space */
		break;

	default:
		fprintRTF("%c", theChar);	/* other characters are
						 * written out unchanged */
	}
}
