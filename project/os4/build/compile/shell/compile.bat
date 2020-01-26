@title makefile
@echo off
@rem make file

if exist *.obj (
    del /f /q *.obj
)

echo.
call ..\common.bat
set project_path=..\..\..\vs\shell
set obj_name=shell.obj
echo.

call project.bat

echo link...
%link% -r -o %obj_name% *.o

del /f /q *.o

if "%1" equ "nopause" goto end
pause
:end
