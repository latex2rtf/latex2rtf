/* $Id: cfg.c,v 1.16 2001/10/12 05:45:07 prahl Exp $

     purpose : Read config files and provide lookup routines

 * LEG200698 I would have prefered to make the reading of the language file
 * separate, since the language is known some steps after reading the
 * configuration files. Since the search functions rely on the index into
 * configinfo this is not trivial. So I reread the language file to the array
 * at the moment the name is known.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "convert.h"
#include "funct1.h"
#include "cfg.h"
#include "util.h"

typedef struct ConfigInfoT {
	 /* @observer@ */ char *filename;
	 /* @owned@ *//* @null@ */ ConfigEntryT **config_info;
	size_t          config_info_size;
	bool            remove_leading_backslash;
}               ConfigInfoT;

static ConfigInfoT configinfo[] =
{
	{"direct.cfg", NULL, 0, FALSE},
	{"fonts.cfg", NULL, 0, FALSE},
	{"ignore.cfg", NULL, 0, FALSE},
	{"english.cfg", NULL, 0, FALSE},

};
#define CONFIG_SIZE (sizeof(configinfo) / sizeof(ConfigInfoT))

static int 
cfg_compare(ConfigEntryT ** el1, ConfigEntryT ** el2)
/****************************************************************************
 * purpose:  compare-function for bsearch
 * params:   el1, el2: Config Entries to be compared
 ****************************************************************************/
{
	return strcmp((*el1)->TexCommand, (*el2)->TexCommand);
}

/*
 * LEG240698
 * Tries to open the config file NAME. First all of the paths
 * specified in RTFPATH are searched. Then the precompiled default
 * path is searched.
 * At compiletime the defaults for the pathseparator end the separator
 * between directory names in the path can be overridden to match
 * e.g. MS-DOS naming conventions: ENVSEP = ';' PATHSEP = '\\'
 * leading or trailing ENVSEP's are allowed and ignored, trailing
 * PATHSEP's are allowed and ignored too.
 */
FILE           *
open_cfg(const char *name)
/****************************************************************************
purpose: open config files specified in name
params:  name: config-file-name
 ****************************************************************************/
{
	char           *cfg_path = getenv("RTFPATH");
	 /* @only@ */ static char *path = NULL;
	static size_t   size = BUFFER_INCREMENT;
	size_t          len;
	FILE           *fp;

	diagnostics(4, "RTFPATH=`%s'", cfg_path);
	if (path == NULL && (path = (char *) malloc(size)) == NULL) {
		fprintf(stderr, "%s: Fatal Error: Cannot allocate memory\n", progname);
		exit(EXIT_FAILURE);
	}
	if (cfg_path != NULL) {
		char           *s, *t;
		s = cfg_path;
		while (s != NULL && *s != '\0') {
			size_t          pathlen = 0;
			t = s;
			s = strchr(s, ENVSEP);
			if (s) {/* found */
				pathlen = (size_t) (s - t);
				s++;
			} else {/* ENVSEP not found *//* could be last path
				 * in string */
				if (strlen(t) != 0)
					pathlen = strlen(t);
			}
			if ((len = (pathlen + strlen(name) + 2)) > size) {
				size = len;
				if ((path = realloc(path, size)) == NULL) {
					Fatal("Cannot allocate memory for cfg filename");
				}
			}
			strncpy(path, t, pathlen);

			/* now fix up string ending */
			t = &path[pathlen - 1];
			if (*t++ != PATHSEP)
				*t++ = PATHSEP;
			*t = '\0';

			strcat(path, name);
			diagnostics(4, "Trying to open config: %s", path);
			if ((fp = fopen(path, "r")) != NULL) {
				diagnostics(4, "Opened config file %s", path);
				return (fp);
			}
		}
	}
	if ((len = (strlen(LIBDIR) + strlen(name) + 2)) > size) {
		size = len;
		if ((path = (char *) realloc(path, size)) == NULL) {
			fprintf(stderr, "%s: Fatal Error: Cannot allocate memory\n",
				progname);
			exit(EXIT_FAILURE);
		}
	}
	strcpy(path, LIBDIR);
	if (strlen(path) > 2 && path[strlen(path) - 1] != PATHSEP) {	/* append PATHSEP if
									 * needed */
		int             pathlen = strlen(path);
		path[pathlen] = PATHSEP;
		path[pathlen+1] = '\0';
	}
	strcat(path, name);
	if ((fp = fopen(path, "r")) == NULL) {
		diagnostics(4, "Path: %s", path);
		diagnostics(4, "cfg-Path: %s", cfg_path);
		diagnostics(ERROR, "cannot open config file '%s'", name);
	}
	diagnostics(4, "Opened default config file %s", path);

	return (fp);
}


