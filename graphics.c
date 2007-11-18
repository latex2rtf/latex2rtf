
/* graphics.c - routines that handle LaTeX graphics commands

Copyright (C) 2001-2002 The Free Software Foundation

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
    2001-2002 Scott Prahl
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#ifdef UNIX
#include <unistd.h>
#endif
#include "cfg.h"
#include "main.h"
#include "graphics.h"
#include "parser.h"
#include "utils.h"
#include "commands.h"
#include "convert.h"
#include "funct1.h"
#include "preamble.h"
#include "counters.h"
#include "vertical.h"

/* number of points (72/inch) in a meter */
#define POINTS_PER_METER 2834.65

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

typedef struct _WindowsMetaHeader {
    unsigned short FileType;    /* Type of metafile (0=memory, 1=disk) */
    unsigned short HeaderSize;  /* Size of header in WORDS (always 9) */
    unsigned short Version;     /* Version of Microsoft Windows used */
    unsigned long FileSize;     /* Total size of the metafile in WORDs */
    unsigned short NumOfObjects;    /* Number of objects in the file */
    unsigned long MaxRecordSize;    /* The size of largest record in WORDs */
    unsigned short NumOfParams; /* Not Used (always 0) */
} WMFHEAD;

typedef struct _PlaceableMetaHeader {
    unsigned long Key;          /* Magic number (always 0x9AC6CDD7) */
    unsigned short Handle;      /* Metafile HANDLE number (always 0) */
    short Left;                 /* Left coordinate in twips */
    short Top;                  /* Top coordinate in twips */
    short Right;                /* Right coordinate in twips */
    short Bottom;               /* Bottom coordinate in twips */
    unsigned short Inch;        /* Scaling factor, 1440 => 1:1, 360 => 4:1, 2880 => 1:2 (half size) */
    unsigned long Reserved;     /* Reserved (always 0) */
    unsigned short Checksum;    /* Checksum value for previous 10 WORDs */
} PLACEABLEMETAHEADER;

typedef struct _EnhancedMetaHeader {
    unsigned long RecordType;   /* Record type (always 0x00000001) */
    unsigned long RecordSize;   /* Size of the record in bytes */
    long BoundsLeft;            /* Left inclusive bounds */
    long BoundsTop;             /* Top inclusive bounds */
    long BoundsRight;           /* Right inclusive bounds */
    long BoundsBottom;          /* Bottom inclusive bounds */
    long FrameLeft;             /* Left side of inclusive picture frame */
    long FrameTop;              /* Top side of inclusive picture frame */
    long FrameRight;            /* Right side of inclusive picture frame */
    long FrameBottom;           /* Bottom side of inclusive picture frame */
    unsigned long Signature;    /* Signature ID (always 0x464D4520) */
    unsigned long Version;      /* Version of the metafile */
    unsigned long Size;         /* Size of the metafile in bytes */
    unsigned long NumOfRecords; /* Number of records in the metafile */
    unsigned short NumOfHandles;    /* Number of handles in the handle table */
    unsigned short Reserved;    /* Not used (always 0) */
    unsigned long SizeOfDescrip;    /* Size of description string in WORDs */
    unsigned long OffsOfDescrip;    /* Offset of description string in metafile */
    unsigned long NumPalEntries;    /* Number of color palette entries */
    long WidthDevPixels;        /* Width of reference device in pixels */
    long HeightDevPixels;       /* Height of reference device in pixels */
    long WidthDevMM;            /* Width of reference device in millimeters */
    long HeightDevMM;           /* Height of reference device in millimeters */
} ENHANCEDMETAHEADER;

typedef struct _EmrFormat {
    unsigned long Signature;    /* 0x46535045 for EPS, 0x464D4520 for EMF */
    unsigned long Version;      /* EPS version number or 0x00000001 for EMF */
    unsigned long Data;         /* Size of data in bytes */
    unsigned long OffsetToData; /* Offset to data */
} EMRFORMAT;

typedef struct _GdiCommentMultiFormats {
    unsigned long Identifier;   /* Comment ID (0x43494447) */
    unsigned long Comment;      /* Multiformats ID (0x40000004) */
    long BoundsLeft;            /* Left side of bounding rectangle */
    long BoundsRight;           /* Right side of bounding rectangle */
    long BoundsTop;             /* Top side of bounding rectangle */
    long BoundsBottom;          /* Bottom side of bounding rectangle */
    unsigned long NumFormats;   /* Number of formats in comment */
    EMRFORMAT *Data;            /* Array of comment data */
} GDICOMMENTMULTIFORMATS;

#define CONVERT_SIMPLE 1
#define CONVERT_CROP   2
#define CONVERT_LATEX  3
#define CONVERT_PDF    4

static char *g_psset_info   = NULL;
static char *g_psstyle_info = NULL;

/******************************************************************************
     purpose : portable routine to delete filename
 ******************************************************************************/
static void my_unlink(char *filename)
{
#ifdef UNIX
    unlink(filename);
#endif
}

/******************************************************************************
     purpose : create a tmp file name using only the end of the filename
 ******************************************************************************/
static char *strdup_tmp_path(char *s)
{
    char *tmp, *p, *fullname, c;

    if (s == NULL)
        return NULL;

    tmp = getTmpPath();

    c = PATHSEP;
    p = strrchr(s, c);

    if (!p)
        fullname = strdup_together(tmp, s);
    else
        fullname = strdup_together(tmp, p + 1);

    free(tmp);
    return fullname;
}

/******************************************************************************
   purpose :  System specific shell graphics commands 

 Need to worry about the global flags used on the command-line
  -D dpi           g_dots_per_inch
  -P path          g_script_dir
  -T /path/to/tmp  g_tmp_path
  
	We need e.g. to create a system command to convert a PDF to a PNG file.
	In principle, this could be as simple as
	
		convert file.pdf file.png
		
	Unfortunately, we need to specify the pixel density, and more importantly,
	crop whitespace out of the images appropriately.  The command then becomes
	
	    convert -crop 0x0 -units PixelsPerInch -density 300 file.pdf file.png
	    
	Now the problem arises that apparently ImageMagick reads the wrong /MediaBox
	for PDF files and this gives a full-page image.  Since GhostScript is 
	required for ImageMagick, the solution is to use GhostScript directly
	
		gs -dNOPAUSE -dSAFER -dBATCH -sDEVICE=pngalpha -sOutputFile=file.png -r300 file.pdf
		
	Unfortunately, this fails to work under Windows XP because gs (gswin32c)
	fails to execute with message "Program too large for working storage" 
	(in German: "Programm zu gross fuer den Arbeitsspeicher").  Wilfried wrote
	the script pdf2pnga to solve this problem.

	So here we are, creating different commands for Windows XP and Unix!
	
	Parameters:
	
	opt		type of conversion
	offset	vertical offset
	in		input filename
	out		output filename
	
 ******************************************************************************/

static char *SysGraphicsConvert(int opt, int offset, char *in, char *out)

