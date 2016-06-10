/* graphics.c - routines that handle LaTeX graphics commands

Copyright (C) 2001-2010 The Free Software Foundation

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

This file is available from http://sourceforge.net/projects/latex2rtf/
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include "main.h"
#ifdef UNIX
#include <unistd.h>
#endif
#include "cfg.h"
#include "graphics.h"
#include "parser.h"
#include "utils.h"
#include "commands.h"
#include "convert.h"
#include "funct1.h"
#include "preamble.h"
#include "counters.h"
#include "vertical.h"
#include "fields.h"

/* number of points (72/inch) in a meter */
#define POINTS_PER_METER 2834.65

/* Little endian macros to convert to and from host format to network byte ordering */
#define LETONS(A) ((((A) & 0xFF00) >> 8) | (((A) & 0x00FF) << 8))
#define LETONL(A) ((((A) & 0xFF000000) >> 24) | (((A) & 0x00FF0000) >>  8) | \
                  (((A) & 0x0000FF00) <<  8) | (((A) & 0x000000FF) << 24) )

/*
Version 1.6 RTF files can include pictures as follows

<pict>                  '{' \pict (<brdr>? & <shading>? & <picttype> & <pictsize> & <metafileinfo>?) <data> '}'
<picttype>              \emfblip | \pngblip | \jpegblip | \macpict | \pmmetafile | \wmetafile 
                                         | \dibitmap <bitmapinfo> | \wbitmap <bitmapinfo>
<bitmapinfo>    \wbmbitspixel & \wbmplanes & \wbmwidthbytes
<pictsize>              (\picw & \pich) \picwgoal? & \pichgoal? \picscalex? & \picscaley? & \picscaled? & \piccropt? & \piccropb? & \piccropr? & \piccropl?
<metafileinfo>  \picbmp & \picbpp
<data>                  (\bin #BDATA) | #SDATA

\emfblip                                Source of the picture is an EMF (enhanced metafile).
\pngblip                                Source of the picture is a PNG.
\jpegblip                               Source of the picture is a JPEG.
\shppict                                Specifies a Word 97-2000 picture. This is a destination control word.
\nonshppict                     Specifies that Word 97-2000 has written a {\pict destination that it 
                                                will not read on input. This keyword is for compatibility with other readers.
\macpict                Source of the picture is PICT file (Quickdraw)
\pmmetafileN            Source of the picture is an OS/2 metafile
\wmetafileN             Source of the picture is a Windows metafile
\dibitmapN              Source of the picture is a Windows device-independent bitmap
\wbitmapN               Source of the picture is a Windows device-dependent bitmap
*/

typedef struct _WindowsMetaHeader {
    uint16_t FileType;    /* Type of metafile (0=memory, 1=disk) */
    uint16_t HeaderSize;  /* Size of header in WORDS (always 9) */
    uint16_t Version;     /* Version of Microsoft Windows used */
    uint32_t FileSize;     /* Total size of the metafile in WORDs */
    uint16_t NumOfObjects;    /* Number of objects in the file */
    uint32_t MaxRecordSize;    /* The size of largest record in WORDs */
    uint16_t NumOfParams; /* Not Used (always 0) */
} WMFHEAD;

typedef struct _PlaceableMetaHeader {
    uint32_t Key;          /* Magic number (always 0x9AC6CDD7) */
    uint16_t Handle;      /* Metafile HANDLE number (always 0) */
    int16_t Left;                 /* Left coordinate in twips */
    int16_t Top;                  /* Top coordinate in twips */
    int16_t Right;                /* Right coordinate in twips */
    int16_t Bottom;               /* Bottom coordinate in twips */
    uint16_t Inch;        /* Scaling factor, 1440 => 1:1, 360 => 4:1, 2880 => 1:2 (half size) */
    uint32_t Reserved;     /* Reserved (always 0) */
    uint16_t Checksum;    /* Checksum value for previous 10 WORDs */
} PLACEABLEMETAHEADER;

typedef struct _EnhancedMetaHeader {
    uint32_t RecordType;   /* Record type (always 0x00000001) */
    uint32_t RecordSize;   /* Size of the record in bytes */
    int32_t BoundsLeft;            /* Left inclusive bounds */
    int32_t BoundsTop;             /* Top inclusive bounds */
    int32_t BoundsRight;           /* Right inclusive bounds */
    int32_t BoundsBottom;          /* Bottom inclusive bounds */
    int32_t FrameLeft;             /* Left side of inclusive picture frame */
    int32_t FrameTop;              /* Top side of inclusive picture frame */
    int32_t FrameRight;            /* Right side of inclusive picture frame */
    int32_t FrameBottom;           /* Bottom side of inclusive picture frame */
    uint32_t Signature;    /* Signature ID (always 0x464D4520) */
    uint32_t Version;      /* Version of the metafile */
    uint32_t Size;         /* Size of the metafile in bytes */
    uint32_t NumOfRecords; /* Number of records in the metafile */
    uint16_t NumOfHandles;    /* Number of handles in the handle table */
    uint16_t Reserved;    /* Not used (always 0) */
    uint32_t SizeOfDescrip;    /* Size of description string in WORDs */
    uint32_t OffsOfDescrip;    /* Offset of description string in metafile */
    uint32_t NumPalEntries;    /* Number of color palette entries */
    int32_t WidthDevPixels;        /* Width of reference device in pixels */
    int32_t HeightDevPixels;       /* Height of reference device in pixels */
    int32_t WidthDevMM;            /* Width of reference device in millimeters */
    int32_t HeightDevMM;           /* Height of reference device in millimeters */
} ENHANCEDMETAHEADER;

typedef struct _EmrFormat {
    uint32_t Signature;    /* 0x46535045 for EPS, 0x464D4520 for EMF */
    uint32_t Version;      /* EPS version number or 0x00000001 for EMF */
    uint32_t Data;         /* Size of data in bytes */
    uint32_t OffsetToData; /* Offset to data */
} EMRFORMAT;

typedef struct _GdiCommentMultiFormats {
    uint32_t Identifier;   /* Comment ID (0x43494447) */
    uint32_t Comment;      /* Multiformats ID (0x40000004) */
    int32_t BoundsLeft;            /* Left side of bounding rectangle */
    int32_t BoundsRight;           /* Right side of bounding rectangle */
    int32_t BoundsTop;             /* Top side of bounding rectangle */
    int32_t BoundsBottom;          /* Bottom side of bounding rectangle */
    uint32_t NumFormats;   /* Number of formats in comment */
    EMRFORMAT *Data;            /* Array of comment data */
} GDICOMMENTMULTIFORMATS;

/**********************************
 table for graphics format handling
 **********************************/

typedef void PutFileFnc(char *, double, double, double, double);

typedef struct {
    char       *extension;
    PutFileFnc *encoder;
} GraphConvertElement;

static void PutPictFile(char *, double, double, double, double);
static void PutPngFile (char *, double, double, double, double);
static void PutJpegFile(char *, double, double, double, double);
static void PutEmfFile (char *, double, double, double, double);
static void PutWmfFile (char *, double, double, double, double);
static void PutPdfFile (char *, double, double, double, double);
static void PutEpsFile (char *, double, double, double, double);
static void PutPsFile  (char *, double, double, double, double);
static void PutTiffFile(char *, double, double, double, double);
static void PutGifFile (char *, double, double, double, double);

