set include=-I %vs_path% -I %libc_path% -I %arch_path% -I %project_path%

echo compile shell...
%c_compiler% -c -O %include% %project_path%\*.c