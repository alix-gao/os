
set os_version=____RELEASE_VERSION____
call compile

if "%1" equ "nopause" goto end
pause
:end
