/* $Id: cfg.h,v 1.14 2002/04/21 22:49:59 prahl Exp $*/

#ifndef __CFG_H
#define __CFG_H

typedef int (*fptr) (const void*, const void*);

typedef struct ConfigEntryT
{
   /*@shared@*/ const char  *TexCommand;
   /*@shared@*/ const char  *RtfCommand;
} ConfigEntryT;

extern void ReadCfg (void)
/*@globals configinfo@*/
/*@modifies configinfo@*/
;

extern size_t SearchRtfIndex (const char *theCommand, int WhichArray);
extern char *SearchRtfCmd (const char *theCommand, int WhichArray);
extern ConfigEntryT **CfgStartIterate (int WhichCfg);
extern ConfigEntryT **CfgNext (int WhichCfg, ConfigEntryT **last);


/* Values for WhichArray */
#define DIRECT_A	0
#define FONT_A		1
#define IGNORE_A	2
#define LANGUAGE_A      3

extern void ReadLanguage(char *lang);
void ConvertBabelName(char *name);

#ifndef CFGDIR
#define CFGDIR ""
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#endif /* ndefined __CFG_H */
