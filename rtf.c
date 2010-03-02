#include <stdio.h>
#include "main.h"
#include "utils.h"
#include "convert.h"
#include "labels.h"
#include "rtf.h"

/******************************************************************************
 purpose: emit RTF code for a bookmark
 ******************************************************************************/
void emitBookmark(char *extra, char *presignet,char *signet,char *text)
{
    fprintRTF ("{%s\\*\\bkmkstart %s%s}", extra,presignet,signet);
    ConvertString(text);
    fprintRTF ("{\\*\\bkmkend %s%s}", presignet,signet);
}
/******************************************************************************
 purpose: emit RTF code to reference a bookmark
 ******************************************************************************/
void InsertReference(char *text,
             referenceGetter *getRef)
{
    char *s = getRef(text);
    ConvertString(NULL != s ? s : "\\textbf{??}");
    safe_free(s);
}

/*
  referenceBookmark("REF","\\\\* MERGEFORMAT","BM",signet,text,getLabelSection);
  referenceBookmark("REF","\\\\* MERGEFORMAT","BIB_",signet,t,NULL);
  referenceBookmark("PAGEREF","\\\\p","BM",signet,text,NULL);
  referenceBookmark("PAGEREF","\\\\* MERGEFORMAT","BM",signet,text,NULL);
*/
static void callBookmark(char *pre,
             char *post,
             char *type,
             char *signet,
             char *text,
             referenceGetter *getRef)
{
    fprintRTF("{\\field{\\*\\fldinst{\\lang1024 %s %s%s %s }}",
          pre,type,signet,post);
    
    if (NULL != text) {
    fprintRTF ("{\\fldrslt{");
    if (NULL == getRef) {
        fprintRTF(text);
    } else {
        InsertReference(text,getRef);
    }
    fprintRTF("}}");
    }
    fprintRTF("}");
}

void referenceBookmark(char *type,
               char *signet,
               char *text,
               referenceGetter *getRef)
{
    callBookmark("REF","\\\\* MERGEFORMAT",type,signet,text,getRef);
}

void pageBookmark(char *type,
          char *signet)
{
    callBookmark("PAGEREF","\\\\* MERGEFORMAT",type,signet,NULL,NULL);
}
 
