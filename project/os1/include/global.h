
#include<process.h>
/* ��ͷ�ļ�ֻ������һ�� */
//here are some globle value
//not macro define.
//ȫ�ֱ���
//---
task_struct process_table[max_count_process]; //not process link table.
volatile int state[max_count_process]; //state,could set it in process_table.
//int process_ing_no;//��ǰ������ڽ������������ƫ���������ߵڼ���(0~2)���̡�
volatile char int_c; //display for time int
volatile int last_task_no; //used by time int,to switch task
//int time_int_switch;
int text_frame_x;
int text_frame_y;
int text_frame_h;
int text_frame_l;
volatile int my_task_no=3;
int text_g=0;
//----
//#define p_no    0
//#define p_ready 1
//#define p_ing   2
//#define p_ed    3
//---
//#define my_task_no 3
//--------