{
    char cmd[512], *out_tmp;
	int err;

    int N = 512;	
	int dpi   = g_dots_per_inch;
	
    out_tmp = strdup_tmp_path(out);
   
#ifdef UNIX

	if (strchr(in, (int) '\'')) {
		diagnostics(WARNING, "single quote found in filename '%s'.  skipping conversion", in);
		free(out_tmp);
		return NULL;
	}
	
	if (out && strchr(out_tmp, (int) '\'')) {
		diagnostics(WARNING, "single quote found in filename '%s'.  skipping conversion", out_tmp);
		free(out_tmp);
		return NULL;
	}

	if (opt == CONVERT_SIMPLE) {
		char format_simple[] = "convert '%s' '%s'";
		snprintf(cmd, N, format_simple, in, out_tmp);
	}

	if (opt == CONVERT_CROP) {
		char format_crop[]   = "convert -crop 0x0 -units PixelsPerInch -density %d '%s' '%s'";
		snprintf(cmd, N, format_crop, dpi, in, out_tmp);
	}

	if (opt == CONVERT_LATEX) {

		if (g_home_dir == NULL) {
			char format_unix[] = "%slatex2png -d %d -o %d '%s'";
			
			if (g_script_dir)
				snprintf(cmd, N, format_unix, g_script_dir, dpi, offset, in);
			else
				snprintf(cmd, N, format_unix, "", dpi, offset, in);
			
		} else {
			char format_unix[] = "%slatex2png -d %d -o %d -H '%s' '%s'";
			if (g_script_dir)
				snprintf(cmd, N, format_unix, g_script_dir, dpi, offset, g_home_dir, in);
			else
				snprintf(cmd, N, format_unix, "", dpi, offset, g_home_dir, in);
		}
	}
	
	if (opt == CONVERT_PDF) {
		#ifdef __APPLE__
		char format_apple[] = "/usr/bin/sips -s format png -s dpiHeight %d -s dpiWidth %d --out '%s' '%s' > /dev/null";
		snprintf(cmd, N, format_apple, dpi, dpi, out_tmp, in);
		#else
		char format_unix[] = "gs -q -dNOPAUSE -dSAFER -dBATCH -sDEVICE=pngalpha -r%d -sOutputFile='%s' '%s'";
		snprintf(cmd, N, format_unix, dpi, out_tmp, in);
		#endif
	}

#else

	if (opt == CONVERT_SIMPLE) {
		char format_simple[] = "convert \"%s\" \"%s\"";
		snprintf(cmd, N, format_simple, in, out_tmp);
	}

	if (opt == CONVERT_CROP) {
		char format_crop[]   = "convert -crop 0x0 -units PixelsPerInch -density %d \"%s\" \"%s\"";
		snprintf(cmd, N, format_crop, dpi, in, out_tmp);
	}

	if (opt == CONVERT_LATEX) {
		if (g_home_dir == NULL){
			char format_xp[] = "bash latex2png -d %d -o %d \"%s\"";
			snprintf(cmd, N, format_xp, dpi, offset, in);
		} else {
			char format_xp[] = "bash latex2png -d %d -o %d -H \"%s\" \"%s\"";
			snprintf(cmd, N, format_xp, dpi, offset, g_home_dir, in);
		}
	}
	
	if (opt == CONVERT_PDF) {
		char format_xp[] = "bash pdf2pnga \"%s\" \"%s\" %d";
		snprintf(cmd, N, format_xp, in, out_tmp, dpi);
	}
	
#endif

    diagnostics(3, "command `%s`", cmd);
    err = system(cmd);

    if (err != 0) {
        diagnostics(WARNING, "error=%d when converting %s", err, in);
		free(out_tmp);
        return NULL;
    }
	
	return out_tmp;
}

static void PicComment(short label, short size, FILE * fp)
{
    short long_comment = 0x00A1;
    short short_comment = 0x00A0;
    short tag;

    tag = (size) ? long_comment : short_comment;

    if (g_little_endian) {
        tag = LETONS(tag);
        label = LETONS(label);
        size = LETONS(size);
    }

    if (fwrite(&tag, 2, 1, fp) != 2)
        return;
    if (fwrite(&label, 2, 1, fp) != 2)
        return;
    if (size) {
        if (fwrite(&size, 2, 1, fp) != 2)
            return;
    }
}

static char *strdup_new_extension(char *s, char *old_ext, char *new_ext)
{
    char *new_name, *p;

    p = strstr(s, old_ext);
    if (p == NULL)
        return NULL;

    new_name = strdup_together(s, new_ext);
    p = strstr(new_name, old_ext);
    strcpy(p, new_ext);
    return new_name;
}

/******************************************************************************
     purpose : return a string containing an absolute path
 ******************************************************************************/
static char *strdup_absolute_path(char *s)
{
    char c = PATHSEP;
    char *abs_path = NULL;

    if (s) {
        if (*s == c || g_home_dir == NULL)
            abs_path = strdup(s);
        else
            abs_path = strdup_together(g_home_dir, s);
    }

    return abs_path;
}


/******************************************************************************
     purpose : create a pict file from an EPS file and return file name for
               the pict file.  Ideally this file will contain both the bitmap
               and the original EPS embedded in the PICT file as comments.  If a
               bitmap cannot be created, then the EPS is still embedded in the PICT
               file so that at least the printed version will be good.
 ******************************************************************************/
static char *eps_to_pict(char *s)
{
    char *p, *pict, buffer[560];
    long ii, pict_bitmap_size, eps_size;
    short handle_size;
    unsigned char byte;
    short PostScriptBegin = 190;
    short PostScriptEnd = 191;
    short PostScriptHandle = 192;
    char *pict_eps = NULL;
    char *eps = NULL;
    char *return_value = NULL;
    FILE *fp_eps = NULL;
    FILE *fp_pict_bitmap = NULL;
    FILE *fp_pict_eps = NULL;
	int offset = 0;
	
    diagnostics(3, "eps_to_pict '%s'", s);

    /* Create filename for bitmap */
    pict = strdup_new_extension(s, ".eps", "a.pict");
    if (pict == NULL) {
        pict = strdup_new_extension(s, ".EPS", "a.pict");
        if (pict == NULL)
            goto Exit;
    }

    /* Create filename for eps file */
    p = strdup_new_extension(s, ".eps", ".pict");
    if (p == NULL) {
        p = strdup_new_extension(s, ".EPS", ".pict");
        if (p == NULL)
            goto Exit;
    }
    pict_eps = strdup(p);
    free(p);

    eps = strdup_together(g_home_dir, s);

    /* create a bitmap version of the eps file */
    return_value = SysGraphicsConvert(CONVERT_CROP, offset, eps, pict);
    free(pict);
	if (return_value == NULL) goto Exit;
	
    /* open the eps file and make sure that it is less than 32k */
    fp_eps = fopen(eps, "rb");
    if (fp_eps == NULL)
        goto Exit;
    fseek(fp_eps, 0, SEEK_END);
    eps_size = ftell(fp_eps);
    if (eps_size > 32000) {
        diagnostics(WARNING, "EPS file >32K ... using bitmap only");
        goto Exit;
    }
    rewind(fp_eps);
    diagnostics(WARNING, "eps size is 0x%X bytes", eps_size);

    /* open bitmap pict file and get file size */
    fp_pict_bitmap = fopen(pict, "rb");
    if (fp_pict_bitmap == NULL)
        goto Exit;
    fseek(fp_pict_bitmap, 0, SEEK_END);
    pict_bitmap_size = ftell(fp_pict_bitmap);
    rewind(fp_pict_bitmap);

    /* open new pict file */
    fp_pict_eps = fopen(pict_eps, "w");
    if (fp_pict_eps == NULL)
        goto Exit;

    /* copy header 512 buffer + 40 byte header */
    if (fread(&buffer, 1, 512 + 40, fp_pict_bitmap) != 512 + 40)
        goto Exit;
    if (fwrite(&buffer, 1, 512 + 40, fp_pict_eps) != 512 + 40)
        goto Exit;

    /* insert comment that allows embedding postscript */
    PicComment(PostScriptBegin, 0, fp_pict_eps);

    /* copy bitmap 512+40 bytes of header + 2 bytes at end */
    for (ii = 512 + 40 + 2; ii < pict_bitmap_size; ii++) {
        if (fread(&byte, 1, 1, fp_pict_bitmap) != 1)
            goto Exit;
        if (fwrite(&byte, 1, 1, fp_pict_eps) != 1)
            goto Exit;
    }

    /* copy eps graphic (write an even number of bytes) */
    handle_size = eps_size;
    if (odd(eps_size))
        handle_size++;

    PicComment(PostScriptHandle, handle_size, fp_pict_eps);
    for (ii = 0; ii < eps_size; ii++) {
        if (fread(&byte, 1, 1, fp_eps) != 1)
            goto Exit;
        if (fwrite(&byte, 1, 1, fp_pict_eps) != 1)
            goto Exit;
    }
    if (odd(eps_size)) {
        byte = ' ';
        if (fwrite(&byte, 1, 1, fp_pict_eps) != 1)
            goto Exit;
    }

    /* close file */
    PicComment(PostScriptEnd, 0, fp_pict_eps);
    byte = 0x00;
    if (fwrite(&byte, 1, 1, fp_pict_eps) != 1)
        goto Exit;
    byte = 0xFF;
    if (fwrite(&byte, 1, 1, fp_pict_eps) != 1)
        goto Exit;

    return_value = pict_eps;

  Exit:
    if (eps)
        free(eps);
    if (pict_eps)
        free(pict_eps);
    if (pict)
        free(pict);

    if (fp_eps)
        fclose(fp_eps);
    if (fp_pict_eps)
        fclose(fp_pict_eps);
    if (fp_pict_bitmap)
        fclose(fp_pict_bitmap);
    return return_value;
}

/******************************************************************************
     purpose : create a png file from an EPS or PS file and return file name
 ******************************************************************************/
static char *eps_to_png(char *name)
{
    char *png, *out;
	
    diagnostics(3, " eps_to_png '%s'", name);

    if (strstr(name, ".eps") != NULL)
    	png = strdup_new_extension(name, ".eps", ".png");
	else if (strstr(name, ".EPS") != NULL)
    	png = strdup_new_extension(name, ".EPS", ".png");
	else if (strstr(name, ".ps") != NULL)
    	png = strdup_new_extension(name, ".ps", ".png");
	else if (strstr(name, ".PS") != NULL)
    	png = strdup_new_extension(name, ".PS", ".png");
    else
    	return NULL;
		
    out = SysGraphicsConvert(CONVERT_CROP, 0, name, png);

    free(png);
    return out;
}

