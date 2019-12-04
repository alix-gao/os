/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : configuration.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
/* tick: 10ms */
#define OS_HZ 100

/* idt addr */
#define IDT_ADDR 0x100000

/* gdt addr */
#define GDT_ADDR 0x110000

/* ҳĿ¼���ַ */
#define PAGE_DIR_ADDR   0x120000

/* ҳ���ַ */
#define PAGE_TABLE_ADDR 0x400000

/* �ں��������ջ���� */
#define OS_STACK_LEN 0x100000
#define OS_STACK_BOTTOM 0x800000

/* �ں������ջ���� */
#define OS_KTASK_STACK_LEN 0x4000

/* ϵͳ���������� */
#define SYS_TASK_NUM 0x100

/* ʵģʽ�´�������ջ�� */
#define RM_CODE_STACK_SEG_NO 8

/* ʵģʽ�����ݶ� */
#define RM_DATA_SEG_NO 9

/* it's a joke */
#define OS_DYNAMIC_MEM_OFFSET 0x40000000

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/

/***************************************************************
 extern function
 ***************************************************************/

#pragma pack()

#endif

