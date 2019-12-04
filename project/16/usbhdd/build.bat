@title makefile
@echo off
@rem make file

del /f /q *.obj
del /f /q *.bin

..\..\..\tools\masm\masm.exe mbr.asm;
..\..\..\tools\masm\link.exe mbr.obj;
mbr_exe2bin.exe nodelay

..\..\..\tools\masm\masm.exe dbr.asm;
..\..\..\tools\masm\link.exe dbr.obj;
dbr_exe2bin.exe nodelay

..\..\..\tools\masm\masm.exe asm16.asm;
..\..\..\tools\masm\link.exe asm16.obj;
asm16_exe2bin.exe nodelay

if "%1" equ "nopause" goto end
pause
:end