/******************************************************************************
     purpose : create a png file from a PDF file and return file name
 ******************************************************************************/
static char *pdf_to_png(char *pdf)
{
    char *png, *out;

    diagnostics(2, "converting '%s' to png file", pdf);

    if (strstr(pdf, ".pdf") != NULL)
    	png = strdup_new_extension(pdf, ".pdf", ".png");
	else if (strstr(pdf, ".PDF") != NULL)
    	png = strdup_new_extension(pdf, ".PDF", ".png");
	else
		return NULL;
		
    out = SysGraphicsConvert(CONVERT_PDF, 0, pdf, png);
	free(png);
    return out;
}

/******************************************************************************
     purpose : create a wmf file from an EPS file and return file name
 ******************************************************************************/
static char *eps_to_emf(char *name)
{
    FILE *fp;
    char *cmd, *emf, *outfile;
    size_t cmd_len;
	
    char ans[50];
    long width, height;

	outfile = NULL;
    diagnostics(3, "filename = '%s'", name);

    if (strstr(name, ".eps") != NULL)
    	outfile = strdup_new_extension(name, ".eps", ".wmf");
	else if (strstr(name, ".EPS") != NULL)
    	outfile = strdup_new_extension(name, ".EPS", ".wmf");

	if (outfile == NULL) return NULL;

    emf = strdup_tmp_path(outfile);

    /* Determine bounding box for EPS file */
    cmd_len = strlen(name) + strlen("identify -format \"%w %h\" ") + 3;
    cmd = (char *) malloc(cmd_len);
    snprintf(cmd, cmd_len, "identify -format \"%%w %%h\" \"%s\"", name);
    fp = popen(cmd, "r");
    if (fgets(ans, 50, fp) != NULL)
        sscanf(ans, "%ld %ld", &width, &height);
    pclose(fp);
    free(cmd);

    fp = fopen(emf, "wb");

    /* write ENHANCEDMETAHEADER */

    /* write GDICOMMENTMULTIFORMATS */

    /* write EMRFORMAT containing EPS */

    free(outfile);
    fclose(fp);
    return emf;
}

/******************************************************************************
     purpose : figure out appropriate x and y scaling
		h        and w        are the actual height and width of the image in twips
        target_h and target_w are the desired height and width of the image in twips
        s                     is the scaling desired as a fraction
 ******************************************************************************/
static void AdjustScaling(double h, double w, double target_h, double target_w, double s, int *sx, int *sy)
{
	diagnostics(5,"AdjustScaling h       =%f w       =%f s=%f", h, w, s);
	diagnostics(5,"AdjustScaling target_h=%f target_w=%f", target_h, target_w);

	if (target_h != 0 && h != 0) 
		*sy = (int) my_rint(100.0 * target_h / h);
	else
		*sy = (int) my_rint(s * 100);
	
	if (target_w == 0 || w == 0)
		*sx = *sy;
	else
		*sx = (int) my_rint(100.0 * target_w / w);

	/* catch the case when width is specified, but not height */
	if (target_h == 0 && target_w != 0)
		*sy = *sx;

	diagnostics(5,"AdjustScaling xscale=%d yscale=%d", *sx, *sy);
}

/******************************************************************************
     purpose : write entire file to RTF as hex
 ******************************************************************************/
static void PutHexFile(FILE * fp)
{
    int i, c;

    i = 0;
    while ((c = fgetc(fp)) != EOF) {
        fprintRTF("%.2x", c);
        if (++i > 126) {        /* keep lines 254 chars long */
            i = 0;
            fprintRTF("\n");
        }
    }
}

/******************************************************************************
     purpose : Include .pict file in RTF
 ******************************************************************************/
static void PutPictFile(char *s, double height0, double width0, double scale, double baseline, int full_path)
{
    FILE *fp;
    char *pict;
    short buffer[5];
    short top, left, bottom, right;
    int width, height;
    int sx,sy;

    if (full_path)
        pict = strdup(s);
    else
        pict = strdup_together(g_home_dir, s);
    diagnostics(2, "PutPictFile '%s'", pict);

    fp = fopen(pict, "rb");
    free(pict);
    if (fp == NULL)
        return;

    if (fseek(fp, 514L, SEEK_SET) || fread(buffer, 2, 4, fp) != 4) {
        diagnostics(WARNING, "Cannot read graphics file '%s'", s);
        fclose(fp);
        return;
    }

    top = buffer[0];
    left = buffer[1];
    bottom = buffer[2];
    right = buffer[3];

    width = right - left;
    height = bottom - top;

    if (g_little_endian) {
        top = LETONS(top);
        bottom = LETONS(bottom);
        left = LETONS(left);
        right = LETONS(right);
    }

    diagnostics(4, "top = %d, bottom = %d", top, bottom);
    diagnostics(4, "left = %d, right = %d", left, right);
    diagnostics(4, "width = %d, height = %d", width, height);
    fprintRTF("\n{\\pict\\macpict\\picw%d\\pich%d\n", width, height);

	AdjustScaling(height*20,width*20,height0,width0,scale,&sx,&sy);
    if (sx != 100 && sy != 100)
        fprintRTF("\\picscalex%d\\picscaley%d", sx,sy);

    fseek(fp, -10L, SEEK_CUR);
    PutHexFile(fp);
    fprintRTF("}\n");
    fclose(fp);
}

/*
typedef struct _PngChunk
{
    DWORD DataLength;   Size of Data field in bytes
    DWORD Type;         Code identifying the type of chunk
    BYTE  Data[];       The actual data stored by the chunk
    DWORD Crc;          CRC-32 value of the Type and Data fields
} PNGCHUNK;

typedef struct _pHYsChunkEntry
{
   DWORD PixelsPerUnitX;    Pixels per unit, X axis 
   DWORD PixelsPerUnitY;    Pixels per unit, Y axis 
   BYTE  UnitSpecifier;     0 = unknown, 1 = meter 
} PHYSCHUNKENTRY;
*/

static unsigned char * getPngChunk(FILE *fp, char *s)
{
	unsigned long size, crc;
	char head[5];
	unsigned char *data;
	
	head[4]='\0';
	
	diagnostics(6, "getPngChunk ... seeking '%s'",s);
	data = NULL;
	do {
		int i;
		if (data!=NULL) free(data);
				
		fread(&size, 4, 1, fp);
		if (g_little_endian) size = LETONL(size);
		
		for (i=0; i<4; i++) {
			head[i] = (char) fgetc(fp);
			if (feof(fp)) return NULL;
		}

		if (strcmp(head,"IEND") == 0) return NULL;
		
		diagnostics(6,"found chunk '%s' size %ld bytes",head,size);
		data = malloc(size);
		if (data == NULL) return NULL;
		
		fread(data, size, 1, fp);
		fread(&crc, 4, 1, fp);
	
		
	} while (strcmp(s,head)!=0);
	
	return data;	
}

/******************************************************************************
     purpose : determine height and width of file
               w is the size in pixels
               xres is the number of pixels per meter
 ******************************************************************************/
