@echo off
rem This version uses latex and dvips
rem              with convert (Part of ImageMagick)

rem USAGE: latex2png -d density latex_file [-H home_dir]
rem 
rem OPTIONS: -d density  (required! where density is in pixels per inch)
rem          [-H /home/dir] (optional) directory to be included in tex search path

rem Set path to latex here (ending with \) if it isn't in your search path:
rem e.g. SET TEXDIR=C:\texmf\miktex\bin\
SET TEXDIR=

rem Set path to ImageMagick's convert here (ending with \) if it isn't in your search path:
rem e.g. SET MAGICKDIR=C:\ImageMagick\
SET MAGICKDIR=

rem Set path to Ghostscript's gswin32c.exe here if it isn't in your search path:
rem e.g. SET GSDIR=C:\Aladdin\gs6.01\bin
SET GSDIR=

rem input check:
IF NOT EXIST %3.tex GOTO ERR1

del %3.dvi
del %3.png
set TEXINPUTS=%5
%TEXDIR%latex  --interaction batchmode %3 >NUL
set TEXINPUTS=

IF NOT EXIST %3.dvi GOTO ERR2

%TEXDIR%dvips -E -o %3.eps %3.dvi >NUL

IF NOT EXIST %3.eps GOTO ERR3

set OLDCPATH=%PATH%
PATH=%GSDIR%;%PATH%
%MAGICKDIR%convert -crop 0x0 -density %2x%2 %3.eps %3.png >NUL
PATH=%OLDCPATH%
set OLDCPATH=

IF NOT EXIST %3.png GOTO ERR4

del %3.tex
del %3.dvi
del %3.aux
del %3.log
del %3.eps

exit

:ERR1
echo ERROR: file %3.tex not found
exit

:ERR2
echo ERROR: latex failed to create %3.dvi from %3.tex
exit

:ERR3
echo ERROR: dvips failed to create %3.eps from %3.dvi
exit

:ERR4
echo ERROR: ImageMagick convert failed to create %3.png from %3.eps
exit
