/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : int.h
 * version     : 1.0
 * description : 中断类cpu指令宏定义
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __INT_H__
#define __INT_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/

/* memory, 寄存器中的buffer全部写入内存 */
#define cli() __asm__ __volatile__("cli":::"memory")

#define sti() __asm__ __volatile__("sti":::"memory")

#define hlt() __asm__ __volatile__("hlt")

/***************************************************************
 * description :
 ***************************************************************/
typedef struct {
    volatile u32 eflag;
} lock_t;

/* 保存标志寄存器 */
#define save_eflag(x) __asm__ __volatile__("pushfl; popl %0" :"=g"(x.eflag):/* no input */:"memory")

/* 恢复标志寄存器 */
#define restore_eflag(x) __asm__ __volatile__("pushl %0; popfl":/* no output */:"g"(x.eflag):"memory","cc")

/* 备份eflag, 锁中断 */
#define lock_int(x) __asm__ __volatile__("pushfl; popl %0; cli" :"=g"(x.eflag):/* no input */:"memory")

/* 恢复eflag */
#define unlock_int(x) __asm__ __volatile__("pushl %0; popfl":/* no output */:"g"(x.eflag):"memory","cc")

#define dead() \
    do { \
        sti(); \
        while (1) hlt(); \
    } while (0)

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

#endif /* end of header */

