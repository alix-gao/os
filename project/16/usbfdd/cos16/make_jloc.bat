@title makefile
@echo off
@rem make file

del cos16.bin
del *.obj
del main.asm
del rmho.asm
del load.asm
del fat32.asm
del lib.asm
del vfs.asm
del crc16.asm
..\..\..\..\tools\TC\TCC -c -ml -S main.c
..\..\..\..\tools\TC\TCC -c -ml -S rmho.c
..\..\..\..\tools\TC\TCC -c -ml -S load.c
..\..\..\..\tools\TC\TCC -c -ml -S fat32.c
..\..\..\..\tools\TC\TCC -c -ml -S lib.c
..\..\..\..\tools\TC\TCC -c -ml -S vfs.c
..\..\..\..\tools\TC\TCC -c -ml -S crc16.c
..\..\..\..\tools\TASM\BIN\TASM /ml cos16.asm
..\..\..\..\tools\TASM\BIN\TASM /ml int.asm
..\..\..\..\tools\TASM\BIN\TASM /ml pm.asm
..\..\..\..\tools\TASM\BIN\TASM /ml main.asm
..\..\..\..\tools\TASM\BIN\TASM /ml rmho.asm
..\..\..\..\tools\TASM\BIN\TASM /ml load.asm
..\..\..\..\tools\TASM\BIN\TASM /ml fat32.asm
..\..\..\..\tools\TASM\BIN\TASM /ml lib.asm
..\..\..\..\tools\TASM\BIN\TASM /ml vfs.asm
..\..\..\..\tools\TASM\BIN\TASM /ml crc16.asm
..\..\..\..\tools\jloc07\JLOC.EXE jloc.txt cos16.bin

echo.

if "%1" equ "nopause" goto end
pause
:end
