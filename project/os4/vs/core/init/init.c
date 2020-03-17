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
    /* ��ʼ��cpu���ԼĴ��� */
    init_debugger();

    /* ��ʼ������ */
    init_task();

    /* �������ʼ�� */
    init_core_task();

    init_watch_dog();

    /* ��ʼ��ϵͳ���� */
    init_desktop_task();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_vms(os_void)
{
    /* ��ʼ���ں˾�̬�ڴ� */
    init_ksm();

    init_kpm();

    /* ��ʼ���ڴ���ƿ� */
    init_kdm();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_vbs(os_void)
{
    /* ��ʼ���жϿ����� */
    init_apic();

    /* ��ʼ���ź��� */
    init_semaphore();

    /* ��ʼ��station */
    init_task_station();

    /* ��ʼ������station id */
    init_idle_station();

    /* ��ʼ����Ϣ���ƿ� */
    init_bmmcb();

    /* ��ʼ������ */
    init_window_class();

    /* ��ʼ�����ھ����Դ�� */
    init_window_handle_tab();

    /* ��ʼ��������Դ�� */
    init_idle_window_rc();

    /* ��ʼ��pci���� */
    init_pci();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_vds(os_void)
{
    /* ��ʼ��ϵͳ��ʱ�� */
    init_pit_int();

    /* ��ʼ������ */
    init_keyboard_int();

    /* ��ʼ��ʵʱ��ʱ�� */
    init_rtc_int();

    /* ��ʼ��Ӳ���ж� */
    init_harddisk_int();

    Eth_Init();

    /* ��ʼ��ͼ�� */
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
 * description : ��ʼ���ں�
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
 * description : ��ʼ���ں�
 * history     :
 ***************************************************************/
os_void init_ukernel(os_void)
{
    /* brain: ��ʼ��cpu */
    init_processor();

    /* �ı�ģʽ��ӡ */
    show_text("ucore...", 1);

    /* eye: �Կ�����Ϊͼ��ģʽ.
     * some HP computers do not support VGA!
     */
    open_graphics_mode(GRAPHICES_MODE_SVGA);

    /* open_graphics_mode makes ram 0x10c changed */
    save_data(0);

    /* mouth: ��ʼ��print */
    init_print();
    init_paint();

    /* ���ж� */
    cli();

    /* ��ʼ�����Թ��� */
    init_dump();

    /* memory: ��ʼ���ڴ� */
    init_vms();

    /* brain: ��ʼ������ */
    init_vts();

    /* nerve: ��ʼ������ */
    init_vbs();

    /* body: ��ʼ������ */
    init_vds();

    flog("ksm: %x\n", get_current_ksm());

    /* �������� */
    active_loader();

    check_data(0);

    idle();
}

