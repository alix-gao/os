
//here are some const value.
//use macro define......
#ifndef _const_h
#define _const_h
//--------存储段描述符类型值说明---------
#define at_data_read 0x90 //存在的只读数据段类型值
#define at_data_write 0x92 //存在的可读写数据段类型值
#define at_data_write_a 0x93 //存在的已访问可读写数据段类型值
#define at_code_exe 0x98 //存在的只执行代码段类型值
#define at_code_exe_read 0x9a //存在的可执行可读代码段类型值
#define at_code_consistent 0x9c //存在的只执行一致代码段类型值
#define at_code_consistent_read 0x9e //存在的可执行可读一致代码段类型值
//--------------------------------

//-------系统段描述符和门描述符类型值说明---------
#define at_ldt 0x82 //局部描述符表段类型值
#define at_task_gate 0x85 //任务门类型值
#define at_386_tss 0x89 //386tss类型值
#define at_386_c_gate 0x8c //386调用门类型值
#define at_386_i_gate 0x8c //386中断门类型值
#define at_386_t_gate 0x8c //386陷阱门类型值
//----------------------

//-----dpl和rpl------
#define dpl_1 0x20 //dpl=1
#define dpl_2 0x40 //dpl=2
#define dpl_3 0x60 //dpl=3
#define rpl_1 0x1 //rpl=1
#define rpl_2 0x2 //rpl=2
#define rpl_3 0x3 //rpl=3
#define io_pl_1 0x1000 //iopl=1
#define io_pl_2 0x2000 //iopl=2
#define io_pl_3 0x3000 //iopl=3
//-------------------

//------------
#define d_32 0xc000 //0x4000 //32位代码段标志
#define til 0x4 //ti=1（描述符表标志）
#define vmfl 0x2 //vmf=1
#define ifl 0x200 //if=1
//-------------

#define max_count_process 23 //进程数最大为23个。
#define ldt_size 8 //ldt中最大为8个描述符。
#define task_stack_size 100 //任务栈大小
//----
#define SA_RPL_MASK 0xFFFC
#define SA_TI_MASK 0xFFFB
//--权限--
#define RPL_KRNL 0
#define RPL_TASK 1
#define RPL_USER 3

#endif

