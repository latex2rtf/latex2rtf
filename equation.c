#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "convert.h"
#include "commands.h"
#include "stack.h"
#include "l2r_fonts.h"
#include "cfg.h"
#include "ignore.h"
#include "parser.h"
#include "equation.h"
#include "counters.h"
#include "funct1.h"
#include "lengths.h"
#include "util.h"
#include "graphics.h"
#include "xref.h"

int g_equation_column = 1;

void
CmdNonumber(int code)
/******************************************************************************
 purpose   : Handles \nonumber to suppress numbering in equations
 ******************************************************************************/
{	
	if (g_processing_eqnarray || !g_processing_tabular)
		g_suppress_equation_number = TRUE;
}

static char *
SlurpDollarEquation(void)
/******************************************************************************
 purpose   : reads an equation delimited by $...$
 			 this routine is needed to handle $ x \mbox{if $x$} $ 
 ******************************************************************************/
{
int brace=0;
int slash=0;
int i;
char *s, *t, *u;

	s = malloc(1024*sizeof(char));
	t = s;
	
	for (i=0; i<1024; i++) {
		*t = getTexChar();
		if (*t == '\\') 
			slash++;
		else if (*t == '{' && (slash % 2) == 0 &&
			!(i>5 && strncmp(t-5,"\\left{", 6)==0) &&
			!(i>6 && strncmp(t-6,"\\right{", 7)==0))
				brace++;
		else if (*t == '}' && (slash % 2) == 0 &&
			!(i>5 && !strncmp(t-5,"\\left}", 6)) &&
			!(i>6 && !strncmp(t-6,"\\right}", 7)))
				brace--;
		else if (*t == '$' && (slash % 2) == 0 && brace == 0) {
			break;
		} else
			slash = 0;
		t++;
	}
	*t = '\0';
	
	u = strdup(s);	/* return much smaller string */
	free(s);		/* release the big string */
	return u;
}

void
SlurpEquation(int code, char **pre, char **eq, char **post)
{
	int true_code = code & ~ON;
	
	switch (true_code) {
	
		case EQN_MATH:
			diagnostics(4, "SlurpEquation() ... \\begin{math}");
			*pre  = strdup("\\begin{math}");
			*post = strdup("\\end{math}");
			*eq = getTexUntil(*post,0);
			break;
			
		case EQN_DOLLAR:
			diagnostics(4, "SlurpEquation() ... $");
			*pre  = strdup("$");
			*post = strdup("$");
			*eq = SlurpDollarEquation();
			break;
	
		case EQN_RND_OPEN:
			diagnostics(4, "SlurpEquation() ... \\(");
			*pre  = strdup("\\(");
			*post = strdup("\\)");
			*eq = getTexUntil(*post,0);
			break;
	
		case EQN_DISPLAYMATH:
			diagnostics(4,"SlurpEquation -- displaymath");
			*pre  = strdup("\\begin{displaymath}");
			*post = strdup("\\end{displaymath}");
			*eq = getTexUntil(*post,0);
			break;
	
		case EQN_EQUATION_STAR:
			diagnostics(4,"SlurpEquation() -- equation*");
			*pre  = strdup("\\begin{equation*}");
			*post = strdup("\\end{equation*}");
			*eq = getTexUntil(*post,0);
			break;
	
		case EQN_DOLLAR_DOLLAR:
			diagnostics(4,"SlurpEquation() -- $$");
			*pre  = strdup("$$");
			*post = strdup("$$");
			*eq = getTexUntil(*post,0);
			break;
	
		case EQN_BRACKET_OPEN:
			diagnostics(4,"SlurpEquation()-- \\[");
			*pre  = strdup("\\[");
			*post = strdup("\\]");
			*eq = getTexUntil(*post,0);
			break;
	
		case EQN_EQUATION:
			diagnostics(4,"SlurpEquation() -- equation");
			*pre  = strdup("\\begin{equation}");
			*post = strdup("\\end{equation}");
			*eq = getTexUntil(*post,0);
			break;
	
		case EQN_ARRAY_STAR:
			diagnostics(4,"Entering CmdDisplayMath -- eqnarray* ");
			*pre  = strdup("\\begin{eqnarray*}");
			*post = strdup("\\end{eqnarray*}");
			*eq = getTexUntil(*post,0);
			break;
	
		case EQN_ARRAY:
			diagnostics(4,"SlurpEquation() --- eqnarray");
			*pre  = strdup("\\begin{eqnarray}");
			*post = strdup("\\end{eqnarray}");
			*eq = getTexUntil(*post,0);
			break;
	}
}

