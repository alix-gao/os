
#include<message_types.h>
//��global.hһ����ֻ���ڳ���һ�Ρ�
//���Ծ�������global.h��ͬһ���ط����ְɡ�

//���ﱾ���붨��һ����ǰ�����ID�ı�����
//������process_ing_no�����档

volatile int active_window_id=0;//����ڵ�ID�ţ���ʼΪϵͳ���ڡ�
volatile int window_count=3;//���д��ڵĸ�����
//���д��ڵ���Ϣ���С�
//������Ұ�ÿ�����ڵ���Ϣ���д�С�����1��
//�൱�ڻ������ڴ�СΪ1��
volatile msg window_msg_queue[window_max_count];
volatile int time_window_no=0;
volatile wndclass_gc regist_window_buffer[window_max_count];

