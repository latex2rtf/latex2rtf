/*
 * $Id: direct.c,v 1.11 2001/08/12 19:40:25 prahl Exp $
 * History:
 * $Log: direct.c,v $
 * Revision 1.11  2001/08/12 19:40:25  prahl
 * 1.9g
 *         Added commands to read and set TeX lengths
 *         Added commands to read and set TeX counters
 *         Fixed bug in handling of \item[text]
 *         Eliminated comparison of fpos_t variables
 *         Revised getLinenumber ... this is not perfect
 *         Fixed bug in getTexChar() routine
 *         Clearly separate preamble from the document in hopes that
 *           one day more appropriate values for page size, margins,
 *           paragraph spacing etc, will be used in the RTF header
 *         I have added initial support for page sizes still needs testing
 *         added two more test files misc3.tex and misc4.tex
 *         misc4.tex produces a bad rtf file currently
 *         separated all letter commands into letterformat.c
 *         cleaned up warning calls throughout code
 *         added \neq \leq \geq \mid commands to direct.cfg
 *         collected and added commands to write RTF header in preamble.c
 *         broke isolatin1 and hyperlatex support, these will be fixed next version
 *
 * Revision 1.7  1998/10/28 04:09:56  glehner
 * (WriteFontName): Cleaned up. Eliminated unecessary warning
 * and not completed rtf-output when using *Font*.
 *
 * Revision 1.6  1998/07/03 07:03:16  glehner
 * lclint cleaning
 *
 * Revision 1.5  1997/02/15 20:45:41  ralf
 * Some lclint changes and corrected variable declarations
 *
 * Revision 1.4  1995/03/23 15:58:08  ralf
 * Reworked version by Friedrich Polzer and Gerhard Trisko
 *
 *
 * Revision 1.3  1994/06/21  08:14:11  ralf
 * Corrected Bug in keyword search
 *
 * Revision 1.2  1994/06/17  14:19:41  ralf
 * Corrected various bugs, for example interactive read of arguments
 *
 * Revision 1.1  1994/06/17  11:26:29  ralf
 * Initial revision
 *
 */
/***************************************************************************
     name : direct.c
   author : DORNER Fernando, GRANZER Andreas
            POLZER Friedrich,TRISKO Gerhard
  * changed TryDirectConvert: use search on sorted array
  purpose : This file is used for converting LaTeX commands by simply text exchange
 ******************************************************************************/

/**********************************  includes ***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "direct.h"
#include "l2r_fonts.h"
#include "cfg.h"
/******************************************************************************/

/*************************** prototypes **************************************/

static bool     WriteFontName(const char **buffpoint, FILE * fRtf);

/******************************* defines *************************************/
#define MAXFONTLEN 100
/******************************************************************************/


/******************************************************************************/
bool 
WriteFontName(const char **buffpoint, FILE * fRtf)
/******************************************************************************
  purpose: reads from the font-array to write correct font-number into
           Rtf-File
parameter: buffpoint: font and number
	   fRtf: File-Pointer to Rtf-File
globals:   progname
 ******************************************************************************/
{
	char            buffer[MAXFONTLEN + 1];
	int             i;
	size_t          fnumber;

	if (**buffpoint == '*') {
		fprintf(fRtf, "*");
		return TRUE;
	}
	i = 0;
	while (**buffpoint != '*') {
		if ((i >= MAXFONTLEN) || (**buffpoint == '\0')) {
			fprintf(stderr, "\n%s: ERROR: Invalid fontname in direct command",
				progname);
			exit(EXIT_FAILURE);
		}
		buffer[i] = **buffpoint;
		i++;
		(*buffpoint)++;
	}
	buffer[i] = '\0';
	if ((fnumber = GetFontNumber(buffer)) < 0) {
		fprintf(stderr, "\n%s: ERROR: Unknown fontname in direct command", progname);
		fprintf(stderr, "\nprogram aborted\n");
		exit(EXIT_FAILURE);
	} else {
		fprintf(fRtf, "%u", (unsigned int) fnumber);
		return TRUE;
	}
}


/******************************************************************************
  purpose: reads from the direct-array how some easy LaTex-commands can be
	   converted into Rtf-commands by text exchange
parameter: command: LaTex-command and Rtf-command
	   fRtf: File-Pointer to Rtf-File
globals:   progname
 ******************************************************************************/
bool
TryDirectConvert(char *command, FILE * fRtf)
{
	const char     *buffpoint;
	const char     *RtfCommand;
	char            TexCommand[128];

	if (strlen(command) >= 100) {
		diagnostics(WARNING, "Command %s is too long (failed in TryDirectConvert)", command);
		return FALSE;	/* command too long */
	}
	TexCommand[0] = '\\';
	TexCommand[1] = '\0';
	strcat(TexCommand, command);

	RtfCommand = SearchRtfCmd(TexCommand, DIRECT_A);
	if (RtfCommand == NULL)
		return FALSE;

	buffpoint = RtfCommand;
	diagnostics(4, "Directly converting %s to %s", TexCommand, RtfCommand);
	while (buffpoint[0] != '\0') {
		if (buffpoint[0] == '*') {
			++buffpoint;
			(void) WriteFontName(&buffpoint, fRtf);

			/*
			 * From here on it is not necesarry if
			 * (WriteFontName(&buffpoint, fRtf)) {
			 * fprintf(stderr, "\n%s: WARNING: error in direct
			 * command file" " - invalid font name , \n",
			 * progname); return FALSE; }
			 */
		} else {
			fprintf(fRtf, "%c", *buffpoint);
		}

		++buffpoint;

	}			/* end while */
	return TRUE;
}