static void GetPngSize(char *s, unsigned long *w, unsigned long *h, unsigned long *xres, unsigned long *yres, int *bad_res)
{
    FILE *fp;
    unsigned long *p;
    unsigned char buffer[16];
    char reftag[9] = "\211PNG\r\n\032\n";
    unsigned char *data = NULL;

    *w = 0;
    *h = 0;
    *xres = (unsigned long) POINTS_PER_METER;
    *yres = (unsigned long) POINTS_PER_METER;
    *bad_res = 1;
    
    fp = fopen(s, "rb");
    if (fp == NULL)
        return;

    if (fread(buffer, 1, 8, fp) < 8) {
        diagnostics(WARNING, "Cannot read graphics file '%s'", s);
        fclose(fp);
        return;
    }

    if (memcmp(buffer, reftag, 8) != 0) {
        diagnostics(WARNING, "Graphics file '%s' is not a PNG file!", s);
        fclose(fp);
        return;
    }

	data = getPngChunk(fp,"IHDR");
	if (data == NULL) {
        diagnostics(WARNING, "Graphics file '%s': could not locate IHDR chunk!", s);
        return;
	}

	p = (unsigned long *) data;	
	*w = (g_little_endian) ? LETONL(*p) : *p;
	p++;
	*h = (g_little_endian) ? LETONL(*p) : *p;
	free(data);
	
	data = getPngChunk(fp,"pHYs");
	if (data == NULL) {
        diagnostics(4, "Graphics file '%s': could not locate pHYs chunk!", s);
        return;
	}

	p = (unsigned long *) data;	
	*xres = (g_little_endian) ? LETONL(*p) : *p;
	p++;
	*yres = (g_little_endian) ? LETONL(*p) : *p;
	
	free(data);

	/* dots per inch, not per meter! */
	if (*xres < POINTS_PER_METER) {
		*bad_res = 1;
		diagnostics(6, "bogus resolution in png image! ");
		diagnostics(6, "xres = %ld, yres = %ld, pixels/meter", *xres, *yres);
		diagnostics(6, "xres = %ld, yres = %ld, pixels/in", 
		(unsigned long)( (double)(*xres *72.0)/POINTS_PER_METER), 
		(unsigned long)((double)(*yres * 72.0) /POINTS_PER_METER));
		*xres *= POINTS_PER_METER/72.0;
		*yres *= POINTS_PER_METER/72.0;
	} else 
		*bad_res = 0;
	
	
    diagnostics(6, "xres = %ld, yres = %ld, pixels/meter", *xres, *yres);
    diagnostics(6, "xres = %ld, yres = %ld, pixels/in", 
    (unsigned long)( (double)(*xres *72.0)/POINTS_PER_METER), 
    (unsigned long)((double)(*yres * 72.0) /POINTS_PER_METER));
    
    fclose(fp);
}

/******************************************************************************
     purpose : Include .png file in RTF
     
\pict {...}
\picscalex22\picscaley22
\piccropl0\piccropr0\piccropt0\piccropb0
\picw32032\pich29633\picwgoal18160\pichgoal16800
\pngblip

There are three scaling factors floating around
	convert_scale = the scaling factor from converting the PNG ... 
	encode_scale  = POINTS_PER_METER/xres (scaling factor inside the PNG)
	scale         = the scaling factor given by \includegraphics

Basically, if the PNG was created with a correct value for the resolution
then we leave things as they are.  On the other hand, an incorrect value 
means that the Word will assume the image resolution is 1/72 of an inch.  
Consequently, when this happens, scale must be altered by the convert_scale 
value.  As you can see below, we never use encode_scale.

On entry baseline should be in pixels.
 ******************************************************************************/
void PutPngFile(char *s, double height_goal, double width_goal, double scale, 
                         double convert_scale, double baseline, int full_path)
{
    FILE *fp;
    char *png;
    unsigned long width, height, w, h, b, xres, yres;
	int sx, sy, bad_res;
	
    if (full_path)
        png = strdup(s);
    else
        png = strdup_together(g_home_dir, s);
    diagnostics(2, "PutPngFile '%s'", png);

    GetPngSize(png, &width, &height, &xres, &yres,&bad_res);
    if (width == 0 || height == 0) return;

	/* make sure that we can open the file */
    fp = fopen(png, "rb");
    free(png);
    if (fp == NULL) return;

	/* only modify when the PNG resolution is bad and convert_scale is non-zero */
	if (bad_res && convert_scale != 0) 
		scale *= convert_scale;
		
	/* twips calculation                                             */
	/*                       points        meter       20 twips      */
	/* width =  (pixels) * ----------- * ---------  * -----------    */
	/*                       meter         pixels      1 point       */
	width    *= POINTS_PER_METER / xres * 20.0;
	height   *= POINTS_PER_METER / yres * 20.0;
	
    /* size is in units that equal 1/100 of a millimeter  (10 microns). */    
	/*       dwips       100,000 (units)     1 points        meter      */
	/* w =  -------- * ------------------ * ---------  * -----------    */
	/*         1            1 meter          20 dwips      1 point      */
    w = (unsigned long) (100000.0 * width           ) / (20 * POINTS_PER_METER);
    h = (unsigned long) (100000.0 * height          ) / (20 * POINTS_PER_METER);
    b = (unsigned long) (100000.0 * baseline * scale) / (20 * POINTS_PER_METER); 
    
	AdjustScaling(height,width,height_goal,width_goal,scale,&sx,&sy);
	
	diagnostics(5, "scale      = %6.3f,            convert     = %6.3f", scale, convert_scale);
    diagnostics(5, "width_goal = %6ld twips,      height_goal = %6ld twips", (int)width_goal, (int)height_goal);
    diagnostics(5, "picw       = %6ld microns,    pich        = %6ld microns", w*10, h*10);
    diagnostics(5, "picwgoal   = %6ld twips,      pichgoal    = %6ld twips", width, height);
    diagnostics(5, "sx         = %6ld percent,    sy          = %6ld percent", sx, sy);
    
    /* Write the header for the png bitmap */
    fprintRTF("\n{");
    if (b) fprintRTF("\\dn%ld", b); 
    fprintRTF("\\pict");
    if (sx != 100 && sy != 100) fprintRTF("\\picscalex%d\\picscaley%d", sx,sy);
    fprintRTF("\\picw%ld\\pich%ld", w, h);
    fprintRTF("\\picwgoal%ld\\pichgoal%ld", width, height);
    fprintRTF("\\pngblip\n");

	/* Now actually write the PNG file out */
    rewind(fp);
    PutHexFile(fp);
    fprintRTF("}\n");
    fclose(fp);
}

/******************************************************************************
     purpose : Include .jpeg file in RTF
 ******************************************************************************/
static void PutJpegFile(char *s, double height0, double width0, double scale, double baseline, int full_path)
{
    FILE *fp;
    char *jpg;
    unsigned short buffer[2];
    int m=0, c;
    unsigned short width, height;
    unsigned long w, h;
    int sx, sy;

    jpg = strdup_together(g_home_dir, s);
    diagnostics(2, "PutJpegFile '%s'", jpg);

    fp = fopen(jpg, "rb");
    free(jpg);
    if (fp == NULL)
        return;

    if ((c = fgetc(fp)) != 0xFF && (c = fgetc(fp)) != 0xD8) {
        fclose(fp);
        diagnostics(WARNING, "'%s' is not really a JPEG file --- skipping");
        return;
    }

    do {                        /* Look for SOFn tag */

        while (!feof(fp) && fgetc(fp) != 0xFF) {
        }                       /* Find 0xFF byte */

        while (!feof(fp) && (m = fgetc(fp)) == 0xFF) {
        }                       /* Skip multiple 0xFFs */

    } while (!feof(fp) && m != 0xC0 && m != 0xC1 && m != 0xC2 && m != 0xC3 && m != 0xC5 && m != 0xC6 && m != 0xC7 &&
      m != 0xC9 && m != 0xCA && m != 0xCB && m != 0xCD && m != 0xCE && m != 0xCF);

    if (fseek(fp, 3, SEEK_CUR) || fread(buffer, 2, 2, fp) != 2) {
        diagnostics(WARNING, "Cannot read graphics file '%s'", s);
        fclose(fp);
        return;
    }

    width = buffer[1];
    height = buffer[0];

    if (g_little_endian) {
        width = (unsigned short) LETONS(width);
        height = (unsigned short) LETONS(height);
    }

    diagnostics(4, "width = %d, height = %d", width, height);

    w = (unsigned long) (100000.0 * width) / (20 * POINTS_PER_METER);
    h = (unsigned long) (100000.0 * height) / (20 * POINTS_PER_METER);
    fprintRTF("\n{\\pict\\jpegblip\\picw%ld\\pich%ld", w, h);
    fprintRTF("\\picwgoal%ld\\pichgoal%ld\n", width * 20, height * 20);

	AdjustScaling(height*20,width*20,height0,width0,scale,&sx,&sy);
    if (sx != 100 && sy != 100)
        fprintRTF("\\picscalex%d\\picscaley%d", sx,sy);

    rewind(fp);
    PutHexFile(fp);
    fprintRTF("}\n");
    fclose(fp);
}

