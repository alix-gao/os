
#include<message_types.h>
#include<message_cite.h>
#include<global_cite.h>
#include<process.h>
//-----------------------
extern void disp_color(int begx,int begy,int endx,int endy,int color);
extern bool get_message(msg*msg);
extern void modify_descriptor(desc_struct*addr,int no,bit_32 base,bit_32 limit,bit_16 attribute);
extern void init_tss(task_struct*p,int i,int task_addr);
extern void init_ldt(task_struct*p,int i);
extern void dead();
extern void print_char(char strp,int color,int row,int col);
extern void time_delay_i(int time);
extern void print_char_g(int offset,int x,int y,int color);
extern void init_application(int begx,int begy,int endx,int endy,int no);
extern void init_instance(int begx,int begy,int endx,int endy,int color);
extern int note_book(int begx,int begy,int endx,int endy,int offset,int char_pos,int color);
extern void create_window_gc(int no);
extern void registerclass_gc(wndclass_gc*wc);
extern void dispatchmessage(msg*msg);
extern void print_int(int disp,int color,int row,int col);

extern desc_struct*gdt_addr;

void wndproc_1(msg*tmp_msg)//消息处理函数。
{
     static int char_pos=2;//
     //print_char('t',14,14,0);
     //print_char_g(tmp_msg->wparam,40,40,3);
     disp_color(1,1,10,10,tmp_msg->wparam%16);
     char_pos=note_book(260,220,420,380,tmp_msg->wparam,char_pos,4);
}

void init_application_gc_1()
{
     wndclass_gc wc;

     wc.begx=260;
     wc.begy=220;
     wc.endx=420;
     wc.endy=380;
     wc.no=1;
     wc.color=4;
     wc.lpfnwndproc=wndproc_1;

     registerclass_gc(&wc);
}

void init_instance_gc_1()
{
     //create_windows.
     create_window_gc(1);//wc.no/time_window_no.
}

void msg_task_1() //第一个进程。
{//int t;
     //int i;
     //state[1]=p_ing;
     //__asm__("sti");
//     //real code.-------------------------------------------
//     for(i=0;i<80;i++)
//     {
//                      print_char('t',14,14,i);
//                      disp_color(0,0,10,10,i%16);
//                      time_delay_i(10);//slow:(10).medium(5).fast:(1)
//     }
//     //real code.-------------------------------------------
//     dead();
     init_application_gc_1();
     init_instance_gc_1();
     msg task_msg;
     while(get_message(&task_msg))//消息循环。
     {//t=task_msg.time;//print_int(32,15,0,1);
                                               //if(task_msg.time!=t)//(task_msg.time)
                                               //disp_color(1,10,10,20,2);
                                               //{print_int(task_msg.time,15,1,1);print_int(t,15,2,1);}
                                               //if(task_msg.time)
                                               //regist_window_buffer[time_window_no].lpfnwndproc(&task_msg);
                                               dispatchmessage(&task_msg);
     }
     //有了上面的消息循环，下面的就不执行了。
     //因为我这里面没设置msg_close的发送。
     //其实，msg_close可以结束这个while。
     //__asm__("ljmp $7*8,$0x2000");
}

void wndproc_2(msg*tmp_msg)
{
     static int char_pos=2;
     //static int note_book_record[30][20];//最大写2页more纸。
     //print_char('h',14,15,0);
     disp_color(11,11,20,20,tmp_msg->wparam%16);
     char_pos=note_book(460,40,620,200,tmp_msg->wparam,char_pos,2);
}

void init_application_gc_2()
{
     wndclass_gc wc;

     wc.begx=460;
     wc.begy=40;
     wc.endx=620;
     wc.endy=200;
     wc.no=2;
     wc.color=2;
     wc.lpfnwndproc=wndproc_2;

     registerclass_gc(&wc);
}

void init_instance_gc_2()
{
     create_window_gc(2);
}

void msg_task_2()
{//int t;
     //int i;
     //state[2]=p_ing;
     //__asm__("sti");
//          for(i=0;i<80;i++)
//     {
//                      print_char('h',14,15,i);
//                      disp_color(10,10,20,20,i%16);
//                      time_delay_i(10);//slow:(10).medium(5).fast:(1)
//     }
//     dead();
     //real code.-------------------------------------------
     init_application_gc_2();
     init_instance_gc_2();
     msg task_msg;
     while(get_message(&task_msg))//消息循环。
     {//t=task_msg.time;
                                               //if(task_msg.time!=t)//(task_msg.time)
                                               //disp_color(11,10,20,20,4);
                                               //{print_int(task_msg.time,15,3,1);print_int(t,15,4,1);}
                                               //regist_window_buffer[time_window_no].lpfnwndproc(&task_msg);
                                               dispatchmessage(&task_msg);
                                               //if(task_msg.time)
                                               //regist_window_buffer[time_window_no].lpfnwndproc(&task_msg);//ok
//                                  wndproc_2(&task_msg);//ok
     }
     //__asm__("ljmp $3*8,$0x2000");
}

void init_msg_test()
{//while(1);
     __asm__("cli");

     int task_1=3;
     int task_2=4;
     process_table[task_1].pid=0x000001;//find it
     init_tss(process_table,task_1,(bit_32)msg_task_1); //task 0
     init_ldt(process_table,task_1);
     //--//o,null.1,code.2,data.--
     modify_descriptor(gdt_addr,11,(bit_32)(&(process_table[task_1].tss)),0xfffff,at_386_tss+d_32);//tss0 0x12345678
     modify_descriptor(gdt_addr,12,(bit_32)(process_table[task_1].ldt),0xfffff,at_ldt+d_32);//ldt
     //////////////////////////
     process_table[task_1].pid=0x000002;//find it
     init_tss(process_table,task_2,(bit_32)msg_task_2); //task 0
     init_ldt(process_table,task_2);
     //--//o,null.1,code.2,data.--
     modify_descriptor(gdt_addr,13,(bit_32)(&(process_table[task_2].tss)),0xfffff,at_386_tss+d_32);//tss0 0x12345678
     modify_descriptor(gdt_addr,14,(bit_32)(process_table[task_2].ldt),0xfffff,at_ldt+d_32);//ldt

     //init_state();
     //init_switch();
     state[3]=state[4]=task_status_ready;
     my_task_no=5;

     __asm__("sti");
}

