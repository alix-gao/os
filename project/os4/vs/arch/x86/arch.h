/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : arch.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2013-04-22
 ***************************************************************/

#ifndef __ARCH_H__
#define __ARCH_H__

/***************************************************************
 include header file
 ***************************************************************/
#include <compiler.h>
#include <cpu.h>
#include <instruction.h>
#include <atomic.h>

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/

/* intel 8253/8254 frequency: 1.193180MHZ */
#define I8253_FREQ 1193187
/* counter0: irq0 */
#define I8253_COUNTER0_ADDR 0x40
/* counter1: fresh dram */
#define I8253_COUNTER1_ADDR 0x41
/* counter2: speaker */
#define I8253_COUNTER2_ADDR 0x42
#define I8253_CMD_ADDR 0x43

/* pci */
#define PCI_ADDR_REG 0xcf8
#define PCI_DATA_REG 0xcfc

#define le16_to_cpu(val) (val)
#define le32_to_cpu(val) (val)

#define __le16 os_u16
#define __le32 os_u32

#define get_caller_func(c) do { __asm__ __volatile__("movl 4(%%ebp), %%eax; movl %%eax,%0" :"=g"(c) : :"eax", "memory"); } while (0)

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