static void PutEmfFile(char *s, double height0, double width0, double scale, double baseline, int full_path)
{
    FILE *fp;
    char *emf;
    unsigned long RecordType;   /* Record type (always 0x00000001) */
    unsigned long RecordSize;   /* Size of the record in bytes */
    long BoundsLeft;            /* Left inclusive bounds */
    long BoundsRight;           /* Right inclusive bounds */
    long BoundsTop;             /* Top inclusive bounds */
    long BoundsBottom;          /* Bottom inclusive bounds */
    long FrameLeft;             /* Left side of inclusive picture frame */
    long FrameRight;            /* Right side of inclusive picture frame */
    long FrameTop;              /* Top side of inclusive picture frame */
    long FrameBottom;           /* Bottom side of inclusive picture frame */
    unsigned long Signature;    /* Signature ID (always 0x464D4520) */
    unsigned long w, h, width, height;
	int sx, sy;
	
    if (full_path)
        emf = strdup(s);
    else
        emf = strdup_together(g_home_dir, s);
    diagnostics(2, "PutEmfFile '%s'", emf);
    fp = fopen(emf, "rb");
    free(emf);
    if (fp == NULL)
        return;

/* extract size information*/
    if (fread(&RecordType, 4, 1, fp) != 1)
        goto out;
    if (fread(&RecordSize, 4, 1, fp) != 1)
        goto out;
    if (fread(&BoundsLeft, 4, 1, fp) != 1)
        goto out;
    if (fread(&BoundsTop, 4, 1, fp) != 1)
        goto out;
    if (fread(&BoundsRight, 4, 1, fp) != 1)
        goto out;
    if (fread(&BoundsBottom, 4, 1, fp) != 1)
        goto out;
    if (fread(&FrameLeft, 4, 1, fp) != 1)
        goto out;
    if (fread(&FrameRight, 4, 1, fp) != 1)
        goto out;
    if (fread(&FrameTop, 4, 1, fp) != 1)
        goto out;
    if (fread(&FrameBottom, 4, 1, fp) != 1)
        goto out;
    if (fread(&Signature, 4, 1, fp) != 1)
        goto out;

    if (!g_little_endian) {
        RecordType = LETONL(RecordType);
        RecordSize = LETONL(RecordSize);
        BoundsLeft = LETONL(BoundsLeft);
        BoundsTop = LETONL(BoundsTop);
        BoundsRight = LETONL(BoundsRight);
        BoundsBottom = LETONL(BoundsBottom);
        FrameLeft = LETONL(FrameLeft);
        FrameRight = LETONL(FrameRight);
        FrameTop = LETONL(FrameTop);
        FrameBottom = LETONL(FrameBottom);
        Signature = LETONL(Signature);
    }

    if (RecordType != 1 || Signature != 0x464D4520)
        goto out;
    height = (unsigned long) (BoundsBottom - BoundsTop);
    width = (unsigned long) (BoundsRight - BoundsLeft);

    w = (unsigned long) ((100000.0 * width) / (20 * POINTS_PER_METER));
    h = (unsigned long) ((100000.0 * height) / (20 * POINTS_PER_METER));
    diagnostics(4, "width = %ld, height = %ld", width, height);
    fprintRTF("\n{\\pict\\emfblip\\picw%ld\\pich%ld", w, h);
    fprintRTF("\\picwgoal%ld\\pichgoal%ld\n", width * 20, height * 20);

	AdjustScaling(height*20,width*20,height0,width0,scale,&sx,&sy);
    if (sx != 100 && sy != 100)
        fprintRTF("\\picscalex%d\\picscaley%d", sx,sy);

/* write file */
    rewind(fp);
    PutHexFile(fp);
    fprintRTF("}\n");
    fclose(fp);
    return;

  out:
    diagnostics(WARNING, "Problem with file %s --- not included", s);
    fclose(fp);
}

/******************************************************************************
 purpose   : Insert WMF file (from g_home_dir) into RTF file
 ******************************************************************************/
static void PutWmfFile(char *s, double height0, double width0, double scale, double baseline, int full_path)
{
    FILE *fp;
    char *wmf;
    unsigned long Key;          /* Magic number (always 0x9AC6CDD7) */
    unsigned short FileType;    /* Type of metafile (0=memory, 1=disk) */
    unsigned short HeaderSize;  /* Size of header in WORDS (always 9) */
    unsigned short Handle;      /* Metafile HANDLE number (always 0) */
    short Left;                 /* Left coordinate in twips */
    short Top;                  /* Top coordinate in twips */
    short Right;                /* Right coordinate in twips */
    short Bottom;               /* Bottom coordinate in twips */
    int width, height, sx, sy;
    unsigned long int magic_number = (unsigned long int) 0x9AC6CDD7;

    /* open the proper file */
    wmf = strdup_together(g_home_dir, s);
    diagnostics(2, "PutWmfFile '%s'", wmf);
    fp = fopen(wmf, "rb");
    free(wmf);
    if (fp == NULL)
        return;

    /* verify file is actually WMF and get size */
    if (fread(&Key, 4, 1, fp) != 1)
        goto out;
    if (!g_little_endian)
        Key = LETONL(Key);

    if (Key == magic_number) {  /* file is placeable metafile */
        if (fread(&Handle, 2, 1, fp) != 1)
            goto out;
        if (fread(&Left, 2, 1, fp) != 1)
            goto out;
        if (fread(&Top, 2, 1, fp) != 1)
            goto out;
        if (fread(&Right, 2, 1, fp) != 1)
            goto out;
        if (fread(&Bottom, 2, 1, fp) != 1)
            goto out;

        if (!g_little_endian) {
            Left = LETONS(Left);
            Top = LETONS(Top);
            Right = LETONS(Right);
            Bottom = LETONS(Bottom);
        }

        width = abs(Right - Left);
        height = abs(Top - Bottom);

    } else {                    /* file may be old wmf file with no size */

        rewind(fp);
        if (fread(&FileType, 2, 1, fp) != 1)
            goto out;
        if (fread(&HeaderSize, 2, 1, fp) != 1)
            goto out;

        if (!g_little_endian) {
            FileType = (unsigned short) LETONS(FileType);
            HeaderSize = (unsigned short) LETONS(HeaderSize);
        }

        if (FileType != 0 && FileType != 1)
            goto out;
        if (HeaderSize != 9)
            goto out;

        /* real wmf file ... just assume size */
        width = 200;
        height = 200;
    }

    diagnostics(4, "width = %d, height = %d", width, height);
    fprintRTF("\n{\\pict\\wmetafile1\\picw%d\\pich%d\n", width, height);

	AdjustScaling(height,width,height0,width0,scale,&sx,&sy);
    if (sx != 100 && sy != 100)
        fprintRTF("\\picscalex%d\\picscaley%d", sx,sy);

    rewind(fp);
    PutHexFile(fp);
    fprintRTF("}\n");
    fclose(fp);
    return;

  out:
    diagnostics(WARNING, "Problem with file %s --- not included", s);
    fclose(fp);
}

/******************************************************************************
 purpose   : convert pdf to png and insert in RTF file
 ******************************************************************************/
static void PutPdfFile(char *s, double height0, double width0, double scale, double baseline, int full_path)
{
    char *png;
    double convert_scale = 72.0 / g_dots_per_inch;
    
    diagnostics(WARNING, "Rendering '%s' as PNG", s);

    png = pdf_to_png(s);
        	
    if (png) {
        PutPngFile(png, height0, width0, scale, convert_scale, baseline, TRUE);
        my_unlink(png);
        free(png);
    }
}

/******************************************************************************
 purpose   : convert eps to png and insert in RTF file
 ******************************************************************************/
static void PutEpsFile(char *s, double height0, double width0, double scale, double baseline, int full_path)
{
    char *png, *emf, *pict;
    double convert_scale = 72.0 / g_dots_per_inch;

    diagnostics(WARNING, "Rendering '%s' as PNG", s);

    if (1) {
        png = eps_to_png(s);
        if (png) {
            PutPngFile(png, height0, width0, scale, convert_scale, baseline, TRUE);
            my_unlink(png);
            free(png);
        }
    }

    if (0) {
        pict = eps_to_pict(s);
        if (pict) {
            PutPictFile(pict, height0, width0, scale, baseline, TRUE);
            my_unlink(pict);
            free(pict);
        }
    }

    if (0) {
        emf = eps_to_emf(s);
        if (emf) {
            PutEmfFile(emf, height0, width0, scale, baseline, TRUE);
            my_unlink(emf);
            free(emf);
        }
    }
}

/******************************************************************************
 purpose   : Insert TIFF file (from g_home_dir) into RTF file as a PNG image
 ******************************************************************************/
static void PutTiffFile(char *s, double height0, double width0, double scale, double baseline, int full_path)
{
    char *tiff, *png, *out;
    double convert_scale = 0.0;

    diagnostics(2, "PutTiffFile '%s'", s);
    png = strdup_new_extension(s, ".tiff", ".png");
    if (png == NULL) {
        png = strdup_new_extension(s, ".TIFF", ".png");
        if (png == NULL)
            return;
    }

    tiff = strdup_together(g_home_dir, s);
	out = SysGraphicsConvert(CONVERT_SIMPLE, 0, tiff, png);
	
	if (out != NULL) {
    	PutPngFile(out, height0, width0, scale, convert_scale, baseline, TRUE);
		my_unlink(out);
    	free(out);
    }
    
    free(tiff);
    free(png);
}

