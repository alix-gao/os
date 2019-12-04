
#include<io.h>

extern int get_cmos_second();
extern void write_pix(int x,int y,int color);
extern void disp_color(int begx,int begy,int endx,int endy,int color);//this is fill.

#define series 2
#define length 10
#define elsfk_c 4

int get_random_no()
{
     int no;
     no=get_cmos_second();
     no=no%series;
     return no;
}

void all_shape(int kind,int begx,int begy)
{
     switch(kind)
     {
                 case 0:disp_color(begx,begy,begx+10*2,begy+10*2,elsfk_c);break;//·½
                 case 1:disp_color(begx,begy,begx+10,begy+10*4,elsfk_c);break;//³¤
     }
}

void start_elsfk()
{
     int i,j;
     for(i=0;i<600;i=i+40)
     for(j=0;j<400;j=j+40)
     //write_pix(i,j,get_random_no());
     all_shape(get_random_no(),i,j);
}

