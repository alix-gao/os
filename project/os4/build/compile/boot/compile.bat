@title makefile
@echo off
@rem make file

if exist *.o (
    del /f /q *.o
)
if exist *.obj (
    del /f /q *.obj
)

set arch=x86

echo.
echo set option...
call ..\common.bat
call option.bat
echo.

call project.bat

echo link...
%link% -r -o %obj_name% *.o
echo.

if "%1" equ "nopause" goto end
pause
:end
