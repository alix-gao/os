/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : init.c
 * version     : 1.0
 * description : �ں˳�ʼ���ļ�
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <configuration.h>
#include <core.h>

/***************************************************************
 global variable declare
 ***************************************************************/
/* �ں˾�̬�ڴ��ַ */
LOCALD os_u32 ksm_addr = 0;
/* �����ں˾�̬�ڴ� */
#define ABSOLUTE_KSM_SIZE 0x1000000
LOCALD os_u32 aksm_addr = 0;

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_u64 _alloc_ksm(os_u32 *base, os_u32 len, os_u32 doffset)
{
    os_u32 offset = 0;
    os_u32 mask = 0;
    os_u32 static_addr = 0;

    if (0 == base) {
        /* panic */
        return 0;
    }

/* �ں˾�̬�ڴ�ֱ��� 2^n */
#define OS_KSM_POWER 16

    /* 1<<OS_KSM_RESOLUTION - 1 (error) */
    mask = 1 << OS_KSM_POWER;
    mask = mask - 1;
    offset = (*base & mask) ? (1 << OS_KSM_POWER) : 0;

    mask = ~mask;
    static_addr = (*base & mask) + offset; // ()����Ҫ��!

    /* fresh ksm_addr */
    *base = static_addr + len;

    /* map physical address to virtual address */
    amap(static_addr + doffset, static_addr, len);

    do {
        os_uint i;
        static uint cc = 1;
        cc++;
        for (i = 0; i < (len >> 12); i++) {
            *(os_u32 *)(static_addr + doffset + i * 0x1000) = cc + i;
        }
        for (i = 0; i < (len >> 12); i++) {
            if (*(os_u32 *)(static_addr + i * 0x1000) != cc + i) {
                print("error addr %x %x %x\n", static_addr + i * 0x1000, *(os_u32 *)(static_addr + i * 0x1000), *(os_u32 *)(static_addr + doffset + i * 0x1000));
                dead();
            }
        }
    } while (0);

    /* return virtual address */
    return static_addr + doffset;
}

/***************************************************************
 * description : ������̬�ڴ��������
 *               ��ʼ��ʱ���Ե���, ��֧������, �첽.
 * history     :
 ***************************************************************/
os_u64 alloc_ksm(os_u32 len)
{
    return _alloc_ksm(&ksm_addr, len, OS_DYNAMIC_MEM_OFFSET);
}

/***************************************************************
 * description : allocate absolute kernel static memory (absolute virtual address)
 *               virtual memory whose linear address is identity mapped to physical address
 * history     :
 ***************************************************************/
os_u64 alloc_ksm_absv(os_u32 len)
{
    os_u64 abs_addr;

    abs_addr = _alloc_ksm(&aksm_addr, len, 0);
    cassert((get_end_section_addr() + ABSOLUTE_KSM_SIZE) >= abs_addr);
    return abs_addr;
}

/***************************************************************
 * description : ��ʼ����̬�ڴ�
 * history     :
 ***************************************************************/
os_void init_ksm(os_void)
{
    aksm_addr = get_end_section_addr();
    ksm_addr = get_end_section_addr() + ABSOLUTE_KSM_SIZE;
}

/***************************************************************
 * description : ��ѯ��ǰ��̬�ڴ�ʹ�����
 * history     :
 ***************************************************************/
os_u32 get_current_ksm(os_void)
{
    return ksm_addr;
}

/***************************************************************
 * description : ��ѯ��ǰ��̬�ڴ���ʼ��ַ
 * history     :
 ***************************************************************/
os_u32 ksm_base(os_void)
{
    return get_end_section_addr() + ABSOLUTE_KSM_SIZE;
}

