
#include<io.h>

extern char*curse_x;
extern char*curse_y;
extern int text_frame_x;
extern int text_frame_y;
extern int text_frame_h;
extern int text_frame_l;
int mus_bar_add_1=4;
int mus_bar_add_2=73;

void io_delay()
{
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
}

//移动光标。
void move_curse(int row,int col)
{
             int position=80*row+col;
             //函数调用，故不用i/o延迟。
     outb(0x3d4,0xf);
     outb(0x3d5,(int)(position&0xff));
     outb(0x3d4,0x0e);
     outb(0x3d5,(int)((position>>8)&0xff));//强制类型转换。
}

void print_int(int disp,int color,int row,int col)
{
     int i,flag=1;
     char j;
     char*pvideo=(char*)(0xb8000+(80*row+col)*2);
     for(i=10000;i>0;i=i/10)//最大99999。
     {
                           j=disp/i+0x30;
                           if(j!=0x30||flag==0)//不是0则显示。
                           {
                                                      flag=0;//表示前面的0不显示，中间的显示。最后的显示。
                                      disp=disp-(j-0x30)*i;
                                      *pvideo=j;
                                      pvideo++;
                                      *pvideo=color;
                                      pvideo++;
                           }
     }
     if(flag)
     {
              *pvideo=0x30;
              pvideo++;
              *pvideo=color;
              pvideo++;
     }
}

//显示字符串。
//strp字符串指针，count数量，color颜色，row行，col列。
void print_string_partly(char*strp,int count,int color,int row,int col)
{
            char*pvideo=(char*)(0xb8000+(80*row+col)*2);
            int i;
            for(i=0;i<count;i++)
            {
                 *pvideo=*strp;
                 pvideo++;
                 strp++;
                 *pvideo=color;
                 pvideo++;
            }
}

void print_string(char*strp,int color,int row,int col)
{
     char*pvideo=(char*)(0xb8000+(80*row+col)*2);
            while(*strp)
            {
                 *pvideo=*strp;
                 pvideo++;
                 strp++;
                 *pvideo=color;
                 pvideo++;
            }
}

//显示字符。
void print_char(char strp,int color,int row,int col)
{
     char*pvideo=(char*)(0xb8000+(80*row+col)*2);
                 *pvideo=strp;
                 pvideo++;
                 *pvideo=color;
                 pvideo++;
}

char read_screen(int row,int col)
{
     char*pvideo=(char*)(0xb8000+(80*row+col)*2);
     return *pvideo;
}

void save_curse(int x,int y)
{
     *curse_x=x;
     *curse_y=y;
}

void cls_mus_bar()
{
     //10
     int i;
     for(i=0;i<10;i++)
     {
                      print_string("  ",15,i+1,mus_bar_add_1);
                      print_string("  ",15,i+1,mus_bar_add_2);
     }
}

// int kkk=0;
void disp_mus_bar(unsigned int k)
{
     int i;
     for(i=0;i<k;i++)
     {
                     print_string("==",2,10-i,mus_bar_add_1);
                     print_string("==",3,10-i,mus_bar_add_2);
     }
}

void time_delay(int time)//time of 15.08us.
{
     unsigned char t;
     unsigned char save;
     unsigned int i;//cx
     //cls_mus_bar();//----bar----
     for(i=0;i<165*time;i++)//331 is 5 ms
     {
                                  disp_mus_bar(i/165/25*2+2);//i is 25*n.----bar----
                        inb(0x61,t);
                        t=t&0x10;
                        save=t;
                        while(1)
                        {
                                inb(0x61,t);
                                t=t&0x10;
                                if(save!=t)
                                break;
                        }
     }
     //i=(i-1)/165/25*2+2;
     //print_int(i,15,20,kkk);
     //print_int(time,14,21,kkk);
     //kkk=kkk+2;
}

/*
void time_delay(char time)
{
     int i,j;
     for(i=0;i<320*time;i++)
     for(j=0;j<3200;j++)
     ;
 }
*/

void ring(int music_freq,char music_time)
{
     unsigned char save;
     unsigned char t;
     int ti;
     outb(0x43,0x0b6);
     ti=1193100/music_freq;//1193100 is inited by bios.
     outb(0x42,(int)(ti&0xff));//low byte
     io_delay();
     outb(0x42,(int)((ti>>8)&0xff));//hign byte
     inb(0x61,t);
     save=t;
     outb(0x61,(t|3));//ring.
     //print_int((t|3)&0xf,15,11,31);
     time_delay(music_time);
     outb(0x61,save);//turn off ring.
}

int music_combine(char music1,char music2)
{
    int i;
    i=music1*100+music2;
    return i;
}

void play_music(char*music_freq,char*music_time)
{
     int i,j,ch;
     i=j=0;
     ch=music_combine(music_freq[i],music_freq[i+1]);
     //int k;
     //for(k=10;k<30;k++)
     //print_int(music_time[j+k],15,17,(k-10)*4);
     while(ch)//0 end
     {
              //print_int(ch,15,16,0);
              //print_int(music_time[j],15,17,0);//
                  ring(ch,music_time[j++]);
                  i=i+2;
                  ch=music_combine(music_freq[i],music_freq[i+1]);
                  cls_mus_bar();
     //kkk++;
     }
}

void key_place_control(int dis,int attr)
{
     int i,j;
     char k;
     if(*curse_y>text_frame_y+text_frame_l)//换行。
     {
                                                    if(*curse_x-text_frame_x+2>text_frame_h)//翻页。
                                                    {
                                                                                                     for(i=text_frame_x+2;i<text_frame_x+text_frame_h;i++)//行
                                                                                                     for(j=text_frame_y+1;j<text_frame_y+text_frame_l+1;j++)//列
                                                                                                     {
                                                                                                                                     k=read_screen(i,j);
                                                                                                                                     print_char(k,15,i-1,j);
                                                                                                     }
                                                                                                     //最后一行清空.
                                                                                                     for(j=text_frame_y+1;j<text_frame_y+text_frame_l+1;j++)
                                                                                                     print_char(' ',15,text_frame_x+text_frame_h-1,j);
                                                                                                     *curse_x=*curse_x-1;//抵消*curse_x=*curse_x+1;
                                                    }
                       *curse_x=*curse_x+1;
                       *curse_y=text_frame_y+1;
                       if(attr)
                       print_char(dis,15,*curse_x,*curse_y);
                       else
                       print_int(dis,15,*curse_x,*curse_y);
                       save_curse(*curse_x,*curse_y+1);
                       move_curse(*curse_x,*curse_y);
     }
     else
     {
         if(attr)
         {
                 print_char(dis,15,*curse_x,*curse_y);
                 save_curse(*curse_x,*curse_y+1);
                 move_curse(*curse_x,*curse_y);
         }
         else
         {
             print_int(dis,15,*curse_x,*curse_y);
             save_curse(*curse_x,*curse_y+1);
             move_curse(*curse_x,*curse_y);
         }
     }
}

void clean_screen(int a,int b)
{
     int i,j;
     for(i=a;i<b;i++)
     for(j=0;j<80;j++)
     print_char(' ',15,i,j);
}

