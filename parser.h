/*
 * Description: Contains declarations for generic recursive parsering
 * routines and other help routines for parsing LaTeX code
 * 
 * 26th June 1998 - Created initial version - fb                            LEG
 * 070798 adapted Frank Barnes contribution to r2l coding conventions SAP
 */

#ifndef _PARSER_H_INCLUDED
#define _PARSER_H_INCLUDED 1

char    *CurrentFileName(void);
int     PushSource(const char * filename, const char * string);
int     StillSource(void);
void    PopSource(void);

char    getRawTexChar(void);
char    getTexChar(void);
char    getNonSpace(void);
char    getNonBlank(void);
int     getSameChar(char c);

void    ungetTexChar(char c);

void    skipToEOL(void);
void    skipSpaces(void);
void    skipWhiteSpace(void);
int     skipBOM(int cThis);

void    CmdIgnoreParameter(int);
void    CmdInclude(int code);
char    *getBraceParam(void);
char    *getBraceRawParam(void);
char    *getLeftRightParam(void);
char    *getBracketParam(void);
char    *getSimpleCommand(void);
char    *getTexUntil(char * target, int raw);
char    *getSpacedTexUntil(char *target, int raw);
int     getDimension(void);
void    parseBrace(void);
char    *getDelimitedText(char left, char right, int raw);

int     CurrentLineNumber(void);
void    PushTrackLineNumber(int flag);
void    PopTrackLineNumber(void);
void    UpdateLineNumber(char *s);
void    EndSource(void);
int     CurrentFileDescriptor(void);
int     getParserDepth(void);
int     getSlashSlashParam(void);

void	ignoreBraceParam(void);
void	ignoreBracketParam(void);

#endif
