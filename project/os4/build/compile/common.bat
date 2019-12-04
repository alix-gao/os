set c_tool_path=..\..\..\..\..\tools\Dev-Cpp\bin

set c_compiler_name=gcc
set asm_compiler_name=as
set link_tool=ld
set pack_tool=ar
set nm_tool=nm

set asm_compiler=%c_tool_path%\%asm_compiler_name%
set c_compiler=%c_tool_path%\%c_compiler_name%
set link=%c_tool_path%\%link_tool%
set pack=%c_tool_path%\%pack_tool%
set symbol_table=%c_tool_path%\%nm_tool%

set cflag=-D%os_version% -D_X86_ -D_align_=4 -c -O
rem -Wunused-but-set-variable

set arch=x86

set obj_path=object

set vs_path=..\..\..\header
set libc_path=..\..\..\vs\header
set arch_path=..\..\..\vs\arch\%arch%
set core_path=..\..\..\vs\core\header

