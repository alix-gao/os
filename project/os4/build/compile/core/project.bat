echo delete object...
if exist %obj_path% (
    rmdir /s /q %obj_path%
)
md %obj_path%
echo.

set include= -I %arch_path% ^
             -I %libc_path% ^
             -I %vs_path% ^
             -I %project_path%\header ^
             -I %project_path%\vms ^
             -I %project_path%\vts ^
             -I %project_path%\vbs ^
             -I %project_path%\vds

echo compile core...

%c_compiler% %cflag% %include% %project_path%\init\*.c

%c_compiler% %cflag% %include% %project_path%\vts\*.c
%asm_compiler% -o trap.o %project_path%\vts\ia32\trap.s
%c_compiler% %cflag% %include% %project_path%\vts\ia32\*.c
%c_compiler% %cflag% %include% %project_path%\vts\debug\*.c
%c_compiler% %cflag% %include% %project_path%\vts\desktop\*.c

%c_compiler% %cflag% %include% %project_path%\vbs\*.c
%asm_compiler% -o interrupt.o %project_path%\vbs\isr\interrupt.s
%asm_compiler% -o lapic.o %project_path%\vbs\isr\lapic.s
%c_compiler% %cflag% %include% %project_path%\vbs\isr\*.c
%c_compiler% %cflag% %include% %project_path%\vbs\synchro\*.c
%c_compiler% %cflag% %include% %project_path%\vbs\pci\*.c
%c_compiler% %cflag% %include% %project_path%\vbs\usb\*.c

%c_compiler% %cflag% %include% %project_path%\vms\*.c

%c_compiler% %cflag% %include% %project_path%\vds\*.c
%c_compiler% %cflag% %include% %project_path%\vds\video\*.c
%c_compiler% %cflag% %include% %project_path%\vds\video\vga\*.c
%c_compiler% %cflag% %include% %project_path%\vds\video\vbe\*.c
%c_compiler% %cflag% %include% %project_path%\vds\timer\*.c
%c_compiler% %cflag% %include% %project_path%\vds\kb\*.c
%c_compiler% %cflag% %include% %project_path%\vds\mouse\ps2\*.c
%c_compiler% %cflag% %include% %project_path%\vds\mouse\hid\*.c
%c_compiler% %cflag% %include% %project_path%\vds\disk\*.c
%c_compiler% %cflag% %include% %project_path%\vds\disk\hard_disk\*.c
%c_compiler% %cflag% %include% %project_path%\vds\disk\mass_storage\*.c
%c_compiler% %cflag% %include% %project_path%\vds\vfs\*.c
%c_compiler% %cflag% %include% %project_path%\vds\vfs\fat32\*.c
%c_compiler% %cflag% %include% %project_path%\vds\video\bmp\*.c
echo.

echo copy file...
copy *.o %obj_path%\*.o
echo.

echo delete file...
del /f /q *.o
echo.

