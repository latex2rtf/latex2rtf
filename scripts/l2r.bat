@echo off
rem LEG251098
rem This batchfile is part of LaTeX2rtf, version 1.7
rem It is used in Windows 95 (R)(c)(@)(etc...) when LaTeX2rtf is
rem compiled with the Cygnus-gcc-Compiler.
rem latex2rtf.exe has to be somewhere in the PATH, as has to be
rem czgwinb19.dll.  the configuration files have to be in
rem C:\l2r\cfg, but Wilfried also initializes RTFPATH and uses
rem absolute filenames.
rem 
rem The author is Wilfried Hennings, <W.Hennings@fz-juelich.de>
rem
rem Bug reports please to <latex2rtf@fz-juelich.de> or to the
rem maintainer of LaTeX2rtf, Georg Lehner <glehner@unanleon.edu.ni>.
rem
set RTFPATH=//C/l2r/cfg
IF x%2 == x Goto End
C:\l2r\latex2rtf <%1 >%2
:End

