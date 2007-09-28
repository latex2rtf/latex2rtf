@echo off 
@rem Convert PDF to png.

echo -dNOPAUSE -dBATCH -dSAFER -sDEVICE#pngalpha -r%3 >_.at

rem Watcom C deletes = signs, so use # instead.
gswin32c -q -sOutputFile#%2 @_.at %1
if exist _.at erase _.at