int
EquationNeedsFields(char *eq)
/******************************************************************************
 purpose   : Determine if equation needs EQ field for RTF conversion
 ******************************************************************************/
{
	if (strstr(eq,"\\frac")) return 1;
	if (strstr(eq,"\\sum")) return 1;
	if (strstr(eq,"\\int")) return 1;
	if (strstr(eq,"\\iint")) return 1;
	if (strstr(eq,"\\iiint")) return 1;
	if (strstr(eq,"\\prod")) return 1;
	if (strstr(eq,"\\begin{array}")) return 1;
	if (strstr(eq,"\\left")) return 1;
	if (strstr(eq,"\\right")) return 1;
	if (strstr(eq,"\\root")) return 1;
	if (strstr(eq,"\\sqrt")) return 1;
	if (strstr(eq,"\\over")) return 1;
	if (strstr(eq,"\\stackrel")) return 1;
	if (strstr(eq,"\\dfrac")) return 1;
	return 0;
}

void
WriteEquationAsComment(char *pre, char *eq, char *post)
/******************************************************************************
 purpose   : Writes equation to RTF file as text of COMMENT field
 ******************************************************************************/
{
	fprintRTF("{\\field{\\*\\fldinst{ COMMENTS \"\" ");
 	while (*pre)  putRtfChar(*pre++);
	while (*eq)   putRtfChar(*eq++);
	while (*post) putRtfChar(*post++);
	fprintRTF("}{ }}{\\fldrslt }}");
}

char *
SaveEquationAsFile(char *pre, char *eq, char *post)
{	
	FILE * f;
	char name[15];
	char *tmp_dir, *fullname, *texname;
	static int file_number = 0;
	
	/* create needed file names */
	file_number++;
	tmp_dir = getTmpPath();
	sprintf(name, "l2r_%04d", file_number);
	fullname = strdup_together(tmp_dir, name);	
	texname = strdup_together(fullname,".tex");

	diagnostics(4, "SaveEquationAsFile =%s", texname);
	
	f = fopen(texname,"w");
	while (eq && (*eq == '\n' || *eq == ' ')) eq++;  /* skip whitespace */
	if (f) {
		fprintf(f, "%s", g_preamble);
		fprintf(f, "\\thispagestyle{empty}\n");
		fprintf(f, "\\begin{document}\n");
		fprintf(f, "\\setcounter{equation}{%d}\n",getCounter("equation"));
		if (strstr(pre, "equation"))
			fprintf(f, "$$%s$$", eq);
		else
			fprintf(f, "%s%s%s", pre, eq, post);
		fprintf(f, "\n\\end{document}");
		fclose(f);
	} else {
		free(fullname);
		fullname = NULL;
	}
	
	free(tmp_dir);
	free(texname);
	return(fullname);
}

	
void
WriteLatexAsBitmap(char *pre, char *eq, char *post)
/******************************************************************************
 purpose   : Convert LaTeX to Bitmap and write to RTF file
 ******************************************************************************/
{
	char *p, *name;
	
	diagnostics(4, "Entering WriteEquationAsBitmap");

/* suppress bitmap equation numbers in eqnarrays with zero or one \label{}'s*/
	if (strcmp(pre,"\\begin{eqnarray}")==0){
		p = strstr(eq, "\\label");
		if (p != NULL && strlen(p)>6)			/* found one ... is there a second? */
			p = strstr(p+6, "\\label");
		if (p==NULL) 
			name = SaveEquationAsFile("\\begin{eqnarray*}",eq,"\\end{eqnarray*}");
		else
			name = SaveEquationAsFile(pre,eq,post);
	} else
		name = SaveEquationAsFile(pre,eq,post);
	
	PutLatexFile(name);
}

