@echo off
rem This version uses latex and dvips
rem              with convert (Part of ImageMagick)

rem USAGE: latex2png -d density [-H home_dir] latex_file 
rem 
rem OPTIONS: -d density  (required! where density is in pixels per inch)
rem          [-H /home/dir] (optional) directory to be included in tex search path

rem Set path to latex here (ending with \) 
rem     if it isn't in your search path:
rem e.g. SET TEXDIR=C:\texmf\miktex\bin\
SET TEXDIR=

rem Set path to ImageMagick's convert here (ending with \) 
rem     if it isn't in your search path:
rem e.g. SET MAGICKDIR=C:\ImageMagick\
SET MAGICKDIR=

rem Set path to Ghostscript's gswin32c.exe here (NOT ending with \) 
rem     if it isn't in your search path:
rem e.g. SET GSDIR=C:\gs\gs7.04\bin
SET GSDIR=

:parmloop
if "%1"=="" goto done
if "%1"=="-d" goto dens
if "%1"=="-H" goto thome
set fn=%1
goto done
:dens
shift
set dn=%1
shift
goto :parmloop
:thome
shift
set th=%1
shift
goto :parmloop
:done

rem input check:
IF NOT EXIST %fn%.tex GOTO ERR1

IF EXIST %fn%.dvi del %fn%.dvi
IF EXIST %fn%.png del %fn%.png

set TEXINPUTS=%th%
%TEXDIR%latex  --interaction batchmode %fn% >NUL
set TEXINPUTS=

IF NOT EXIST %fn%.dvi GOTO ERR2

%TEXDIR%dvips -E -o %fn%.eps %fn%.dvi >NUL

IF NOT EXIST %fn%.eps GOTO ERR3

IF NOT x%GSDIR%==x set OLDCPATH=%PATH%
IF NOT x%GSDIR%==x PATH=%GSDIR%;%PATH%
%MAGICKDIR%convert -crop 0x0 -density %dn%x%dn% %fn%.eps %fn%.png >NUL
IF NOT x%GSDIR%==x PATH=%OLDCPATH%
IF NOT x%GSDIR%==x set OLDCPATH=

IF NOT EXIST %fn%.png GOTO ERR4

del %fn%.tex
del %fn%.dvi
del %fn%.aux
del %fn%.log
del %fn%.eps
goto cleanup

:ERR1
echo ERROR: file %fn%.tex not found
goto cleanup

:ERR2
echo ERROR: latex failed to create %fn%.dvi from %fn%.tex
goto cleanup

:ERR3
echo ERROR: dvips failed to create %fn%.eps from %fn%.dvi
goto cleanup

:ERR4
echo ERROR: ImageMagick convert failed to create %fn%.png from %fn%.eps

:cleanup
set fn=
set dn=
set th=

