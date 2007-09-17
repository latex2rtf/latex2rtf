@echo off
echo calling latex2pn.bat (%1 %2 %3 %4 %5 %6 %7 %8 %9) >> latex2pn.log
rem This version uses latex and dvips
rem              with convert (Part of ImageMagick)
rem
rem USAGE: latex2pn -d density [-o offset] [-H home_dir] filename
rem        where filename is the name (without extension) of the file to be converted,
rem        either a LaTeX file or a eps file.
rem
rem OPTIONS: -d density  (required! where density is in pixels per inch)
rem          -o offset   clipping offset (required for inline equations)
rem          [-H /home/dir] (optional) directory to be included in tex search path
rem
rem This batch file REQUIRES that the following folders (directories) 
rem  were already added to the PATH:
rem  - folder where the LaTeX executable resides
rem  - folder where the ImageMagick executables reside
rem  - folder where the Ghostscript executables reside
rem  - folder where the netpbm executables reside
rem  (use the batch file l2rprep.bat to set the path)

set of=6
:parmloop
if "%1"=="" goto endloop
if "%1"=="-d" goto dens
if "%1"=="-o" goto offset
if "%1"=="-H" goto thome
set fn=%1
goto endloop
:dens
shift
set dn=%1
shift
goto :parmloop
:offset
shift
set of=%1
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

set inline=1
grep -q -c INLINE_DOT_ON_BASELINE %fn%.tex >NUL
IF ERRORLEVEL 1 set inline=0

set TEXINPUTS=%th%
latex -quiet --interaction batchmode %fn%
set TEXINPUTS=
IF NOT EXIST %fn%.dvi GOTO ERR2

dvips -q -l 1 -o %fn%.eps %fn%.dvi

:NOTEX
IF EXIST %fn%.eps GOTO ISEPS
IF NOT EXIST %fn%.pdf GOTO ERR3
call pdf2ps %fn%.pdf %fn%.eps
IF NOT EXIST %fn%.eps GOTO ERR3
:ISEPS
call eps2eps %fn%.eps tmp1.eps
convert -crop 0x0 -units PixelsPerInch -density %dn%x%dn% tmp1.eps %fn%.png
del tmp1.eps
IF NOT EXIST %fn%.png GOTO ERR4

IF %inline%==0 GOTO NOIN

pngtopnm %fn%.png | pnmcrop -white -left > %fn%.ppm
IF NOT EXIST %fn%.ppm echo ERROR: NetPBM failed to create %fn%.ppm from %fn%.png >> latex2pn.log

rem ** %fn%.pbm created by next line contains the "INLINE_DOT"
rem ** and is needed by latex2rt.exe 
rem ** for calculating vertical alignment of the bitmap
pnmcut -width 1 %fn%.ppm | ppmtopgm | pgmtopbm > %fn%.pbm
pnmcut -left %of% %fn%.ppm | pnmcrop -left | pnmtopng > %fn%.png
del %fn%.ppm
IF NOT EXIST %fn%.pbm echo ERROR: NetPBM failed to create %fn%.pbm from %fn%.ppm >> latex2pn.log
IF NOT EXIST %fn%.png echo ERROR: NetPBM failed to create %fn%.png from %fn%.ppm >> latex2pn.log

:NOIN
IF NOT EXIST %fn%.tex goto cleanup
del %fn%.eps
del %fn%.tex
del %fn%.dvi
del %fn%.aux
del %fn%.log
goto cleanup

:ERR2
echo ERROR: LaTeX failed to create %fn%.dvi from %fn%.tex >> latex2pn.log
goto cleanup

:ERR3
echo ERROR: file %fn%.eps or %fn%.pdf not found >> latex2pn.log
goto cleanup

:ERR4
echo ERROR: ImageMagick convert failed to create %fn%.png from %fn%.eps >> latex2pn.log

:cleanup
set fn=
set dn=
set of=
set th=
set inline=
