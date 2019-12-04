@title makefile
@echo off
@rem make file

@echo set color
@echo COLOR a7

echo.
echo set option...
call option.bat
echo.

echo delete file...
del /f /q *.bin
del /f /q *.exe
echo.

echo link...
echo %c_compiler% -Wall -fPIC -Wl,-E -Wl,-Map,map.txt -Wl,-Bstatic -Wl,--no-gc-sections -Ttext %image_addr% %linker_script% -o %image_name% -Wl,--start-group compile\boot\boot.obj compile\core\core.o compile\libc\common.lib compile\shell\shell.obj -Wl,--end-group
%c_compiler% -Wall -fPIC -Wl,-E -Wl,-Map,map.txt -Wl,-Bstatic -Wl,--no-gc-sections -T %linker_script% -fno-common -o %image_name% -Wl,--start-group compile\boot\boot.obj compile\core\core.o compile\libc\common.lib compile\shell\shell.obj compile\net\net.o compile\driver\driver.o -Wl,--end-group
tool\iodelay
echo.

tool\os4_image.exe
echo.

echo tool\symbol.exe

if "%1" equ "nopause" goto end
pause
:end
