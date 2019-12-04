
set os_version=____DEBUG_VERSION____
call compile

if "%1" equ "nopause" goto end
pause
:end
