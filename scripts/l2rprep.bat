rem set folder containing latex2rt.exe and shell executables here:
set l2rpath=C:\Progra~1\latex2rtf

set rtfpath=%l2rpath%\cfg

rem set folder containing Ghostscript executables here:
%l2rpath%\which gswin32c >nul
if errorlevel 1 SET PATH=C:\Progra~1\gs\gs9.10\bin;C:\Progra~1\gs\gs9.10\lib;%PATH%

rem set folder containing ImageMagick executables here:
%l2rpath%\which identify >nul
if errorlevel 1 SET PATH=C:\Progra~1\ImageM~1;%PATH%

rem set folder containing LaTeX and dvips executables here:
%l2rpath%\which latex >nul
if errorlevel 1 SET PATH=C:\MiKTeX2.9\miktex\bin;%PATH%

%l2rpath%\which latex2rt >nul
if errorlevel 1 SET PATH=%l2rpath%;%PATH%

set l2rpath=
