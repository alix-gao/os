
#include<io.h>

//外部引用函数。
extern void mouse_int();
extern void disp_color(int begx,int begy,int endx,int endy,int color);

extern void modify_int(void (*p)(),int no);

void init_mouse()
{/*
            outb(0x64,0xa8);
            outb(0x64,0xd4);
            outb(0x60,0xf4);
            outb(0x64,0x60);
            outb(0x60,0x47);*/ //这段代码有问题，因为我的机器没有PS/2鼠标。
            modify_int(mouse_int,0x32);//0x20+0x12
}

void f_mouse_int()
{
     int x_pos=0;
     int y_pos=0;
     static int count=0;
     char ch;
     disp_color(600,400,639,479,15);
     //inb(0x60,ch);
//     switch(++count)
//     {
//                    case 1:
//                         write_pix(20,20,4);
//                         break;
//                    case 2:
//                         write_pix(30,30,4);
//                         break;
//                    case 3:
//                         write_pix(40,40,4);
//     }
}

