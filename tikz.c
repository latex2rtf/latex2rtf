/* tikz.c - routines that handle LaTeX TikZ commands

Copyright (C) 2013 The Free Software Foundation

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
    2013 Vadim Kuznetsov
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include "cfg.h"
#include "main.h"
#include "graphics.h"
#include "tikz.h"
#include "parser.h"
#include "utils.h"
#include "commands.h"
#include "convert.h"
#include "equations.h"
#include "funct1.h"

static char *tikzlibs[32];
static int tikzlibsnum = 0;

void TikzToPng(char *tikzcode, char *exts);

/******************************************************************************
  purpose: handle \begin{tikzpicture} ... \end{tikzpicture}
           by converting to png image and inserting
 ******************************************************************************/
void CmdTikzPicture2(int code)
{
	char *pre, *post, *tikzcode, *param;	

	param = getBracketParam();

	if (code & ON) {
		pre = strdup("\\begin{tikzpicture}");
		post = strdup("\\end{tikzpicture}");
		tikzcode=getTexUntil(post,0);
		TikzToPng(tikzcode,param);
		ConvertString(post);  /* to balance \begin{tikzpicture} */
		free(pre);
		free(post);
		free(tikzcode);
		free(param);
	}
}

void TikzToPng(char *tikzcode,char *exts)
{
    char *fullname, *tmp_dir, *texname, *auxname, *logname, *pdfname, *pngname, *destname;
    char name[15], *cmd, *oldcwd;

    FILE *f;
	static int file_number = 0;
    int i, cmd_len, err;

	file_number++;

    tmp_dir = getTmpPath();
	snprintf(name,15,"tikz_%04d",file_number);
	fullname = strdup_together(tmp_dir,name);

	texname = strdup_together(fullname,".tex");
	pdfname = strdup_together(fullname,".pdf");
	auxname = strdup_together(fullname,".aux");
	logname = strdup_together(fullname,".log");

	fprintf(stderr, "\ntikz file name ='%s'\n", texname);

	f = fopen(texname, "w");

    fprintf(f,"\\documentclass[varwidth=true,border=10pt]{standalone}\n");
    fprintf(f,"\\usepackage{mathtext}\n");
/*
    fprintf(f,"\\usepackage[T2A]{fontenc}\n");
    fprintf(f,"\\usepackage[%s]{inputenc}\n",g_charset_encoding_name);
    fprintf(f,"\\usepackage[russian]{babel}\n"); 
*/
    fprintf(f,"\\usepackage{tikz}\n");
/*	fprintf(f,"\\usepackage{gnuplot-lua-tikz}\n");*/

    fprintf(f,"\\begin{document}\n");

	for (i=0;i<tikzlibsnum;i++)
	    fprintf(f,"\\usetikzlibrary{%s}\n",tikzlibs[i]);

    if (exts!=NULL)
        fprintf(f,"\\begin{tikzpicture}[%s]\n",exts);
	else
        fprintf(f,"\\begin{tikzpicture}\n");

	fprintf(f,"\n%s\n",tikzcode);

	fprintf(f,"\\end{tikzpicture}\n");
	fprintf(f,"\\end{document}\n");
    fclose(f);


/*
void PutLatexFile(const char *tex_file_stem, double scale, const char *pre, conversion_t convertTo, int hinline);

    cmd_len = strlen("pdflatex") + strlen(texname) + 32 + strlen(" >/dev/null");
	cmd = (char *)malloc(cmd_len);

    snprintf(cmd, cmd_len, "pdflatex '%s' >/dev/null", texname);

    oldcwd = (char *) malloc(1024);
	getcwd(oldcwd,1024);
	chdir(tmp_dir);
	err = system(cmd);
	chdir(oldcwd);
	free(cmd);
*/
    /* convert the pdf to a png and insert */
    
    if (0 && !err) PutPdfFile(pdfname,0,0,g_png_figure_scale,0);
/*
    remove(texname);
	remove(auxname);
	remove(pdfname);
	remove(logname);

    free(oldcwd);
    free(fullname);
*/
	free(texname);
	free(auxname);
	free(pdfname);
	free(logname);
	free(tmp_dir);
}

void CmdTikzlib(int code) 
{
    char *tikzlib = getBraceParam();
    tikzlibsnum++;
    tikzlibs[tikzlibsnum-1]=tikzlib;
}
