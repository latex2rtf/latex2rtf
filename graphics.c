/* $Id: graphics.c,v 1.10 2002/03/31 17:13:11 prahl Exp $ 
This file contains routines that handle LaTeX graphics commands
*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#ifdef UNIX
#include <unistd.h>
#endif
#include "main.h"
#include "graphics.h"
#include "parser.h"
#include "util.h"
#include "commands.h"
#include "convert.h"
#include "equation.h"

#define POINTS_PER_M 2834.65

/* Little endian macros to convert to and from host format to network byte ordering */
#define LETONS(A) ((((A) & 0xFF00) >> 8) | (((A) & 0x00FF) << 8))
#define LETONL(A) ((((A) & 0xFF000000) >> 24) | (((A) & 0x00FF0000) >>  8) | \
                  (((A) & 0x0000FF00) <<  8) | (((A) & 0x000000FF) << 24) )
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

typedef struct _WindowsMetaHeader
{
	unsigned short	FileType;		/* Type of metafile (0=memory, 1=disk) */
	unsigned short	HeaderSize;		/* Size of header in WORDS (always 9) */
	unsigned short	Version;		/* Version of Microsoft Windows used */
	unsigned long	FileSize;		/* Total size of the metafile in WORDs */
	unsigned short	NumOfObjects;	/* Number of objects in the file */
	unsigned long	MaxRecordSize;	/* The size of largest record in WORDs */
	unsigned short	NumOfParams;	/* Not Used (always 0) */
} WMFHEAD;

typedef struct _PlaceableMetaHeader
{
	unsigned long	Key;			/* Magic number (always 0x9AC6CDD7) */
	unsigned short	Handle;			/* Metafile HANDLE number (always 0) */
	short			Left;			/* Left coordinate in twips */
	short			Top;			/* Top coordinate in twips */
	short			Right;			/* Right coordinate in twips */
	short			Bottom;			/* Bottom coordinate in twips */
	unsigned short	Inch;			/* Scaling factor, 1440 => 1:1, 360 => 4:1, 2880 => 1:2 (half size) */
	unsigned long	Reserved;		/* Reserved (always 0) */
	unsigned short	Checksum;		/* Checksum value for previous 10 WORDs */
} PLACEABLEMETAHEADER;

typedef struct _EnhancedMetaHeader
{
	unsigned long	RecordType;		/* Record type (always 0x00000001)*/
	unsigned long	RecordSize;		/* Size of the record in bytes */
	long			BoundsLeft;		/* Left inclusive bounds */
	long			BoundsRight;	/* Right inclusive bounds */
	long			BoundsTop;		/* Top inclusive bounds */
	long			BoundsBottom;	/* Bottom inclusive bounds */
	long			FrameLeft;		/* Left side of inclusive picture frame */
	long			FrameRight;		/* Right side of inclusive picture frame */
	long			FrameTop;		/* Top side of inclusive picture frame */
	long			FrameBottom;	/* Bottom side of inclusive picture frame */
	unsigned long	Signature;		/* Signature ID (always 0x464D4520) */
	unsigned long	Version;		/* Version of the metafile */
	unsigned long	Size;			/* Size of the metafile in bytes */
	unsigned long	NumOfRecords;	/* Number of records in the metafile */
	unsigned short	NumOfHandles;	/* Number of handles in the handle table */
	unsigned short	Reserved;		/* Not used (always 0) */
	unsigned long	SizeOfDescrip;	/* Size of description string in WORDs */
	unsigned long	OffsOfDescrip;	/* Offset of description string in metafile */
	unsigned long	NumPalEntries;	/* Number of color palette entries */
	long			WidthDevPixels;	/* Width of reference device in pixels */
	long			HeightDevPixels;/* Height of reference device in pixels */
	long			WidthDevMM;		/* Width of reference device in millimeters */
	long			HeightDevMM;	/* Height of reference device in millimeters */
} ENHANCEDMETAHEADER;

static void
my_unlink(char *filename)
/******************************************************************************
     purpose : portable routine to delete filename
 ******************************************************************************/
{
#ifdef UNIX
	unlink(filename);
#endif
}


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

	diagnostics(6, "processing picture <%s>\n", fullpath);
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
int width, height;

	fp = open_graphics_file(s);
	if (fp == NULL) return;
	
	if (fseek(fp, 514L, SEEK_SET) || fread(buffer, 2, 4, fp) != 4) {
		diagnostics (WARNING, "Cannot read graphics file <%s>", s);
		fclose(fp);
		return;
	}
	
	top    = buffer[0];
	left   = buffer[1];
	bottom = buffer[2];
	right  = buffer[3];

	width  = right - left;
	height = bottom - top;
	
	if (g_little_endian) {
		top    = LETONS(top);
		bottom = LETONS(bottom);
		left   = LETONS(left);
		right  = LETONS(right);
	}

	diagnostics(1,"top = %d, bottom = %d", top, bottom);
	diagnostics(1,"left = %d, right = %d", left, right);
	diagnostics(1,"width = %d, height = %d", width, height);
	fprintRTF("\n{\\pict\\macpict\\picw%d\\pich%d\n", width, height);

	fseek(fp, -10L, SEEK_CUR);
	PutHexFile(fp);
	fprintRTF("}\n");
	fclose(fp);
}

