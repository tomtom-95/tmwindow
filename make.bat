@echo off

if "%1" == "build" goto build
if "%1" == "test"  goto test
if "%1" == "run"   goto run
if "%1" == "clear" goto clear

echo Invalid argument. Use "build", "test", "run", or "clear".
goto :eof

:build
    echo Building ...
    call cl /nologo /EHsc /Zi main.cpp user32.lib gdi32.lib
    goto :eof

:build_with_resource
    echo Building using resource file
    call rc resource.rc
    call cl /c /nologo /EHsc /Zi main.c
    call cl main.obj resource.res /link /subsystem:windows user32.lib 
    goto :eof

:test
    echo TODO
    goto :eof

:run
    echo Executing ...
    call main.exe
    goto :eof

:clear
    echo Clearing executables and obj files ...
    del main.exe
    del main.obj
    del main.pdb
    del main.ilk
    del resource.res
    del vc140.pdb
    goto :eof

:eof