#ifndef _INCLUDED_XREFS_H
#define _INCLUDED_XREFS_H   1

#define FOOTNOTE            1
#define FOOTNOTE_TEXT       2
#define FOOTNOTE_THANKS     3
#define ENDNOTE             4
#define ENDNOTE_TEXT        5

#define LABEL_LABEL         1
#define LABEL_HYPERREF      2
#define LABEL_REF           3
#define LABEL_HYPERCITE     4
#define LABEL_CITE          5
#define LABEL_HYPERPAGEREF  6
#define LABEL_PAGEREF       7
#define LABEL_HTMLADDNORMALREF 8
#define LABEL_HTMLREF       9
#define LABEL_EQREF         10
#define LABEL_VREF          11
#define LABEL_HREF          12
#define LABEL_URL           13
#define LABEL_NO_LINK_URL   14
#define LABEL_BASE_URL      15
#define LABEL_URLSTYLE      16
#define LABEL_NAMEREF       17
#define LABEL_URL_HYPER     18

#define BIBSTYLE_STANDARD   1
#define BIBSTYLE_APALIKE    2
#define BIBSTYLE_APACITE    3
#define BIBSTYLE_NATBIB     4
#define BIBSTYLE_AUTHORDATE 5
#define BIBSTYLE_HARVARD    6

#define CITE_CITE           1
#define CITE_FULL           2
#define CITE_SHORT          3
#define CITE_CITE_NP        4
#define CITE_FULL_NP        5
#define CITE_SHORT_NP       6
#define CITE_CITE_A         7
#define CITE_FULL_A         8
#define CITE_SHORT_A        9
#define CITE_CITE_AUTHOR   10
#define CITE_FULL_AUTHOR   11
#define CITE_SHORT_AUTHOR  12
#define CITE_YEAR          13
#define CITE_YEAR_NP       14

#define CITE_APA_CITE_YEAR     201
#define CITE_APA_CITE_METASTAR 202
#define CITE_APA_CITE_A_TITLE  203
#define CITE_APA_CITE_B_TITLE  204
#define CITE_APA_CITE_INSERT   205
#define CITE_APA_YMD           206
#define CITE_APA_REF_A_TITLE   207
#define CITE_APA_REF_B_TITLE   208
#define CITE_APA_JVNP          209
#define CITE_APA_REF_YEAR      210
#define CITE_APA_ADD_PUB       211
#define CITE_PRINT_CARDINAL    212
#define CITE_PRINT_BACK_REFS   213
#define CITE_APA_ADD_PUB_EQ_AUTHOR 214
#define CITE_APA_REF_A_E_TITLE 215
#define CITE_APA_MONTH         216
#define CITE_APA_B_VOL_ED_TR   217
#define CITE_APA_ADD_INST      218
#define CITE_APA_HOW           219
#define CITE_APA_ORIG_YEAR_NOTE   220
#define CITE_APA_REF_NOTE      221
#define CITE_APA_REF_B_E_TITLE 223
#define CITE_APA_ORIG_JOUR     224
#define CITE_APA_B_VOL_ED_TR_PGS 225
#define CITE_APA_UNSKIP        226
#define CITE_PRINT_ORDINAL     227

#define CITE_T             16
#define CITE_T_STAR        17
#define CITE_P             18
#define CITE_P_STAR        19
#define CITE_ALT           20
#define CITE_ALP           21
#define CITE_ALT_STAR      22
#define CITE_ALP_STAR      23
#define CITE_TEXT          24
#define CITE_AUTHOR        25
#define CITE_AUTHOR_STAR   26
#define CITE_YEAR_P        27

#define CITE_T_CAP         128
#define CITE_P_CAP         129
#define CITE_ALT_CAP       130
#define CITE_ALP_CAP       131
#define CITE_AUTHOR_CAP    132

#define CITE_AS_NOUN            28
#define CITE_POSSESSIVE         29
#define CITE_AFFIXED            30
#define CITE_YEAR_STAR          31
#define CITE_HARVARD_ITEM       32
#define CITE_HARVARD_YEAR_LEFT  33
#define CITE_HARVARD_YEAR_RIGHT 34
#define CITE_NAME               35
#define CITE_HARVARD_AND        36

#define LIST_OF_FIGURES     1
#define LIST_OF_TABLES      2
#define TABLE_OF_CONTENTS   3

void InitializeBibliography(void);
void CmdTheEndNotes(int code);
void CmdFootNote(int code);
void CmdLabel(int code);
void CmdNoCite(int code);
void CmdBibliographyStyle(int code);
void CmdBibStyle(int code);
void CmdBibliography(int code);
void CmdThebibliography(int code);
void CmdBibitem(int code);
void CmdNewblock(int code);
void CmdIndex(int code);
void CmdPrintIndex(int code);
void CmdHtml(int code);
void InsertBookmark(char *name, char *text);
void InsertContentMark(char marker, char *s1, char *s2, char *s3);
void CmdCite(int code);
void CmdHarvardCite(int code);
void CmdBCAY(int code);
void CmdApaCite(int code);
void set_longnamesfirst(void);

void set_bibpunct_style_super(void);
void set_bibpunct_style_number(void);
void set_bibpunct_style_separator(char *s);
void set_bibpunct_style_paren(char *open, char *close);
void set_sorted_citations(void);
void set_compressed_citations(void);

void CmdCiteName(int code);
void CmdNumberLine(int code);
void CmdContentsLine(int code);
void CmdListOf(int code);
void CmdHarvard(int code);
void CmdNatbibCite(int code);
void CmdBibpunct(int code);
void CmdBibEntry(int code);
void CmdNatexlab(int code);

enum {
    BIBCITE_TOKEN = 0,
    NEWLABEL_TOKEN
};

enum {
    SCANAUX_NUMBER = 0,
    SCANAUX_SECT
};

#endif
