
//��Ϣ���ơ�
#include<message_types.h>
#include<message_cite.h>

void init_msg_queue()
{
     int i;
     //ʹ��time��Ϊ��Ϣ�Ƿ���ڵı�־��
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

//��������൱���ں˵���Ϣ��������
//���ں˲������ж���Ϣ�����͵���Ӧ�Ĵ�����Ϣ���С�
//�ں���Ϣ���п�����һ�����飬��Ϊ��������
//������û�û��������൱�ڻ�������Ϊ1��
void send_message_to_kernal(msg tmp_msg)
{
     //static char ch='a';
     //print_char(ch++,15,20,0);
     __asm__("cli");
     window_msg_queue[active_window_id].wparam=tmp_msg.wparam;//send.
     //......
     window_msg_queue[active_window_id].time=1;//����Ϣ��
     __asm__("sti");
}

//����Ϣ������ȡ���Լ����ڵ���Ϣ��
//no����Ϣ���е�ƫ������
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
                                   return true;//����û����Ϣ�ȴ���
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

