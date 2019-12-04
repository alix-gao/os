@title makefile
@echo off
@rem make file

set os_version=DEBUG_VERSION

echo off
call ..\common.bat
set project_path=..\..\..\vs\driver
set name=driver.o
echo.

del /f /q *.o
echo call project.bat wn823n
call project.bat wn725n

echo packing
echo %pack% -cvq %lib_name% %obj_path%\*.o
%link% -r -o %name% %obj_path%\*.o
echo.

if "%1" equ "nopause" goto end
pause
:end
