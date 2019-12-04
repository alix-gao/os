
#include<io.h>

//默认为extern。不写了。

void init_graphics()
{
               //设置图形界面的写方式为2.
               /*__asm__(
               "movw $0x3ce,%%dx;\
                movw $5,%%al;\
                out %%al,%%dx;\
                inc dx;\
                movw $0x6,%%al;\
                out %%al,%%dx;"\
                :::
               );*/
               outb(0x3ce,0x5);
               outb(0x3cf,0x6);
}

//movw $00000110b,%%al;\
//movw $0x6,%%al;\

void write_pix(int x,int y,int color)
{
     //caculate.
     __asm__("pushl %3\n\t"
             "pushl %2\n\t"
             "pushl %0\n\t"
             "movl %1,%%eax\n\t"
             "movw $80,%%cx\n\t"
             "mulw %%cx\n\t"
             //"movl $0,%%ebx\n\t"//clear.
             "movw %%ax,%%bx\n\t"
             "popl %%ecx\n\t"
             "movw %%cx,%%ax\n\t"
             "movb $8,%%cl\n\t"
             "divb %%cl\n\t"
             "movb %%ah,%%cl\n\t"
             "movb $0,%%ah\n\t"
             "addw %%ax,%%bx\n\t"
             "movb $0x80,%%ah\n\t"
             "shrb %%cl,%%ah\n\t"
             "movw $0x3ce,%%dx\n\t"
             "movb $8,%%al\n\t"
             "outb %%al,%%dx\n\t"
             "movw $0x3cf,%%dx\n\t"
             "movb %%ah,%%al\n\t"
             "outb %%al,%%dx\n\t"
             "popl %%eax\n\t"
             //"addl %%eax,%%ebx\n\t"
             "movb (%%ebx),%%cl\n\t"
             "popl %%ecx\n\t"
             "movb %%cl,(%%ebx)"
             :
             :"c"(x),"d"(y),"b"(0x0a0000),"a"(color)
             :);
}

void disp_color(int begx,int begy,int endx,int endy,int color)
{
//     char*video=(char*)0x0a0000;
//     outb(0x3ce,0x8);
//     outb(0x3cf,0xff);
//     *video=15;
//code.use.c.
//     int i,j;
//     int offset,mask;
//     char*video=(char*)0x0a0000;
//     for(i=begy;i<=endy;i++)//逐行扫描
//     for(j=begx;j<=endx;j++)
//     {
//                             offset=i*80+j/8;//取整。
//                             mask=0x80;
//                             //设置掩码。
//                             outb(0x3ce,0x8);
//                             outb(0x3cf,mask>>(j%8));
//                             //显示。
//                             mask=*(video+offset);//必须先读出来，再写进去才行。
//                             *(video+offset)=color;//否则就只有间隔线。
//     }
     //write_pix(10,10,4);
//code.use.asm.
     int i,j;
     int offset,mask;
     char*video=(char*)0x0a0000;
     for(i=begy;i<=endy;i++)//逐行扫描
     for(j=begx;j<=endx;j++)
     write_pix(j,i,color);
}

void draw_line(int begx,int begy,int endx,int endy,int color)
{
     int x,y;
     if(begx>endx)
     {
                  begx=begx+endx;endx=begx-endx;begx=begx-endx;
                  begy=begy+endy;endy=begy-endy;begy=begy-endy;
     }
//     if(begy>endy)
//     {begy=begy+endy;endy=begy-endy;begy=begy-endy;}
     for(x=begx;x<=endx;x++)
     write_pix(x,(endy-begy)*(x-begx)/(endx-begx)+begy,color);
}

void draw_ellipse(int x,int y,int r,int color)
{
}

void openmov()
{
     disp_color(0,0,639,479,1);//1 is blue.
     draw_line(0,0,639,479,4);//x1<x2,y1<y2
     draw_line(500,5,5,300,5);//x1>x2,y1<y2
     draw_line(10,400,200,10,6);//x1<x2,y1>y2
     draw_line(300,100,15,15,7);//x1>x2,y1>y2
     //draw_ellipse(60,60,5,4);
     //write_pix(10,10,4);//这个没有显示表明IP已经跳走了。
}

