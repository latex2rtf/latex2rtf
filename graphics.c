/* $Id: graphics.c,v 1.2 2002/03/03 06:29:06 prahl Exp $ 
This file contains routines that handle LaTeX graphics commands
*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "graphics.h"
#include "parser.h"

/* macros to extract big-endian short and long ints: */
/*
#define SH(p) (((unsigned short)((p)[1])) | ((unsigned short)((p)[0]) << 8))
#define LG(p) (((unsigned long)(SH((p)+2))) | ((unsigned long)(SH(p)) << 16))
#define SH(p) ((unsigned short)(unsigned char)((p)[1]) | ((unsigned short)(unsigned char)((p)[0]) << 8))
#define LG(p) ((unsigned long)(SH((p)+2)) | ((unsigned long)(SH(p)) << 16))
#define SH(p) ((unsigned short)(unsigned char)((p)[0]) | ((unsigned short)(unsigned char)((p)[1]) << 8))
#define LG(p) ((unsigned long)(SH((p))) | ((unsigned long)(SH((p)+2)) << 16))
*/
#define SH(p) (((unsigned short)((p)[0])) | ((unsigned short)((p)[1]) << 8))
#define LG(p) (((unsigned long)(SH((p)))) | ((unsigned long)(SH((p)+2)) << 16))

/*
Version 1.6 RTF files can include pictures as follows

<pict> 			'{' \pict (<brdr>? & <shading>? & <picttype> & <pictsize> & <metafileinfo>?) <data> '}'
<picttype>		\emfblip | \pngblip | \jpegblip | \macpict | \pmmetafile | \wmetafile 
			 	         | \dibitmap <bitmapinfo> | \wbitmap <bitmapinfo>
<bitmapinfo> 	\wbmbitspixel & \wbmplanes & \wbmwidthbytes
<pictsize> 		(\picw & \pich) \picwgoal? & \pichgoal? \picscalex? & \picscaley? & \picscaled? & \piccropt? & \piccropb? & \piccropr? & \piccropl?
<metafileinfo> 	\picbmp & \picbpp
<data> 			(\bin #BDATA) | #SDATA

\emfblip 				Source of the picture is an EMF (enhanced metafile).
\pngblip 				Source of the picture is a PNG.
\jpegblip 				Source of the picture is a JPEG.
\shppict 				Specifies a Word 97-2000 picture. This is a destination control word.
\nonshppict 			Specifies that Word 97-2000 has written a {\pict destination that it 
						will not read on input. This keyword is for compatibility with other readers.
\macpict                Source of the picture is PICT file (Quickdraw)
\pmmetafileN            Source of the picture is an OS/2 metafile
\wmetafileN             Source of the picture is a Windows metafile
\dibitmapN              Source of the picture is a Windows device-independent bitmap
\wbitmapN               Source of the picture is a Windows device-dependent bitmap
*/

static FILE * 
open_graphics_file(char * s)
{
char           *fullpath;
FILE		   *fp = NULL;

	if (s==NULL) return NULL;
	
#ifdef __MWERKS__
	{
		char * path, *dp;			/* use directory of latex file */
		path = strdup(latexname);		
		dp = strrchr(path, ':');
		if (dp != NULL) {
			dp++;
			*dp = '\0';
		} else
			*path = '\0';
		fullpath = strdup_together(path, s);
		free(path);
	}
#else
	fullpath = strdup(s);
#endif

	diagnostics(1, "processing picture <%s>\n", fullpath);
	fp=fopen(fullpath, "rb");
	free(fullpath);
	return fp;
}

static void 
PutHexFile(FILE *fp)
/******************************************************************************
     purpose : write entire file to RTF as hex
 ******************************************************************************/
{
int i, c;

	i = 0;
	while ((c = fgetc(fp)) != EOF) {
		fprintRTF("%.2x", c);
		if (++i > 126) {		/* keep lines 254 chars long */
			i = 0;
			fprintRTF("\n");
		}	
	}
}

static void 
PutPictFile(char * s)
/******************************************************************************
     purpose : Include .pict file in RTF
 ******************************************************************************/
{
FILE *fp;
short buffer[5];
short top, left, bottom, right;
int width, height,i;

	fp = open_graphics_file(s);
	if (fp == NULL) return;
	
	if (fseek(fp, 514L, SEEK_SET) || fread(buffer, 2, 4, fp) != 4) {
		diagnostics (WARNING, "Cannot read graphics file <%s>", s);
		fclose(fp);
		return;
	}
	
	fprintf(stderr,"\n");
	for (i=0; i<10; i++)
		fprintf(stderr,"%0X ", buffer[i]);
	fprintf(stderr,"\n");
	
	top    = buffer[0];
	left   = buffer[1];
	bottom = buffer[2];
	right  = buffer[3];

	width  = right - left;
	height = bottom - top;
	
	diagnostics(1,"top = %d, bottom = %d", top, bottom);
	diagnostics(1,"left = %d, right = %d", left, right);
	diagnostics(1,"width = %d, height = %d", width, height);
	fprintRTF("\n{\\pict\\macpict\\picw%d\\pich%d\n", width, height);

	fseek(fp, -10L, SEEK_CUR);
	PutHexFile(fp);
	fprintRTF("}\n");
	fclose(fp);
}

