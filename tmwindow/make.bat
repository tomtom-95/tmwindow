@echo off

set filename="main"

if "%1" == "build"     goto build
if "%1" == "run"       goto run
if "%1" == "clear"     goto clear

echo Invalid argument. Use "build", "run", or "clear".
goto :eof

:build
    echo Building ...
    call cl /nologo /EHsc /Zi %filename%.cpp user32.lib gdi32.lib
    goto :eof

:run
    echo Running ...
    call main.exe
    goto :eof

:clear
    echo Clearing executables and obj files ...
    del %filename%.exe
    del %filename%.obj
    del %filename%.pdb
    del %filename%.ilk
    del vc140.pdb
    goto :eof

:eof