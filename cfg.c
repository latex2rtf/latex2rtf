/* $Id: cfg.c,v 1.22 2002/03/14 06:42:21 prahl Exp $

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
#include <stdarg.h>
#include <ctype.h>

#include "main.h"
#include "convert.h"
#include "funct1.h"
#include "cfg.h"
#include "util.h"

typedef struct ConfigInfoT {
	char           *filename;
	ConfigEntryT  **config_info;
	size_t          config_info_size;
	bool            remove_leading_backslash;
} ConfigInfoT;

static ConfigInfoT configinfo[] =
{
	{"direct.cfg", NULL, 0, FALSE},
	{"fonts.cfg", NULL, 0, FALSE},
	{"ignore.cfg", NULL, 0, FALSE},
	{"english.cfg", NULL, 0, FALSE},

};
#define CONFIG_SIZE (sizeof(configinfo) / sizeof(ConfigInfoT))
#define BUFFER_INCREMENT 1024

extern void     Fatal(const char *fmt,...);
void            ParseError(const char *fmt,...);
char           *ReadUptoMatch(FILE * infile, const char *scanchars);

static char * g_cfg_filename;

static int 
cfg_compare(ConfigEntryT ** el1, ConfigEntryT ** el2)
/****************************************************************************
 * purpose:  compare-function for bsearch
 * params:   el1, el2: Config Entries to be compared
 ****************************************************************************/
{
	return strcmp((*el1)->TexCommand, (*el2)->TexCommand);
}

FILE *
try_path(const char *path, const char *file)
{
	char * both;
	FILE * fp=NULL;
	int    lastchar;
		
	diagnostics(4, "trying path=<%s> file=<%s>", path, file);
	
	if (path==NULL || file == NULL) return NULL;

	lastchar = strlen(path);

	both = malloc(strlen(path) + strlen(file) + 2);
	if (both == NULL)
		diagnostics(ERROR, "Could not allocate memory for both strings.");

	strcpy(both, path);
	
	/* fix path ending if needed */
	if (both[lastchar] != PATHSEP) {
		both[lastchar] = PATHSEP;
		both[lastchar+1] = '\0';
	}
	
	strcat(both, file);
	fp = fopen(both, "r");
	free(both);
	return fp;
}

FILE *
open_cfg(const char *name)
/****************************************************************************
purpose: open config by trying multiple paths
 ****************************************************************************/
{
	char           *env_path,*p,*p1;
	char		   *lib_path;
	FILE           *fp;

/* try path specified on the line */
	fp=try_path(g_config_path, name);
	if (fp) return fp;
		
/* try the environment variable RTFPATH */
	p = getenv("RTFPATH");	
	if (p) {
		env_path = strdup(p);  /* create a copy to work with */
		p = env_path;
		while (p) {
			p1 = strchr(p, ENVSEP);
			if (p1) *p1 = '\0';
			
			fp=try_path(p, name);
			if (fp) {free(env_path); return fp;}
			
			p= (p1) ? p1+1 : NULL;
		}
		free(env_path);
	}		

/* last resort.  try LIBDIR */
	lib_path = strdup(LIBDIR);
	if (lib_path) {
		p = lib_path;
		while (p) {
			p1 = strchr(p, ENVSEP);
			if (p1) *p1 = '\0';
			
			fp=try_path(p, name);
			if (fp) {free(lib_path); return fp;}
			
			p= (p1) ? p1+1 : NULL;
		}
		free(lib_path);
	}

/* failed ... give some feedback */
	diagnostics(WARNING, "Cannot open the latex2rtf .cfg files");
	diagnostics(WARNING, "Locate the directory containing the .cfg files and");
	diagnostics(WARNING, "   (1) define the environment variable RTFPATH, *or*");
	diagnostics(WARNING, "   (2) use command line path option \"-P /path/to/cfg/file\", *or*");
	diagnostics(WARNING, "   (3) recompile latex2rtf with LIBDIR defined properly");
	diagnostics(WARNING, "Current RTFPATH: %s", getenv("RTFPATH"));
	diagnostics(WARNING, "Current  LIBDIR: %s", LIBDIR);
	diagnostics(ERROR,   " Giving up.  Have a nice day.");
	return NULL;
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
		g_cfg_filename = configinfo[i].filename;
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

	langfn = malloc(strlen(lang) + strlen(".cfg") + 1);
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

/* @exits@ */
void 
ParseError(const char *fmt,...)
{
	va_list         ap;

	fprintf(stderr, "%s: %s: ", progname, g_cfg_filename);
	va_start(ap, fmt);
	(void) vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

void 
Fatal(const char *fmt,...)
{
	va_list         ap;

	fprintf(stderr, "%s: Fatal error: ", progname);
	va_start(ap, fmt);
	(void) vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

static char *buffer;
static size_t   bufsize = 0;

#define CR (char) 0x0d
#define LF (char) 0x0a

/*
 * This function assumes there are no '\0' characters in the input.
 * if there are any, they are ignored.
 */
char           *
ReadUptoMatch(FILE * infile, /* @observer@ */ const char *scanchars)
{
	size_t          bufindex = 0;
	int             c;

	if (feof(infile) != 0) {
		return NULL;
	}
	if (buffer == NULL) {
		if ((buffer = malloc(BUFFER_INCREMENT)) == NULL) {
			Fatal("Cannot allocate memory for input buffer\n");
		}
		bufsize = BUFFER_INCREMENT;
	}
	while ((c = getc(infile)) != EOF ) {
	
		if (c == CR || c == LF)
			c = '\n';
		
		if (strchr(scanchars, c))
			break;
			
		if (c == (int) '\0') {
			continue;
		}
/*		if (c == (int) '\n') {
			linenumber++;
		}
*/		buffer[bufindex++] = (char) c;
		if (bufindex >= bufsize) {
			if ((buffer = realloc(buffer, bufsize += BUFFER_INCREMENT)) == NULL) {
				Fatal("Cannot allocate memory for input buffer\n");
			}
		}
	}
	buffer[bufindex] = '\0';
	if (c != EOF) {
		ungetc(c, infile);	/* LEG210698*** lclint, GNU libc
					 * doesn't say what's the return
					 * value */
	}
	return buffer;
}

