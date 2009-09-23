#if defined(VMS) && defined(NEED_SNPRINTF)
#include <X11VMS/vmsutil.h>
#endif

#define DIRECT_A    0
#define FONT_A      1
#define IGNORE_A    2
#define STYLE_A     3
#define LANGUAGE_A  4

#ifndef CFGDIR
#ifdef VMS
#define CFGDIR "L2RCFGDIR:"
#else
#define CFGDIR ""
#endif
#endif

#ifndef TRUE
#define TRUE  1
#define FALSE 0
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
ConfigEntryT  **CfgStartIterate (int WhichCfg);
ConfigEntryT  **CfgNext (int WhichCfg, ConfigEntryT **last);
ConfigEntryT  **CfgNextByInsertion(int WhichCfg, ConfigEntryT ** last);

void *			open_cfg(const char *name, int quit_on_error);