/******************************************************************************
 purpose   : Insert GIF file (from g_home_dir) into RTF file as a PNG image
 ******************************************************************************/
static void PutGifFile(char *s, double height0, double width0, double scale, double baseline, int full_path)
{
    char *gif, *png, *out;
	double convert_scale = 0.0;
	
    diagnostics(2, "PutGifFile '%s'", s);
    png = strdup_new_extension(s, ".gif", ".png");
    if (png == NULL) {
        png = strdup_new_extension(s, ".GIF", ".png");
        if (png == NULL)
            return;
    }

    gif = strdup_together(g_home_dir, s);
    out = SysGraphicsConvert(CONVERT_SIMPLE, 0, gif, png);

	if (out != NULL) {
    	PutPngFile(out, height0, width0, scale, convert_scale, baseline, TRUE);
   	 	my_unlink(out);
    	free(out);
    }
    free(gif);
    free(png);
}

/****************************************************************************
purpose: reads up to and and including a line ending (CR, CRLF, or LF)
 ****************************************************************************/
static int ReadLine(FILE * fp)
{
    int thechar;

    while (1) {
        thechar = getc(fp);
        if (thechar == EOF) {
            fclose(fp);
            return 0;
        }
        if (thechar == 0x0a)
            return 1;           /* LF */
        if (thechar == 0x0d) {
            thechar = getc(fp);
            if (thechar == EOF) {
                fclose(fp);
                return 0;
            }
            if (thechar == 0x0d)
                return 1;       /* CR LF */
            ungetc(thechar, fp);    /* CR */
            return 1;
        }
    }
}

/****************************************************************************
purpose: reads a .pbm file to determine the baseline for an equation
		 the .pbm file should have dimensions of 1 x height
		 returns the baseline height in pixels
 ****************************************************************************/
long GetBaseline(char *s, char *pre)
{
    FILE *fp;
    int thechar;
    char *pbm;
    char magic[250];
    long baseline, width, height, items, top, bottom;

    /* baseline=0 if not an inline image */
    if ((strcmp(pre, "$") != 0) && (strcmp(pre, "\\begin{math}") != 0) && (strcmp(pre, "\\(") != 0))
        return 0;

    pbm = strdup_together(s, ".pbm");
    baseline = 4;

    diagnostics(4, "GetBaseline opening='%s'", pbm);

    fp = fopen(pbm, "rb");
    if (fp == NULL) {
        free(pbm);
        return baseline;
    }

    items = fscanf(fp, "%2s", magic);   /* ensure that file begins with "P4" */
    if ((items != 1) || (strcmp(magic, "P4") != 0))
        goto Exit;

    items = fscanf(fp, " %s", magic);
    while ((items == 1) && (magic[0] == '#')) { /* skip any comment lines in pbm file */
        if (!ReadLine(fp))
            goto Exit;
        items = fscanf(fp, "%s", magic);
    }

    items = sscanf(magic, "%ld", &width);   /* make sure image width is 1 */
    if ((items != 1) || (width != 1))
        goto Exit;

    items = fscanf(fp, " %ld", &height);    /* read height */
    if (items != 1)
        goto Exit;

    diagnostics(4, "width=%ld height=%ld", width, height);

    if (!ReadLine(fp))
        goto Exit;              /* pixel map should start on next line */

    for (top = height; top > 0; top--) {    /* seek first black pixel (0x00) */
        thechar = getc(fp);
        if (thechar == EOF)
            goto Exit;
        if (thechar != 0)
            break;
    }

    for (bottom = top - 1; bottom > 0; bottom--) {  /* seek first black pixel (0x00) */
        thechar = getc(fp);
        if (thechar == EOF)
            goto Exit;
        if (thechar == 0)
            break;
    }

    baseline = (bottom + top) / 2;

    diagnostics(4, "top=%ld bottom=%ld baseline=%ld", top, bottom, baseline);

  Exit:
    free(pbm);
    fclose(fp);
    return baseline;
}

/******************************************************************************
 purpose   : Convert LaTeX to Bitmap and insert in RTF file
 ******************************************************************************/
void PutLatexFile(char *latex, double height0, double width0, double scale, char *pre)
{
    char *png=NULL;
    char *pngpath = NULL;
    int baseline, second_pass, bmoffset,bad_res;
    unsigned long width, height, rw, rh, xres, yres;
    unsigned long maxsize = (unsigned long) (32767.0 / 20.0);
	double convert_scale = 72.0 / g_dots_per_inch;
	double resolution = g_dots_per_inch;
	
    diagnostics(2, "Rendering LaTeX construct (e.g. equation) as a bitmap...");

    bmoffset = g_dots_per_inch / 72 + 2;
    png = strdup_together(latex, ".png");

    do {
        second_pass = FALSE;    /* only needed if png is too large for Word */

    	pngpath = SysGraphicsConvert(CONVERT_LATEX, bmoffset, latex, "");
    	if (pngpath == NULL) break;

        GetPngSize(png, &width, &height,&xres,&yres,&bad_res);
        baseline = GetBaseline(latex, pre);
        diagnostics(4, "png='%s' size height=%d baseline=%d width=%d",png, height, baseline, width);

        if ((width > maxsize && height != 0) || (height > maxsize && width != 0)) {
            second_pass = TRUE;
            rw = (unsigned long) ((resolution * maxsize) / width);
            rh = (unsigned long) ((resolution * maxsize) / height);
            resolution = rw < rh ? (int) rw : (int) rh;
        }
    } while (resolution > 10 && ((width > maxsize) || (height > maxsize)));

    diagnostics(2, "calling PutPngFile %s, path=%s", png, pngpath);
        
    if (pngpath != NULL) {
        PutPngFile(png, height0, width0, scale, convert_scale, (double) baseline, TRUE);
        free(pngpath);
    }

    free(png);
}

static char *SaveEquationAsFile(const char *post_begin_document,
                                const char *pre, const char *eq_with_spaces, const char *post)
{
    FILE *f;
    char name[15];
    char *tmp_dir, *fullname, *texname, *eq;
    static int file_number = 0;

    if (!pre || !eq_with_spaces || !post)
        return NULL;

    eq = strdup_noendblanks(eq_with_spaces);

/* create needed file names */
    file_number++;
    tmp_dir = getTmpPath();
    snprintf(name, 15, "l2r_%04d", file_number);
    fullname = strdup_together(tmp_dir, name);
    texname = strdup_together(fullname, ".tex");

    diagnostics(4, "SaveEquationAsFile =%s", texname);

    f = fopen(texname, "w");
    while (eq && (*eq == '\n' || *eq == ' '))
        eq++;                   /* skip whitespace */
    if (f) {
        fprintf(f, "%s", g_preamble);
        fprintf(f, "\\thispagestyle{empty}\n");
        fprintf(f, "\\begin{document}\n");
        if (post_begin_document) fprintf(f, "%s\n", post_begin_document);
        fprintf(f, "\\setcounter{equation}{%d}\n", getCounter("equation"));
        if ((strcmp(pre, "$") == 0) || (strcmp(pre, "\\begin{math}") == 0) || (strcmp(pre, "\\(") == 0)) {
            fprintf(f, "%%INLINE_DOT_ON_BASELINE\n");
            fprintf(f, "%s\n.\\quad %s\n%s", pre, eq, post);
        } else if (strstr(pre, "equation"))
            fprintf(f, "$$%s$$", eq);
        else
            fprintf(f, "%s\n%s\n%s", pre, eq, post);
        fprintf(f, "\n\\end{document}");
        fclose(f);
    } else {
        free(fullname);
        fullname = NULL;
    }

    free(eq);
    free(tmp_dir);
    free(texname);
    return (fullname);
}

void PrepareDisplayedBitmap(char *the_type)
/******************************************************************************
 purpose   : Call before WriteLatexAsBitmap()
 ******************************************************************************/
{
    CmdEndParagraph(0);
    CmdVspace(VSPACE_SMALL_SKIP);
    CmdIndent(INDENT_NONE);
    startParagraph(the_type, FIRST_PARAGRAPH);
}

void FinishDisplayedBitmap(void)
/******************************************************************************
 purpose   : Call after WriteLatexAsBitmap()
 ******************************************************************************/
{
    CmdEndParagraph(0);
    CmdVspace(VSPACE_SMALL_SKIP);
    CmdIndent(INDENT_INHIBIT);
}

static char * abbreviate(const char *s, int len)
{
	char *t;
	int i,n,half;
	
	if (s==NULL) return strdup("<NULL>");
	
	t = strdup(s);
	n = strlen(t);
	if (n > len) {	
		half = (len-5)/2;
		t[half+0] = ' ';
		t[half+1] = '.';
		t[half+2] = '.';
		t[half+3] = '.';
		t[half+4] = ' ';
		for (i=0; i<half; i++)
			t[half+5+i] = s[n-half+i];
		t[len-1] = '\0';
	}
	
	for (i=0; i<len; i++)
		if (t[i] == '\n') t[i] = ' ';
		
	return t;
	
}

