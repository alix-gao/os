/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : core.h
 * version     : 1.0
 * description : core������Ϣ
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __CORE_H__
#define __CORE_H__

/***************************************************************
 include header file
 ***************************************************************/
#include <lib.h>
#include <os.h>
#include <configuration.h>
#include <compiler.h>

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/

/* ����ϵͳ�������� = SYS_TASK_NUM % CPU_MAX_TASK_NUM */
#define OS_TASK_NUM (SYS_TASK_NUM & (CPU_MAX_TASK_NUM - 1))

#define INVALID_ID 0

/* ��Ч����id */
#define INVALID_TASK_ID (INVALID_ID)

/* ������id */
#define MAIN_TASK_ID 1

/* ��ռʽ���Ȳ��� */
#define PREEMPTIVE_SCHEDULE

#define init_afunc(x) init_abstract_call(x)

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/
/***************************************************************
 * description : ������
 ***************************************************************/
struct task_handle {
    os_u32 core_id; // hardware
    os_u32 cpu_id; // hardware
    os_u32 task_id; // software
#define HTASK_CHECK 0xaaaaaaaa
    os_u32 check;
};

/***************************************************************
 * description : HWINDOW, ���ھ�� (��Ϣ����ʵ��)
 ***************************************************************/
struct window_handle {
    /* �߳̾�� */
    struct task_handle *htask;
    os_u32 window_id;
};

/***************************************************************
 * description : HDEVICE, �豸���
 ***************************************************************/
struct device_context {
    /* ���ڵ���Ļ����ϵ */
    screen_csys csys;
    /* ���ڵ�ǰ����ɫ */
    enum vga_color foreground_color;
    /* ���ڵı�����ɫ */
    enum vga_color background_color;
    /* ���ڿ��,x */
    os_u32 width;
    /* ���ڳ���,y */
    os_u32 length;
    /* ����λ�� */
    cursor_csys cursor_pos;
    /* �ֺŴ�С */
    os_u32 font_size;
};

/***************************************************************
 extern function
 ***************************************************************/

#define kmalloc(size) alloc_kdm(size, 1, __LINE__, __FILE__)
#define kfree(addr) free_kdm((os_void **)&(addr), __LINE__)

/* ����һ���Ե�4G���ڴ�ҳ */
#define cmalloc(size, align) alloc_coherent_mem(size, align, __LINE__)
#define cfree(addr) free_coherent_mem((os_void **)&(addr), __LINE__)

#include <vms.h>
#include <vts.h>
#include <vbs.h>
#include <vds.h>

#pragma pack()

#endif /* end of header */

