/* $Id: direct.c,v 1.16 2001/10/12 05:45:07 prahl Exp $ 

  purpose : Convert simple LaTeX commands using direct.cfg
*/

#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "direct.h"
#include "l2r_fonts.h"
#include "cfg.h"

static bool     WriteFontName(const char **buffpoint, FILE * fRtf);

#define MAXFONTLEN 100

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
		fprintRTF("*");
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
	if ((fnumber = RtfFontNumber(buffer)) < 0) {
		fprintf(stderr, "\n%s: ERROR: Unknown fontname in direct command", progname);
		fprintf(stderr, "\nprogram aborted\n");
		exit(EXIT_FAILURE);
	} else {
		fprintRTF("%u", (unsigned int) fnumber);
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
			fprintRTF("%c", *buffpoint);
		}

		++buffpoint;

	}			/* end while */
	return TRUE;
}