void
GetPngSize(char *s, unsigned long *w, unsigned long *h)
/******************************************************************************
     purpose : determine height and width of file
 ******************************************************************************/
{
FILE *fp;
unsigned char buffer[16];
unsigned long width, height;
char reftag[9] = "\211PNG\r\n\032\n";
char refchunk[5] = "IHDR";

	*w = 0;
	*h = 0;
	fp = open_graphics_file(s);
	if (fp == NULL) return;

	if (fread(buffer,1,16,fp)<16) {
		diagnostics (WARNING, "Cannot read graphics file <%s>", s);
		fclose(fp);
		return;
	}

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

	if (g_little_endian) {
		width  = LETONL(width);
		height = LETONL(height);
	}

	*w = width;
	*h = height;
	fclose(fp);
}

void 
PutPngFile(char * s, double scale)
/******************************************************************************
     purpose : Include .png file in RTF
 ******************************************************************************/
{
FILE *fp;
unsigned long width, height, w, h;
int iscale;

	GetPngSize(s, &width, &height);

	diagnostics(1,"width = %ld, height = %ld", width, height);
	if (width==0) return;
	
	fp = open_graphics_file(s);
	if (fp == NULL) return;

	w = (unsigned long)( 100000.0*width  )/ ( 20* POINTS_PER_M );
	h = (unsigned long)( 100000.0*height )/ ( 20* POINTS_PER_M );
	fprintRTF("\n{\\pict\\pngblip\\picw%ld\\pich%ld", w, h);
	fprintRTF("\\picwgoal%ld\\pichgoal%ld", width*20, height*20);
	if (scale != 1.0) {
		iscale = (int) (scale * 100);
		fprintRTF("\\picscalex%d\\picscaley%d", iscale, iscale);
	}
	fprintRTF("\n");
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
int m,c;
unsigned short width, height;
unsigned long w, h;

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

	if (g_little_endian) {
		width  = LETONS(width);
		height = LETONS(height);
	}

	diagnostics(1,"width = %d, height = %d", width, height);

	w = (unsigned long)( 100000.0*width  )/ ( 20* POINTS_PER_M );
	h = (unsigned long)( 100000.0*height )/ ( 20* POINTS_PER_M );
	fprintRTF("\n{\\pict\\jpegblip\\picw%ld\\pich%ld", w, h);
	fprintRTF("\\picwgoal%ld\\pichgoal%ld\n", width*20, height*20);

	rewind(fp);
	PutHexFile(fp);
	fprintRTF("}\n");
	fclose(fp);
}

static void
PutEmfFile(char *s)
{
	FILE *fp;
	
	fp = open_graphics_file(s);
	if (fp == NULL) return;

/* identify file type */
/* extract size information */

	diagnostics(1, "EMF file inclusion not implemented yet");

/*	width = buffer[1];
	height = buffer[0];
	diagnostics(1,"width = %d, height = %d", width, height);

	fprintRTF("\n{\\pict\\emfblip\\picw%d\\pich%d\n", width, height);
	rewind(fp);
	PutHexFile(fp);
	fprintRTF("}\n");
*/
	fclose(fp);
}

static void
PutEpsFile(char *s)
{
	char *cmd, *s1, *p, *png, *tmp;
	diagnostics(1, "filename = <%s>", s);
	s1 = strdup(s);
	if ((p=strstr(s1,".eps")) == NULL && (p=strstr(s1,".EPS")) == NULL) {
		diagnostics(1, "<%s> is not an EPS file", s);
		free(s1);
		return;
	}
	strcpy(p,".png");
	tmp = getTmpPath();
	png = strdup_together(tmp,s1);
	cmd = (char *) malloc(strlen(s)+strlen(png)+10);
	sprintf(cmd, "convert %s %s", s, png);	
	system(cmd);
	
	PutPngFile(png,1.0);
	my_unlink(png);
	
	free(png);
	free(cmd);
	free(s1);
	free(tmp);
}

