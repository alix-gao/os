set include=-I %libc_path% -I %arch_path%

echo compile vs...
%c_compiler% -c -O %include% -o dummy.o %project_path%\dummy.c
%asm_compiler% -o entry.o %project_path%\crt.s
%c_compiler% -c -O %include% -o main.o %project_path%\main.c
%c_compiler% -c -mpush-args -mno-accumulate-outgoing-args -mpreferred-stack-boundary=2 %include% -o version.o %project_path%\version.c