void 
PrepareRtfEquation(int code, int EQ_Needed)
{
	int width,a,b,c;
		
	width = getLength("textwidth");
	a = 0.45 * width;
	b = 0.50 * width;
	c = 0.55 * width;
	
	switch (code) {
	
		case EQN_MATH:
			diagnostics(4, "PrepareRtfEquation ... \\begin{math}");
			SetTexMode(MODE_MATH);
			break;
	
		case EQN_DOLLAR:
			diagnostics(4, "PrepareRtfEquation ... $");
			fprintRTF("{");
			SetTexMode(MODE_MATH);
			break;
	
		case EQN_RND_OPEN:
			diagnostics(4, "PrepareRtfEquation ... \\(");
			fprintRTF("{");
			SetTexMode(MODE_MATH);
			break;
	
		case EQN_DOLLAR_DOLLAR:
			diagnostics(4,"PrepareRtfEquation -- $$");
			CmdEndParagraph(0);
			SetTexMode(MODE_DISPLAYMATH);
			g_show_equation_number = FALSE;
			fprintRTF("{\\pard\\tqc\\tx%d\\tab ", b);
			break;
	
		case EQN_BRACKET_OPEN:
			diagnostics(4,"PrepareRtfEquation -- \\[");
			SetTexMode(MODE_DISPLAYMATH);
			g_show_equation_number = TRUE;
			fprintRTF("\\par\\par\n{\\pard\\tqc\\tx%d\\tqr\\tx%d\n\\tab ", b, width);
			break;

		case EQN_DISPLAYMATH:
			diagnostics(4,"PrepareRtfEquation -- displaymath");
			g_show_equation_number = FALSE;
			fprintRTF("\\par\\par\n\\pard");
			fprintRTF("\\tqc\\tx%d", b);
			fprintRTF("\\tab ");
			SetTexMode(MODE_DISPLAYMATH);
			break;

		case EQN_EQUATION_STAR:
			diagnostics(4,"PrepareRtfEquation -- equation*");
			g_show_equation_number = FALSE;
			fprintRTF("\\par\\par\n\\pard");
			fprintRTF("\\tqc\\tx%d", b);
			fprintRTF("\\tab ");
			SetTexMode(MODE_DISPLAYMATH);
			break;

		case EQN_EQUATION:
			diagnostics(4,"PrepareRtfEquation -- equation");
			g_equation_column = 5;				/* avoid adding \tabs when finishing */
			g_show_equation_number = TRUE;
			fprintRTF("\\par\\par\n\\pard");
			fprintRTF("\\tqc\\tx%d\\tqr\\tx%d", b, width);
			fprintRTF("\\tab ");
			SetTexMode(MODE_DISPLAYMATH);
			break;

		case EQN_ARRAY_STAR:
			diagnostics(4,"PrepareRtfEquation -- eqnarray* ");
			g_show_equation_number = FALSE;
			g_processing_eqnarray = TRUE;
			g_processing_tabular = TRUE;
			g_equation_column = 1;
			fprintRTF("\\par\\par\n\\pard");
			fprintRTF("\\tqr\\tx%d\\tqc\\tx%d\\tql\\tx%d", a, b, c);
			fprintRTF("\\tab ");
			SetTexMode(MODE_DISPLAYMATH);
			break;

		case EQN_ARRAY:
		    diagnostics(4,"PrepareRtfEquation --- eqnarray");
			g_show_equation_number = TRUE;
			g_processing_eqnarray = TRUE;
			g_processing_tabular = TRUE;
			g_equation_column = 1;
			fprintRTF("\\par\\par\n\\pard");
			fprintRTF("\\tqr\\tx%d\\tqc\\tx%d\\tql\\tx%d\\tqr\\tx%d", a, b, c, width);
			fprintRTF("\\tab ");
			SetTexMode(MODE_DISPLAYMATH);
			break;
			
		default:
			diagnostics(ERROR, "calling PrepareRtfEquation with OFF code");
			break;
		}

	if (EQ_Needed && g_processing_fields==0) {
		fprintRTF("{\\field{\\*\\fldinst{ EQ ");
		g_processing_fields++;	
	}
	
}

