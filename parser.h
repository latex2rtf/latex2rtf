/* Description:
    Contains declarations for generic recursive parsering routines
    and other help routines for parsing LaTeX code

 * 26th June 1998 - Created initial version - fb                            
 * LEG 070798 adapted Frank Barnes contribution to r2l coding conventions
 * SAP Added several more routines
*/

#define POSSTACKSIZE   256  /* Size of stack to save positions              */

void CmdIgnoreParameter(int);
char *getParam(void);
char *getMathParam(void);
bool getBracketParam(char *string, int size);
void getBraceParam(char *string, int size);
void rewind_one(char c);

char getNonBlank(void);
char getTexChar(void);
char getNonSpace(void);

void skipToEOL(void);
void skipSpaces(void);

