#ifndef _CFG_H_INCLUDED
#define _CFG_H_INCLUDED 1

#include <stdio.h> /* for FILE* */

#if defined(VMS) && defined(NEED_SNPRINTF)
#include <X11VMS/vmsutil.h>
#endif
 
enum { DIRECT_A = 0,
       FONT_A,
       IGNORE_A,
       STYLE_A,
       LANGUAGE_A };

#ifndef CFGDIR
#ifdef VMS
#define CFGDIR "L2RCFGDIR:"
#else
#define CFGDIR ""
#endif
#endif

 
typedef int (*fptr) (const void*, const void*);

typedef struct ConfigEntryT
{
   const char  *TexCommand;
   const char  *RtfCommand;
   int      original_id;
} ConfigEntryT;

void ReadLanguage(char *lang);
void ConvertBabelName(char *name);
char *GetBabelName(char *name);

void            ReadCfg (void);
ConfigEntryT  **SearchCfgEntry(const char *theTexCommand, int WhichCfg);
char           *SearchCfgRtf(const char *theCommand, int WhichArray);
ConfigEntryT  **SearchCfgEntryByID(const int id, int WhichCfg);
ConfigEntryT  **CfgStartIterate (void);
ConfigEntryT  **CfgNext (int WhichCfg, ConfigEntryT **last);
ConfigEntryT  **CfgNextByInsertion(int WhichCfg, ConfigEntryT ** last);

FILE *          open_cfg(const char *name, int quit_on_error);
#endif
