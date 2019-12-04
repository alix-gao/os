
#include<io.h>
#include<global_cite.h>

extern void print_char(char strp,int color,int row,int col);
extern void move_curse(int row,int col);
extern void save_curse(int x,int y);
extern void test_keyboard_int();
extern void clean_screen(int a,int b);
extern void print_string(char*strp,int color,int row,int col);

void display_frame(int row,int col,int hig,int len)
{
     int i,j;
     //print_string("┏━━━━━━━━━━━┓",15,row,col);
     for(i=0;i<len+2;i++)//第一行。
     print_char('+',2,row,col+i);
     move_curse(row+1,col+1);
     save_curse(row+1,col+1);
     //for(i=0;i<hig;i++)//中间。
     //print_string("┃                      ┃",2,++row,col);
     for(i=0;i<hig;i++)//中间。
     {
                               print_char('+',2,++row,col);
                               for(j=1;j<len;j++)
                               print_char(' ',15,row,col+j);
                               print_char('+',2,row,col+j+1);
     }
     //print_string("┗━━━━━━━━━━━┛",15,row,col);
     for(i=0;i<len+2;i++)//最后一行。
     print_char('+',2,row,col+i);
}

void practice_text()
{
     unsigned char t;
     inb(0x60,t);
     test_keyboard_int();
}

void edit_text()
{
     text_frame_x=14; //row
     text_frame_y=50; //col
     text_frame_h=9; //hign
     text_frame_l=23;
     display_frame(text_frame_x,text_frame_y,text_frame_h,text_frame_l);
     practice_text();
}

void disp_my_msg()
{
     int i=9,j=15,k=1;
     char head_line[80]={"---------------------------gc cos---------------------------"};
     char some_msg1[80]={"      this computer operating system is just for a try .    "};
     char some_msg2[80]={"  so it is very simply and its functions is very limited .  "};
     char some_msg3[80]={"      at the beginning , you will hear a music << happy     "};
     char some_msg4[80]={"  birthday to you >> to make the os vivid and dramatic .    "};
     char some_msg5[80]={"  and then there will be two processes .                    "};
     char some_msg6[80]={"      code is very mussy ......                             "};
     char some_msg7[80]={"                                       ---author gaocheng   "};
     char some_msg8[80]={"                                                     2007   "};
     char some_msg9[80]={"============================================================"};
     char other_msg0[32]={"-------contact me-------"};
     char other_msg1[32]={"email: 155360026@163.com"};
     char other_msg2[32]={"QQ:    155360026"};

     //----some message----
     print_string(head_line,15,k++,i);
     print_string(some_msg1,15,k++,i);
     print_string(some_msg2,15,k++,i);
     print_string(some_msg3,15,k++,i);
     print_string(some_msg4,15,k++,i);
     print_string(some_msg5,15,k++,i);
     print_string(some_msg6,15,k++,i);
     //-------
     print_string(some_msg7,15,k++,i);
     print_string(some_msg8,15,k++,i);
     print_string(some_msg9,15,k++,i);
     //-----other message----
     print_string(other_msg0,15,j++,i);
     print_string(other_msg1,15,++j,i);
     print_string(other_msg2,15,++j,i);
}

