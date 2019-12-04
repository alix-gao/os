@title makefile
@echo off
@rem make file

set arch=x86

echo.
call ..\common.bat
set project_path=..\..\..\vs\core
set name=core.o
echo.

del /f /q *.o
call project.bat

echo packing
echo %pack% -cvq %lib_name% %obj_path%\*.o
%link% -r -o %name% %obj_path%\*.o
echo.

echo %symbol_table% -f posix %name% > symbol.txt

echo make_symbol

echo %c_compiler% -c symbol.c
