/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : main.c
 * version     : 1.0
 * description : virtual system
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <lib.h>
#include <initial.h>

/***************************************************************
 global variable declare
 ***************************************************************/
#define END_FLAG 0x00521029
LOCALD u32 cvs_end __attribute__((section(".end_section"))) = END_FLAG;

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description : 获取最后段的地址
 * history     :
 ***************************************************************/
u32 STARTUP get_end_section_addr(void)
{
    return (pointer) &cvs_end;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC void STARTUP modify_return_addr(void)
{
    __asm__ __volatile__("pushl %%eax\n\t"
                         "movl 0x04(%%ebp),%%eax\n\t"
                         "inc %%eax\n\t"
                         "movl %%eax,0x04(%%ebp)\n\t"
                         "popl %%eax\n\t"
                         ::); /* 不需要通知编译器寄存器的使用情况 */
}

/***************************************************************
 * description : 反跟踪代码
 *               此处只能反函数级跟踪, 如F10
 *               对于单步跟踪F11无能为力
 * history     :
 ***************************************************************/
LOCALC void STARTUP anti_track(void)
{
    modify_return_addr();

    /* 函数级跟踪将会错误的等待下面的指令 */
    __asm__ __volatile__(".byte 0x78\n\t");
}

#include <stdarg.h>
/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC void STARTUP init_lib(void)
{
GLOBALREFC uint vprint_buffer(const u8 *format, va_list args);
    register_print(vprint_buffer);
}

/***************************************************************
 * description : main()
 * history     :
 ***************************************************************/
void STARTUP os_main(void)
{
    /* intergrate check */
    cassert(END_FLAG != cvs_end);

    /* 初始化库函数 */
    init_lib();

    /* 反跟踪 */
    anti_track();

    /* 内核初始化 */
    init_ukernel();
}

