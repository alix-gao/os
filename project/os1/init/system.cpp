
#include<const.h>
#include<io.h>
#include<extern_function.h>
#include<global.h>//ֻ���������Ψһ��һ�Ρ�
#include<message.h>
#include<message_types.h>

extern char*curse_x;
extern char*curse_y;

void system_pause()
{
     __asm__("hlt");//--ʡ��-- sti;\ -����'\'-
}

void genuine_main()
{
    char sign[13]={"devcpp......"};
    char cos_gen_time[22]={"1969,1972,1987,1991,."};
    char music_freq[60]={2,62,2,62,2,94,2,62,3,49,3,30,2,62,2,62,2,94,2,62,3,92,3,49,2,62,2,62,5,23,4,40,3,49,3,30,2,94,4,66,4,66,4,40,2,62,3,92,3,49,0,0};
    char music_time[30]={25,25,50,50,50,100,25,25,50,50,50,100,25,25,50,50,50,50,125,25,25,50,50,50,100};
    char dis_c={'T'};

    clean_screen(0,24);//��0�е���11�С�

    print_string(sign,15,10,10);//6������ɫ��10�У�10�С�
    print_string(cos_gen_time,15,6,10);
    print_char(dis_c,2,10,1);
    print_int(256,15,6,5);
    print_int((int)cos_gen_time,20,20,1);
    move_curse(8,3);
    //------
    init_msg_queue();
    text_g=0;//**************
    test_time_int();
    //dead();
    //system_pause();
    //-----
    //go_task();//should omit
    print_char('#',13,11,38);
    edit_text();
    disp_my_msg();
    //ͼ�β���ϵͳ�£�����ľ�û���ˡ�
    //init_graphics();
    play_music(music_freq,music_time);
    while(state[1]!=task_status_zombie&&state[2]!=task_status_zombie)//˫�߳̽���
    {
                                                     system_pause();
    }

    __asm__ __volatile__("cli");

    /* init 8253/8254, channel 0. timer is 10ms. */
    outb_p(0x43, 0x36);
    outb_p(0x40, 0x00);
    outb(0x40, 0x00);

    set_vga_mode();//new

    //dead();
    outb(0x3ce,0x5);
    outb(0x3cf,0x6);
    text_g=1;
    //char*video=(char*)0x0a0000;
    //*video=15;//���Գɹ���

    /* init 8253/8254, channel 0. timer is 10ms. */
    outb_p(0x43, 0x36);
    outb_p(0x40, (1193180/100) & 0xff);
    outb(0x40, (1193180/100) >> 8);

	init_page();

    __asm__ __volatile__("sti");

    openmov();//�������档
    __asm__ __volatile__("int $0x20");
    outb(0x20,0x20);

    inb(0x60,dis_c);//��ռ��̻�������

    __asm__ __volatile__("cli");
    outb(0x21,0xfc);
    outb(0xa1,0xff);
    __asm__ __volatile__("sti");
    init_msg_test();
//    init_mouse();
//    //__asm__("int $0x32":::);//test mouse int.
//    start_elsfk();

    /* all init is here */
    kcurse_init();
    kmem_init();

    /* driver all init */
    driver_init();

    /* ipc all init */
    ipc_init();

    //save power.
    while(1)
    {
            //print_char(dis_c,15,11,23);dis_c++;
            system_pause();//different from dead
    }
}