void 
FinishRtfEquation(int code, int EQ_Needed)
{	
	if (EQ_Needed && g_processing_fields==1) {
		fprintRTF("}}{\\fldrslt }}");
		g_processing_fields--;	
	}
	
	switch (code) {
	
		case EQN_MATH:
			diagnostics(4,"FinishRtfEquation -- \\end{math}");
			CmdIndent(INDENT_INHIBIT);
			SetTexMode(MODE_HORIZONTAL);
			break;
	
		case EQN_DOLLAR:
			diagnostics(4,"FinishRtfEquation -- $");
			fprintRTF("}");
			SetTexMode(MODE_HORIZONTAL);
			break;
	
		case EQN_RND_OPEN:	
			diagnostics(4,"FinishRtfEquation -- \\)");
			fprintRTF("}");
			SetTexMode(MODE_HORIZONTAL);
			break;
	
		case EQN_DOLLAR_DOLLAR:
			diagnostics(4,"FinishRtfEquation -- $$");
			CmdEndParagraph(0);
			CmdIndent(INDENT_INHIBIT);
			fprintRTF("}");
			break;
	
		case EQN_BRACKET_OPEN:
			diagnostics(4,"FinishRtfEquation -- \\[");
			SetTexMode(MODE_VERTICAL);
			fprintRTF("\\par\\par\n}");
			break;

		case EQN_DISPLAYMATH:
			diagnostics(4,"FinishRtfEquation -- displaymath");
			CmdEndParagraph(0);
			CmdIndent(INDENT_INHIBIT);
			break;

		case EQN_EQUATION_STAR:
			diagnostics(4,"FinishRtfEquation -- equation*");
			CmdEndParagraph(0);
			CmdIndent(INDENT_INHIBIT);
			break;

		case EQN_ARRAY_STAR:
			diagnostics(4,"FinishRtfEquation -- eqnarray* ");
			CmdEndParagraph(0);
			CmdIndent(INDENT_INHIBIT);
			g_processing_eqnarray = FALSE;
			g_processing_tabular = FALSE;
			break;

		case EQN_EQUATION:
		case EQN_ARRAY:
		    diagnostics(4,"FinishRtfEquation --- equation or eqnarray");
			if (g_show_equation_number && !g_suppress_equation_number) {
				char number[20];
				incrementCounter("equation");
				for (; g_equation_column < 3; g_equation_column++)
						fprintRTF("\\tab ");
				fprintRTF("\\tab{\\b0 (");
				sprintf(number,"%d",getCounter("equation"));
				InsertBookmark(g_equation_label,number);
				if (g_equation_label) {
					free(g_equation_label);
					g_equation_label = NULL;
				}
				fprintRTF(")}");
			}
			g_processing_eqnarray = FALSE;
			g_processing_tabular = FALSE;
			CmdEndParagraph(0);
			CmdIndent(INDENT_INHIBIT);
			break;
			
		default:
			diagnostics(ERROR, "calling FinishRtfEquation with OFF code");
			break;
		}
}

char *
scanback(char *s, char *t)
/******************************************************************************
 purpose   : Find '{' that starts a fraction designated by \over 
 			 Consider \int_0 { \{a_{x+y} + b \} \over a_{x+y} }
 	                  ^		 ^                  ^
 	                  s    result               t
 ******************************************************************************/
{
	int braces = 1;
	if (!s || !t || t < s) return NULL;
	
	while (braces && s < t) {
		if (*t=='{' && 
		    !(*(t-1) == '\\') &&						/* avoid \{ */
		    !(s+5 <= t && !strncmp(t-5,"\\left",5)) &&	/* avoid \left{ */
		    !(s+6 <= t && !strncmp(t-6,"\\right",6))	/* avoid \right{ */
		    ) braces--;

		if (*t=='}' && 
		    !(*(t-1) == '\\') &&						/* avoid \} */
		    !(s+5 <= t && !strncmp(t-5,"\\left",5)) &&	/* avoid \left} */
		    !(s+6 <= t && !strncmp(t-6,"\\right",6))	/* avoid \right} */
		    ) braces++;

		if (braces) t--;
	}
	return t;
}

char *
scanahead(char *s)
/******************************************************************************
 purpose   : Find '}' that ends a fraction designated by \over 
 			 Consider \int_0 { \{a_{x+y} + b \} \over a_{x+y} }
 	                                            ^             ^ 
 	                                            s             t
 ******************************************************************************/
{
	char *t;
	int braces =  1;
	int slashes = 0;
	if (!s) return NULL;
	t = s;
	
	while (braces && t && *t != '\0') {

		if (slashes % 2 == 0) {
			if (*t=='}' &&
		    	!(s+5 <= t && !strncmp(t-5,"\\left",5)) &&	/* avoid \left} */
		    	!(s+6 <= t && !strncmp(t-6,"\\right",6))	/* avoid \right} */
			) braces--;
			
			if (*t=='{' &&
				!(s+5 <= t && !strncmp(t-5,"\\left",5)) &&	/* avoid \left{ */
				!(s+6 <= t && !strncmp(t-6,"\\right",6))	/* avoid \right{ */
			) braces++;
		}
		
		if (*t=='\\') slashes++; else slashes = 0;	
		if (braces) t++;
	}

	return t;
}

