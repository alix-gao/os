@title makefile
@echo off
@rem make file

set arch=x86

echo.
echo set option...
call ..\common.bat
call option.bat
echo.

call project.bat

echo link...
del /f /q *.lib
%pack% -r %lib_name% %obj_path%\*.o
echo.

if "%1" equ "nopause" goto end
pause
:end
