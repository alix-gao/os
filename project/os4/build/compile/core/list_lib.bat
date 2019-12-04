del /f /q nm.txt
del /f /q objdump.txt
del /f /q asm.txt
D:\Dev-Cpp\bin\nm -s ukernel.o >> nm.txt
D:\Dev-Cpp\bin\objdump -t ukernel.o >> objdump.txt
D:\Dev-Cpp\bin\objdump -d ukernel.o >> asm.txt
pause