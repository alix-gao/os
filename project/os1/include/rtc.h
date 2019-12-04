
#ifndef _rtc_h
#define _rtc_h

#include <kmem_struct.h>

#define timer_mem_map_num 2*(2*2*2*2*2*2*2*2*2*2*2-1)+2

#define timer_mem_setup_num 13

enum
{
    enum_timer_mem_not_use,
    enum_timer_mem_start_end,
    enum_timer_mem_start_no_end,
    enum_timer_mem_mid,
    enum_timer_mem_end,
    enum_timer_mem_butt
};

/* struct define */
#define timer_mem_setup_node_stru kernel_mem_setup_node_stru

#define timer_mem_map_node_stru kmem_map_node_stru

extern void timer_mem_init();

#endif

