#ifndef _BIBLIO_INCLUDED
#define _BIBLIO_INCLUDED 1

typedef enum { BIBLIO_BIBCITE = 0,
               BIBLIO_HARVARD,
               BIBLIO_NATBIB } BiblioType;

typedef struct _biblioElem {
    char       *biblioKey;
    char       *biblioN;
    char       *biblioFull;
    char       *biblioAbbr;
    char       *biblioYear;
    BiblioType  biblioType;
} biblioElem;


biblioElem *getBiblio(char *cite);
biblioElem *newBibCite(char *cite, char *tag);
biblioElem *newHarvardCite(char *cite,char *full,char* abbr,char *year);
biblioElem *newNatBibCite(char *cite,char *full,char* abbr,char *year,char *n);

char *getBiblioRef(char *key);
char *getBiblioFirst(char *key);
void CmdBibCite(int code);
void CmdAuxHarvardCite(int code);
#endif
