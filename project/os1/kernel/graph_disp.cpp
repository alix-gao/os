
#include<types.h>
#include<word.h>
#include <kcurse_ctrl.h>
#include <8259a.h>

//text is console.
//0x30-0,0x5a-z
bit_8 num_to_offset[10]={10,1,2,3,4,5,6,7,8,9};/* 0-9 */
bit_8 char_to_offset[0x7a-0x61+1]={25,44-1,42-1,27,//a-d
                                   13,28,29,30,//e-h
                                   18,31,32,33,//i-l
                                   46-1,45-1,19,20,//m-p
                                   11,14,26,15,//q-t
                                   17,43-1,12,41-1,//u-x
                                   16,40-1//y-z
                                   };

char aucDtoX[17] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

void print_char_graph(char temp,int x,int y,int color)//像素坐标
{/* char is 8x16 */
     if(temp>0x60&&temp<0x7b)//char
     {
                        temp=temp-0x61;
                        print_char_g(char_to_offset[temp],x,y,color);
     }
     else if(temp>0x2f&&temp<0x3a)//num
     {
         temp=temp-0x30;
         print_char_g(num_to_offset[temp],x,y,color);
     }
}

void print_string_graph(char*pstr,int x,int y,int color)
{
     int i=0;
     int pos_x=x;
     while(pstr[i])
     {
            print_char_graph(pstr[i],pos_x,y,color);
            i++;
            pos_x=pos_x+8;
     }
}

void print_int_graph(int temp,int x,int y,int color)
{
     int i,flag=1;
     char j;
     int pos_x=x;

     for(i=10000;i>0;i=i/10)//最大99999。
     {
                           j=temp/i%10;
                           if(j||flag==0)//不是0则显示。
                           {
                                      flag=0;//表示前面的0不显示，中间的显示。最后的显示。
                                      print_char_g(num_to_offset[j],pos_x,y,color);
                                      pos_x=pos_x+8;
                           }
     }
     if(flag)
     {
             print_char_g(num_to_offset[0],pos_x,y,color);//num_to_offset[0]=10,10 is 0.
     }
}

void print_int_graph_x(int temp, int x, int y, int color)
{
     char output = 0;
     int ulLoop = 0;

     print_string_graph("0x", x, y, color);
     x = x + 8*2;

     for (ulLoop = 7; ulLoop >= 0; ulLoop--)
     {
         output = temp >> ulLoop*4;
         output = output & 0xf;
         print_char_graph(aucDtoX[output], x, y, color);
         x = x + 8;
     }
}

/* add kernel display curse */
void print_int_x_no_pos(int temp)
{
     curse_pos_stru pos = {0};

     cli();

     pos = kcurse_get_pos(sizeof(temp)*2 + 2 + 1);

     print_int_graph_x(temp, pos.x, pos.y, 3);

     sti();
}