void
ConvertOverToFrac(char ** equation)
/******************************************************************************
 purpose   : Convert {A \over B} to \frac{A}{B} 
 ******************************************************************************/
{
	char cNext, *eq, *mid, *first, *last, *s, *p, *t;
	eq = *equation;
	p = eq;
	diagnostics(4,"ConvertOverToFrac before <%s>",p);
	while ((mid = strstr(p,"\\over")) != NULL) {
		diagnostics(5,"Matched at <%s>",mid);
		cNext = *(mid+5);
		diagnostics(5,"Next char is <%c>",cNext);
	 	if (!(('A'<= cNext && cNext <= 'Z') || ('a'<= cNext && cNext <= 'z'))) {
			first = scanback(eq, mid);
			diagnostics(6, "first = <%s>", first);
			last  = scanahead(mid);
			diagnostics(6, "last = <%s>", last);
	
			strncpy(mid,"  }{ ",5);
			diagnostics(6, "mid = <%s>", mid);
			s = (char *) malloc(strlen(eq)+7);
			t = s;

			strncpy(t, eq, first-eq);			/* everything up to {A\over B} */
			t += first-eq;

			strncpy(t, "\\frac", 5);			/* insert new \frac */
			t += 5;
			if (*first!='{') {*t='{'; t++;}		/* add { if missing */
			
			strncpy(t, first, last-first);		/* copy A}{B */
			t += last-first;
			
			if (*last!='}') {*t='}'; t++;}		/* add } if missing */

			strcpy(t, last);					/* everything after {A\over B} */
			free(eq);
			eq = s;
			p = eq;
		} else
			p = mid+5;
		diagnostics(6,"ConvertOverToFrac current <%s>",eq);
	}
	*equation = eq;
	diagnostics(4,"ConvertOverToFrac after <%s>",eq);
}

void 
WriteEquationAsRTF(int code, char **eq)
/******************************************************************************
 purpose   : Translate equation to RTF 
 ******************************************************************************/
{
	int EQ_Needed;

	EQ_Needed = EquationNeedsFields(*eq);

	PrepareRtfEquation(code,EQ_Needed);
	ConvertOverToFrac(eq);
	ConvertString(*eq);
	FinishRtfEquation(code,EQ_Needed);
}

void
CmdEquation(int code)
/******************************************************************************
 purpose   : Handle everything associated with equations
 ******************************************************************************/
{
	char *pre, *eq, *post;
	int inline_equation, number, true_code;

	true_code = code & ~ON;	
			
	if (!(code & ON)) return ;

	SlurpEquation(code,&pre,&eq,&post);
	
	diagnostics(4, "Entering CmdEquation --------%x\n<%s>\n<%s>\n<%s>",code,pre,eq,post);

	inline_equation = (true_code == EQN_MATH) || (true_code == EQN_DOLLAR) || (true_code == EQN_RND_OPEN);
	
	number=getCounter("equation");
	
	if (g_equation_comment)
		WriteEquationAsComment(pre,eq,post);
	
	diagnostics(4,"inline=%d  inline_bitmap=%d",inline_equation,g_equation_inline_bitmap);
	diagnostics(4,"inline=%d display_bitmap=%d",inline_equation,g_equation_display_bitmap);
	diagnostics(4,"inline=%d  inline_rtf   =%d",inline_equation,g_equation_inline_rtf);
	diagnostics(4,"inline=%d display_rtf   =%d",inline_equation,g_equation_display_rtf);

	if ((inline_equation && g_equation_inline_bitmap)  || 
		(!inline_equation && g_equation_display_bitmap) ) {
			if (true_code != EQN_ARRAY) {
				PrepareRtfEquation(true_code,FALSE);
				WriteLatexAsBitmap(pre,eq,post);
				FinishRtfEquation(true_code,FALSE);
			} else {
				char *s, *t;
				s=eq;
				diagnostics(4,"eqnarray whole = <%s>",s);
				do {
					t=strstr(s,"\\\\");
					if (t) *t = '\0';
					diagnostics(4,"eqnarray piece = <%s>",s);
					if (strstr(s,"\\nonumber"))
						g_suppress_equation_number = TRUE;
					else
						g_suppress_equation_number = FALSE;

					PrepareRtfEquation(true_code,FALSE);
					WriteLatexAsBitmap("\\begin{eqnarray*}",s,"\\end{eqnarray*}");
					FinishRtfEquation(true_code,FALSE);
					if (t) s = t+2;
				} while (t);
			}
	}

	if ((inline_equation && g_equation_inline_rtf)  || 
		(!inline_equation && g_equation_display_rtf) ) {
		setCounter("equation",number);
		WriteEquationAsRTF(true_code,&eq);	
	}

/* balance \begin{xxx} with \end{xxx} call */	
	if (true_code == EQN_MATH     || true_code == EQN_DISPLAYMATH   ||
		true_code == EQN_EQUATION || true_code == EQN_EQUATION_STAR ||
		true_code == EQN_ARRAY    || true_code == EQN_ARRAY_STAR      )
			ConvertString(post);
	
	free(pre);
	free(eq);
	free(post);
	
}