void WriteLatexAsBitmap(char *pre, char *eq, char *post)

/******************************************************************************
 purpose   : Convert LaTeX to Bitmap and write to RTF file
 ******************************************************************************/
{
    char *p, *name, *abbrev;
    double scale;

	/* go to a bit a trouble to give the user some feedback */
	name = strdup_together3(pre,eq,post);
	abbrev = abbreviate(name, 50);
    diagnostics(WARNING, "rendering PNG for '%s'", abbrev);
	free(abbrev);
	free(name);
	
    if (eq == NULL)
        return;

    scale = g_png_equation_scale;
    if (strstr(pre, "music") || strstr(pre, "figure") 
                             || strstr(pre, "picture")
                             || strstr(pre, "longtable")
                             || strstr(pre, "psgraph")
                             || strstr(pre, "pspicture"))
        scale = g_png_figure_scale;

/* suppress bitmap equation numbers in eqnarrays with zero or one \label{}'s*/
    if (strcmp(pre, "\\begin{eqnarray}") == 0) {
    
        p = strstr(eq, "\\label");
        if (p != NULL && strlen(p) > 6) /* found one ... is there a second? */
            p = strstr(p + 6, "\\label");
        if (p == NULL)
            name = SaveEquationAsFile(NULL, "\\begin{eqnarray*}", eq, "\\end{eqnarray*}");
        else
            name = SaveEquationAsFile(NULL, pre, eq, post);

    } else if (strcmp(pre, "\\begin{align}") == 0) {
    
        p = strstr(eq, "\\label");
        if (p != NULL && strlen(p) > 6) /* found one ... is there a second? */
            p = strstr(p + 6, "\\label");
        if (p == NULL)
            name = SaveEquationAsFile(NULL, "\\begin{align*}", eq, "\\end{align*}");
        else
            name = SaveEquationAsFile(NULL, pre, eq, post);
            
    } else if (strstr(pre, "psgraph") != NULL || strstr(pre, "pspicture") != NULL ){
    	char *s = strdup_together(g_psset_info, g_psstyle_info);
        name = SaveEquationAsFile(s, pre, eq, post);
        if (s) free(s);
    } else
        name = SaveEquationAsFile(NULL, pre, eq, post);

    PutLatexFile(name, 0, 0, scale, pre);
}

char *upper_case_string(char *s)
{
    char *t, *x;

    if (!s)
        return NULL;

    t = strdup(s);
    x = t;

    while (*x) {
        if (islower(*x))
            *x = toupper(*x);
        x++;
    }

    return t;
}

/******************************************************************************
 purpose   : return s.ext or s.EXT if it exists otherwise return NULL
 ******************************************************************************/
char *exists_with_extension(char *s, char *ext)
{
    char *t, *x;
    FILE *fp;

    t = strdup_together(s, ext);
    fp = fopen(t, "rb");
    diagnostics(4, "trying to open %s, result = %0x", t, fp);
    if (fp) {
        fclose(fp);
        return t;
    }
    free(t);

/* now try upper case version of ext */
    x = upper_case_string(ext);
    t = strdup_together(s, x);
    free(x);

    fp = fopen(t, "rb");
    diagnostics(4, "trying to open %s, result = %0x", t, fp);
    if (fp) {
        fclose(fp);
        return t;
    }
    free(t);
    return NULL;
}

/******************************************************************************
 purpose   : return true if ext is at end of s (case insensitively)
 ******************************************************************************/
int has_extension(char *s, char *ext)
{
    char *t;

    t = s + strlen(s) - strlen(ext);

    if (strcasecmp(t, ext) == 0)
        return TRUE;

    return FALSE;
}

char *append_graphic_extension(char *s)
{
    char *t;

    if (has_extension(s, ".pict") ||
      has_extension(s, ".png") ||
      has_extension(s, ".gif") ||
      has_extension(s, ".emf") ||
      has_extension(s, ".wmf") ||
      has_extension(s, ".eps") ||
      has_extension(s, ".pdf") ||
      has_extension(s, ".ps") ||
      has_extension(s, ".tiff") || has_extension(s, ".tif") || has_extension(s, ".jpg") || has_extension(s, ".jpeg"))
        return strdup(s);

    t = exists_with_extension(s, ".png");
    if (t)
        return t;

    t = exists_with_extension(s, ".jpg");
    if (t)
        return t;

    t = exists_with_extension(s, ".jpeg");
    if (t)
        return t;

    t = exists_with_extension(s, ".tif");
    if (t)
        return t;

    t = exists_with_extension(s, ".tiff");
    if (t)
        return t;

    t = exists_with_extension(s, ".gif");
    if (t)
        return t;

    t = exists_with_extension(s, ".eps");
    if (t)
        return t;

    t = exists_with_extension(s, ".pdf");
    if (t)
        return t;

    t = exists_with_extension(s, ".ps");
    if (t)
        return t;

    t = exists_with_extension(s, ".pict");
    if (t)
        return t;

    t = exists_with_extension(s, ".emf");
    if (t)
        return t;

    t = exists_with_extension(s, ".wmf");
    if (t)
        return t;

    /* failed to find any file */
    return strdup(s);

}

/******************************************************************************
  purpose: handle Includegraphics options 

		bb=llx lly urx ury (bounding box),
		width=h_length,
		height=v_length,
		angle=angle,
		scale=factor,
		clip=true/false,
		draft=true/false.
 ******************************************************************************/
static void HandleGraphicsOptions(char *opt, char *opt2, double *h, double *w, double *s)
{
	char *key, *value;
	double llx=0;
	double lly=0;
	double urx=0;
	double ury=0;

	*s=1.0;
	*h=0;
	*w=0;

	diagnostics(4,"HandleGraphicsOptions <%s> <%s>",opt,opt2);
			
/*  \includegraphics[llx,lly][urx,ury]{filename} */
	if (opt && opt2) {
		
		opt = keyvalue_pair(opt,&key,&value);
		if (!key) return;
		llx=getStringDimension(key);
		free(key);
		
		opt = keyvalue_pair(opt,&key,&value);
		if (!key) return;
		lly=getStringDimension(key);
		free(key);

		opt2 = keyvalue_pair(opt2,&key,&value);
		if (!key) return;
		urx=getStringDimension(key);
		free(key);
		
		opt2 = keyvalue_pair(opt2,&key,&value);
		if (!key) return;
		ury=getStringDimension(key);
		free(key);

		*h = ury-lly;
		*w = urx-llx;
		return;
	} 
		

/*  \includegraphics[width,height]{filename} for graphics */

	if (g_graphics_package == GRAPHICS_GRAPHICS && opt) {

		opt = keyvalue_pair(opt,&key,&value);
		if (!key) return;
		*w=getStringDimension(key);
		free(key);
		
		opt = keyvalue_pair(opt,&key,&value);
		if (!key) return;
		*h=getStringDimension(key);
		free(key);

		return;
	}

/*  \includegraphics[key=value,key1=value1]{filename} for graphicx */

	while (opt) {		
		opt = keyvalue_pair(opt,&key,&value);
		
		if (key) {
			diagnostics(5,"graphicx key=%s, value=%s", key, value);

			if (strstr(key,"height"))
				*h=getStringDimension(value);
	
			else if (strstr(key,"natheight")) 
				*h=getStringDimension(value);
	
			else if (strstr(key,"totalheight"))
				*h=getStringDimension(value);
	
			else if (strstr(key,"width")) 
				*w=getStringDimension(value);
	
			else if (strstr(key,"natwidth")) 
				*w=getStringDimension(value);
			
			else if (strstr(key,"bbllx")) 
				llx=getStringDimension(value);
	
			else if (strstr(key,"bblly"))
				lly=getStringDimension(value);
	
			else if (strstr(key,"bburx"))
				urx=getStringDimension(value);
	
			else if (strstr(key,"bbury"))
				ury=getStringDimension(value);
			
			else if (strstr(key,"bb")) {
				llx=getStringDimension(value);
				lly=getStringDimension(value);
				urx=getStringDimension(value);
				ury=getStringDimension(value);
				
			} else if (strstr(key,"scale")) {
				sscanf(value, "%lf", s);   /* just a float, not a dimension */
			}
			
			free(key);
		}
		
		if (value) free(value);	
	}

    diagnostics(5, "image scale  = %lf", *s);
    diagnostics(5, "image height = %lf", *h);
    diagnostics(5, "image width  = %lf", *w);
	
	if (urx)
		*w=urx-llx;
	if (ury)
		*h=ury-lly;
}

