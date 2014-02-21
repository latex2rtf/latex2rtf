
/* cfg.c - Read config files and provide lookup routines

Copyright (C) 1995-2002 The Free Software Foundation

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

Authors:
    1995-1997 Ralf Schlatterbeck
    1998-2000 Georg Lehner
    2001-2002 Scott Prahl

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
#include "utils.h"

typedef struct ConfigInfoT {
    char *filename;
    ConfigEntryT **config_info;
    int config_info_size;
    int remove_leading_backslash;
} ConfigInfoT;

static ConfigInfoT configinfo[] = {
    {"direct.cfg", NULL, 0, FALSE},
    {"fonts.cfg", NULL, 0, FALSE},
    {"ignore.cfg", NULL, 0, FALSE},
    {"style.cfg", NULL, 0, FALSE},
    {"english.cfg", NULL, 0, FALSE},
};

#define CONFIG_SIZE 5
#define BUFFER_INCREMENT 1024

char *ReadUptoMatch(FILE * infile, const char *scanchars);

/****************************************************************************
 * purpose:  compare-function for bsearch
 * params:   el1, el2: Config Entries to be compared
 ****************************************************************************/
static int cfg_compare(ConfigEntryT ** el1, ConfigEntryT ** el2)
{
/*diagnostics(1, "'%s'<=>'%s'", (**el1).TexCommand, (**el2).TexCommand);*/
    return strcmp((**el1).TexCommand, (**el2).TexCommand);
}

/****************************************************************************
 * purpose:  append path to .cfg file name and open
             return NULL upon failure,
             return filepointer otherwise
 ****************************************************************************/
static FILE *try_path(const char *path, const char *cfg_file)
{
    char *both;
    FILE *fp;
    char separator[2];
    
    separator[0] = PATHSEP;
    separator[1] = '\0';
    
    if (path == NULL || cfg_file == NULL)
        return NULL;

    /* fix path ending if needed */
    if (path[strlen(path)] != PATHSEP) 
        both = strdup_together3(path,separator,cfg_file);
    else
        both = strdup_together(path,cfg_file);

    diagnostics(2, "trying to open '%s'", both);

    fp = fopen(both, "rb");
    free(both);
    return fp;
}

/*****************************************************
 * run through 'path' (from environment, e.g. $PATH)
 * try to find a file called 'name' 
 * if 'subdir' is given, prepend it to 'name'
 *****************************************************/
static FILE *open_path(const char *path, const char* name, const char* subdir)
{
    FILE *fp = NULL;
    char *pf = strdup(path);
    char *p  = pf;
    
    while (NULL != p) {
        char *p1 = strchr(p, ENVSEP);
        if (NULL != p1)
            *p1 = '\0';
        if (NULL != subdir) {
            char *pft = strdup_together(p, subdir);
            fp = try_path(pft, name);
            free(pft);
        } else {
            fp = try_path(p, name);
        }               
        if (NULL != fp)
            break;
        
        p = (p1) ? p1 + 1 : NULL;
    }
    free(pf);
    return fp;
}
 
/*
 * if the environment variable 'env' is defined
 * try to find a file called 'name' using it's contents
 * as a path. if subdir is given prepend it to 'name'
 */
static FILE *open_env(const char *env, const char* name, const char*subdir)
{
    FILE  *result = NULL;
    char *p = getenv(env);
    
    if (NULL != p) 
        result = open_path(p,name,subdir);

    return result;
}


FILE *open_cfg(const char *name, int quit_on_error)
 /****************************************************************************
 purpose: open config by trying multiple paths
  ****************************************************************************/
 {
    FILE *fp;

/* try path specified on the line */
    fp = try_path(g_config_path, name);
    if (fp)
        return fp;
 
  /* try the environment variable RTFPATH */
    fp = open_env("RTFPATH", name, NULL);
    if (NULL != fp) return fp;
  
  /* try the environment variable PROGRAMFILES */
    fp = open_env("PROGRAMFILES", name, "/latex2rtf/cfg");
    if (NULL != fp) return fp;
  
  /* last resort.  try CFGDIR */
    fp = open_path(CFGDIR, name, NULL);
    if (NULL != fp) return fp;
 
/* failed ... give some feedback */
    if (quit_on_error) {
        diagnostics(WARNING, "Cannot open the latex2rtf config file '%s'",name);
        diagnostics(WARNING, "Locate the directory containing the .cfg files and");
        diagnostics(WARNING, "   (1) define the environment variable RTFPATH, *or*");
        diagnostics(WARNING, "   (2) use command line path option \"-P /path/to/cfg/file\", *or*");
        diagnostics(WARNING, "   (3) recompile latex2rtf with CFGDIR defined properly");
        diagnostics(WARNING, "Current RTFPATH: %s", getenv("RTFPATH"));
        diagnostics(WARNING, "Current  CFGDIR: %s", CFGDIR);
        diagnostics(ERROR, " Giving up.  Please don't hate me.");
    }
    return NULL;
}