/***/
static size_t 
read_cfg(FILE * cfgfile
	 ,ConfigEntryT *** pointer_array
	 ,bool do_remove_backslash
)
/****************************************************************************
 * purpose: Read config file and provide sorted lookup table
 ****************************************************************************/
/* @modifies pointer_array@ */
{
	size_t          bufindex = 0, bufsize = 0;
	char           *line, *cmdend;

	if (*pointer_array == NULL) {
		if ((*pointer_array = malloc(BUFFER_INCREMENT * sizeof(char *))) == NULL) {
			Fatal("Cannot allocate memory for pointer list\n");
		}
		bufsize = BUFFER_INCREMENT;
	}
	while ((line = ReadUptoMatch(cfgfile, "\n")) != NULL) {
		(void) getc(cfgfile);	/* skip newline */
		/* Skip leading white space */
		while (isspace((unsigned char) *line)) {
			line++;
		}
		if (*line == '#' || *line == '\0') {
			continue;
		}
		cmdend = strrchr(line, '.');
		if (cmdend == NULL) {
			ParseError("Bad config file, missing final period\nBad line is \"%s\"", line);
		}
		*cmdend = '\0';
		if (do_remove_backslash) {
			if (*line != '\\') {
				ParseError("Bad config file, missing initial'\\'\nBad line is\"%s\"", line);
			} else {
				line++;
			}
		}
		if (bufindex >= bufsize) {
			/*
			 * LEG210698*** Here we know, that pointer array is
			 * not null! What to do with the second allocation?
			 */
			if ((*pointer_array
			     = malloc((bufsize += BUFFER_INCREMENT) * sizeof(char *))
			     ) == NULL
				) {
				Fatal("Cannot allocate memory for pointer list\n");
			}
		}
		line = strdup(line);
		cmdend = strchr(line, ',');
		if (cmdend == NULL) {
			ParseError("Bad config file, missing ',' between elements\nBad line is\"%s\"", line);
		}
		*cmdend++ = '\0';

		if (((*pointer_array)[bufindex] = malloc(sizeof(ConfigEntryT))) == NULL) {
			Fatal("Cannot allocate memory for config entry\n");
		}
		(*pointer_array)[bufindex]->TexCommand = line;
		(*pointer_array)[bufindex]->RtfCommand = cmdend;
		bufindex++;
	}
	qsort(*pointer_array
	      ,bufindex
	      ,sizeof(**pointer_array)
	      ,(fptr) cfg_compare
		);
	return bufindex;
}


void 
ReadCfg(void)
/****************************************************************************
 * purpose: opens config-files & reads them
 * globals: Direct-, Font- IgnoreArray[Size/Root]
 ****************************************************************************/
{
	size_t          i;
	FILE           *fp;

	for (i = 0; i < CONFIG_SIZE; i++) {
		linenumber = 1;
		currfile = configinfo[i].filename;
		fp = open_cfg(configinfo[i].filename);
		configinfo[i].config_info_size
			= read_cfg(fp
				   ,&(configinfo[i].config_info)
				   ,configinfo[i].remove_leading_backslash
			);
		(void) fclose(fp);
	}
}

