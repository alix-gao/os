
#include<process.h>
//---
#ifndef _global_cite_h
#define _global_cite_h
extern task_struct process_table[]; //not process link table.
extern volatile int state[]; //state,could set it in process_table.
extern volatile char int_c; //display for time int
extern volatile int last_task_no; //used by time int,to switch task
//extern int time_int_switch;
//extern int process_ing_no;
extern int text_frame_x;
extern int text_frame_y;
extern int text_frame_h;
extern int text_frame_l;
extern volatile int my_task_no;
extern int text_g;
//----
//#ifndef _global_cite_h
//#define _global_cite_h
//#define p_no    0
//#define p_ready 1
//#define p_ing   2
//#define p_ed    3
////---
////#define my_task_no 3
//--------
#endif