static int read_cfg(FILE * cfgfile, ConfigEntryT *** pointer_array, int do_remove_backslash)

/****************************************************************************
 * purpose: Read config file and provide sorted lookup table
 ****************************************************************************/
{
    int bufindex = 0, bufsize = 0;
    char *line, *cmdend;

    if (*pointer_array == NULL) {
        *pointer_array = (struct ConfigEntryT **) malloc(BUFFER_INCREMENT * sizeof(char *));
        bufsize = BUFFER_INCREMENT;
        if (*pointer_array == NULL)
            diagnostics(ERROR, "Cannot allocate memory for pointer list");
    }

    while ((line = ReadUptoMatch(cfgfile, "\n")) != NULL) {

        /* skip newline */
        getc(cfgfile);

        /* Skip leading white space */
        while (isspace((unsigned char) *line))
            line++;

        /* Skip comment line */
        if (*line == '#')
            continue;

        /* Skip blank line */
        if (*line == '\0')
            continue;

        /* Find period that terminates command */
        cmdend = strrchr(line, '.');
        if (cmdend == NULL){
            diagnostics(ERROR, "Bad config file, missing final period\nBad line is \"%s\"", line);
            exit(1);
        }

        /* Replace period with NULL */
        *cmdend = '\0';

        /* Skip backslash if specified */
        if (do_remove_backslash) {
            if (*line != '\\')
                diagnostics(ERROR, "Bad config file, missing initial'\\'\nBad line is\"%s\"", line);
            else
                line++;
        }

        /* resize buffer if needed */
        if (bufindex >= bufsize) {
            bufsize += BUFFER_INCREMENT;
            *pointer_array = (struct ConfigEntryT **) realloc(*pointer_array, bufsize * sizeof(char *));
            if (*pointer_array == NULL)
                diagnostics(ERROR, "Cannot allocate memory for pointer list");
        }

        /* find start of definition */
        line = strdup(line);
        cmdend = strchr(line, ',');
        if (cmdend == NULL) {
            diagnostics(ERROR, "Bad config file, missing ',' between elements\nBad line is\"%s\"", line);
            exit(1);
        }

        /* terminate command */
        *cmdend = '\0';

        (*pointer_array)[bufindex] = (struct ConfigEntryT *) malloc(sizeof(ConfigEntryT));

        if ((*pointer_array)[bufindex] == NULL)
            diagnostics(ERROR, "Cannot allocate memory for config entry");

        (*pointer_array)[bufindex]->TexCommand = line;
        (*pointer_array)[bufindex]->RtfCommand = cmdend + 1;
        (*pointer_array)[bufindex]->original_id = bufindex;
        bufindex++;
        diagnostics(6,"%3d Tex='%s' RTF='%s'", bufindex, line, cmdend+1);
    }

    qsort(*pointer_array, bufindex, sizeof(**pointer_array), (fptr) cfg_compare);

    return bufindex;
}

void ReadCfg(void)

/****************************************************************************
 * purpose: opens config-files & reads them
 * globals: Direct-, Font- IgnoreArray[Size/Root]
 ****************************************************************************/
{
    int i;
    FILE *fp;
    char *fname;

    for (i = 0; i < CONFIG_SIZE; i++) {
        fname = configinfo[i].filename;
        fp = (FILE *) open_cfg(fname, TRUE);

        configinfo[i].config_info_size = read_cfg(fp, &(configinfo[i].config_info)
          , configinfo[i].remove_leading_backslash);
        (void) fclose(fp);
        
         diagnostics(2, "read %d entries for file %s", configinfo[i].config_info_size, fname);
       
    }
}

ConfigEntryT **SearchCfgEntry(const char *theTexCommand, int WhichCfg)

/****************************************************************************
 * purpose:  search theTexCommand in specified config data and return
 *           pointer to the data
 ****************************************************************************/
{
    ConfigEntryT compare_item;
    ConfigEntryT *compare_ptr, **p, **base;
    int size;
    
    compare_item.TexCommand = theTexCommand;
    compare_item.RtfCommand = "";
    compare_item.original_id= 0;
    compare_ptr = &compare_item;
    
    size = configinfo[WhichCfg].config_info_size;
    base = configinfo[WhichCfg].config_info;
    
    if (theTexCommand == NULL) return NULL;

    assert(WhichCfg >= 0 &&  WhichCfg < CONFIG_SIZE);
    assert(configinfo[WhichCfg].config_info != NULL);

    diagnostics(5, "seeking '%s' in %d of size %d  ", theTexCommand, WhichCfg, size);
    p = (ConfigEntryT **) bsearch(&compare_ptr, base, size, sizeof(compare_ptr), (fptr) cfg_compare);
    
    if (p)
        diagnostics(5, "seeking '%s'  found '%s'", theTexCommand, (**p).TexCommand);
    return p;
}