void
CmdMath(int code)
/******************************************************************************
 purpose   : sets the TeX mode to math or horizontal as appropriate
             for $...$ \( ... \) and \begin{math} ... \end{math}
 ******************************************************************************/
{
	int true_code = code & ~ON;
	
	switch (true_code) {
	
		case EQN_MATH:
			if (code & ON) {
				diagnostics(4, "CmdMath() ... \\begin{math}");
				SetTexMode(MODE_MATH);
			} else {
				diagnostics(4, "CmdMath() ... \\end{math}");
				SetTexMode(MODE_HORIZONTAL);
			}
			break;
	
		case EQN_DOLLAR:
			if (GetTexMode() != MODE_MATH) {
				diagnostics(4, "Entering CmdMath() ... $");
				fprintRTF("{");
				SetTexMode(MODE_MATH);
			} else {
				diagnostics(4, "Exiting CmdMath() ... $");
				fprintRTF("}");
				SetTexMode(MODE_HORIZONTAL);
			}
			break;
	
		case EQN_RND_OPEN:	/* \( */
			diagnostics(4, "CmdMath() ... \\(");
			fprintRTF("{");
			SetTexMode(MODE_MATH);
			break;
	
		case EQN_RND_CLOSE:	/* \) */
			diagnostics(4, "CmdMath() ... \\)");
			fprintRTF("}");
			SetTexMode(MODE_HORIZONTAL);
			break;
	}
}

void 
CmdDisplayMath(int code)
/******************************************************************************
 purpose: creates a displayed equation
          \begin{equation} gets a right justified equation number
          \begin{displaymath} gets no equation number
          \[ gets no equation number
          $$ gets no equation number
 ******************************************************************************/
{
	int width, mid, mode, true_code,a,b,c;
	width = getLength("textwidth");
	mid = width/2;
	mode = GetTexMode();
	true_code = code & ~ON;
	
	if (true_code == EQN_DOLLAR_DOLLAR) {
		if (mode != MODE_DISPLAYMATH) {
			diagnostics(4,"Entering CmdDisplayMath -- $$");
			CmdEndParagraph(0);
			SetTexMode(MODE_DISPLAYMATH);
			g_show_equation_number = FALSE;
			fprintRTF("{\\pard\\tqc\\tx%d\\tab ", mid);
		} else {
			diagnostics(4,"Exiting CmdDisplayMath -- $$");
			CmdEndParagraph(0);
			CmdIndent(INDENT_INHIBIT);
			fprintRTF("}");
		}
		return;
	}
	
	if (true_code == EQN_BRACKET_OPEN) {
		diagnostics(4,"Entering CmdDisplayMath -- \\[");
		SetTexMode(MODE_DISPLAYMATH);
		g_show_equation_number = TRUE;
		fprintRTF("\\par\\par\n{\\pard\\tqc\\tx%d\\tqr\\tx%d\n\\tab ", mid, width);
		return;
	}

	if (true_code == EQN_BRACKET_CLOSE) {
		diagnostics(4,"Exiting CmdDisplayMath -- \\]");
		SetTexMode(MODE_VERTICAL);
		fprintRTF("\\par\\par\n}");
		return;
	}

	if (code & ON) {  /* \begin{equation}, etc. */

		g_suppress_equation_number = FALSE;
		
		a = 0.45 *width;
		b = 0.5 * width;
		c = 0.55 * width;
		fprintRTF("\\par\\par\n\\pard");
		switch (true_code) {
		case EQN_DISPLAYMATH:
			diagnostics(4,"Entering CmdDisplayMath -- displaymath");
			g_show_equation_number = FALSE;
			fprintRTF("\\tqc\\tx%d", mid);
			break;

		case EQN_EQUATION_STAR:
			diagnostics(4,"Entering CmdDisplayMath -- equation*");
			g_show_equation_number = FALSE;
			fprintRTF("\\tqc\\tx%d", mid);
			break;

		case EQN_EQUATION:
			diagnostics(4,"Entering CmdDisplayMath -- equation");
			g_equation_column = 5;				/* avoid adding \tabs when finishing */
			g_show_equation_number = TRUE;
			fprintRTF("\\tqc\\tx%d\\tqr\\tx%d", mid, width);
			break;

		case EQN_ARRAY_STAR:
			diagnostics(4,"Entering CmdDisplayMath -- eqnarray* ");
			g_show_equation_number = FALSE;
			g_processing_eqnarray = TRUE;
			g_processing_tabular = TRUE;
			g_equation_column = 1;
			fprintRTF("\\tqr\\tx%d\\tqc\\tx%d\\tql\\tx%d", a, b, c);
			break;

		case EQN_ARRAY:
		    diagnostics(4,"Entering CmdDisplayMath --- eqnarray");
			g_show_equation_number = TRUE;
			g_processing_eqnarray = TRUE;
			g_processing_tabular = TRUE;
			g_equation_column = 1;
			fprintRTF("\\tqr\\tx%d\\tqc\\tx%d\\tql\\tx%d\\tqr\\tx%d", a, b, c, width);
			break;
		}
		fprintRTF("\\tab ");
		SetTexMode(MODE_DISPLAYMATH);
		
	} else {
	
		diagnostics(4,"Exiting CmdDisplayMath");
		
		if (g_show_equation_number && !g_suppress_equation_number) {
			char number[20];
			incrementCounter("equation");
			for (; g_equation_column < 3; g_equation_column++)
					fprintRTF("\\tab ");
			fprintRTF("\\tab{\\b0 (");
			sprintf(number,"%d",getCounter("equation"));
			InsertBookmark(g_equation_label,number);
			if (g_equation_label) {
				free(g_equation_label);
				g_equation_label = NULL;
			} 
			fprintRTF(")}");
		}

		CmdEndParagraph(0);
		CmdIndent(INDENT_INHIBIT);

		if (true_code == EQN_ARRAY || true_code == EQN_ARRAY_STAR) {
			g_processing_tabular = FALSE;
			g_processing_eqnarray = FALSE;
		}
	}
}

