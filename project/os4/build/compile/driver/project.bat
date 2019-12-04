echo delete object...
del /f /q %obj_path%\*.o
echo.

set include= -I %arch_path% ^
             -I %libc_path% ^
             -I %vs_path% ^
             -I %core_path% ^
             -I %project_path%\header

echo compile driver...

%c_compiler% %cflag% %include% %project_path%\*.c

%c_compiler% %cflag% %include% %project_path%\ethernet\*.c

if "%1" neq "wn823n" goto wn823n_end
echo wn823n driver
set wn823n_include= -I %project_path%\wn823nv2\include ^
                    -I %project_path%\wn823nv2\include\byteorder ^
                    -I %project_path%\wn823nv2\platform

%c_compiler% %cflag% %include% %wn823n_include% %project_path%\wn823nv2\os_dep\*.c
%c_compiler% %cflag% %include% %wn823n_include% %project_path%\wn823nv2\os_dep\linux\*.c
%c_compiler% %cflag% %include% %wn823n_include% %project_path%\wn823nv2\core\efuse\*.c
%c_compiler% %cflag% %include% %wn823n_include% %project_path%\wn823nv2\core\*.c
%c_compiler% %cflag% %include% %wn823n_include% %project_path%\wn823nv2\hal\*.c
%c_compiler% %cflag% %include% %wn823n_include% %project_path%\wn823nv2\hal\led\*.c
%c_compiler% %cflag% %include% %wn823n_include% %project_path%\wn823nv2\hal\hal_hci\*.c
%c_compiler% %cflag% %include% %wn823n_include% %project_path%\wn823nv2\hal\rtl8192e\*.c
%c_compiler% %cflag% %include% %wn823n_include% %project_path%\wn823nv2\hal\rtl8192e\usb\*.c
%c_compiler% %cflag% %include% %wn823n_include% %project_path%\wn823nv2\hal\efuse\rtl8192e\HalEfuseMask8192E_USB.c
%c_compiler% %cflag% %include% %wn823n_include% %project_path%\wn823nv2\hal\OUTSRC\*.c
%c_compiler% %cflag% %include% %wn823n_include% %project_path%\wn823nv2\hal\OUTSRC\rtl8192e\*.c
%c_compiler% %cflag% %include% %wn823n_include% %project_path%\wn823nv2\hal\OUTSRC-BTCoexist\*.c
%c_compiler% %cflag% %include% %wn823n_include% %project_path%\wn823nv2\platform\platform_ops.c
:wn823n_end

if "%1" neq "wn725n" goto wn725n_end
echo wn725n driver
set wn725n_include= -I %project_path%\wn725nv2\include ^
                    -I %project_path%\wn725nv2\include\byteorder ^
                    -I %project_path%\wn725nv2\platform

%c_compiler% %cflag% %include% %wn725n_include% %project_path%\wn725nv2\os_dep\*.c
%c_compiler% %cflag% %include% %wn725n_include% %project_path%\wn725nv2\os_dep\linux\*.c
%c_compiler% %cflag% %include% %wn725n_include% %project_path%\wn725nv2\core\*.c
%c_compiler% %cflag% %include% %wn725n_include% %project_path%\wn725nv2\core\efuse\*.c
%c_compiler% %cflag% %include% %wn725n_include% %project_path%\wn725nv2\hal\*.c
%c_compiler% %cflag% %include% %wn725n_include% %project_path%\wn725nv2\hal\led\*.c
%c_compiler% %cflag% %include% %wn725n_include% %project_path%\wn725nv2\hal\hal_hci\*.c
%c_compiler% %cflag% %include% %wn725n_include% %project_path%\wn725nv2\hal\rtl8188e\*.c
%c_compiler% %cflag% %include% %wn725n_include% %project_path%\wn725nv2\hal\rtl8188e\usb\*.c
%c_compiler% %cflag% %include% %wn725n_include% %project_path%\wn725nv2\hal\OUTSRC\*.c
%c_compiler% %cflag% %include% %wn725n_include% %project_path%\wn725nv2\hal\OUTSRC\rtl8188e\*.c
%c_compiler% %cflag% %include% %wn725n_include% %project_path%\wn725nv2\hal\OUTSRC-BTCoexist\*.c
%c_compiler% %cflag% %include% %wn725n_include% %project_path%\wn725nv2\platform\platform_ops.c
:wn725n_end

echo.

echo copy file...
copy *.o %obj_path%\*.o
echo.

echo delete file...
del /f /q *.o
echo.

