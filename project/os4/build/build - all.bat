@title makefile
@echo off
@rem make file

echo.

if exist log.txt (del /f /q log.txt)

cd compile
@echo on

cd boot
call compile.bat nopause >> ..\..\log.txt
cd ..
cd core
call debug.bat nopause >> ..\..\log.txt
cd ..
cd driver
call compile.bat nopause >> ..\..\log.txt
cd ..
cd libc
call compile.bat nopause >> ..\..\log.txt
cd ..
cd net
call compile.bat nopause >> ..\..\log.txt
cd ..
cd shell
call compile.bat nopause >> ..\..\log.txt
cd ..\..
call link.bat nopause >> log.txt
echo.

copy os4.bin ..\..\..\usbfdd\system\os4.bin
copy os4.bin ..\..\..\usbhdd\system\os4.bin

pause