void 
CmdRoot(int code)
/******************************************************************************
 purpose: converts \sqrt{x} or \root[\alpha]{x+y}
******************************************************************************/
{
	char           *root;
	char           *power;

	power = getBracketParam();
	root = getBraceParam();
	fprintRTF(" \\\\R(");
	if (power && strlen(power)>0)
		ConvertString(power);
	fprintRTF("%c", g_field_separator);
	ConvertString(root);
	fprintRTF(")");
	
	if (power) free(power);
	free(root);
}

void 
CmdFraction(int code)
/******************************************************************************
 purpose: converts \frac{x}{y} (following Taupin's implementation in ltx2rtf)
******************************************************************************/
{
	char           *denominator, *numerator, *nptr, *dptr;

	numerator = getBraceParam();
	nptr = strdup_noendblanks(numerator);
	skipSpaces();
	denominator = getBraceParam();
	dptr = strdup_noendblanks(denominator);

	free(numerator);
	free(denominator);
	diagnostics(4,"CmdFraction -- numerator   = <%s>", nptr);
	diagnostics(4,"CmdFraction -- denominator = <%s>", dptr);

	fprintRTF(" \\\\F(");
	ConvertString(nptr);
	fprintRTF("%c", g_field_separator);
	ConvertString(dptr);
	fprintRTF(")");

	free(nptr);
	free(dptr);
}

void 
CmdIntegral(int code)
/******************************************************************************
 purpose: converts integral symbol + the "exponent" and "subscript" fields
parameter: type of operand
 ******************************************************************************/
{
	char           *upper_limit = NULL;
	char           *lower_limit = NULL;
	char            cThis;

	/* is there an exponent/subscript ? */
	cThis = getNonBlank();

	if (cThis == '_')
		lower_limit = getBraceParam();
	else if (cThis == '^')
		upper_limit = getBraceParam();
	else
		ungetTexChar(cThis);

	if (upper_limit || lower_limit) {
		cThis = getNonBlank();
		if (cThis == '_')
			lower_limit = getBraceParam();
		else if (cThis == '^')
			upper_limit = getBraceParam();
		else
			ungetTexChar(cThis);
	}

	fprintRTF(" \\\\I");
	  switch(code)
	  {
		case 4 : fprintRTF("\\\\in( %c %c )\\\\I", g_field_separator, g_field_separator); /*\iiint --- fall through*/
		case 3 : fprintRTF("\\\\in( %c %c )\\\\I", g_field_separator, g_field_separator); /* \iint --- fall through*/
		case 0 : fprintRTF("\\\\in("); break;	
		case 1 : fprintRTF("\\\\su("); break;
		case 2 : fprintRTF("\\\\pr("); break;
		default: diagnostics(ERROR, "Illegal code to CmdIntegral");
	  }

	if (lower_limit)
		ConvertString(lower_limit);
	fprintRTF("%c", g_field_separator);
	if (upper_limit)
		ConvertString(upper_limit);
	fprintRTF("%c )", g_field_separator);

	if (lower_limit)
		free(lower_limit);
	if (upper_limit)
		free(upper_limit);
}

