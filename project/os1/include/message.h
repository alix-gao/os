
#include<message_types.h>
//和global.h一样，只能在出现一次。
//所以就让它和global.h在同一个地方出现吧。

//这里本来想定义一个当前活动窗口ID的变量。
//不能用process_ing_no来代替。

volatile int active_window_id=0;//活动窗口的ID号，初始为系统窗口。
volatile int window_count=3;//所有窗口的个数。
//所有窗口的消息队列。
//在这里，我把每个窗口的消息队列大小设成了1。
//相当于滑动窗口大小为1。
volatile msg window_msg_queue[window_max_count];
volatile int time_window_no=0;
volatile wndclass_gc regist_window_buffer[window_max_count];