GraphConvertElement GraphConvertTable[] = {
    { ".png",  PutPngFile  },
    { ".pdf",  PutPdfFile  },
    { ".jpg",  PutJpegFile },
    { ".jpeg", PutJpegFile },
    { ".gif",  PutGifFile  },
    { ".eps",  PutEpsFile  },
    { ".tiff", PutTiffFile },
    { ".tif",  PutTiffFile },
    { ".ps",   PutPsFile   },
    { ".pict", PutPictFile },
    { ".emf",  PutEmfFile  },
    { ".wmf",  PutWmfFile  },
    { NULL, NULL }
};

/********************************************************************************
    purpose: implement \graphicspath{{dir1}{dir2}{...}}
 ********************************************************************************/
static char **graphicsPath = NULL;
static int nGraphicsPathElems = 0;

static void appendGraphicsPath (char *newPath)
{
    int i;
    void *ptr;
    char *add;
    
    for (i = 0; i < nGraphicsPathElems; i++)
    {
        if (streq (graphicsPath[i], newPath)) {
            diagnostics(WARNING,"Repeated graphics path element {%s}",newPath);
            return;
        }
    }
    ptr = (void *) graphicsPath;
    ptr = realloc (ptr, sizeof (char *) * (nGraphicsPathElems + 1));
    graphicsPath = (char **) ptr;
    
    /* path must end with a '/' */
    if (*(newPath+strlen(newPath)-1) == '/')
    	add = strdup(newPath);
    else
    	add = strdup_together(newPath,"/");
    graphicsPath[nGraphicsPathElems++] = add;
    diagnostics (WARNING, "Included %s in graphics search path", add);
}

void CmdGraphicsPath(int code)
{
    char *directories = getBraceParam();
    if (directories != NULL) {
        char *candidate=strtok(directories,"{}");
        while (NULL != candidate) {
            appendGraphicsPath(candidate);
            candidate = strtok(NULL,"{}");
        }
    }
    safe_free(directories);
}

#define CONVERT_SIMPLE        1
#define CONVERT_CROP          2
#define CONVERT_LATEX_TO_PNG  3
#define CONVERT_LATEX_TO_EPS  4
#define CONVERT_PDF           5
#define CONVERT_PS_TO_EPS     6

static char *g_psset_info   = NULL;
static char *g_psstyle_info = NULL;

