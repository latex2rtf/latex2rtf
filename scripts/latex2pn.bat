@echo off
rem echo latex2pn.bat (%1 %2 %3 %4 %5 %6 %7 %8 %9) >> l2p.log
rem This version uses latex and dvips
rem              with convert (Part of ImageMagick)

rem USAGE: latex2png -d density [-H home_dir] filename
rem        where filename is the name (without extension) of the file to be converted,
rem        either a LaTeX file or a eps file.
rem
rem OPTIONS: -d density  (required! where density is in pixels per inch)
rem          [-H /home/dir] (optional) directory to be included in tex search path
rem
rem 

rem This batch file REQUIRES that the following folders (directories) 
rem  were already added to the PATH:
rem  - folder where the LaTeX executable resides
rem  - folder where the ImageMagick executables reside
rem  - folder where the Ghostscript executables reside
rem  - folder where the netpbm executables reside

:parmloop
if "%1"=="" goto endloop
if "%1"=="-d" goto dens
if "%1"=="-H" goto thome
set fn=%1
goto endloop
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
:endloop

set inline=0
rem input check:
IF NOT EXIST %fn%.tex GOTO NOTEX

IF EXIST %fn%.dvi del %fn%.dvi
IF EXIST %fn%.png del %fn%.png

grep -c -E INLINE_DOT_ON_BASELINE %fn%.tex
IF NOT ERRORLEVEL 1 set inline=1

set TEXINPUTS=%th%
latex  --interaction batchmode %fn%
set TEXINPUTS=

IF NOT EXIST %fn%.dvi GOTO ERR2

dvips -E -o %fn%.eps %fn%.dvi

:NOTEX
IF NOT EXIST %fn%.eps GOTO ERR3

convert -crop 0x0 -density %dn%x%dn% %fn%.eps %fn%.png

IF NOT EXIST %fn%.png GOTO ERR4

IF inline==0 GOTO NOIN

pngtopnm %fn%.png > %fn%.pgm
pnmcut -width 1 %fn%.pgm | ppmtopgm | pgmtopbm > %fn%.pbm
pnmcut -left 6 %fn%.pgm | pnmcrop -left | pnmtopng > %fn%.png
del %fn%.pbm
del %fn%.pgm

:NOIN
del %fn%.tex
del %fn%.dvi
del %fn%.aux
del %fn%.log
del %fn%.eps
goto cleanup

:ERR2
echo ERROR: latex failed to create %fn%.dvi from %fn%.tex
goto cleanup

:ERR3
echo ERROR: file %fn%.eps not found
goto cleanup

:ERR4
echo ERROR: ImageMagick convert failed to create %fn%.png from %fn%.eps

:cleanup
set fn=
set dn=
set th=