static void 
PutPngFile(char * s)
/******************************************************************************
     purpose : Include .png file in RTF
 ******************************************************************************/
{
FILE *fp;
int i;
unsigned char buffer[16];
unsigned long width, height;
char reftag[9] = "\211PNG\r\n\032\n";
char refchunk[5] = "IHDR";

	fp = open_graphics_file(s);
	if (fp == NULL) return;

	if (fread(buffer,1,16,fp)<16) {
		diagnostics (WARNING, "Cannot read graphics file <%s>", s);
		fclose(fp);
		return;
	}

	fprintf(stderr,"\n");
	for (i=0; i<20; i++)
		fprintf(stderr,"%0X ", buffer[i]);
	fprintf(stderr,"\n");
	
	
	if (memcmp(buffer,reftag,8)!=0 || memcmp(buffer+12,refchunk,4)!=0) {
		diagnostics (WARNING, "Graphics file <%s> is not a PNG file!", s);
		fclose(fp);
		return;
	}

	if (fread(&width,4,1,fp)!=1 || fread(&height,4,1,fp)!=1) {
		diagnostics (WARNING, "Cannot read graphics file <%s>", s);
		fclose(fp);
		return;
	}
	diagnostics(1,"width = %ld, height = %ld", width, height);

	fprintRTF("\n{\\pict\\pngblip\\picw%ld\\pich%ld\n", width, height);
	rewind(fp);
	PutHexFile(fp);
	fprintRTF("}\n");
	fclose(fp);
}

static void 
PutJpegFile(char * s)
/******************************************************************************
     purpose : Include .jpeg file in RTF
 ******************************************************************************/
{
FILE *fp;
unsigned short buffer[2];
int m,c,i;
unsigned short width, height;

	fp = open_graphics_file(s);
	if (fp == NULL) return;

	if ((c=fgetc(fp)) != 0xFF && (c=fgetc(fp)) != 0xD8) {
		fclose(fp);
		diagnostics(WARNING, "<%s> is not really a JPEG file --- skipping");
		return;
	}
	
	do {  /* Look for SOFn tag */
	
		  while (!feof(fp) && fgetc(fp) != 0xFF);   		/* Find 0xFF byte */
		  
		  while (!feof(fp) && (m=fgetc(fp)) == 0xFF);  	/* Skip multiple 0xFFs */
		  
	} while (!feof(fp) && m!=0xC0 && m!=0xC1 && m!=0xC2 && m!=0xC3 && m!=0xC5 && m!=0xC6 && m!=0xC7 &&
					      m!=0xC9 && m!=0xCA && m!=0xCB && m!=0xCD && m!=0xCE && m!=0xCF );    
	
	if (fseek(fp, 3, SEEK_CUR) || fread(buffer,2,2,fp) != 2) {
		diagnostics (WARNING, "Cannot read graphics file <%s>", s);
		fclose(fp);
		return;
	}

	width = buffer[1];
	height = buffer[0];
	diagnostics(1,"width = %d, height = %d", width, height);

	fprintRTF("\n{\\pict\\jpegblip\\picw%d\\pich%d\n", width, height);
	rewind(fp);
	PutHexFile(fp);
	fprintRTF("}\n");
	fclose(fp);
}

void 
CmdGraphics(int code)
{
	char           *options;
	char           *filename;

	/* could be \includegraphics*[0,0][5,5]{file.pict} */

	options = getBracketParam();
	if (options) free(options);

	options = getBracketParam();
	if (options) free(options);
	
	filename = getBraceParam();

	if (strstr(filename, ".pict") || strstr(filename, ".PICT"))
		PutPictFile(filename);
		
	else if (strstr(filename, ".png")  || strstr(filename, ".PNG"))
		PutPngFile(filename);

	else if (strstr(filename, ".jpg")  || strstr(filename, ".JPG") ||
		strstr(filename, ".jpeg") || strstr(filename, ".JPEG"))
		PutJpegFile(filename);

	else 
		diagnostics(WARNING, "Conversion of '%s' not supported", filename);
	
	free(filename);
}


