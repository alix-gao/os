/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : cmd.c
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <lib.h>
#include <os.h>
#include <shell.h>
#include <cmd.h>

/***************************************************************
 global variable declare
 ***************************************************************/

/* cmd���ھ�� */
LOCALD HWINDOW cmd_handle = OS_NULL;

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void cmd_print(os_u8 *str)
{
    struct message *msg;
    os_u32 len;

    len = str_len(str) + 1;
    msg = (struct message *) alloc_msg(sizeof(struct message) + len);
    if (OS_NULL != msg) {
        msg->msg_name = CMD_MSG_INFO;
        msg->msg_len = sizeof(struct message) + len;
        mem_cpy((os_u8 *) msg + sizeof(struct message), str, len);
        send_msg(cmd_handle, msg);
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_exit(os_u32 argc, os_u8 *argv[])
{
    if (0 != argc) {
        return OS_FAIL;
    }

    /* ж�ص����� */
    uninstall_debugger(current_task_handle());
    /* �������� */
    exit_task();
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret display_win(os_u8 *app_name)
{
    HWINDOW handle;
    struct message *msg;
    /* os_u8 app_name[][0x20] = { "cmd", "taskmgr text", "taskmgr graph" };
     * bug: ������c�⺯��memcpy
     */

    handle = lookup_handle_by_app_name(app_name);
    if (OS_NULL == handle) {
        return OS_FAIL;
    }

    msg = (struct message *) alloc_msg(sizeof(struct message));
    if (OS_NULL == msg) {
        return OS_FAIL;
    }

    msg->msg_name = OS_MSG_PAINT;
    msg->msg_len = sizeof(msg);

    if (OS_SUCC != post_msg(handle, msg)) {
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void redisplay_shell(os_void)
{
    os_ret rslt;

    rslt = display_win("taskmgr text");
    if (OS_SUCC != rslt) {
        flog("taskmgr text fresh fail.\n");
    }

    rslt = display_win("taskmgr graph");
    if (OS_SUCC != rslt) {
        flog("taskmgr graph fresh fail.\n");
    }

    rslt = display_win("cmd");
    if (OS_SUCC != rslt) {
        flog("cmd fresh fail.\n");
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_get_vesa(os_u32 argc, os_u8 *argv[])
{
    if (0 != argc) {
        return OS_FAIL;
    }
    show_vesa_mode_info();
    return OS_SUCC;
}

/***************************************************************
 * description : svesa
 *                  800x600
 *               svesa h
 *                  high resolution
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_set_vesa(os_u32 argc, os_u8 *argv[])
{
    screen_csys p0,p1;

    switch (argc) {
    case 0:
        set_desktop_resolution(GRAPHICES_MODE_SAFE);
        break;
    case 1:
        if ('h' != *argv[0]) {
            return OS_FAIL;
        }
        set_desktop_resolution(GRAPHICES_MODE_VESA);
        break;
    default:
        return OS_FAIL;
        break;
    }

    /* ��ʾ����Ӱ */
    p0.x = 0;
    p0.y = 0;
    p1.x = 640;
    p1.y = 480;
    draw_rect(p0, p1, 0xf);
    p0.y = 760;
    p1.y = 760;
    draw_rect(p0, p1, 0xf);

    redisplay_shell();

    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_mouse(os_u32 argc, os_u8 *argv[])
{
    if (0 != argc) {
        return OS_FAIL;
    }
    init_ps2_mouse();
    return OS_SUCC;
}

/***************************************************************
 * description : show memory
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_d(os_u32 argc, os_u8 *argv[])
{
    HDEVICE hdc;
    os_u32 i;
    os_u32 addr;
    os_u32 count;

    if (2 != argc) {
        return OS_FAIL;
    }

    addr = str2num(argv[0]);
    count = str2num(argv[1]);

    hdc = open_hdc(cmd_handle);

    for (i = 0; i < count; i++) {
        kprint(hdc, "%x ", *(os_u32 *) addr);
        addr += 4;
    }

    close_hdc(cmd_handle, hdc);

    return OS_SUCC;
}

/***************************************************************
 * description : show io
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_io(os_u32 argc, os_u8 *argv[])
{
    HDEVICE hdc;
    os_u32 count;
    os_u16 addr;
    os_u8 data;

    if (2 != argc) {
        return OS_FAIL;
    }

    addr = str2num(argv[0]);
    count = str2num(argv[1]);

    hdc = open_hdc(cmd_handle);

     while (count--) {
        inb(addr, data);
        addr++;
        kprint(hdc, "%d ", data);
    }

    close_hdc(cmd_handle, hdc);

    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_cls(os_u32 argc, os_u8 *argv[])
{
    HDEVICE hdc;

    if (0 != argc) {
        return OS_FAIL;
    }

    hdc = open_hdc(cmd_handle);

    /* ˢ���ı��� */
    win_clear_screen(hdc);

    close_hdc(cmd_handle, hdc);

    return OS_SUCC;
}

/* cmd֧�ֵĶϵ����� */
#define CMD_BP_NUM 4

/* �ϵ�ĵ�ַ */
LOCALD os_u32 cmd_bpaddr[CMD_BP_NUM];

/***************************************************************
 * description : ��ֹ���öϵ�debug������
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_break(os_u32 argc, os_u8 *argv[])
{
    os_u32 addr = 0;
    os_u32 i = 0;

    if (1 != argc) {
        return OS_FAIL;
    }

    /* ���1������ */
    addr = str2num(argv[0]);
    if (0 == addr) {
        return OS_FAIL;
    }

    /* for example, addr = (pointer) timer_function; */

    /* ��¼�ϵ� */
    for (i = 0; i < CMD_BP_NUM; i++) {
        if (0 == cmd_bpaddr[i]) {
            cmd_bpaddr[i] = addr;
            /* set break-point */
            set_bp(addr, BREAK_ON_EXE, BREAK_LEN_4, current_task_handle());
        }
    }
    return OS_SUCC;
}

/* b��c��ͬ�� */
LOCALD HEVENT cmd_bp_sem_id = 0;

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_continue(os_u32 argc, os_u8 *argv[])
{
    if (0 != argc) {
        return OS_FAIL;
    }
    notify_event(cmd_bp_sem_id, __LINE__);
    return OS_SUCC;
}

/***************************************************************
 * description : delete break-point
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_bdall(os_u32 argc, os_u8 *argv[])
{
    os_u32 i;

    if (0 != argc) {
        return OS_FAIL;
    }

    for (i = 0; i < CMD_BP_NUM; i++) {
        if (0 != cmd_bpaddr[i]) {
            /* delete break-point */
            clear_bp(cmd_bpaddr[i]);
            cmd_bpaddr[i] = 0;
        }
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_dir(os_u32 argc, os_u8 *argv[])
{
    os_u32 device_id;

    if (0 != argc) {
        return OS_FAIL;
    }

    device_id = curr_disk_device_id();
    list_file(device_id);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_change_dir(os_u32 argc, os_u8 *argv[])
{
    os_u32 device_id;
    os_u8 *dir_name;

    if (1 != argc) {
        return OS_FAIL;
    }

    dir_name = argv[0];
    device_id = curr_disk_device_id();
    open_dir(device_id, dir_name);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_u8 *lookup_symbol_table(os_u8 *eip)
{
    return OS_NULL;
}

/***************************************************************
 * description : backtrace
 * history     :
 ***************************************************************/
LOCALC os_void test_dump_stack(os_u32 reg_ebp)
{
    os_u8 *reg_eip = OS_NULL;
    /* os_u8 prolog[] = {0xec, 0x8b, 0x55}; //5589e5 */
    HDEVICE hdc;

    hdc = open_hdc(cmd_handle);

    /* ����task_wrapper_funcֹͣ */
    while (0 != reg_ebp) {
        /* return address */
        reg_eip = (os_u8 *)(*(os_u32 *)(reg_ebp + 4));
        /* the value of the frame pointer of the caller */
        reg_ebp = *(os_u32 *) reg_ebp; /* caller function variables address */

        /* �˴����Ҳ��ʺ�kmp�㷨 */
        while (OS_NULL != reg_eip) {
            if ((reg_eip[0] == 0x55) && (reg_eip[1] == 0x89) && (reg_eip[2] == 0xe5)) {
                kprint(hdc, "(%x:%s)", reg_eip, lookup_symbol_table(reg_eip));
                break;
            }
            reg_eip--;
        }
    }

    close_hdc(cmd_handle, hdc);
}

/***************************************************************
 * description : trace task stack
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_tt(os_u32 argc, os_u8 *argv[])
{
    os_u32 reg_ebp = 0;

    if (0 != argc) {
        return OS_FAIL;
    }

    /* current function variables addr */
    __asm__ __volatile__("movl %%ebp, %0"
                        :"=g"(reg_ebp)
                        :
                        :"memory");

    /* ����ջ */
    test_dump_stack(reg_ebp);

    return OS_SUCC;
}

/***************************************************************
 * description : ϵͳ��dll�ļ�ָ��
 * history     :
 ***************************************************************/
LOCALC os_void global_instruction_for_worm(os_void)
{
    /* ����ϵͳ��dll�ļ��а�����ָ��, ��ô��ַӦ���ǹ̶���. */
    __asm__ __volatile__("jmp *%%esp"::);
}

LOCALD os_u32 remote_worm_data[] = {
    0, /* ������������Ҫ���� */
    0, /* stack pointer, ebp */
    (pointer) global_instruction_for_worm + 3, /* ���ص�ַ(���Ե�ַ) */
    /* shell code */
    0x90909090, 0x90909090, 0x90909090, 0x90909090
};

/***************************************************************
 * description : ��ʾ��没��
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_remote_worm(os_u32 argc, os_u8 *argv[])
{
    os_u8 input[4];

    if (0 != argc) {
        return OS_FAIL;
    }
    /* ģ�⽫��������ϵͳ�Ļ�����, ������ */
    mem_cpy(input, remote_worm_data, sizeof(remote_worm_data));
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void worm2(os_void)
{
    flog("%s", "crack");
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void worm_code(os_void)
{
    /* ��δ���ͨ��jmp label2, ��call��label1, ���Ի�ȡ��ִ��ָ���ַ. */
    __asm__ __volatile__("jmp 2f\n\t"
                         "1:\n\t"
                         "popl %%eax\n\t" /* eax��������ַ */
                         "call _worm2\n\t"
                         "jmp 3f\n\t"
                         "2:\n\t"
                         "call 1b\n\t"
                         "3:\n\t"
                         :
                         :
                         :"eax");
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void worm(os_void)
{
    flog("%s", "crack");

    worm_code();

    flog("%s", "exit");

    exit_task();
}

/* �������������, �����Ĳ�������Ҫ����������д���ľ��Ե�ַ��д�� */
LOCALD os_u32 worm_data[] = {
    0, /* ������������Ҫ���� */
    0, /* stack pointer, ebp */
    (pointer) worm /* ���ص�ַ(���Ե�ַ) */
};

/***************************************************************
 * description : ��ʾ��没��
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_worm(os_u32 argc, os_u8 *argv[])
{
    os_u8 input[4];

    if (0 != argc) {
        return OS_FAIL;
    }
    /* ģ�⽫��������ϵͳ�Ļ�����, ������ */
    mem_cpy(input, worm_data, sizeof(worm_data));
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_read_harddisk(os_u32 argc, os_u8 *argv[])
{
    HDEVICE hdc;
    os_u32 i;
    //os_u8 buff[512] = {0}; // �����ᵼ�±�����ʹ��ϵͳ��memset����
    os_u8 buff[512];

    if (0 != argc) {
        return OS_FAIL;
    }

    mem_set(buff, 0, 512);

    if (OS_SUCC != read_harddisk(0,0,0,1,buff)) {
        return OS_FAIL;
    }

    hdc = open_hdc(cmd_handle);
    for (i = 0; i < 32; i++) {
        kprint(hdc, "%x", buff[i]);
    }
    close_hdc(cmd_handle, hdc);

    return OS_SUCC;
}

/***************************************************************
 * description : set current disk id
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_set_cdi(os_u32 argc, os_u8 *argv[])
{
    os_u32 id;

    if (1 == argc) {
        id = str2num(argv[0]);
        set_disk_device_id(id);
        return OS_SUCC;
    }
    return OS_FAIL;
}

/***************************************************************
 * description : create new file
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_new_file(os_u32 argc, os_u8 *argv[])
{
    if (1 == argc) {
        os_u32 id;
        os_ret ret;

        id = curr_disk_device_id();
        ret = create_file(id, argv[0]);
        if (OS_SUCC != ret) {
            cmd_print("create file fail\n");
        }
        return ret;
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_del_file(os_u32 argc, os_u8 *argv[])
{
    if (1 == argc) {
        os_u32 id;
        os_ret ret;

        id = curr_disk_device_id();
        ret = delete_file(id, argv[0]);
        if (OS_SUCC != ret) {
            cmd_print("delete file fail\n");
        }
        return ret;
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_ping(os_u32 argc, os_u8 *argv[])
{
    switch (argc) {
    case 0:
        ping_status();
        break;
    case 2:
        ping(argv[0], OS_NULL, str2num(argv[1]));
        return OS_SUCC;
        break;
    case 3:
        ping(argv[0], argv[1], str2num(argv[2]));
        return OS_SUCC;
        break;
    default:
        break;
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_log_file(os_u32 argc, os_u8 *argv[])
{
    os_u32 control;

    if (1 == argc) {
        control = str2num(argv[0]);
        switch (control) {
        case FLOG_DISK:
        case FLOG_SCREEN:
        case FLOG_BOTH:
        case FLOG_NONE:
            direct_log_file(control);
            break;
        default:
            dump_log_file();
            return OS_FAIL;
            break;
        }
        return OS_SUCC;
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_dump(os_u32 argc, os_u8 *argv[])
{
    if (0 == argc) {
        dump(OS_NULL);
        return OS_SUCC;
    } else if (1 == argc) {
        dump(argv[0]);
        return OS_SUCC;
    } else {
        /* input para error */
        return OS_FAIL;
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_desktop(os_u32 argc, os_u8 *argv[])
{
    if (0 == argc) {
        show_image(get_desktop_window(), "desktop.bmp", SI_MODE_1);
        redisplay_shell();
        return OS_SUCC;
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_version(os_u32 argc, os_u8 *argv[])
{
    if (0 == argc) {
        version();
        print("%s, %s\n", __DATE__, __TIME__);
        return OS_SUCC;
    }
    return OS_FAIL;
}

LOCALC os_ret cmd_help(os_u32 argc, os_u8 *argv[]);

/***************************************************************
 * description :
 ***************************************************************/
struct cmd_item {
    os_bool valid;
    const os_u8 *cmd; /* const buffer */
    MAIN_FUNC_PTR func;
};

LOCALD struct cmd_item cmd_table[0x100] = {
    { OS_TRUE, "help", cmd_help }, // reserved

    { OS_TRUE, "version", cmd_version },
    { OS_TRUE, "exit", cmd_exit },
    { OS_TRUE, "d", cmd_d },
    { OS_TRUE, "io", cmd_io },
    { OS_TRUE, "cls", cmd_cls },
    { OS_TRUE, "b", cmd_break },
    { OS_TRUE, "c", cmd_continue },
    { OS_TRUE, "bdall", cmd_bdall },
    { OS_TRUE, "tt", cmd_tt },
    { OS_FALSE, "worm", cmd_worm },
    { OS_FALSE, "rworm", cmd_remote_worm },
    { OS_FALSE, "rhd", cmd_read_harddisk },
    { OS_TRUE, "gvesa", cmd_get_vesa },
    { OS_TRUE, "svesa", cmd_set_vesa },
    { OS_TRUE, "mouse", cmd_mouse },
    { OS_TRUE, "dump", cmd_dump, },
    /* file system */
    { OS_TRUE, "cdi", cmd_set_cdi },
    { OS_TRUE, "dir", cmd_dir },
    { OS_TRUE, "cd", cmd_change_dir },
    { OS_TRUE, "new", cmd_new_file },
    { OS_TRUE, "del", cmd_del_file },
    { OS_TRUE, "flog", cmd_log_file },
    /* network */
    { OS_TRUE, "ping", cmd_ping },
    /* misc */
    { OS_TRUE, "desktop", cmd_desktop }
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_help(os_u32 argc, os_u8 *argv[])
{
    HDEVICE hdc;
    os_u32 i;

    hdc = open_hdc(cmd_handle);
    for (i = 1; i < array_size(cmd_table); i++) {
        if (cmd_table[i].valid) {
            kprint(hdc, "(%s) ", cmd_table[i].cmd);
        }
    }
    close_hdc(cmd_handle, hdc);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret register_cmd(const os_u8 *cmd, os_ret (*func)(os_u32 argc, os_u8 *argv[]))
{
    os_uint i, j;

    j = 0;
    for (i = 1; i < array_size(cmd_table); i++) {
        if ((cmd_table[i].valid) && (0 == strcmp(cmd, cmd_table[i].cmd))) {
            return OS_FAIL;
        }
        if ((0 == j) && (!cmd_table[i].valid)) {
            j = i;
        }
    }
    if (j) {
        cmd_table[j].valid = OS_TRUE;
        cmd_table[j].func = func;
        cmd_table[j].cmd = cmd;
        return OS_SUCC;
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void unregister_cmd(const os_u8 *cmd)
{
    os_uint i;

    for (i = 1; i < array_size(cmd_table); i++) {
        if ((cmd_table[i].valid) && (0 == strcmp(cmd, cmd_table[i].cmd))) {
            cmd_table[i].valid = OS_FALSE;
            return;
        }
    }
}

#define CMD_BUFFER_LEN 0x100

/***************************************************************
 * description :
 ***************************************************************/
struct cmd_input_buffer {
    /* buffer */
    os_u8 buffer[CMD_BUFFER_LEN];
    os_u32 index;
};

/* buffer */
LOCALD struct cmd_input_buffer cmd_buffer = {0};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void clear_input_buffer(os_void)
{
    cmd_buffer.index = 0;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void pop_input_buffer(os_void)
{
    if (cmd_buffer.index) {
        cmd_buffer.index--;
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void push_input_buffer(os_u8 c)
{
    if (CMD_BUFFER_LEN > cmd_buffer.index) {
        cmd_buffer.buffer[cmd_buffer.index] = c;
        cmd_buffer.index++;
    }
}

/***************************************************************
 * description : �������뻺����
 * history     :
 ***************************************************************/
LOCALC os_void deal_input_buffer(os_void)
{
    os_u32 i;
    const struct cmd_item *t;
    os_u32 argc;
    os_u32 index;
#define CMD_ARG_NUM 0x10
    os_u8 *argv[CMD_ARG_NUM];
    os_bool first_character;
    HDEVICE hdc;

    if ((CMD_BUFFER_LEN - 1) <= cmd_buffer.index) {
        /* ĩβ���� */
        cmd_buffer.buffer[CMD_BUFFER_LEN-1] = '\0';
        cmd_buffer.index = CMD_BUFFER_LEN - 1;
    }

    first_character = OS_FALSE;
    index = argc = 0;
    argv[0] = OS_NULL; /* init cmd */
    for (i = 0; i < cmd_buffer.index - 1; i++) { // cmd_buffer.buffer[cmd_buffer.index - 1] is '\0'
        if (' ' != cmd_buffer.buffer[i]) {
            if (!first_character) {
                first_character = OS_TRUE;
                argv[index++] = &cmd_buffer.buffer[i];
                argc++;
            }
        } else {
            first_character = OS_FALSE;
            cmd_buffer.buffer[i] = '\0';
        }
    }

    if (OS_NULL != argv[0]) {
        for (i = 0, t = cmd_table; i < array_size(cmd_table); i++, t++) {
            if ((t->valid) && (0 == str_cmp(t->cmd, argv[0]))) {
                hdc = open_hdc(cmd_handle);
                kprint(hdc, "\n");
                close_hdc(cmd_handle, hdc);
                t->func(argc - 1, &argv[1]);
                break;
            }
        }
        if (array_size(cmd_table) == i) {
            hdc = open_hdc(cmd_handle);
            kprint(hdc, "\nbad command");
            close_hdc(cmd_handle, hdc);
        }
    }
}

/***************************************************************
 * description : cmd���̴������
 * history     :
 ***************************************************************/
LOCALC os_void cmd_kb_handle(os_void *msg)
{
    struct keyboard_msg *kb_msg;
    HDEVICE hdc;

    /* ����������� */

    kb_msg = msg;

    if (KEYBOARD_UP == kb_msg->up_down) {
        /* ������break code */
        return;
    }

    switch (kb_msg->asc) {
    case 0x0d: /* �س��� */
        /* ������������ */
        push_input_buffer('\0');
        /* ����shell���� */
        deal_input_buffer();
        /* ���������� */
        clear_input_buffer();
        /* ���δ�ӡ��Ҫ���»�ȡ���, �����Ż�Ϊȫ��hdc */
        hdc = open_hdc(cmd_handle);
        kprint(hdc, "\n>");
        close_hdc(cmd_handle, hdc);
        break;

    case 0x08: /* �˸�� */
        if (cmd_buffer.index) {
            hdc = open_hdc(cmd_handle);
            win_backspace(hdc);
            close_hdc(cmd_handle, hdc);
            pop_input_buffer();
        }
        break;

    /* null */
    case 0x0:
    /* left shift and right shift */
    case 0xf:
    /* left ctrl and right ctrl */
    case 0x1d:
        break;

    default:
        hdc = open_hdc(cmd_handle);
        kprint(hdc, "%c", kb_msg->asc);
        close_hdc(cmd_handle, hdc);
        /* ���뵽���� */
        push_input_buffer(kb_msg->asc);
        break;
    }
}

/***************************************************************
 * description : MSG_PROC_PTR��������, ��Ϣ����
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_msgproc(IN os_void *msg)
{
    struct message *cmd_msg;
    HDEVICE hdc;

    if (OS_NULL == msg) {
        return OS_FAIL;
    }

    cmd_msg = (struct message *) msg;
    switch (cmd_msg->msg_name) {
    case OS_MSG_KEYBOARD:
        cmd_kb_handle(cmd_msg);
        break;

    case OS_MSG_PAINT:
        /* �ػ������� */
        repaint_window(cmd_handle);
        /* ��ʾ��ʾ�� */
        hdc = open_hdc(cmd_handle);
        win_clear_screen(hdc);
        kprint(hdc, ">");
        close_hdc(cmd_handle, hdc);
        break;

    case CMD_MSG_INFO:
        do {
            os_u8 *str;
            str = (os_u8 *) msg + sizeof(struct message);
            hdc = open_hdc(cmd_handle);
            kprint(hdc, "%s", str);
            close_hdc(cmd_handle, hdc);
        } while (0);
        break;

    default:
        break;
    }
    return OS_SUCC;
}

/***************************************************************
 * description : VOID_FUNCPTR
 * history     :
 ***************************************************************/
LOCALC os_void cmd_debuger(os_void)
{
    wait_event(cmd_bp_sem_id, 0);
}

/***************************************************************
 * description : ��װshell������
 * history     :
 ***************************************************************/
LOCALC os_void init_cmd_debuger(os_void)
{
    /* ��նϵ��¼ */
    mem_set(cmd_bpaddr, 0, CMD_BP_NUM * sizeof(os_u32));

    /* b��cͬ���ź��� */
    cmd_bp_sem_id = create_event_handle(EVENT_VALID, "cmd", __LINE__);
    cassert(OS_NULL != cmd_bp_sem_id);

    /* ע������� */
    install_debugger(cmd_debuger, current_task_handle());
}

/***************************************************************
 * description : ��cmd��������Ϊ��ǰ����
 * history     :
 ***************************************************************/
LOCALC os_void cmd_init(os_void)
{
    cmd_handle = lookup_handle_by_app_name("cmd");

    /* buffer��ʼ�� */
    clear_input_buffer();

    /* ��װ������ */
    init_cmd_debuger();

    /* ���µ�ǰ�Ĵ��� */
    modify_current_window_handle(cmd_handle);

    /* ���´��� */
    //update_window(cmd_handle);
}

/***************************************************************
 * description : shell
 * history     :
 ***************************************************************/
LOCALC os_void init_command_prompt(os_void)
{
    /* �ı����� */
#define CMD_WIN_X 220
#define CMD_WIN_Y 20
#define CMD_WIN_LEN 300
#define CMD_WIN_WID 400

    struct window_class window_class;

    /* ��䴰���� */
    mem_cpy(window_class.app_name, "cmd", MAX_APP_NAME_LEN);
    mem_cpy(window_class.attr.title, "debug", MAX_TITLE_NAME_LEN);
    window_class.attr.background_color = VGA_COLOR_BLACK;
    window_class.attr.csys.x = CMD_WIN_X;
    window_class.attr.csys.y = CMD_WIN_Y;
    window_class.attr.wframe = OS_TRUE;
    window_class.attr.cursor_pos.x = 0;
    window_class.attr.cursor_pos.y = 0;
    window_class.attr.font_size = FONT_SIZE;
    window_class.attr.foreground_color = VGA_COLOR_SILVER;
    window_class.attr.length = CMD_WIN_LEN;
    window_class.attr.width = CMD_WIN_WID;

    window_class.app_init = cmd_init;
    window_class.msg_proc = cmd_msgproc;

    /* �����û������߳� */
    create_ui_task("cmd", &window_class, CMD_TASK_PRIO);

    /* �˴���ʼ�����ھ����ʧ��, ������δ����. */
}
shell_init_func(init_command_prompt);

