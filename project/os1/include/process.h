
//���ݽṹ����
#ifndef _process_h
#define _process_h

#include<types.h>
#include<const.h>

/* task status */
#define task_status_ready           0
#define task_status_running         1
#define task_status_interruptible   2
#define task_status_uninterruptible 3
#define task_status_stopped         4
#define task_status_zombie          5

//----------------------------
typedef struct bad_descriptor //���-�洢��������/ϵͳ��������
{
    bit_16 limit; //�ν���
    bit_16 base_l; //�λ���ַ��0-15��
    bit_8 base_m; //�λ���ַ��16-23��
    bit_16 attribute; //������
    bit_8 base_h; //�λ���ַ��24-31��
}bad_descriptor;
/*
typedef struct s_descriptor    //��Ԩ
{
    t_16    limit_low;
    t_16    base_low;
    t_8        base_mid;
    t_8        attr1;
    t_8        limit_high_attr2;
    t_8        base_high;
}s_DESCRIPTOR;
*/
typedef struct desc_struct //use it
{
    bit_8 limit_l_1;
    bit_8 limit_l_2;
    bit_8 base_l_1;
    bit_8 base_l_2;
    bit_8 base_m;
    bit_8 attr1;
    bit_8 limit_high_attr2;
    bit_8 base_h;
}desc_struct;

//--------------------------------

typedef struct bad_tss //����״̬�� task static segment
{
    bit_32    link; //32bit
    bit_32    stack0_len; //0����ջָ��
    bit_16    stack0_sel;
    bit_16  stack0_null; //null,0
    bit_32    stack1_len; //1����ջָ��
    bit_16    stack1_sel;
    bit_16  stack1_null;
    bit_32    stack2_len; //2����ջָ��
    bit_16    stack2_sel;
    bit_16  stack2_null;
    bit_32    cr3;
    bit_16    eip_low; //eip
    bit_16  eip_high;
    bit_32    eflags;
    bit_32    eax;
    bit_32    ecx;
    bit_32    edx;
    bit_32    ebx;
    bit_32    esp_len; //stack2_len
    bit_32    ebp;
    bit_32    esi;
    bit_32    edi;
    bit_16    es_sel;
    bit_16  es_null; //null,0
    bit_16    cs_sel;
    bit_16  cs_null;
    bit_16    ss_sel;
    bit_16  ss_null;
    bit_16    ds_sel;
    bit_16  ds_null;
    bit_16    fs_sel;
    bit_16  fs_null;
    bit_16    gs_sel;
    bit_16  gs_null;
    bit_16    ldtr;
    bit_16  ldtr_null; //null,0
    bit_16    zero; //null,0,16bit
    bit_16    io_base;
    bit_8            io_end; //8bit,0xff
}bad_tss;
//�������ԣ��ڴ�ϵͳ��bit_8����8λ�ģ�bit_16��16λ�ģ�bit_32��32λ�ġ�

typedef struct linus_desc_struct
{
    bit_32  a,b;
}linus_desc_struct;

typedef struct tss_struct //use it
{
    bit_32    link; //32bit
    bit_32    stack0_len; //0����ջָ��
    bit_32    stack0_sel;
    bit_32    stack1_len; //1����ջָ��
    bit_32    stack1_sel;
    bit_32    stack2_len; //2����ջָ��
    bit_32    stack2_sel;
    bit_32    cr3;
    bit_32    eip; //eip
    bit_32    eflags;
    bit_32    eax,ecx,edx,ebx;
    bit_32    esp; //stack2_len
    bit_32    ebp;
    bit_32    esi,edi;
    bit_32    es_sel,cs_sel,ss_sel,ds_sel,fs_sel,gs_sel;
    bit_32    ldtr; //null,0
    bit_16  tss_attr;
    bit_16    trace_bitmap;
    bit_8   bitmap;
}tss_struct;

//------------------------------------

typedef struct task_struct
{
    bit_32        pid;
    tss_struct  tss; //tss
    desc_struct    ldt[ldt_size]; //ldtΪ8����������
    bit_8       stack_0[1024*4]; //0����ջ��
}task_struct;
#endif