static ConfigEntryT **
search_rtf(const char *theTexCommand, int WhichCfg)
/****************************************************************************
 * purpose:  search theTexCommand in specified config data and return
 *           pointer to the data
 ****************************************************************************/
{
	ConfigEntryT    compare_item;
	ConfigEntryT   *compare_ptr;

	compare_item.TexCommand = theTexCommand;
	compare_item.RtfCommand = "";
	compare_ptr = &compare_item;

	assert(WhichCfg >= 0 && (size_t) WhichCfg < CONFIG_SIZE);
	assert(configinfo[WhichCfg].config_info != NULL);

	return (ConfigEntryT **) bsearch
		(&compare_ptr
		 ,configinfo[WhichCfg].config_info
		 ,configinfo[WhichCfg].config_info_size
		 ,sizeof(compare_ptr)
		 ,(fptr) cfg_compare
		);
}

size_t 
SearchRtfIndex(const char *theTexCommand, int WhichCfg)
/****************************************************************************
 * purpose:  search theTexCommand in a specified config data and return
 *           index
 ****************************************************************************/
{
	ConfigEntryT  **help = search_rtf(theTexCommand, WhichCfg);
	if (help == NULL) {
		return 0;
	}
	/* LEG210698*** subtraction of two ConfigEntryT pointers */
	return help - configinfo[WhichCfg].config_info;
}

char     *
SearchRtfCmd(const char *theTexCommand, int WhichCfg)
/****************************************************************************
 * purpose:  search theTexCommand in a specified config data and return
 *           pointer to the data
 ****************************************************************************/
{
	ConfigEntryT  **help;
	
	help = search_rtf(theTexCommand, WhichCfg);
	
	if (help == NULL)
		return NULL;
	else
		return (char *)(*help)->RtfCommand;
}

ConfigEntryT **
CfgStartIterate(int WhichCfg)
/****************************************************************************
 * purpose:  Start iterating of configuration data
 ****************************************************************************/
{
	return NULL;
}

ConfigEntryT **
CfgNext(int WhichCfg, ConfigEntryT ** last)
/****************************************************************************
 * purpose:  Get the next entry from specified configuration data
 ****************************************************************************/
{
	if (last == NULL) {
		return (ConfigEntryT **) configinfo[WhichCfg].config_info;
	}
	last++;
	if (last
	    > (ConfigEntryT **) configinfo[WhichCfg].config_info
	    + configinfo[WhichCfg].config_info_size
	    - 1
		) {
		return NULL;
	}
	return last;
}

/****************************************************************************
 * opens and reads the language configuration file named in lang

Opens language file & builds a search tree for the translation of
"Hardcoded" latex headings like "Part", "References", ...
The file format is:
LATEXTOKEN,Translation.

 ****************************************************************************/
void
ReadLanguage(char *lang)
{
	FILE           *fp;
	char           *langfn;

	langfn = malloc(strlen(lang) + strlen(".cfg"));
	if (langfn == NULL)
		diagnostics(ERROR, "Could not allocate memory for language filename.");

	strcpy(langfn, lang);
	strcat(langfn, ".cfg");

	fp = open_cfg(langfn);
	free(langfn);

	configinfo[LANGUAGE_A].config_info_size
		= read_cfg(fp,
			   &(configinfo[LANGUAGE_A].config_info),
			   configinfo[LANGUAGE_A].remove_leading_backslash);

	(void) fclose(fp);
}

/****************************************************************************
 purpose : returns a pointer to the Printout name of a Heading, since
           this is read from a language file it provides translation
           capability.
 params  : name, name of heading.
 ****************************************************************************/
void
ConvertBabelName(char *name)
{
	char *s = SearchRtfCmd(name, LANGUAGE_A);
	if (s != NULL)
		ConvertString(s);
}
