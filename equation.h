#define EQN_DOLLAR         2	/* ('$')  */
#define EQN_RND_OPEN       3	/* ('/(') */
#define EQN_BRACKET_OPEN   4	/* ('/[') */
#define EQN_RND_CLOSE      5	/* ('/)') */
#define EQN_BRACKET_CLOSE  6	/* ('/]') */
#define EQN_ARRAY          8	/* eqnarray environment */
#define EQN_ARRAY_STAR     9	/* eqnarray* environment */
#define EQN_EQUATION      10	/* equation environment */
#define EQN_EQUATION_STAR 11  /* equation* environment */
#define EQN_MATH          12  /* \begin{math} ... \end{math} */
#define EQN_DISPLAYMATH   13  /* \begin{displaymath} ... \end{displaymath} */
#define EQN_DOLLAR_DOLLAR 14  /* \begin{displaymath} ... \end{displaymath} */
#define EQN_NO_NUMBER     15	/* \nonumber */

void            CmdMath(int code);
void            CmdDisplayMath(int code);
void            CmdFraction(int code);
void            CmdRoot(int code);
void            CmdIntegral(int code);
void            CmdSuperscript(int code);
void            CmdSubscript(int code);
void            CmdNonumber(int code);
