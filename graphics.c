/* $Id: graphics.c,v 1.1 2002/03/02 20:16:28 prahl Exp $ 
This file contains routines that handle LaTeX graphics commands
*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "graphics.h"
#include "parser.h"

void 
CmdGraphics(int code)
{
	char           *options;
	char            fullpath[1023];
	char           *filename;
	int             cc, i;
	short           top, left, bottom, right;
	FILE           *fp;

	/* could be \includegraphics*[0,0][5,5]{file.pict} */

	options = getBracketParam();
	if (options) free(options);

	options = getBracketParam();
	if (options) free(options);
	
	filename = getBraceParam();

	if (strstr(filename, ".pict") || strstr(filename, ".PICT")) {
		/* SAP fixes for Mac Platform */
#ifdef __MWERKS__
		{
		char           *dp;
		strcpy(fullpath, latexname);
		dp = strrchr(fullpath, ':');
		if (dp != NULL) {
			dp++;
			*dp = '\0';
		} else
			strcpy(fullpath, "");
		strcat(fullpath, filename);
		}
#else
		strcpy(fullpath,filename);
#endif

		fprintf(stderr, "processing picture %s\n", fullpath);
		fp = fopen(fullpath, "rb");

		if (fseek(fp, 514L, SEEK_CUR) ||     /* skip 512 byte header + 2 bytes for version info */
		    (fread(&top, 2, 1, fp) < 1) ||    /* read the pict file dimensions in points */
		    (fread(&left, 2, 1, fp) < 1) || 
			(fread(&bottom, 2, 1, fp) < 1) || 
			(fread(&right, 2, 1, fp) < 1) || 
			fseek(fp, -10L, SEEK_CUR)) {    /* move back ten bytes so that entire file will be encoded */
				free(filename);
				fclose(fp);
				return;
			}
		fprintRTF("\n{\\pict\\macpict\\picw%d\\pich%d\n", right - left, bottom - top);

		i = 0;
		while ((cc = fgetc(fp)) != EOF) {
			fprintRTF("%.2x", cc);
			if (++i > 126) {
				i = 0;
				fprintRTF("\n");
			}	/* keep lines 254 chars long */
		}

		fprintRTF("}\n");
		fclose(fp);
		free(filename);
	}
}


