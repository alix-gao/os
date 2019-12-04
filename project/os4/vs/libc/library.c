/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : library.c
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <typedef.h>

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
void _dump_stack(uint __cdecl (*show)(const u8 *format, ...), u32 stack_frame)
{
    u8 *reg_eip;
    /* u8 prolog[] = {0xec, 0x8b, 0x55}; //5589e5 */

    if (show) {
        show("stack info: ");
    }

    /* stop when occur task_wrapper_func */
    while (0 != stack_frame) {
        /* return address */
        reg_eip = (u8 *)(*(u32 *)(stack_frame + 4));
        /* the value of the frame pointer of the caller */
        stack_frame = *(u32 *) stack_frame; /* caller function variables address */

        /* do not use kmp */
        while (NULL != reg_eip) {
            if ((reg_eip[0] == 0x55) && (reg_eip[1] == 0x89) && (reg_eip[2] == 0xe5)) {
                if (show) show("%x() ", reg_eip);
                break;
            }
            reg_eip--;
        }
    }
    if (show) show("\n");
}

/***************************************************************
 * description : do not forget modify macro LOCALC
 * history     :
 ***************************************************************/
void dump_stack(uint __cdecl (*show)(const u8 *format, ...))
{
    u32 reg_ebp;

    __asm__ __volatile__("movl %%ebp, %0"
                        :"=g"(reg_ebp)
                        :
                        :"memory");

    _dump_stack(show, reg_ebp);
}

