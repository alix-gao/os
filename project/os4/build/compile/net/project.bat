echo delete object...
del /f /q %obj_path%\*.o
echo.

set include= -I %arch_path% ^
             -I %libc_path% ^
             -I %vs_path% ^
             -I %core_path% ^
             -I %project_path%\header ^
             -I %project_path%\tcpip\header

echo compile network...

%c_compiler% %cflag% %include% %project_path%\*.c
%c_compiler% %cflag% %include% %project_path%\wpa\*.c
%c_compiler% %cflag% %include% %project_path%\tcpip\*.c

echo.

echo copy file...
copy *.o %obj_path%\*.o
echo.

echo delete file...
del /f /q *.o
echo.

