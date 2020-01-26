@title makefile
@echo off
@rem make file

if exist log.txt (del /f /q log.txt)

@echo on
echo "build 16 bit binary for usbfdd" >> log.txt
@echo off
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" >> log.txt
cd usbfdd
call build.bat nopause >> ..\log.txt
cd cos16
call make_jloc.bat nopause >> ..\..\log.txt
cd ../../

@echo on
@copy usbfdd\mbr.bin ..\..\usbfdd\mbr.bin >> log.txt
@copy usbfdd\dbr.bin ..\..\usbfdd\dbr.bin >> log.txt
@copy usbfdd\asm16.bin ..\..\usbfdd\asm16.bin >> log.txt
@copy usbfdd\cos16\cos16.bin ..\..\usbfdd\cos16.bin >> log.txt

echo "build 16 bit binary for usbhdd" >> log.txt
@echo off
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" >> log.txt
cd usbhdd
call build.bat nopause >> ..\log.txt
cd cos16
call make_jloc.bat nopause >> ..\..\log.txt
cd ../../

@echo on
@copy usbhdd\mbr.bin ..\..\usbhdd\mbr.bin >> log.txt
@copy usbhdd\dbr.bin ..\..\usbhdd\dbr.bin >> log.txt
@copy usbhdd\asm16.bin ..\..\usbhdd\asm16.bin >> log.txt
@copy usbhdd\cos16\cos16.bin ..\..\usbhdd\cos16.bin >> log.txt

@echo off
if "%1" equ "nopause" goto end
pause
:end

