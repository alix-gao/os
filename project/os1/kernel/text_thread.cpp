
#include<global_cite.h>

extern void time_delay(int time);
extern void print_char(char strp,int color,int row,int col);

void dead()
{
     while(1);
}

void time_delay_i(int time)
{
     int i,j;
     for(i=0;i<64*time;i++)
     for(j=0;j<32000;j++)
     {
                         ;
     }
}

void my_task_1() //��һ�����̡�
{
     int i;
     //state[1]=p_ing;
     //__asm__("sti");
     //real code.-------------------------------------------
     //state[1]=p_ed;//for test
     for(i=0;i<80;i++)
     {
                      print_char('P',14,12,i);
                      //time_delay_i(10);//slow:(10).medium(5).fast:(1)
                      time_delay(50);
     }
     //real code.-------------------------------------------
     state[1]=task_status_zombie;
     dead();
//     //�����������Ϣѭ��������ľͲ�ִ���ˡ�
//     //��Ϊ��������û����msg_close�ķ��͡�
//     //��ʵ��msg_close���Խ������while��
//     for(i=0;i<80;i++)
//     {
//                      print_char('P',14,12,i);
//                      time_delay_i(10);//slow:(10).medium(5).fast:(1)
//     }
//     //real code.-------------------------------------------
//     state[1]=p_ed;
//     //__asm__("ljmp $7*8,$0x2000");
//     dead();//dead
}

void my_task_2()
{
     int i;
     //state[2]=p_ing;
     //__asm__("sti");
     //state[2]=p_ed;//for test
     for(i=0;i<80;i++)
     {
                      print_char('S',11,13,i);
                      //time_delay_i(10);
                      time_delay(50);
     }
     state[2]=task_status_zombie;
     dead();
     //__asm__("ljmp $3*8,$0x2000");
}

