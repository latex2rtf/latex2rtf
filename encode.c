/*
 * $Id: encode.c,v 1.1 2001/08/12 21:15:46 prahl Exp $
 * History:
 * $Log: encode.c,v $
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
		/* case 246:fprintf(fRtf, "\\ansi\\'f6\\pc"); */
		case 0366:fprintf(fRtf, "{\ansi\\'f6}");
		break;		/* "o */
	case 214:
		fprintf(fRtf, "{\ansi\\'d6}");
		break;		/* "O */
	case 228:
		fprintf(fRtf, "{\ansi\\'e4}");
		break;		/* "a */
	case 196:
		fprintf(fRtf, "{\ansi\\'c4}");
		break;		/* "A */
	case 252:
		fprintf(fRtf, "{\ansi\\'fc}");
		break;		/* "u */
	case 220:
		fprintf(fRtf, "{\ansi\\'dc}");
		break;		/* "U */
	case 203:
		fprintf(fRtf, "{\ansi\\'cb}");
		break;		/* "E */
	case 207:
		fprintf(fRtf, "{\ansi\\'cf}");
		break;		/* "I */
	case 235:
		fprintf(fRtf, "{\ansi\\'eb}");
		break;		/* "e */
	case 239:
		fprintf(fRtf, "{\ansi\\'ef}");
		break;		/* "i */
	case 255:
		fprintf(fRtf, "{\ansi\\'ff}");
		break;		/* "y */
		/* LEG140498End */

	case 192:
		fprintf(fRtf, "{\ansi\\'c0}");
		break;		/* `A */
	case 200:
		fprintf(fRtf, "{\ansi\\'c8}");
		break;		/* `E */
	case 204:
		fprintf(fRtf, "{\ansi\\'cc}");
		break;		/* `I */
	case 210:
		fprintf(fRtf, "{\ansi\\'d2}");
		break;		/* `O */
	case 217:
		fprintf(fRtf, "{\ansi\\'d9}");
		break;		/* `U */
	case 224:
		fprintf(fRtf, "{\ansi\\'e0}");
		break;		/* `a */
	case 232:
		fprintf(fRtf, "{\ansi\\'e8}");
		break;		/* `e */
	case 236:
		fprintf(fRtf, "{\ansi\\'ec}");
		break;		/* `i */
	case 242:
		fprintf(fRtf, "{\ansi\\'f2}");
		break;		/* `o */
	case 249:
		fprintf(fRtf, "{\ansi\\'f9}");
		break;		/* `u */

	case 193:
		fprintf(fRtf, "{\ansi\\'c1}");
		break;		/* 'A */
	case 201:
		fprintf(fRtf, "{\ansi\\'c9}");
		break;		/* 'E */
	case 205:
		fprintf(fRtf, "{\ansi\\'cd}");
		break;		/* 'I */
	case 211:
		fprintf(fRtf, "{\ansi\\'d3}");
		break;		/* 'O */
	case 218:
		fprintf(fRtf, "{\ansi\\'da}");
		break;		/* 'U */
	case 225:
		fprintf(fRtf, "{\ansi\\'e1}");
		break;		/* 'a */
	case 233:
		fprintf(fRtf, "{\ansi\\'e9}");
		break;		/* 'e */
	case 237:
		fprintf(fRtf, "{\ansi\\'ed}");
		break;		/* 'i */
	case 243:
		fprintf(fRtf, "{\ansi\\'f3}");
		break;		/* 'o */
	case 250:
		fprintf(fRtf, "{\ansi\\'fa}");
		break;		/* 'u */
	case 221:
		fprintf(fRtf, "{\ansi\\'dd}");
		break;		/* 'Y */
	case 253:
		fprintf(fRtf, "{\ansi\\'fd}");
		break;		/* 'y */

	case 194:
		fprintf(fRtf, "{\ansi\\'c2}");
		break;		/* ^A */
	case 202:
		fprintf(fRtf, "{\ansi\\'ca}");
		break;		/* ^E */
	case 206:
		fprintf(fRtf, "{\ansi\\'ce}");
		break;		/* ^I */
	case 212:
		fprintf(fRtf, "{\ansi\\'d4}");
		break;		/* ^O */
	case 219:
		fprintf(fRtf, "{\ansi\\'db}");
		break;		/* ^U */
	case 226:
		fprintf(fRtf, "{\ansi\\'e2}");
		break;		/* ^a */
	case 234:
		fprintf(fRtf, "{\ansi\\'ea}");
		break;		/* ^e */
	case 238:
		fprintf(fRtf, "{\ansi\\'ee}");
		break;		/* ^i */
	case 244:
		fprintf(fRtf, "{\ansi\\'f4}");
		break;		/* ^o */
	case 251:
		fprintf(fRtf, "{\ansi\\'fb}");
		break;		/* ^u */

	case 195:
		fprintf(fRtf, "{\ansi\\'c3}");
		break;		/* ~A */
	case 213:
		fprintf(fRtf, "{\ansi\\'d5}");
		break;		/* ~O */
	case 227:
		fprintf(fRtf, "{\ansi\\'e3}");
		break;		/* ~a */
	case 245:
		fprintf(fRtf, "{\ansi\\'f5}");
		break;		/* ~o */
	case 209:
		fprintf(fRtf, "{\ansi\\'d1}");
		break;		/* ~N */
	case 241:
		fprintf(fRtf, "{\ansi\\'f1}");
		break;		/* ~n */

	case 223:
		fprintf(fRtf, "{\ansi\\'df}");
		break;		/* sz */
	case 161:
		fprintf(fRtf, "{\ansi\\'a1}");
		break;		/* ! */
	case 162:
		fprintf(fRtf, "{\ansi\\'a2}");
		break;		/* cent */
	case 163:
		fprintf(fRtf, "{\ansi\\'a3}");
		break;		/* pound */
	case 164:
		fprintf(fRtf, "{\ansi\\'a4}");
		break;		/* */
	case 165:
		fprintf(fRtf, "{\ansi\\'a5}");
		break;		/* Yen */
	case 166:
		fprintf(fRtf, "{\ansi\\'a6}");
		break;		/* pipe */
	case 167:
		fprintf(fRtf, "{\ansi\\'a7}");
		break;		/* paragraph */
	case 168:
		fprintf(fRtf, "{\ansi\\'a8}");
		break;		/* dots */
	case 169:
		fprintf(fRtf, "{\ansi\\'a9}");
		break;		/* copyright */
	case 170:
		fprintf(fRtf, "{\ansi\\'aa}");
		break;		/* a_ */
	case 171:
		fprintf(fRtf, "{\ansi\\'ab}");
		break;		/* << */
	case 172:
		fprintf(fRtf, "{\ansi\\'ac}");
		break;		/* -| */
	case 173:
		fprintf(fRtf, "{\ansi\\'ad}");
		break;		/* - */
	case 174:
		fprintf(fRtf, "{\ansi\\'ae}");
		break;		/* registered */
	case 175:
		fprintf(fRtf, "{\ansi\\'af}");
		break;		/* highscore */
	case 176:
		fprintf(fRtf, "{\ansi\\'b0}");
		break;		/* degree */
	case 177:
		fprintf(fRtf, "{\ansi\\'b1}");
		break;		/* +- */
	case 178:
		fprintf(fRtf, "{\ansi\\'b2}");
		break;		/* ^2 */
	case 179:
		fprintf(fRtf, "{\ansi\\'b3}");
		break;		/* ^3 */
	case 180:
		fprintf(fRtf, "{\ansi\\'b4}");
		break;		/* ' */
	case 181:
		fprintf(fRtf, "{\ansi\\'b5}");
		break;		/* my */
	case 182:
		fprintf(fRtf, "{\ansi\\'b6}");
		break;		/* pi */
	case 183:
		fprintf(fRtf, "{\ansi\\'b7}");
		break;		/* bullet */
	case 184:
		fprintf(fRtf, "{\ansi\\'b8}");
		break;		/* dot */
	case 185:
		fprintf(fRtf, "{\ansi\\'b9}");
		break;		/* ^1 */
	case 186:
		fprintf(fRtf, "{\ansi\\'ba}");
		break;		/* ^0_ */
	case 187:
		fprintf(fRtf, "{\ansi\\'bb}");
		break;		/* >> */
	case 188:
		fprintf(fRtf, "{\ansi\\'bc}");
		break;		/* 1/4 */
	case 189:
		fprintf(fRtf, "{\ansi\\'bd}");
		break;		/* 1/2 */
	case 190:
		fprintf(fRtf, "{\ansi\\'be}");
		break;		/* 3/4 */
	case 191:
		fprintf(fRtf, "{\ansi\\'bf}");
		break;		/* ? */

	case 197:
		fprintf(fRtf, "{\ansi\\'c5}");
		break;		/* oA */
	case 198:
		fprintf(fRtf, "{\ansi\\'c6}");
		break;		/* AE */
	case 199:
		fprintf(fRtf, "{\ansi\\'c7}");
		break;		/* french C */

	case 208:
		fprintf(fRtf, "{\ansi\\'d0}");
		break;		/* D */

	case 215:
		fprintf(fRtf, "{\ansi\\'d7}");
		break;		/* x */
	case 216:
		fprintf(fRtf, "{\ansi\\'d8}");
		break;		/* /O */

	case 222:
		fprintf(fRtf, "{\ansi\\'de}");
		break;		/* */

	case 229:
		fprintf(fRtf, "{\ansi\\'e5}");
		break;		/* oa */
	case 230:
		fprintf(fRtf, "{\ansi\\'e6}");
		break;		/* ae */
	case 231:
		fprintf(fRtf, "{\ansi\\'e7}");
		break;		/* french c */

	case 240:
		fprintf(fRtf, "{\ansi\\'f0}");
		break;		/* */

	case 247:
		fprintf(fRtf, "{\ansi\\'f7}");
		break;		/* / */
	case 248:
		fprintf(fRtf, "{\ansi\\'f8}");
		break;		/* /o */

	case 254:
		fprintf(fRtf, "{\ansi\\'fe}");
		break;		/* */

	case 127:
		fprintf(fRtf, " ");	/* tilde should be translated to a
					 * space */
		break;		/* ~ */

	default:
		fprintf(fRtf, "%c", theChar);	/* other characters are
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
		case '~':fprintf(fRtf, " ");	/* tilde should be translated
						 * to a space */
		break;

	default:
		fprintf(fRtf, "%c", theChar);	/* other characters are
						 * written out unchanged */
	}
}
