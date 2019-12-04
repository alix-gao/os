
//消息机制。
#include<message_types.h>
#include<message_cite.h>

void init_msg_queue()
{
     int i;
     //使用time作为消息是否存在的标志。
     for(i=0;i<window_max_count;i++)
     {
                                    window_msg_queue[i].hwnd=0;
                                    window_msg_queue[i].message=0;
                                    window_msg_queue[i].lparam=0;
                                    window_msg_queue[i].wparam=0;
                                    window_msg_queue[i].time=0;
                                    window_msg_queue[i].pt=0;
     }
}

//这个函数相当于内核的消息处理函数。
//把内核产生的中断消息，发送到对应的窗口消息队列。
//内核消息对列可以是一个数组，作为缓冲区。
//这里我没用缓冲区，相当于滑动窗口为1。
void send_message_to_kernal(msg tmp_msg)
{
     //static char ch='a';
     //print_char(ch++,15,20,0);
     __asm__("cli");
     window_msg_queue[active_window_id].wparam=tmp_msg.wparam;//send.
     //......
     window_msg_queue[active_window_id].time=1;//有消息。
     __asm__("sti");
}

//从消息队列中取出自己窗口的消息。
//no是消息队列的偏移量。
bool get_message(msg*msg)
{
     __asm__("cli");
     if(!window_msg_queue[time_window_no].time)
     {
                                   msg->hwnd=0;
                                   msg->message=0;
                                   msg->lparam=0;
                                   msg->wparam=0;
                                   msg->time=0;
                                   msg->pt=0;
                                   __asm__("sti");
                                   return true;//假设没有消息等待。
     }
     else
     {
         msg->wparam=window_msg_queue[time_window_no].wparam;
         msg->time=1;//window_msg_queue[time_window_no].time;//important.
         window_msg_queue[time_window_no].time=0;
         __asm__("sti");
         return true;
     }
}

void dispatchmessage(msg*msg)
{
     __asm__("cli");
     if(msg->time)
     regist_window_buffer[time_window_no].lpfnwndproc(msg);
     __asm__("sti");
}

