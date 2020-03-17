/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : init.c
 * version     : 1.0
 * description : 内核初始化文件
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <vbs.h>
#include <vds.h>
#include <vms.h>
#include <vts.h>

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
LOCALC os_void init_vts(os_void)
{
    /* 初始化cpu调试寄存器 */
    init_debugger();

    /* 初始化任务 */
    init_task();

    /* 主任务初始化 */
    init_core_task();

    init_watch_dog();

    /* 初始化系统任务 */
    init_desktop_task();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_vms(os_void)
{
    /* 初始化内核静态内存 */
    init_ksm();

    init_kpm();

    /* 初始化内存控制块 */
    init_kdm();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_vbs(os_void)
{
    /* 初始化中断控制器 */
    init_apic();

    /* 初始化信号量 */
    init_semaphore();

    /* 初始化station */
    init_task_station();

    /* 初始化空闲station id */
    init_idle_station();

    /* 初始化消息控制块 */
    init_bmmcb();

    /* 初始化窗口 */
    init_window_class();

    /* 初始化窗口句柄资源表 */
    init_window_handle_tab();

    /* 初始化窗口资源表 */
    init_idle_window_rc();

    /* 初始化pci总线 */
    init_pci();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_vds(os_void)
{
    /* 初始化系统定时器 */
    init_pit_int();

    /* 初始化键盘 */
    init_keyboard_int();

    /* 初始化实时定时器 */
    init_rtc_int();

    /* 初始化硬盘中断 */
    init_harddisk_int();

    Eth_Init();

    /* 初始化图像 */
    init_image();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void idle(os_void)
{
    sti();
    for (;;) {
        hlt();
        hlt();
        hlt();
        hlt();
    }
}

#define IVT_DATA_LEN 0x400
LOCALD os_u8 data[IVT_DATA_LEN];

/***************************************************************
 * description : 初始化内核
 * history     :
 ***************************************************************/
os_void save_data(os_u8 *addr)
{
    os_u32 i;

    for (i = 0; i < IVT_DATA_LEN; i++) {
        data[i] = *addr;
        addr++;
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void check_data(os_u8 *addr)
{
    os_u32 i;

    for (i = 0; i < IVT_DATA_LEN; i++) {
        if (data[i] != *addr) {
            flog("ram 0x%x is broken\n", addr);
            return;
        }
        addr++;
    }
}

/***************************************************************
 * description : 初始化内核
 * history     :
 ***************************************************************/
os_void init_ukernel(os_void)
{
    /* brain: 初始化cpu */
    init_processor();

    /* 文本模式打印 */
    show_text("ucore...", 1);

    /* eye: 显卡设置为图形模式.
     * some HP computers do not support VGA!
     */
    open_graphics_mode(GRAPHICES_MODE_SVGA);

    /* open_graphics_mode makes ram 0x10c changed */
    save_data(0);

    /* mouth: 初始化print */
    init_print();
    init_paint();

    /* 关中断 */
    cli();

    /* 初始化调试功能 */
    init_dump();

    /* memory: 初始化内存 */
    init_vms();

    /* brain: 初始化任务 */
    init_vts();

    /* nerve: 初始化总线 */
    init_vbs();

    /* body: 初始化外设 */
    init_vds();

    flog("ksm: %x\n", get_current_ksm());

    /* 启动加载 */
    active_loader();

    check_data(0);

    idle();
}

