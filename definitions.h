int     maybeDefinition(char * s, int n);
int     existsDefinition(char * s);
void	newDefinition(char *name, char *def, int params);
void	renewDefinition(char * name, char * def, int params);
char *	expandDefinition(int thedef);

int		existsEnvironment(char * s);
void	newEnvironment(char *name, char *begdef, char *enddef, int params);
void	renewEnvironment(char *name, char *begdef, char *enddef, int params);
char *	expandEnvironment(int thedef, int starting);

void	newTheorem(char *name, char *caption, char *numbered_like, char *within);
int		existsTheorem(char * s);
char 	*expandTheorem(int i, char *option);
void	resetTheoremCounter(char *unit);