ConfigEntryT **SearchCfgEntryByID(const int id, int WhichCfg)

/****************************************************************************
 * purpose:  Get the entry with the given id
 ****************************************************************************/
{
    int i, max;
    ConfigEntryT ** entry;
            
    max = configinfo[WhichCfg].config_info_size;
    if (id > (int) max) return NULL;

    /* now iterate through all the entries looking for the right one */
    entry = (ConfigEntryT **) configinfo[WhichCfg].config_info;
    for (i=0; i<max; i++) {
        if ( id == (**entry).original_id ) return entry;
        entry++;
    }
    
    /* not found, should not be reached */
    return NULL;
}

char *SearchCfgRtf(const char *theTexCommand, int WhichCfg)

/****************************************************************************
 * purpose:  search theTexCommand in a specified config data and return
 *           pointer to the corresponding RTF string
 ****************************************************************************/
{
    ConfigEntryT **p;

    p = SearchCfgEntry(theTexCommand, WhichCfg);

    if (p == NULL)
        return NULL;
    else
        return (char *) (**p).RtfCommand;
}

ConfigEntryT **CfgStartIterate(void)

/****************************************************************************
 * purpose:  Start iterating of configuration data
 ****************************************************************************/
{
    return NULL;
}

ConfigEntryT **CfgNext(int WhichCfg, ConfigEntryT ** last)

/****************************************************************************
 * purpose:  Get the next entry from specified configuration data
 ****************************************************************************/
{
    if (last == NULL) 
        return (ConfigEntryT **) configinfo[WhichCfg].config_info;
    
    last++;
    if (last > (ConfigEntryT **) configinfo[WhichCfg].config_info + configinfo[WhichCfg].config_info_size - 1) 
        return NULL;
    
    return last;
}

ConfigEntryT **CfgNextByInsertion(int WhichCfg, ConfigEntryT ** last)

/****************************************************************************
 * purpose:  Get the next entry from specified configuration data
 ****************************************************************************/
{
    int next_id;
    
    if (last == NULL)
        next_id = 0;
    else
        next_id = (**last).original_id + 1;
        
    return SearchCfgEntryByID(next_id, WhichCfg);
}

/****************************************************************************
 * opens and reads the language configuration file named in lang

Opens language file & builds a search tree for the translation of
"Hardcoded" latex headings like "Part", "References", ...
The file format is:
LATEXTOKEN,Translation.

 ****************************************************************************/
void ReadLanguage(char *lang)
{
    FILE *fp;
    char *langfn;

	if (lang == NULL) return;
    langfn = strdup_together(lang, ".cfg");

    fp = (FILE *) open_cfg(langfn, TRUE);
    free(langfn);
    if (fp == NULL) return;

    configinfo[LANGUAGE_A].config_info_size
      = read_cfg(fp, &(configinfo[LANGUAGE_A].config_info), configinfo[LANGUAGE_A].remove_leading_backslash);

    (void) fclose(fp);
}

/****************************************************************************
 purpose : returns a pointer to the Printout name of a Heading, since
           this is read from a language file it provides translation
           capability.
 params  : name, name of heading.
 ****************************************************************************/
void ConvertBabelName(char *name)
{
    char *s = SearchCfgRtf(name, LANGUAGE_A);

    if (s != NULL)
        ConvertString(s);
}

char *GetBabelName(char *name)
{
    char *s = NULL;  
    s = SearchCfgRtf(name, LANGUAGE_A);
    return s;
}


static char *buffer = NULL;
static int bufsize = 0;

#define CR (char) 0x0d
#define LF (char) 0x0a

/*
 * This function assumes there are no '\0' characters in the input.
 * if there are any, they are ignored.
 */
char *ReadUptoMatch(FILE * infile, const char *scanchars)
{
    int bufindex = 0;
    int c;

    if (feof(infile) != 0)
        return NULL;

    if (buffer == NULL) {
        buffer = (char *) malloc(BUFFER_INCREMENT * sizeof(char));
        if (buffer == NULL) {
            diagnostics(ERROR, "Cannot allocate memory for input buffer");
            exit(1);
        }
        bufsize = BUFFER_INCREMENT;
    }

    while ((c = getc(infile)) != EOF) {

        if (c == CR || c == LF)
            c = '\n';

        if (strchr(scanchars, c))
            break;

        if (c == (int) '\0')
            continue;

        buffer[bufindex++] = (char) c;
        if (bufindex >= bufsize) {
            bufsize += BUFFER_INCREMENT;
            buffer = (char *) realloc(buffer, bufsize);
            if (buffer == NULL) {
                diagnostics(ERROR, "Cannot allocate memory for input buffer");
                exit(1);
            }
        }
    }
    buffer[bufindex] = '\0';
    if (c != EOF)
        ungetc(c, infile);

    return buffer;
}
