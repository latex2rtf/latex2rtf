#define FORM_DOLLAR    2	/* ('$')  */
#define FORM_RND_OPEN  3	/* ('/(') */
#define FORM_ECK_OPEN  4	/* ('/[') */
#define FORM_RND_CLOSE 5	/* ('/)') */
#define FORM_ECK_CLOSE 6	/* ('/]') */
#define FORM_NO_NUMBER 7	/* \nonumber */
#define EQNARRAY       8	/* eqnarray environment */
#define EQNARRAY_1     9	/* eqnarray* environment */
#define EQUATION      10	/* equation environment */
#define EQUATION_1    11	/* equation* environment */
#define FORM_MATH     12	/* math environment */

void            CmdFormula(int code);
void            CmdFormula2(int code);
void            CmdFraction(int code);
void            CmdRoot(int code);
void            CmdIntegral(int code);