static void
PutTiffFile(char *s)
{
	char *cmd, *s1, *p, *png, *tmp;
	diagnostics(1, "filename = <%s>", s);
	s1 = strdup(s);
	if ((p=strstr(s1,".tiff")) == NULL && (p=strstr(s1,".TIFF")) == NULL) {
		diagnostics(1, "<%s> is not an EPS file", s);
		free(s1);
		return;
	}
	strcpy(p,".png");
	tmp = getTmpPath();
	png = strdup_together(tmp,s1);
	cmd = (char *) malloc(strlen(s)+strlen(png)+10);
	sprintf(cmd, "convert %s %s", s, png);	
	system(cmd);
	
	PutPngFile(png,1.0);
	my_unlink(png);
	
	free(png);
	free(cmd);
	free(s1);
	free(tmp);
}

static void
PutGifFile(char *s)
{
	char *cmd, *s1, *p, *png, *tmp;
	diagnostics(1, "filename = <%s>", s);
	s1 = strdup(s);
	if ((p=strstr(s1,".gif")) == NULL && (p=strstr(s1,".GIF")) == NULL) {
		diagnostics(1, "<%s> is not an gif file", s);
		free(s1);
		return;
	}
	strcpy(p,".png");
	tmp = getTmpPath();
	png = strdup_together(tmp,s1);
	cmd = (char *) malloc(strlen(s)+strlen(png)+10);
	sprintf(cmd, "convert %s %s", s, png);	
	system(cmd);
	
	PutPngFile(png,1.0);
	my_unlink(png);

	free(png);
	free(cmd);
	free(s1);
	free(tmp);
}

void
PutLatexFile(char *s)
/******************************************************************************
 purpose   : Convert LaTeX to Bitmap and insert in RTF file
 ******************************************************************************/
{
	char *png, *cmd;
	int err, cmd_len;
	unsigned long width, height;
	unsigned long max=32767/20;
	int resolution = g_dots_per_inch*2; /*points per inch */
	
	diagnostics(4, "Entering PutLatexFile");

	png = strdup_together(s,".png");

	cmd_len = strlen(s)+25;
	if (g_home_dir)
		cmd_len += strlen(g_home_dir);

	cmd = (char *) malloc(cmd_len);

	/* iterate until png is small enough for Word */
	do {
		resolution /= 2;
		if (g_home_dir==NULL)
			sprintf(cmd, "latex2png -d %d %s", resolution, s);	
		else
			sprintf(cmd, "latex2png -d %d -H \"%s\" %s", resolution, g_home_dir, s);	

		err = system(cmd);
		diagnostics(1, "cmd = <%s>", cmd);
		if (err==0){
			GetPngSize(png, &width, &height);
			diagnostics(4, "png size width=%d height =%d", width, height);
		}
	} while (!err && resolution>10 && ( (width>max) || (height>max)) );
	
	if (err==0)
		PutPngFile(png,72.0/resolution);
	
	free(png);
	free(cmd);
}

void 
CmdGraphics(int code)
/*
\includegraphics[parameters]{filename}

where parameters is a comma-separated list of any of the following: 
bb=llx lly urx ury (bounding box),
width=h_length,
height=v_length,
angle=angle,
scale=factor,
clip=true/false,
draft=true/false.
*/
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
		PutPngFile(filename,1.0);

	else if (strstr(filename, ".gif")  || strstr(filename, ".GIF"))
		PutGifFile(filename);

	else if (strstr(filename, ".emf")  || strstr(filename, ".EMF"))
		PutEmfFile(filename);

	else if (strstr(filename, ".eps")  || strstr(filename, ".EPS"))
		PutEpsFile(filename);

	else if (strstr(filename, ".ps")  || strstr(filename, ".PS"))
		PutEpsFile(filename);

	else if (strstr(filename, ".tiff")  || strstr(filename, ".TIFF"))
		PutTiffFile(filename);

	else if (strstr(filename, ".jpg")  || strstr(filename, ".JPG") ||
		strstr(filename, ".jpeg") || strstr(filename, ".JPEG"))
		PutJpegFile(filename);

	else 
		diagnostics(WARNING, "Conversion of '%s' not supported", filename);
	
	free(filename);
}

void 
CmdPicture(int code)
/******************************************************************************
  purpose: handle \begin{picture} ... \end{picture}
           by converting to png image and inserting
 ******************************************************************************/
{
	char *pre, *post, *picture;	
	
	if (code & ON) {
		pre = strdup("\\begin{picture}");
		post = strdup("\\end{picture}");
		picture=getTexUntil(post,0);
		WriteLatexAsBitmap(pre,picture,post);
		ConvertString(post);  /* to balance the \begin{picture} */
		free(pre);
		free(post);
		free(picture);
	}
}
