
#include<message_types.h>

#ifndef _message_cite_h
#define _message_cite_h

extern volatile int active_window_id;
extern volatile int window_count;
extern volatile msg window_msg_queue[];
extern volatile int time_window_no;
extern volatile wndclass_gc regist_window_buffer[window_max_count];

#endif