void
CmdSuperscript(int code)
/******************************************************************************
 purpose   : Handles superscripts ^\alpha, ^a, ^{a} and \textsuperscript{a}
 ******************************************************************************/
{
	char           *s = NULL;
	int  size, newsize, upsize;

	if ((s = getBraceParam())) {
		size = CurrentFontSize();
		newsize = size / 1.2;
		upsize = size / 3;
		fprintRTF("{\\up%d\\fs%d ",upsize,newsize);
		ConvertString(s);
		fprintRTF("}");
		free(s);
	}
}

void
CmdSubscript(int code)
/******************************************************************************
 purpose   : Handles superscripts ^\alpha, ^a, ^{a}
 ******************************************************************************/
{
	char           *s = NULL;
	int  size, newsize, upsize;

	if ((s = getBraceParam())) {
		size = CurrentFontSize();
		newsize = size / 1.2;
		upsize = size / 3;
		fprintRTF("{\\dn%d\\fs%d ",upsize,newsize);
		ConvertString(s);
		fprintRTF("}");
		free(s);
	}
}

void
CmdLeftRight(int code)
/******************************************************************************
 purpose   : Handles \left \right
 			 to properly handle \left. or \right. would require prescanning the
 			 entire equation.  
 ******************************************************************************/
{ 
	char delim;

	delim = getTexChar();
	if (delim == '\\')			/* might be \{ or \} */
		delim = getTexChar();
	
	if (code == 0) {
		diagnostics(4, "CmdLeftRight() ... \\left <%c>", delim);

		if (delim == '.')
			diagnostics(WARNING, "\\left. not supported");
		
		fprintRTF(" \\\\b ");
		if (delim == '(' || delim == '.')
			fprintRTF("(");
		else if (delim == '{')
			fprintRTF("\\\\bc\\\\\\{ (");
		else 
			fprintRTF("\\\\bc\\\\%c (", delim);

	} else {
		fprintRTF(")");
		if (delim == '.')
			diagnostics(WARNING, "\\right. not supported");
		diagnostics(4, "CmdLeftRight() ... \\right <%c>", delim);
	}
}

void
CmdArray(int code)
/******************************************************************************
 purpose   : Handles \begin{array}[c]{ccc} ... \end{array}
 ******************************************************************************/
{
char * v_align, * col_align, *s;
int n=0;

	if (code & ON) {
		v_align = getBracketParam();
		col_align = getBraceParam();
		diagnostics(4, "CmdArray() ... \\begin{array}[%s]{%s}", v_align?v_align:"", col_align);
		if (v_align) free(v_align);
		
		s = col_align;
		while (*s) {
			if (*s == 'c' || *s == 'l' || *s == 'r' ) n++;
			s++;
		}
		free(col_align);
		
		fprintRTF(" \\\\a \\\\ac \\\\co%d (", n);
		g_processing_arrays++;
		
	} else {
		fprintRTF(")");
		diagnostics(4, "CmdArray() ... \\end{array}");
		g_processing_arrays--;
	}
}

void
CmdStackrel(int code)
/******************************************************************************
 purpose   : Handles \stackrel{a}{=}
 ******************************************************************************/
{
char * numer, *denom;
int size;
		
	size = CurrentFontSize()/1.2;
	numer = getBraceParam();
	denom = getBraceParam();
	diagnostics(4, "CmdStackrel() ... \\stackrel{%s}{%s}", numer,denom);
	
	fprintRTF(" \\\\a ({\\fs%d ",size);
	ConvertString(numer);
	fprintRTF("}%c", g_field_separator);
	ConvertString(denom);
	fprintRTF(") ");
	
	free(numer);
	free(denom);
}


