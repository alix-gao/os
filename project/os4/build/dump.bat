del /f /q dump\nm.txt
del /f /q dump\objdump.txt
del /f /q dump\asm.txt
D:\Dev-Cpp\bin\nm -s os4.exe >> dump\nm.txt
D:\Dev-Cpp\bin\objdump -t os4.exe >> dump\objdump.txt
D:\Dev-Cpp\bin\objdump -d os4.exe >> dump\asm.txt
pause