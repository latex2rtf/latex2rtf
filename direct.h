/*
 * $Id: direct.h,v 1.12 2001/08/12 19:48:12 prahl Exp $
 * History:
 * $Log: direct.h,v $
 * Revision 1.12  2001/08/12 19:48:12  prahl
 * 1.9h
 * 	Turned hyperlatex back on.  Still not tested
 * 	Turned isolatin1 back on.  Still not tested.
 * 	Eliminated use of \\ in code for comments
 * 	Eliminated \* within comments
 * 	Eliminated silly char comparison to EOF
 * 	Revised README to eliminate DOS stuff
 * 	Added support for \pagebreak
 * 	Added support for \quad, \qquad, \, \; and \> (as spaces)
 * 	Improved support for \r accent
 * 	Made minor changes to accentchars.tex
 * 	fixed bugs in \textit{s_$c$} and $\bf R$
 * 	fixed longstanding bugs in stack cleaning
 * 	fixed ' in math mode
 * 	log-like functions now typeset in roman
 * 	Added test cases to eqns.tex
 * 	default compiler options empty until code is more portable
 *
 * Revision 1.3  1998/07/03 07:03:16  glehner
 * lclint cleaning
 *
 * Revision 1.2  1997/02/15 20:45:41  ralf
 * Some lclint changes and corrected variable declarations
 *
 * Revision 1.1  1994/06/17 11:26:29  ralf
 * Initial revision
 *
 */

bool            TryDirectConvert(char *command, FILE * fRtf);
