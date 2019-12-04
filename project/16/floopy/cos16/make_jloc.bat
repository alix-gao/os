del cos16.bin
del *.obj
del main.asm
del rmho.asm
del load.asm
..\..\..\..\tools\TC\TCC -c -ml -S main.c
..\..\..\..\tools\TC\TCC -c -ml -S rmho.c
..\..\..\..\tools\TC\TCC -c -ml -S load.c
..\..\..\..\tools\TC\TCC -c -ml -S lib.c
..\..\..\..\tools\TASM\BIN\TASM /ml cos16.asm
..\..\..\..\tools\TASM\BIN\TASM /ml int.asm
..\..\..\..\tools\TASM\BIN\TASM /ml pm.asm
..\..\..\..\tools\TASM\BIN\TASM /ml main.asm
..\..\..\..\tools\TASM\BIN\TASM /ml rmho.asm
..\..\..\..\tools\TASM\BIN\TASM /ml load.asm
..\..\..\..\tools\TASM\BIN\TASM /ml lib.asm
..\..\..\..\tools\jloc07\JLOC.EXE jloc.txt cos16.bin

pause
