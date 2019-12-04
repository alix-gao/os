@title makefile
@echo off
@rem make file

del /f /q asm32.bin

..\..\tools\nasm\nasmw.exe asm32.asm -o asm32.bin

copy asm32.bin ..\..\usbfdd\system\asm32.bin
copy asm32.bin ..\..\usbhdd\system\asm32.bin

if "%1" equ "nodelay" goto end
pause
:end