/******************************************************************************
  purpose: handle psfig options \psfig{figure=filename.ps,height=1in,width=3mm}
 ******************************************************************************/
static void HandlePsfigOptions(char *opt, char **filename, double *h, double *w, double *s)
{
	*s=1.0;
	*h=0.0;
	*w=0.0;
	*filename = NULL;
	
	diagnostics(4,"HandlePsfigOptions <%s>",opt);
		
	while (opt) {
		char *key, *value;
		
		opt = keyvalue_pair(opt,&key,&value);
		
		if (key) {
			diagnostics(5,"psfig key=%s, value=%s", key, value);
			if (strstr(key,"figure"))
				*filename=strdup(value);
	
			else if (strstr(key,"height")) {
				*h = getStringDimension(value);
			}
	
			else if (strstr(key,"width")) {
				*w = getStringDimension(value);
			}
			
			free(key);
		}
		
		if (value) free(value);	
	}
}


/******************************************************************************
  purpose: handle various methods for importing graphics
           usually by converting to png image and inserting
			   \includegraphics*[0,0][5,5]{file.pict}
			   \epsffile{filename.eps}
			   \epsfbox[0 0 30 50]{filename.ps}
			   \BoxedEPSF{filename [scaled nnn]}
			   \psfig{figure=filename,height=hhh,width=www}
 ******************************************************************************/
void CmdGraphics(int code)
{
    char *options, *options2;
    char *filename=NULL, *fullpathname, *fullname;
    double scale = 1.0;
    double baseline = 0.0;
    double height =0;
    double width = 0;

    if (code == FIGURE_INCLUDEGRAPHICS) {
        options = getBracketParam();
        options2 = getBracketParam();
        filename = getBraceParam();

        HandleGraphicsOptions(options,options2,&height,&width,&scale);
        
        if (options) free(options);
        if (options2)free(options2);
    }

    if (code == FIGURE_EPSFFILE) {
        filename = getBraceParam();
    }

    if (code == FIGURE_EPSFBOX) { 
        int n,llx,lly,urx,ury;
        options = getBracketParam();
        if (options) {
        	n = sscanf(options,"%d %d %d %d", &llx,&lly,&urx,&ury);
        	if (n==4) {
        		width  = (urx - llx) * 20;
        		height = (ury - lly) * 20;
        	}
        	free(options);
        }
        filename = getBraceParam();
    }

    if (code == FIGURE_BOXEDEPSF) { 
        char *s;

        filename = getBraceParam();
        s = strchr(filename, ' ');
        if (s)
            *s = '\0';
    }

    if (code == FIGURE_PSFIG) { 
        options = getBraceParam();
        HandlePsfigOptions(options,&filename,&height,&width,&scale);
        diagnostics(4,"figure=%s, height=%d, width=%d, scale=%d",filename, height, width, scale);
        free(options);
    }

    if (filename) {
		changeTexMode(MODE_HORIZONTAL);
	
		fullname = strdup_absolute_path(filename);
		fullpathname = append_graphic_extension(fullname);
		free(fullname);
		
		if (has_extension(fullpathname, ".pict"))
			PutPictFile(fullpathname, height, width, scale, baseline, TRUE);
	
		else if (has_extension(fullpathname, ".png"))
			PutPngFile(fullpathname, height, width, scale, 0.0, baseline, TRUE);
	
		else if (has_extension(fullpathname, ".gif"))
			PutGifFile(fullpathname, height, width, scale, baseline, TRUE);
	
		else if (has_extension(fullpathname, ".emf"))
			PutEmfFile(fullpathname, height, width, scale, baseline, TRUE);
	
		else if (has_extension(fullpathname, ".wmf"))
			PutWmfFile(fullpathname, height, width, scale, baseline, TRUE);
	
		else if (has_extension(fullpathname, ".eps"))
			PutEpsFile(fullpathname, height, width, scale, baseline, TRUE);
	
		else if (has_extension(fullpathname, ".pdf"))
			PutPdfFile(fullpathname, height, width, scale, baseline, TRUE);
	
		else if (has_extension(fullpathname, ".ps"))
			PutEpsFile(fullpathname, height, width, scale, baseline, TRUE);
	
		else if (has_extension(fullpathname, ".tiff"))
			PutTiffFile(fullpathname, height, width, scale, baseline, TRUE);
	
		else if (has_extension(fullpathname, ".tif"))
			PutTiffFile(fullpathname, height, width, scale, baseline, TRUE);
	
		else if (has_extension(fullpathname, ".jpg"))
			PutJpegFile(fullpathname, height, width, scale, baseline, TRUE);
	
		else if (has_extension(fullpathname, ".jpeg"))
			PutJpegFile(fullpathname, height, width, scale, baseline, TRUE);
	
		else
			diagnostics(WARNING, "Conversion of \"%s\" not supported", filename);
	
		free(filename);
		free(fullpathname);
    }
}

/******************************************************************************
  purpose: handle \begin{pspicture} ... \end{pspicture}
           by converting to png image and inserting
 ******************************************************************************/
void CmdPsPicture(int code)
{
    char *contents;
    char post[] = "\\end{pspicture}";

    if (!(code & ON)) {
        diagnostics(4, "exiting CmdPsPicture");
        return;
    } else 
        diagnostics(4, "entering CmdPsPicture");
    

	contents = getTexUntil(post, 0);

	PrepareDisplayedBitmap("PS picture");
	WriteLatexAsBitmap("\\begin{pspicture}", contents, post);
	FinishDisplayedBitmap();

	ConvertString(post);    /* to balance the \begin{picture} */
	free(contents);
    
}

/******************************************************************************
  purpose: handle \begin{psgraph} ... \end{psgraph}
           by converting to png image and inserting
 ******************************************************************************/
void CmdPsGraph(int code)
{
    char *contents;
    char post[] = "\\end{psgraph}";

    if (!(code & ON)) {
        diagnostics(4, "exiting CmdPsGraph");
        return;
    } else 
        diagnostics(4, "entering CmdPsGraph");

	contents = getTexUntil(post, 0);

	PrepareDisplayedBitmap("PS Graph");
	WriteLatexAsBitmap("\\begin{psgraph}", contents, post);
	FinishDisplayedBitmap();

	ConvertString(post);    /* to balance the \begin{graph} */
	free(contents);
    
}

/******************************************************************************
  purpose: handle \psset{info}
           by converting saving to a local global
 ******************************************************************************/
void CmdPsset(int code)
{
    char *contents = getBraceParam();
	if (g_psset_info) free(g_psset_info);
	
	g_psset_info = strdup_together3("\\psset{", contents, "}");
	free(contents);
}

/******************************************************************************
  purpose: handle \psnewstyle{info}{moreinfo}
           by appending to g_psstyle_info
 ******************************************************************************/
void CmdNewPsStyle(int code)
{
	char *a, *b, *c;
	
    a = getBraceParam();
    b = getBraceParam();
    c = strdup_together4(g_psstyle_info,"\\newpsstyle{",a,"}{");
	if (g_psstyle_info) free(g_psstyle_info);
    g_psstyle_info = strdup_together3(c,b,"} ");
	
	free(a);
	free(b);
	free(c);
}


/******************************************************************************
  purpose: handle \begin{picture} ... \end{picture}
           by converting to png image and inserting
 ******************************************************************************/
void CmdPicture(int code)
{
    char *picture;
    char post[] = "\\end{picture}";

    if (!(code & ON)) {
        diagnostics(4, "exiting CmdPicture");
        return;
    } else 
        diagnostics(4, "entering CmdPicture");

	picture = getTexUntil(post, 0);

	PrepareDisplayedBitmap("latex picture");
	WriteLatexAsBitmap("\\begin{picture}", picture, post);
	FinishDisplayedBitmap();

	ConvertString(post);    /* to balance the \begin{picture} */
	free(picture);
    
}

/******************************************************************************
  purpose: Process \begin{music} ... \end{music} environment
 ******************************************************************************/
void CmdMusic(int code)
{
    char *contents;
    char endmusic[] = "\\end{music}";

    if (!(code & ON)) {
        diagnostics(4, "exiting CmdMusic");
        return;
    } else 
        diagnostics(4, "entering CmdMusic");

    diagnostics(4, "entering CmdMusic");
    contents = getTexUntil(endmusic, TRUE);

	PrepareDisplayedBitmap("music");
    WriteLatexAsBitmap("\\begin{music}", contents, endmusic);
	FinishDisplayedBitmap();

    ConvertString(endmusic);		 /* to balance the \begin{music} */
    free(contents);
}
