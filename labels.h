#ifndef _LABEL_H_INCLUDED
#define _LABEL_H_INCLUDED 1

enum { LABEL_NEW = 0,
       LABEL_UNDONEW } ;

typedef struct _labelElem {
    char *labelName;
    char *labelDef;
} labelElem;

labelElem *getLabel(char *name);

char *getLabelDefinition(char *name);
char *getLabelSection(char *name);
char *getLabelPage(char *name);
char *getLabelNameref(char *name);

void CmdNewLabel(int code);
#endif
