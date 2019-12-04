echo make file
echo os1

D:\Dev-Cpp\bin\gcc -Wl,-Map,sys_map.txt -Ttext 0x1000000 -o os1.exe init\*.o kernel\*.o drivers\*.o mm\*.o ipc\*.o
tools\iodelay
tools\os1_image.exe
