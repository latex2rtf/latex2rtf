#define PREAMBLE_MODE      1
#define DOCUMENT_MODE      2
#define ITEMIZE_MODE       3
#define ENUMERATE_MODE     4
#define DESCRIPTION_MODE   5
#define LETTER_MODE        8
#define IGNORE_MODE        9
#define HYPERLATEX_MODE   10
#define FIGURE_MODE       11
#define GERMAN_MODE       12
#define FRENCH_MODE       13
#define RUSSIAN_MODE      14
#define GENERIC_MODE      15
#define CZECH_MODE        16
#define APACITE_MODE      17
#define NATBIB_MODE       18
#define HARVARD_MODE      19
#define AUTHORDATE_MODE   20
#define VERSE_MODE        21
#define VERBATIM_MODE     22
#define QUOTE_MODE        23
#define QUOTATION_MODE    24
#define BIBLIOGRAPHY_MODE 25

#define ON 0x4000
#define OFF 0x0000

void PushEnvironment(int code);
void PopEnvironment();
bool CallCommandFunc(char *cCommand);
void CallParamFunc(char *cCommand, int AddParam);
int  CurrentEnvironmentCount(void);