static char *tikzlibs[32];
static int tikzlibsnum = 0;

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
static char *strdup_tmp_path(const char *s)
{
    char *tmp, *p, *fullname;

    if (s == NULL)
        return NULL;
        
    tmp = getTmpPath();

/* Even in Windows, LaTeX uses the forward slash as path separator */
    p = strrchr(s, '/');

    if (!p)
        fullname = strdup_together(tmp, s);
    else
        fullname = strdup_together(tmp, p + 1);

    safe_free(tmp);

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
        
        opt             type of conversion
        offset  vertical offset
        in              input filename
        out             output filename
        
 ******************************************************************************/

static char *SysGraphicsConvert(int opt, int offset, uint16_t dpi, const char *in, const char *out)

{
    char cmd[512], *out_tmp;
    int err;

    int N = 511;        

    diagnostics(3, "SysGraphicsConvert '%s' to '%s'", in, out);

    out_tmp = strdup_tmp_path(out);

    if (in == NULL || out == NULL || out_tmp == NULL)
        return NULL;

#ifdef UNIX

    if (strchr(in, (int) '\'')) {
        diagnostics(WARNING, "single quote found in filename '%s'.  skipping conversion", in);
        free(out_tmp);
        return NULL;
    }

    if (out_tmp && strchr(out_tmp, (int) '\'')) {
        diagnostics(WARNING, "single quote found in filename '%s'.  skipping conversion", out_tmp);
        free(out_tmp);
        return NULL;
    }

    if (opt == CONVERT_SIMPLE) {
        const char format_simple[] = "convert '%s' '%s'";
        snprintf(cmd, N, format_simple, in, out_tmp);
    }

    if (opt == CONVERT_CROP) {
        const char format_crop[]   = "convert -trim +repage -units PixelsPerInch -density %d '%s' '%s'";
        snprintf(cmd, N, format_crop, dpi, in, out_tmp);
    }

    if (opt == CONVERT_LATEX_TO_PNG) {
        if (g_home_dir == NULL) {
            const char format_unix[] = "%slatex2png -d %d -o %d '%s'";
            if (g_script_dir)
                snprintf(cmd, N, format_unix, g_script_dir, dpi, offset, in);
            else
                snprintf(cmd, N, format_unix, "", dpi, offset, in);
        } else {
            const char format_unix[] = "%slatex2png -k -d %d -o %d -H '%s' '%s'";
            if (g_script_dir)
                snprintf(cmd, N, format_unix, g_script_dir, dpi, offset, g_home_dir, in);
            else
                snprintf(cmd, N, format_unix, "", dpi, offset, g_home_dir, in);
        }
    }

    if (opt == CONVERT_LATEX_TO_EPS) {
        if (g_home_dir == NULL) {
            const char format_unix[] = "%slatex2png -e '%s'";
            if (g_script_dir)
                snprintf(cmd, N, format_unix, g_script_dir, in);
            else
                snprintf(cmd, N, format_unix, "", in);
        } else {
            const char format_unix[] = "%slatex2png -e -H '%s' '%s'";
            if (g_script_dir)
                snprintf(cmd, N, format_unix, g_script_dir, g_home_dir, in);
            else
                snprintf(cmd, N, format_unix, "", g_home_dir, in);
        }
    }
    
    if (opt == CONVERT_PDF) {
        const char format_unix[] = "gs -q -dNOPAUSE -dSAFER -dBATCH -sDEVICE=pngalpha -r%d -sOutputFile='%s' '%s'";
        snprintf(cmd, N, format_unix, dpi, out_tmp, in);
    }

    if (opt == CONVERT_PS_TO_EPS) {
        const char format_unix[] = "eps2eps '%s' '%s'";
        snprintf(cmd, N, format_unix, in, out_tmp);
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

    if (opt == CONVERT_LATEX_TO_PNG) {
        if (g_home_dir == NULL){
            char format_xp[] = "bash latex2png -d %d -o %d \"%s\"";
            snprintf(cmd, N, format_xp, dpi, offset, in);
        } else {
            char format_xp[] = "bash latex2png -d %d -o %d -H \"%s\" \"%s\"";
            snprintf(cmd, N, format_xp, dpi, offset, g_home_dir, in);
        }
    }

    if (opt == CONVERT_LATEX_TO_EPS) {
        if (g_home_dir == NULL){
            char format_xp[] = "bash latex2png -e \"%s\"";
            snprintf(cmd, N, format_xp, in);
        } else {
            char format_xp[] = "bash latex2png -e -H \"%s\" \"%s\"";
            snprintf(cmd, N, format_xp, g_home_dir, in);
        }
    }
    
    if (opt == CONVERT_PDF) {
        char format_xp[] = "bash pdf2pnga \"%s\" \"%s\" %d";
        snprintf(cmd, N, format_xp, in, out_tmp, dpi);
    }

    if (opt == CONVERT_PS_TO_EPS) {
        char format_xp[] = "eps2eps \"%s\" \"%s\"";
        snprintf(cmd, N, format_xp, in, out_tmp);
    }
        
#endif
    diagnostics(3, "`%s`", cmd);

    err = system(cmd);

    if (err) {
        diagnostics(WARNING, "\nerror=%d when converting %s", err, in);
        safe_free(out_tmp);
        return NULL;
    }
        
    return out_tmp;
}

static void PicComment(int16_t label, int16_t size, FILE * fp)
{
    int16_t long_comment = 0x00A1;
    int16_t short_comment = 0x00A0;
    int16_t tag;

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

static char *strdup_new_extension(const char *s, const char *old_ext, const char *new_ext)
{
    char *new_name, *p;
    int s_len, o_len, n_len, siz;
    
    if (s==NULL || old_ext == NULL || new_ext == NULL) return NULL;
    
    s_len = (int) strlen(s);
    o_len = (int) strlen(old_ext);
    n_len = (int) strlen(new_ext);
        
    /* make sure that old_ext exists */
    p= (char *) s;
    do {
        p = strstr(p, old_ext);
    } while (p && p-s > s_len-o_len) ;
    
    if (p == NULL) return NULL;

    siz = s_len - o_len + n_len + 1;
    new_name = (char *) malloc(siz);
    my_strlcpy(new_name, s, s_len - o_len + 1);
    my_strlcat(new_name, new_ext, siz);

    return new_name;
}

/******************************************************************************
 purpose   : return true if ext is at end of s (case insensitively)
 ******************************************************************************/
static int has_extension(const char *s, const char *ext)
{
    char *t;

    t = (char *)s + strlen(s) - strlen(ext);

    if (strcasecmp(t, ext) == 0)
        return TRUE;

    return FALSE;
}


/******************************************************************************
 purpose   : if the file has a graphic extension, return it otherwise return NULL
 ******************************************************************************/
static char *has_graphic_extension(const char *s)
{
    GraphConvertElement *thisFormat;
	diagnostics(4,"testing for graphics extension '%s'",s);
	
    for (thisFormat = GraphConvertTable; thisFormat->extension != NULL; thisFormat++) {
        if (has_extension(s,thisFormat->extension)) 
            return strdup(thisFormat->extension);
    }
    
    return NULL;
}


/******************************************************************************
 purpose   : split filename into directory, name, and extension
 ******************************************************************************/
static void split_filename(const char *f, char **dir, char **name, char **ext)
{
	char *s, *t, *x;
	
	*dir = NULL;
	*name = NULL;
	*ext = NULL;
	
	if (f == NULL) return;
	
	/* first figure out the directory */
	s = strrchr(f,'/');
	if (s) {
	    t = strdup(f);
	    x = strrchr(t,'/')+1;
	    *x = '\0';
		if (*f == '/') {                /* absolute names start with '/' */
	    	*dir=t;
	    } else if (g_home_dir) {        /* names are relative to home_dir */
	    	*dir = strdup_together(g_home_dir, t);
	    	safe_free(t);
	    } else {                        
	    	*dir=t;
	    }
	    s++;
	} else {
		if (g_home_dir)
			*dir = strdup(g_home_dir);
		s = (char *) f;
	}
        
	*name = strdup(s);
    *ext = has_graphic_extension(s);

	if (*ext) {
		t = *name + strlen(*name) - strlen(*ext);
		*t = '\0';
	} 
	
	diagnostics(5,"filename='%s', dir='%s', name='%s', ext='%s'", f, *dir, *name, *ext);
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
    int16_t handle_size;
    unsigned char byte;
    int16_t PostScriptBegin = 190;
    int16_t PostScriptEnd = 191;
    int16_t PostScriptHandle = 192;
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
    return_value = SysGraphicsConvert(CONVERT_CROP, offset, g_dots_per_inch, eps, pict);
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
        
    if (name == NULL) return NULL;
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
                
    out = SysGraphicsConvert(CONVERT_CROP, 0, g_dots_per_inch, name, png);

    free(png);
    return out;
}

/******************************************************************************
     purpose : create a png file from a PDF file and return file name
 ******************************************************************************/
static char *pdf_to_png(char *pdf)
{
    char *png, *out;
    if (pdf == NULL) return NULL;
       
    if (strstr(pdf, ".pdf") != NULL)
        png = strdup_new_extension(pdf, ".pdf", ".png");
    else if (strstr(pdf, ".PDF") != NULL)
        png = strdup_new_extension(pdf, ".PDF", ".png");
    else
        return NULL;
                
    out = SysGraphicsConvert(CONVERT_PDF, 0, g_dots_per_inch, pdf, png);
    safe_free(png);
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
    int32_t width, height;

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
        sscanf(ans, "%ld %ld", (long *)&width, (long *)&height);
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
static void AdjustScaling(double h, double w, double target_h, double target_w, double s, uint16_t *sx, uint16_t *sy)
{
        diagnostics(5,"AdjustScaling h       =%f w       =%f s=%f", h, w, s);
        diagnostics(5,"AdjustScaling target_h=%f target_w=%f", target_h, target_w);

        if (target_h != 0 && h != 0) 
            *sy = (uint16_t) my_rint(100.0 * target_h / h);
        else
            *sy = (uint16_t) my_rint(s * 100);
        
        if (target_w == 0 || w == 0)
            *sx = *sy;
        else
            *sx = (uint16_t) my_rint(100.0 * target_w / w);

        /* catch the case when width is specified, but not height */
        if (target_h == 0 && target_w != 0)
            *sy = *sx;

        /* special case if both are zero then just set scaling to one */
        if (target_h == 0 && target_w == 0) {
            *sx = 100. * s;
            *sy = 100. * s;
        }

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
static void PutPictFile(char *s, double height0, double width0, double scale, double baseline)
{
    FILE *fp;
    char *pict;
    int16_t buffer[5];
    int16_t top, left, bottom, right;
    int16_t width, height;
    uint16_t sx,sy;

    pict = strdup(s);
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
    uint32_t DataLength;   Size of Data field in bytes
    uint32_t Type;         Code identifying the type of chunk
    BYTE  Data[];       The actual data stored by the chunk
    uint32_t Crc;          CRC-32 value of the Type and Data fields
} PNGCHUNK;

typedef struct _pHYsChunkEntry
{
   uint32_t PixelsPerUnitX;    Pixels per unit, X axis 
   uint32_t PixelsPerUnitY;    Pixels per unit, Y axis 
   BYTE  UnitSpecifier;     0 = unknown, 1 = meter 
} PHYSCHUNKENTRY;
*/

static unsigned char * getPngChunk(FILE *fp, char *s)
{
    uint32_t crc, DataLength;
    char Type[5];
    unsigned char *data;
    
    Type[4]='\0';
    
    diagnostics(6, "getPngChunk ... seeking '%s'",s);
    data = NULL;
    do {
        if (data) free(data);
        
        /* read chuck size */
        fread(&DataLength, 4, 1, fp);
        if (g_little_endian) DataLength = LETONL(DataLength);
        
        /* read chunk type */
        fread(Type, 1, 4, fp);
        if (strcmp(Type,"IEND") == 0) return NULL;
        
        diagnostics(6,"found chunk '%s' size %u bytes",Type, DataLength);
        data = (unsigned char *) malloc(DataLength);
        if (data == NULL) return NULL;
        
        fread(data, DataLength, 1, fp);
        fread(&crc, 4, 1, fp);
        crc++; /* ignored, but touch to eliminate warning */
    } while (strcmp(s,Type) != 0);
    
    return data;    
}

static void InsertFigureAsComment(const char *s, int hinline)
{
	if (!s) return;
	if (*s=='\0') return;

  if (hinline == 1) fprintRTF("{\\dn10 ");
		putRtfStrEscaped("[###");
		putRtfStrEscaped(s);
		putRtfStrEscaped("###]");
  if (hinline == 1) fprintRTF("}");
}

/******************************************************************************
     purpose : determine height and width of file
               w & h are the size in pixels
               xres is the number of pixels per meter
 ******************************************************************************/
static void GetPngSize(char *s, uint32_t *w_pixels, uint32_t *h_pixels, double *xres, double *yres, int *bad_res)
{
    FILE *fp;
    uint32_t *p;
    unsigned char buffer[16];
    char reftag[9] = "\211PNG\r\n\032\n";
    unsigned char *data = NULL;

    diagnostics(4, "GetPngSize of '%s'", s);
    *xres = POINTS_PER_METER;
    *yres = POINTS_PER_METER;
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
        fclose(fp);
        return;
    }

    p = (uint32_t *) data;  
    *w_pixels = (g_little_endian) ? LETONL(*p) : *p;
    p++;
    *h_pixels = (g_little_endian) ? LETONL(*p) : *p;
    free(data);
                
    data = getPngChunk(fp,"pHYs");
    if (data == NULL) {
        diagnostics(2, "Graphics file '%s': could not locate pHYs chunk!", s);
        diagnostics(2, "defaulting to 72 pixels/inch for resolution");
        fclose(fp);
        return;
    }

    p = (uint32_t *) data;  
    *xres = (g_little_endian) ? LETONL(*p) : *p;
    p++;
    *yres = (g_little_endian) ? LETONL(*p) : *p;
    free(data);

    if (fabs(*xres-POINTS_PER_METER)<2) *xres = POINTS_PER_METER;
    if (fabs(*yres-POINTS_PER_METER)<2) *yres = POINTS_PER_METER;

    /* dots per inch, not per meter! */
    if (*xres < POINTS_PER_METER) {
        *bad_res = 1;
        diagnostics(5, "bogus resolution in png image! ");
        diagnostics(5, "xres = %g, yres = %g, pixels/meter", *xres, *yres);
        diagnostics(5, "xres = %g, yres = %g, pixels/in",  *xres*72.0/POINTS_PER_METER, *yres*72.0/POINTS_PER_METER);
        *xres *= POINTS_PER_METER/72.0;
        *yres *= POINTS_PER_METER/72.0;
    } else 
        *bad_res = 0;
    
    diagnostics(5, "xres = %g, yres = %g, pixels/meter", *xres, *yres);
    diagnostics(5, "xres = %g, yres = %g, pixels/in", *xres*72.0/POINTS_PER_METER, *yres*72.0/POINTS_PER_METER);

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
        encode_scale  = POINTS_PER_METER/xres (scaling factor inside the PNG)
        scale         = the scaling factor given by \includegraphics

Basically, if the PNG was created with a correct value for the resolution
then we leave things as they are.  On the other hand, an incorrect value 
means that the Word will assume the image resolution is 1/72 of an inch.  
Consequently, when this happens, scale must be altered by the convert_scale 
value.  As you can see below, we never use encode_scale.

On entry baseline should be in pixels.

\picwN      width in pixels 
\pichN      height in pixels
\picwgoalN      Desired width in twips
\pichgoalN      Desired height in twips
\picscalexN Horizontal scaling as a percentage (default=100)
\picscaleyN Vertical scaling as a percentage

******************************************************************************/
static void PutPngFile(char *png, double height_goal, double width_goal, double scale, double baseline)
{
    FILE *fp;
    double xres,yres;
    char *s;
    uint32_t w_pixels, h_pixels, b;
    uint32_t w_twips, h_twips;
    uint16_t sx, sy;
    int bad_res;
        
    diagnostics(WARNING,"Encoding  '%s'",png);

    GetPngSize(png, &w_pixels, &h_pixels, &xres, &yres, &bad_res);
    if (w_pixels == 0 || h_pixels == 0) return;

        /* make sure that we can open the file */
    fp = fopen(png, "rb");
    if (fp == NULL) return;

    /*                     pixels     points     20 twips   */
    /* twips = (pixels) / -------- * -------- * ----------  */
    /*                     meter       meter      1 point   */

    w_twips = (int)(w_pixels / xres * POINTS_PER_METER * 20.0 + 0.5);
    h_twips = (int)(h_pixels / yres * POINTS_PER_METER * 20.0 + 0.5);
    
    /* because \dn command requires half-points! */
    b = (uint32_t) baseline * 2;
    
    AdjustScaling(h_twips,w_twips,height_goal,width_goal,scale,&sx,&sy);
    
    if (bad_res) {
        sx = (uint16_t) (sx * POINTS_PER_METER / xres);
        sy = (uint16_t) (sy * POINTS_PER_METER / yres);
    }
        
    diagnostics(4, "picw       = %8lu pixels,     pich        = %8lu pixels", w_pixels, h_pixels);
    diagnostics(4, "picwgoal   = %8lu twips,      pichgoal    = %8lu twips", w_twips, h_twips);
    diagnostics(4, "xres       = %8.2f pix/meter, yres        = %8.2f pix/meter", xres, yres);
    diagnostics(4, "xres       = %8.2f pix/inch,   yres        = %8.2f pix/inch", xres*72.0/POINTS_PER_METER, yres*72.0/POINTS_PER_METER);
        diagnostics(4, "scale      = %8.3f", scale);
    diagnostics(4, "width_goal = %8d twips,      height_goal = %8d twips", (int)width_goal, (int)height_goal);
    diagnostics(4, "baseline   = %8.3g twips", baseline);
    diagnostics(4, "sx         = %8lu percent,    sy          = %8lu percent", sx, sy);
    
    /* Write the header for the png bitmap */
    fprintRTF("\n{");
    
    /* for non-zero baseline shifts, add an extra half-pixel because it looks better */
    if (b) fprintRTF("\\dn%lu", b+1); 
    
    fprintRTF("\\pict");
    if (sx != 100 && sy != 100) fprintRTF("\\picscalex%u\\picscaley%u", sx,sy);
    fprintRTF("\\picw%lu\\pich%lu", w_pixels, h_pixels);
    fprintRTF("\\picwgoal%lu\\pichgoal%lu", w_twips, h_twips);
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
static void PutJpegFile(char *s, double height0, double width0, double scale, double baseline)
{
    FILE *fp;
    char *jpg;
    uint16_t buffer[2];
    int m=0;
    uint16_t width, height;
    uint32_t w, h;
    uint16_t sx, sy;

    jpg = strdup_together(g_home_dir, s);
    diagnostics(2, "PutJpegFile '%s'", jpg);

    fp = fopen(jpg, "rb");
    free(jpg);
    if (fp == NULL)
        return;

    if (fgetc(fp) != 0xFF && fgetc(fp) != 0xD8) {
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
        width = (uint16_t) LETONS(width);
        height = (uint16_t) LETONS(height);
    }

    diagnostics(4, "width = %d, height = %d", width, height);

    w = (uint32_t) (100000.0 * width / (20 * POINTS_PER_METER));
    h = (uint32_t) (100000.0 * height / (20 * POINTS_PER_METER));
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

static void PutEmfFile(char *s, double height0, double width0, double scale, double baseline)
{
    FILE *fp;
    char *emf;
    uint32_t RecordType;   /* Record type (always 0x00000001) */
    uint32_t RecordSize;   /* Size of the record in bytes */
    int32_t BoundsLeft;            /* Left inclusive bounds */
    int32_t BoundsRight;           /* Right inclusive bounds */
    int32_t BoundsTop;             /* Top inclusive bounds */
    int32_t BoundsBottom;          /* Bottom inclusive bounds */
    int32_t FrameLeft;             /* Left side of inclusive picture frame */
    int32_t FrameRight;            /* Right side of inclusive picture frame */
    int32_t FrameTop;              /* Top side of inclusive picture frame */
    int32_t FrameBottom;           /* Bottom side of inclusive picture frame */
    uint32_t Signature;    /* Signature ID (always 0x464D4520) */
    uint32_t w, h, width, height;
        uint16_t sx, sy;
        
    emf = strdup(s);
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
    height = (uint32_t) (BoundsBottom - BoundsTop);
    width = (uint32_t) (BoundsRight - BoundsLeft);

    w = (uint32_t) ((100000.0 * width) / (20 * POINTS_PER_METER));
    h = (uint32_t) ((100000.0 * height) / (20 * POINTS_PER_METER));
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
static void PutWmfFile(char *s, double height0, double width0, double scale, double baseline)
{
    FILE *fp;
    char *wmf;
    uint32_t Key;         /* Magic number (always 0x9AC6CDD7) */
    uint16_t FileType;    /* Type of metafile (0=memory, 1=disk) */
    uint16_t HeaderSize;  /* Size of header in WORDS (always 9) */
    uint16_t Handle;      /* Metafile HANDLE number (always 0) */
    uint16_t Left;        /* Left coordinate in twips */
    uint16_t Top;         /* Top coordinate in twips */
    uint16_t Right;       /* Right coordinate in twips */
    uint16_t Bottom;      /* Bottom coordinate in twips */
    uint16_t width, height, sx, sy;
    uint32_t magic_number = 0x9AC6CDD7;

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
            FileType = (uint16_t) LETONS(FileType);
            HeaderSize = (uint16_t) LETONS(HeaderSize);
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
static void PutPdfFile(char *s, double height0, double width0, double scale, double baseline)
{
    char *png, *pdf, *eps, *out, *tmp_dir;
    
    if (g_figure_include_converted) {
		diagnostics(WARNING, "Rendering '%s'", s);
		diagnostics(3,"Converting PDF to PNG and inserting into RTF.");
	
		png = pdf_to_png(s);
		
		if (png) {
			PutPngFile(png, height0, width0, scale, baseline);
			my_unlink(png);
			safe_free(png);
		}
    }

    if (g_figure_comment_converted) {
       diagnostics(3,"Converting PDF to EPS and inserting file name in text");
       eps = strdup_new_extension(s, ".pdf", ".eps");
       if (eps == NULL) {
           eps = strdup_new_extension(s, ".PDF", ".eps");
           if (eps == NULL) {
		          diagnostics(ERROR,"PutPdfFile: Graphicsfile hat not .pdf extension");
              return;
           }   
       }

       tmp_dir = getTmpPath();
       pdf = strdup_together(g_home_dir, s);
       out = SysGraphicsConvert(CONVERT_PS_TO_EPS, 0, g_dots_per_inch, pdf, eps);
        
       if (out != NULL) {
           eps = strdup_together(tmp_dir, eps);
           putRtfStrEscaped("[###");
           putRtfStrEscaped(eps);
           putRtfStrEscaped("###]");
           free(out);
       }
       free (tmp_dir);
       free (pdf);
       free (eps);
    }
}

/******************************************************************************
 purpose   : convert eps to png and insert in RTF file
 ******************************************************************************/
static void PutEpsFile(char *s, double height0, double width0, double scale, double baseline)
{
    char *png;

    if (g_figure_include_converted) {
    	diagnostics(WARNING, "Rendering '%s'", s);
		  diagnostics(3,"Converting EPS to PNG and inserting in RTF.");
        png = eps_to_png(s);
        if (png) {
            PutPngFile(png, height0, width0, scale, baseline);
            my_unlink(png);
            free(png);
        }
    }

    if (g_figure_comment_converted) {
		  diagnostics(3,"Inserting EPS file name in text");
		  putRtfStrEscaped("[###");
		  putRtfStrEscaped(s);
		  putRtfStrEscaped("###]");
    }
}
/******************************************************************************
 purpose   : convert ps to png and insert in RTF file
 ******************************************************************************/
static void PutPsFile(char *s, double height0, double width0, double scale, double baseline)
{
    char *png, *ps, *eps, *out, *tmp_dir;

    if (g_figure_include_converted) {
    	diagnostics(WARNING, "Rendering '%s'", s);
		  diagnostics(3,"Converting PS to PNG and inserting in RTF.");
        png = eps_to_png(s);
        if (png) {
            PutPngFile(png, height0, width0, scale, baseline);
            my_unlink(png);
            free(png);
        }
    }

    if (g_figure_comment_converted) {
       diagnostics(3,"Converting PS to EPS and inserting file name in text");
       eps = strdup_new_extension(s, ".ps", ".eps");
       if (eps == NULL) { 
           eps = strdup_new_extension(s, ".PS", ".eps");
           if (eps == NULL) { 
		          diagnostics(ERROR,"PutPsFile: Graphicsfile hat not .ps extension");
              return;
           }
       }
  
       tmp_dir = getTmpPath();
       ps  = strdup_together(g_home_dir, s);
       out = SysGraphicsConvert(CONVERT_PS_TO_EPS, 0, g_dots_per_inch, ps, eps);

       if (out != NULL) {
           eps = strdup_together(tmp_dir, eps);
           putRtfStrEscaped("[###");
           putRtfStrEscaped(eps);
           putRtfStrEscaped("###]");
           free (out);
       }      
       free (tmp_dir);
       free (ps);  
       free (eps);  
    }
}

/******************************************************************************
 purpose   : Insert TIFF file (from g_home_dir) into RTF file as a PNG image
 ******************************************************************************/
static void PutTiffFile(char *s, double height0, double width0, double scale, double baseline)
{
    char *tiff, *png, *out;

    diagnostics(2, "PutTiffFile '%s'", s);
    png = strdup_new_extension(s, ".tiff", ".png");
    if (png == NULL) {
        png = strdup_new_extension(s, ".TIFF", ".png");
        if (png == NULL)
            return;
    }

    tiff = strdup_together(g_home_dir, s);
    out = SysGraphicsConvert(CONVERT_SIMPLE, 0, g_dots_per_inch, tiff, png);
        
    if (out != NULL) {
        PutPngFile(out, height0, width0, scale, baseline);
        my_unlink(out);
        free(out);
    }
    
    free(tiff);
    free(png);
}

/******************************************************************************
 purpose   : Insert GIF file (from g_home_dir) into RTF file as a PNG image
 ******************************************************************************/
static void PutGifFile(char *s, double height0, double width0, double scale, double baseline)
{
    char *gif, *png, *out;
        
    diagnostics(2, "PutGifFile '%s'", s);
    png = strdup_new_extension(s, ".gif", ".png");
    if (png == NULL) {
        png = strdup_new_extension(s, ".GIF", ".png");
        if (png == NULL)
            return;
    }

    gif = strdup_together(g_home_dir, s);
    out = SysGraphicsConvert(CONVERT_SIMPLE, 0, g_dots_per_inch, gif, png);

    if (out != NULL) {
        PutPngFile(out, height0, width0, scale, baseline);
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
static double GetBaseline(const char *tex_file_stem, const char *pre)
{
    FILE *fp;
    int thechar;
    char *pbm_file_name;
    char magic[250];
    int width, height, items, top, bottom;
        double baseline = 3.0;
        
    /* baseline=0 if not an inline image */
    if ((strcmp(pre, "$") != 0) && (strcmp(pre, "\\begin{math}") != 0) && (strcmp(pre, "\\(") != 0))
        return 0;

    pbm_file_name = strdup_together(tex_file_stem, ".pbm");

    diagnostics(4, "GetBaseline opening='%s'", pbm_file_name);

    fp = fopen(pbm_file_name, "rb");
    free(pbm_file_name);
    
    if (fp == NULL) 
        return baseline;

    items = fscanf(fp, "%c%c", &magic[0], &magic[1]);   /* ensure that file begins with "P4" */
    if (items != 2 || magic[0]!='P' || magic[1]!='4') {
        diagnostics(WARNING,"Bad header in PBM file");
        fclose(fp);
        return baseline;
    }
    
    items = fscanf(fp, " %s", magic);
    while ((items == 1) && (magic[0] == '#')) { /* skip any comment lines in pbm file */
        if (!ReadLine(fp))
            goto Exit;
        items = fscanf(fp, "%s", magic);
    }

    items = sscanf(magic, "%d", &width);   /* make sure image width is 1 */
    if (items != 1 || width != 1)
        goto Exit;
        
    items = fscanf(fp, " %d", &height);    /* read height */
    if (items != 1)
        goto Exit;
        
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

        /* baseline is in pixels at 72 dots per inch but bitmap may be larger */
        baseline = bottom * 72.0 / g_dots_per_inch;

    diagnostics(4, "height=%d top=%d bottom=%d baseline=%g", height, top, bottom, baseline);
  Exit:
    fclose(fp);
    return baseline;
}

/******************************************************************************
 purpose   : Convert LaTeX to Bitmap and insert in RTF file
 ******************************************************************************/
static void PutLatexFile(const char *tex_file_stem, double scale, const char *pre, conversion_t convertTo, int hinline)
{
    char *png_file_name = NULL;
    char *tmp_path;
    int  bmoffset;
    int bad_res;
    double height_goal, width_goal;
    double baseline = 0;
    double png_xres, png_yres;
    uint32_t png_width = 0;
    uint32_t png_height= 0;
    uint16_t png_resolution=0;
    double max_fig_size = 32767.0 / 20.0;  /* in twips */
        
    if (convertTo == EPS) {
    	char *eps_file_name = NULL;
        eps_file_name = strdup_together(tex_file_stem, ".eps");
        diagnostics(1, "Converting LaTeX to EPS...");
        tmp_path = SysGraphicsConvert(CONVERT_LATEX_TO_EPS, bmoffset, png_resolution, tex_file_stem, eps_file_name);

    	if (NULL == tmp_path)
        	diagnostics(WARNING, "PutLatexFile failed to convert '%s.tex' to '%s'",tex_file_stem,eps_file_name);
        else 
			InsertFigureAsComment(eps_file_name, hinline);
			
        safe_free(eps_file_name);
        return;
    }

    diagnostics(3, "Rendering LaTeX as a bitmap...");

    /* arrived at by trial and error ... works for sizes from 72 to 1200 dpi */
    bmoffset = g_dots_per_inch / 60 + 1;

    /* it is possible that the latex image is too wide or tall for Word
       we only know this after we have tried once.  If the image is too
       large then the resolution is made smaller and the PNG is remade */
    
    png_resolution = (uint16_t) g_dots_per_inch;
    
    png_file_name = strdup_together(tex_file_stem, ".png");
    tmp_path = SysGraphicsConvert(CONVERT_LATEX_TO_PNG, bmoffset, png_resolution, tex_file_stem, png_file_name);

    if (NULL == tmp_path) {
        diagnostics(WARNING, "PutLatexFile failed to convert '%s.tex' to '%s'",tex_file_stem,png_file_name);
        safe_free(png_file_name);
        return;
    }
    
    /* Figures can only have so many bits ... figure out the width and height
       and if these are too large then reduce resolution and make a new bitmap */
    GetPngSize(png_file_name, &png_width, &png_height, &png_xres, &png_yres, &bad_res);

    if (png_width  > max_fig_size || png_height > max_fig_size) {
            
        if (png_height && png_height > png_width) 
            png_resolution = (uint16_t)((double)g_dots_per_inch / (double)png_height * max_fig_size);
        else
            png_resolution = (uint16_t)((double)g_dots_per_inch / (double)png_width * max_fig_size);

        safe_free(tmp_path);
        tmp_path = SysGraphicsConvert(CONVERT_LATEX_TO_PNG, bmoffset, png_resolution, tex_file_stem, png_file_name);
        if (tmp_path == NULL) {
            safe_free(png_file_name);
            return;
        }
        
        GetPngSize(png_file_name, &png_width, &png_height, &png_xres, &png_yres, &bad_res);
    }
    
    /* we have a png file of the latex now ... insert it after figuring out offset and scaling */

    baseline = GetBaseline(tex_file_stem, pre);
    
    diagnostics(3, "PutLatexFile bitmap has (height=%d,width=%d) baseline=%g  resolution=%u", 
                                    png_height, png_width, baseline, png_resolution);
    
    height_goal = (scale * png_height * POINTS_PER_METER / png_yres * 20.0 + 0.5);
    width_goal  = (scale * png_width  * POINTS_PER_METER / png_xres * 20.0 + 0.5);
    
    PutPngFile(png_file_name, height_goal, width_goal, scale*100, baseline);
    
    safe_free(tmp_path);
    safe_free(png_file_name);
}

/* this is more general than just equations because it is used to create
   documents for the latex picture, music, tikzpicture environments also */
   
static char *SaveEquationAsFile(const char *post_begin_document,
                                const char *pre, const char *eq_with_spaces, const char *post)
{
    FILE *f;
    char name[15];
    char *tmp_dir, *tex_file_stem, *tex_file_name, *eq;
    static int file_number = 0;

    if (!pre || !eq_with_spaces || !post)
        return NULL;

/* create needed file names */
    file_number++;
    tmp_dir = getTmpPath();
    snprintf(name, 15, "l2r_%04d", file_number);
    tex_file_stem = strdup_together(tmp_dir, name);
    safe_free(tmp_dir);

    tex_file_name = strdup_together(tex_file_stem, ".tex");

    diagnostics(2, "SaveEquationAsFile = %s", tex_file_name);

    f = fopen(tex_file_name, "w");
    safe_free(tex_file_name);

        /* cannot open the file for writing */
    if (f==NULL) {
        diagnostics(WARNING, "Could not open '%s' to save equation",tex_file_stem);
        safe_free(tex_file_stem);
        return NULL;
    }
    
    eq = strdup_noendblanks(eq_with_spaces);
    
    fprintf(f, "%s", g_preamble);
    fprintf(f, "\\thispagestyle{empty}\n");
    fprintf(f, "\\begin{document}\n");
    if (post_begin_document) 
            fprintf(f, "%s\n", post_begin_document);
            
    fprintf(f, "\\setcounter{equation}{%d}\n", getCounter("equation"));
    
    if ( streq(pre, "$") || streq(pre, "\\begin{math}") || streq(pre, "\\(") ) {
        fprintf(f, "%%INLINE_DOT_ON_BASELINE\n");
        if ((g_equation_inline_eps) || (g_equation_display_eps)) {
        fprintf(f, "%s\n^I_g%s\n%s", pre, eq, post);  
        }
        else {
        fprintf(f, "%s\n.\\quad %s\n%s", pre, eq, post);  
        }
    } 
    else if (strstr(pre, "equation"))
        /* fprintf(f, "$$%s$$", eq);  WH 2014-01-17*/
        fprintf(f, "\\begin{displaymath}\n%s\n\\end{displaymath}", eq);  /* WH 2014-04-03*/
    else
        fprintf(f, "%s\n%s\n%s", pre, eq, post);

    fprintf(f, "\n\\end{document}");
    fclose(f);
    free(eq);

    return tex_file_stem;
}

/******************************************************************************
 purpose   : Call before WriteLatexAsBitmap()
 ******************************************************************************/
void PrepareDisplayedBitmap(char *the_type)
{
    CmdEndParagraph(0);
    CmdVspace(VSPACE_SMALL_SKIP);
    CmdIndent(INDENT_NONE);
    startParagraph(the_type, PARAGRAPH_FIRST);
}

/******************************************************************************
 purpose   : Call after WriteLatexAsBitmap()
 ******************************************************************************/
void FinishDisplayedBitmap(void)
{
    CmdEndParagraph(0);
    CmdVspace(VSPACE_SMALL_SKIP);
    CmdIndent(INDENT_INHIBIT);
}

static char * abbreviate(const char *s, int len)
{
    char *t,*p;
    int i,n,half;
        
    if (s==NULL) return strdup("< EMPTY >");
        
    n = (int) strlen(s);
    
    if (n<len) {
        t =strdup(s);
    } else { 
        t = (char *) malloc((len+1) * sizeof(char));
        
        half = (len - 6)/2;
        for (i=0; i<=half; i++)
            t[i] = s[i];
        
        t[half+1] = ' ';
        t[half+2] = '.';
        t[half+3] = '.';
        t[half+4] = '.';
        t[half+5] = ' ';
    
        for (i=0; i<=half; i++)
            t[half + 6 + i] = s[n-half+i];
    }
    
    /*replace \n by spaces */
    while ( (p=strchr(t,'\n')) ) *p = ' ';
                    
    return t;
}

/******************************************************************************
 purpose   : Convert LaTeX to Bitmap and write to RTF file
 ******************************************************************************/
void WriteLatexAsBitmapOrEPS(char *pre, char *eq, char *post, conversion_t convertTo)
{
    char *p, *abbrev, *latex_to_convert;
    char *name = NULL;
    int hinline = 0;
    
    /* go to a bit a trouble to give the user some feedback */
    latex_to_convert = strdup_together3(pre,eq,post);
    abbrev = abbreviate(latex_to_convert, 50);
    diagnostics(WARNING, "Rendering '%s'", abbrev);
    safe_free(abbrev);
    safe_free(latex_to_convert);
        
    if (eq == NULL) return;

/* suppress bitmap equation numbers in eqnarrays with zero or one \label{}'s*/
    if (pre && streq(pre, "\\begin{eqnarray}")) {
    
        p = strstr(eq, "\\label");
        if (p && strlen(p) > 6) /* found one ... is there a second? */
            p = strstr(p + 6, "\\label");
        if (p == NULL)
            name = SaveEquationAsFile(NULL, "\\begin{eqnarray*}", eq, "\\end{eqnarray*}");
        else
            name = SaveEquationAsFile(NULL, pre, eq, post);

    } else if (pre && streq(pre, "\\begin{align}")) {

        p = strstr(eq, "\\label");
        if (p && strlen(p) > 6) /* found one ... is there a second? */
            p = strstr(p + 6, "\\label");
            
        if (p)
            name = SaveEquationAsFile(NULL, pre, eq, post);
        else
            name = SaveEquationAsFile(NULL, "\\begin{align*}", eq, "\\end{align*}");
            
    } else if (pre && (strstr(pre, "psgraph") || strstr(pre, "pspicture")) ){
        p = strdup_together(g_psset_info, g_psstyle_info);
        name = SaveEquationAsFile(p, pre, eq, post);
        safe_free(p);
        
    } else  {
        name = SaveEquationAsFile(NULL, pre, eq, post);
        if ( streq(pre, "$") || streq(pre, "\\begin{math}") || streq(pre, "\\(") ) 
            hinline=1;
    }
    
    if (name) {
        if (strstr(pre, "music") 
            || strstr(pre, "figure") 
            || strstr(pre, "picture")
            || strstr(pre, "tabular")
            || strstr(pre, "tabbing")
            || strstr(pre, "psgraph")
            || strstr(pre, "pspicture")
            || strstr(pre, "tikzpicture")) 
            PutLatexFile(name, g_png_figure_scale, pre, convertTo, hinline);
        else
            PutLatexFile(name, g_png_equation_scale, pre, convertTo, hinline);

        safe_free(name);
    }
}

static char *upper_case_string(const char *s)
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

/* does file exist with upper or lowercase extension */
static char *exists_with_extension(const char *dir, const char *name, const char *ext) 
{
    char *t, *x;
    t = strdup_together3(dir, name, ext);

    if (file_exists(t))
        return t;

    safe_free(t);

/* now try upper case version of ext */
    x = upper_case_string(ext);
    t = strdup_together3(dir, name, x);
    safe_free(x);

    if (file_exists(t))
        return t;

    safe_free(t);
    
    return NULL;
}

/******************************************************************************
 purpose   : return s.ext or s.EXT if it exists otherwise return NULL
 ******************************************************************************/
static char *exists_with_any_extension(const char *dir, const char *name, const char *ext)
{
    char *x, *newpath;
    int  i;

    /* if no graphics path or a name is fully specified then try the plain file only */
    if (nGraphicsPathElems == 0)
        return exists_with_extension(dir,name,ext);
    
    /* else try the different directories in the graphics path */
    for (i=0; i<nGraphicsPathElems; i++) {
        newpath = strdup_together(graphicsPath[i], dir);
        diagnostics(4,"does '%s%s%s' exist?",newpath,name,ext);
        x = exists_with_extension(newpath,name,ext);
        safe_free(newpath);
        if (x) return x;
    }
    
    return NULL;
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
        
        keyvalue_pair(opt,&key,&value);
        if (!key) return;
        lly=getStringDimension(key);
        free(key);

        opt2 = keyvalue_pair(opt2,&key,&value);
        if (!key) return;
        urx=getStringDimension(key);
        free(key);
        
        keyvalue_pair(opt2,&key,&value);
        if (!key) return;
        ury=getStringDimension(key);
        free(key);

        *h = ury-lly;
        *w = urx-llx;
        return;
    } 
            

/*  \includegraphics[width,height]{filename} for graphics */

    if (g_graphics_package == GRAPHICS_GRAPHICS && opt) {

        keyvalue_pair(opt,&key,&value);
        if (!key) return;
        *w=getStringDimension(key);
        free(key);
        
        keyvalue_pair(opt,&key,&value);
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
    char *filename=NULL;
    double scale    = 1.0;
    double baseline = 0.0;
    double height   = 0.0;
    double width    = 0.0;

    if (code == FIGURE_INCLUDEGRAPHICS) {
        options = getBracketParam();
        options2 = getBracketParam();
        filename = getBraceParam();

        HandleGraphicsOptions(options,options2,&height,&width,&scale);
        
        safe_free(options);
        safe_free(options2);
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
    	GraphConvertElement *thisFormat;
		char *fullpathname=NULL;
		char *dir=NULL;
		char *name=NULL;
		char *ext=NULL;
		
        changeTexMode(MODE_HORIZONTAL);

        split_filename(filename,&dir,&name,&ext);
        
        if (ext) {
        	fullpathname=exists_with_any_extension(dir,name,ext);
        	if (fullpathname) {
				for (thisFormat = GraphConvertTable; thisFormat->extension; thisFormat++) {
					if (strcasecmp(ext,thisFormat->extension) == 0) break;
				}
			}
        } else {   
            /* implicit extension in file name ... try all of them until one is found */
			for (thisFormat = GraphConvertTable; thisFormat->extension; thisFormat++) {
        		fullpathname=exists_with_any_extension(dir,name,thisFormat->extension);
				if (fullpathname) break;
			}   
       }
       
	   if (fullpathname && thisFormat->extension) {
			diagnostics(2,"located graphics file as '%s'",fullpathname);
			thisFormat->encoder(fullpathname, height, width, scale, baseline);
	   } else 
			diagnostics(WARNING, "The graphics file '%s' was not found", filename); 
		
       safe_free(dir);
       safe_free(name);
       safe_free(ext);
       safe_free(fullpathname);
       safe_free(filename);
    }
}

/******************************************************************************
  purpose: handle \begin{pspicture} ... \end{pspicture}
           by converting to png image and inserting
 ******************************************************************************/
void CmdPsPicture(int code)
{
    char *contents = NULL;
    char post[] = "\\end{pspicture}";

    if (!(code & ON)) {
        diagnostics(4, "exiting CmdPsPicture");
        return;
    } else 
        diagnostics(4, "entering CmdPsPicture");
    

    contents = getTexUntil(post, 0);

    PrepareDisplayedBitmap("PS picture");
    WriteLatexAsBitmapOrEPS("\\begin{pspicture}", contents, post, BITMAP);
    FinishDisplayedBitmap();

    ConvertString(post);    /* to balance the \begin{picture} */    
    safe_free(contents);
}

/******************************************************************************
  purpose: handle \begin{psgraph} ... \end{psgraph}
           by converting to png image and inserting
 ******************************************************************************/
void CmdPsGraph(int code)
{
    char *contents = NULL;
    char post[] = "\\end{psgraph}";

    if (!(code & ON)) {
        diagnostics(4, "exiting CmdPsGraph");
        return;
    } else 
        diagnostics(4, "entering CmdPsGraph");

    contents = getTexUntil(post, 0);

    PrepareDisplayedBitmap("PS Graph");
    WriteLatexAsBitmapOrEPS("\\begin{psgraph}", contents, post, BITMAP);
    FinishDisplayedBitmap();

    ConvertString(post);    /* to balance the \begin{graph} */
    safe_free(contents);
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
    safe_free(contents);
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
        
    safe_free(a);
    safe_free(b);
    safe_free(c);
}

/******************************************************************************
  purpose: handle \begin{picture} ... \end{picture}
           by converting to png image and inserting
 ******************************************************************************/
void CmdPicture(int code)
{
    char *picture = NULL;
    char post[] = "\\end{picture}";

    if (!(code & ON)) {
        diagnostics(4, "exiting CmdPicture");
        return;
    } else 
        diagnostics(4, "entering CmdPicture");

    picture = getTexUntil(post, 0);

    PrepareDisplayedBitmap("latex picture");
    WriteLatexAsBitmapOrEPS("\\begin{picture}", picture, post, BITMAP);
    FinishDisplayedBitmap();

    ConvertString(post);    /* to balance the \begin{picture} */
    safe_free(picture);
}

/******************************************************************************
  purpose: handle \begin{tikzpicture} ... \end{tikzpicture}
           by converting to png image and inserting
 ******************************************************************************/
void CmdTikzPicture(int code)
{
    char *picture = NULL;
    char post[] = "\\end{tikzpicture}";

    if (!(code & ON)) {
        diagnostics(4, "exiting CmdTikzPicture");
        return;
    } else 
        diagnostics(4, "entering CmdTikzPicture");

    picture = getTexUntil(post, 0);

    PrepareDisplayedBitmap("picture");
    if (g_figure_comment_converted) {
      WriteLatexAsBitmapOrEPS("\\begin{tikzpicture}", picture, post, EPS);
    } else {
      WriteLatexAsBitmapOrEPS("\\begin{tikzpicture}", picture, post, BITMAP);
    }
    FinishDisplayedBitmap();

    ConvertString(post);    /* to balance \begin{tikzpicture} */
    safe_free(picture);
}


/******************************************************************************
  purpose: Process \begin{music} ... \end{music} environment
 ******************************************************************************/
void CmdMusic(int code)
{
    char *contents = NULL;
    char endmusic[] = "\\end{music}";

    if (!(code & ON)) {
        diagnostics(4, "exiting CmdMusic");
        return;
    } else 
        diagnostics(4, "entering CmdMusic");

    diagnostics(4, "entering CmdMusic");
    contents = getTexUntil(endmusic, TRUE);

    PrepareDisplayedBitmap("music");
    WriteLatexAsBitmapOrEPS("\\begin{music}", contents, endmusic, BITMAP);
    FinishDisplayedBitmap();

    ConvertString(endmusic);             /* to balance the \begin{music} */
    safe_free(contents);
}

void CmdTikzlib(int code) 
{
    char *tikzlib = getBraceParam();
    tikzlibsnum++;
    if (tikzlibsnum<32)
    	tikzlibs[tikzlibsnum-1]=tikzlib;
}
