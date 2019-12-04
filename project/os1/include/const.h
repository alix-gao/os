
//here are some const value.
//use macro define......
#ifndef _const_h
#define _const_h
//--------�洢������������ֵ˵��---------
#define at_data_read 0x90 //���ڵ�ֻ�����ݶ�����ֵ
#define at_data_write 0x92 //���ڵĿɶ�д���ݶ�����ֵ
#define at_data_write_a 0x93 //���ڵ��ѷ��ʿɶ�д���ݶ�����ֵ
#define at_code_exe 0x98 //���ڵ�ִֻ�д��������ֵ
#define at_code_exe_read 0x9a //���ڵĿ�ִ�пɶ����������ֵ
#define at_code_consistent 0x9c //���ڵ�ִֻ��һ�´��������ֵ
#define at_code_consistent_read 0x9e //���ڵĿ�ִ�пɶ�һ�´��������ֵ
//--------------------------------

//-------ϵͳ����������������������ֵ˵��---------
#define at_ldt 0x82 //�ֲ��������������ֵ
#define at_task_gate 0x85 //����������ֵ
#define at_386_tss 0x89 //386tss����ֵ
#define at_386_c_gate 0x8c //386����������ֵ
#define at_386_i_gate 0x8c //386�ж�������ֵ
#define at_386_t_gate 0x8c //386����������ֵ
//----------------------

//-----dpl��rpl------
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
#define d_32 0xc000 //0x4000 //32λ����α�־
#define til 0x4 //ti=1�����������־��
#define vmfl 0x2 //vmf=1
#define ifl 0x200 //if=1
//-------------

#define max_count_process 23 //���������Ϊ23����
#define ldt_size 8 //ldt�����Ϊ8����������
#define task_stack_size 100 //����ջ��С
//----
#define SA_RPL_MASK 0xFFFC
#define SA_TI_MASK 0xFFFB
//--Ȩ��--
#define RPL_KRNL 0
#define RPL_TASK 1
#define RPL_USER 3

#endif

