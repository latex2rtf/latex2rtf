#ifndef _INCLUDED_RTF_H
#define _INCLUDED_RTF_H 1

#include "labels.h"

void referenceBookmark(/* char *post, */
               char *type,
               char *signet,
               char *text,
               referenceGetter *getRef);

void pageBookmark(/* char *post, */
          char *type,
          char *signet
          /* , char *text,
             referenceGetter *getRef */);

void InsertReference(char *text,
             referenceGetter *getRef);

void emitBookmark(char *extra, char *presignet,char *signet,char *text);

#endif
