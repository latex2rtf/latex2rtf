/*
 * Description: Contains declarations for generic recursive parsering
 * routines and other help routines for parsing LaTeX code
 * 
 * 26th June 1998 - Created initial version - fb                            LEG
 * 070798 adapted Frank Barnes contribution to r2l coding conventions SAP
 */

int 			CurrentLineNumber(void);
char 		   *CurrentFileName(void);
int 			PushSource(char * filename, char * string);
int				StillSource(void);
void 			PopSource(void);

char            getRawTexChar(void);
char            getTexChar(void);
char            getNonSpace(void);
char            getNonBlank(void);
int             getSameChar(char c);

void            ungetTexChar(char c);

void            skipToEOL(void);
void            skipSpaces(void);

void            CmdIgnoreParameter(int);
char           *getBraceParam(void);
char           *getBracketParam(void);
char           *getSimpleCommand(void);
char           *getTexUntil(char * target, int raw);
int             getDimension(void);
void			parseBrace(void);
long 			ftellTex(void);
void 			fseekTex(long pos);
char           *getDelimitedText(char left, char right, bool raw);
void			getSection(char **body, char **header, char **label);
