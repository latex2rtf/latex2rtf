/* proof.c - LaTeX to RTF conversion program

This file contains a function used to display data within \begin{proof} ... \end{proof}
environment.

Authors:
    2018 Alex Itkes
*/

#include <stdlib.h>
#include "main.h"
#include "chars.h"
#include "parser.h"
#include "commands.h"
#include "vertical.h"
#include "convert.h"
#include "cfg.h"

/* This function writes the text within \begin{proof} ... \end{proof} environment
   to RTF.
   First it writes the word "Proof" translated to a proper language, then
   converts the text within the environment just as any other text block,
   and then terminates the with a square mark.
   Actually all the work is done while processing the begin tag.
*/
void CmdProof (int code)
{
    if (code & ON) {
        /* Get the text until the end tag. */
        char * proof = getTexUntil ("\\end{proof}", 0);

        diagnostics(4, "Entering CmdProof");

        startParagraph ("Normal", PARAGRAPH_GENERIC);

        /* The PROOFNAME command will display the "Proof" word translated to the right language. */
        fprintRTF("{\\i ");
        ConvertBabelName("PROOFNAME");
        fprintRTF("}. ");

        /* Transform all text until the end tag. */
        ConvertString (proof);

        /* A square marks the end of a proof. */
        CmdUnicodeChar (9633);

        CmdEndParagraph(0);
        ConvertString ("\\end{proof}");

        diagnostics(4, "Exiting CmdProof");

        /* The getTexUntil () function returns the pointer created by strdup (), so free it. */
        free (proof);
    }
}
