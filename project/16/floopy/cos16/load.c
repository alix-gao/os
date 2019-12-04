
#include "typedef.h"
#include "lib.h"

VOID os_8259a(VOID);
VOID bios_8259a(VOID);
VOID keyboard_int(VOID);
VOID move_1m(DWORD src_addr, DWORD dst_addr, DWORD data_len);

#define DISK_CACHE_ADDR 0x20000

#define ASM32_ADDR 0x132000

#define ASM32_LEN 0x2000

#define COS32_ADDR 0x1000000

#define COS32_LEN 0x40000

#define INT13_CX_ORG_SECTOR 0x1

#define DISK_DIRVER_TYPE 0x00

#define DISK_CYLINDER_MSG_POS 9

#define DISK_HEAD_MSG_POS 10

#define DISK_SECTOR_MSG_POS 11

#define DISK_DRIVER_MSG_POS 12

#define OS_CHOOSE_MSG_POS 13

#define DISK_TRACK_DATA_LEN 0x00002400

#define OS2_START_HEAD 0
#define OS2_END_HEAD 1
#define OS2_START_TRACK 5
#define OS2_END_TRACK 16

#define OS3_START_HEAD 0
#define OS3_END_HEAD 1
#define OS3_START_TRACK 17
#define OS3_END_TRACK 32

#define OS4_START_HEAD 0
#define OS4_END_HEAD 1
#define OS4_START_TRACK 33
#define OS4_END_TRACK 79

struct disk_para {
    unsigned int cylinder;
    unsigned char head;
    unsigned char sector;
    unsigned char driver;
};

static WORD cos32_version = 0;

static struct disk_para disk_para_table;

static VOID open_disk_drive(VOID)
{
    asm push ax
    asm xor ax,ax
    asm int 13H
    asm pop ax
}

static struct global_ram_file {
    /* 磁盘参数表 */
    BYTE ns; /* 每磁道扇区数 */
    BYTE nh; /* 最大磁头数 */
    WORD nc; /* 最大磁道号 */

    BYTE bsp_flag; /* 未记录0, 已记录1 */
    BYTE rsvd;
    BYTE ss; /* 起始磁道号,起始为1 */
    BYTE sh; /* 起始磁头数,起始为0 */
    WORD sc; /* 起始磁道号,起始为0 */
} *grf;

#define GRF_ADDR 0x90000

static VOID get_disk_bsp(VOID)
{
    init_32p(grf, GRF_ADDR);
}

static VOID read_disk_track(BYTE head, WORD track, DWORD addr_20bit)
{
    WORD int13_ax = 0;
    WORD int13_bx = 0;
    WORD int13_cx = 0;
    WORD int13_dx = 0;
    WORD int13_es = 0;

#define INT13_AX_FUNC 0x02

    int13_ax = (INT13_AX_FUNC<<8)|grf->ns;
    int13_cx = (((track+grf->sc) & 0xff)<<8)|((((track+grf->sc) & 0x300)>>2)|(grf->ss & 0x3f));
    int13_dx = ((head+grf->sh)<<8)|DISK_DIRVER_TYPE;
    int13_bx = addr_20bit & 0x0f;
    int13_es = ((WORD) addr_20bit)>>4;
    int13_es = int13_es|(((WORD)(addr_20bit>>16))<<12);

    asm push int13_ax
    asm push int13_bx
    asm push int13_cx
    asm push int13_dx
    asm push int13_es
    asm pop ax
    asm mov es,ax
    asm pop dx
    asm pop cx
    asm pop bx
    asm pop ax
    asm int 13H
}

static VOID load_asm32(VOID)
{
    read_disk_track(0, 2, DISK_CACHE_ADDR+0x0000);
    read_disk_track(1, 2, DISK_CACHE_ADDR+0x2400);

    /* move to high memory */
    move_1m(DISK_CACHE_ADDR, ASM32_ADDR, ASM32_LEN);
}

static VOID load_os1(VOID)
{
    read_disk_track(0, 3, DISK_CACHE_ADDR+0x0000);
    read_disk_track(1, 3, DISK_CACHE_ADDR+0x2400);
    read_disk_track(0, 4, DISK_CACHE_ADDR+0x4800);
    read_disk_track(1, 4, DISK_CACHE_ADDR+0x6c00);
#if 0
    read_disk_track(0, 4, DISK_CACHE_ADDR+0x0000);
    read_disk_track(1, 4, DISK_CACHE_ADDR+0x2400);
    read_disk_track(0, 5, DISK_CACHE_ADDR+0x4800);
    read_disk_track(1, 5, DISK_CACHE_ADDR+0x6c00);
    read_disk_track(0, 6, DISK_CACHE_ADDR+0x9000);
    read_disk_track(1, 6, DISK_CACHE_ADDR+0xb400);
    read_disk_track(0, 7, DISK_CACHE_ADDR+0xd800);
    read_disk_track(1, 7, DISK_CACHE_ADDR+0xfc00);
#endif
    /* move to high memory */
    move_1m(DISK_CACHE_ADDR, COS32_ADDR, COS32_LEN);
}

/* 442368 byte */
static VOID load_os2(VOID)
{
#if 1
    unsigned long addr;
    unsigned int track;
    unsigned char head;

    addr = DISK_CACHE_ADDR;

    for (track=OS2_START_TRACK; track<=OS2_END_TRACK; track++)
    for (head=OS2_START_HEAD; head<=OS2_END_HEAD; head++) {
        read_disk_track(head, track, addr);
        tc_32bit_add(&addr, addr, DISK_TRACK_DATA_LEN);
        while (0) {
            print_hex(track, 0xf, 0, 0);
            print_hex(head, 0xf, 0, 0);
            print_hex(addr, 0xf, 0, 0);
        }
    }
#else
    addr = addr + DISK_TRACK_DATA_LEN;
    read_disk_track(0, 6, DISK_CACHE_ADDR+0x0000);
    read_disk_track(1, 6, DISK_CACHE_ADDR+0x2400);
    read_disk_track(0, 7, DISK_CACHE_ADDR+0x4800);
    read_disk_track(1, 7, DISK_CACHE_ADDR+0x6c00);
    read_disk_track(0, 8, DISK_CACHE_ADDR+0x9000);
    read_disk_track(1, 8, DISK_CACHE_ADDR+0xb400);
    read_disk_track(0, 9, DISK_CACHE_ADDR+0xd800);
    read_disk_track(1, 9, DISK_CACHE_ADDR+0xfc00);
    read_disk_track(0, 0xa, DISK_CACHE_ADDR+0x12000);
    read_disk_track(1, 0xa, DISK_CACHE_ADDR+0x14400);
    read_disk_track(0, 0xb, DISK_CACHE_ADDR+0x16800);
    read_disk_track(1, 0xb, DISK_CACHE_ADDR+0x18c00);
    read_disk_track(0, 0xc, DISK_CACHE_ADDR+0x1b000);
    read_disk_track(1, 0xc, DISK_CACHE_ADDR+0x1d400);
    read_disk_track(0, 0xd, DISK_CACHE_ADDR+0x1f800);
    read_disk_track(1, 0xd, DISK_CACHE_ADDR+0x21c00);
    read_disk_track(0, 0xe, DISK_CACHE_ADDR+0x24000);
    read_disk_track(1, 0xe, DISK_CACHE_ADDR+0x26400);
    read_disk_track(0, 0xf, DISK_CACHE_ADDR+0x28800);
    read_disk_track(1, 0xf, DISK_CACHE_ADDR+0x2ac00);
#endif

    /* move to high memory */
    move_1m(DISK_CACHE_ADDR, COS32_ADDR, COS32_LEN);
}

/* 589824 byte */
static VOID load_os3(VOID)
{
    unsigned long addr;
    unsigned int track;
    unsigned char head;

    addr = COS32_ADDR;

    for (track=OS3_START_TRACK; track<=OS3_END_TRACK; track++)
    for (head=OS3_START_HEAD; head<=OS3_END_HEAD; head++) {
        read_disk_track(head, track, DISK_CACHE_ADDR);
        move_1m(DISK_CACHE_ADDR, addr, DISK_TRACK_DATA_LEN);
        tc_32bit_add(&addr, addr, DISK_TRACK_DATA_LEN);
        /* make complier use stack but register */
        while (0) {
            print_hex(track, 0xf, 0, 0);
            print_hex(head, 0xf, 0, 0);
            print_hex(addr, 0xf, 0, 0);
        }
    }
}

static VOID load_os4(VOID)
{
    unsigned long addr;
    unsigned int track;
    unsigned char head;

    addr = COS32_ADDR;

    for (track=OS4_START_TRACK; track<=OS4_END_TRACK; track++)
    for (head=OS4_START_HEAD; head<=OS4_END_HEAD; head++) {
        read_disk_track(head, track, DISK_CACHE_ADDR);
        move_1m(DISK_CACHE_ADDR, addr, DISK_TRACK_DATA_LEN);
        tc_32bit_add(&addr, addr, DISK_TRACK_DATA_LEN);
        /* make complier use stack but register */
        while (0) {
            print_hex(track, 0xf, 0, 0);
            print_hex(head, 0xf, 0, 0);
            print_hex(addr, 0xf, 0, 0);
        }
    }
}

static VOID load_cos32(VOID)
{
    switch (cos32_version) {
        case 1:
            load_os1();
            break;

        case 2:
            load_os2();
            break;

        case 3:
            load_os3();
            break;

        case 4:
            load_os4();
            break;

        default:
            break;
    }
}

static VOID close_disk_drive(VOID)
{
    asm push ax
    asm push dx
    asm mov dx,03f2H
    asm mov al,00H
    asm out dx,al
    asm pop dx
    asm pop ax
}

VOID keyboard_int_func(VOID)
{
    BYTE scan_code;

    asm push ax
    asm in al,0x60
    asm mov scan_code,al
    asm pop ax

    /* F1:0x3b - F10:0x44 */
#define F1_SCAN_CODE 0x3b
#define F10_SCAN_CODE 0x44
    if ((scan_code >= F1_SCAN_CODE) && (scan_code <= F10_SCAN_CODE)) {
        cos32_version = scan_code - F1_SCAN_CODE + 1;
        print_char('0' + cos32_version, 0x0f, OS_CHOOSE_MSG_POS, 17);
    }
}

static VOID modify_keyboard_int(VOID)
{
    asm push ax
    asm push di
    asm mov ax,0
    asm mov ds,ax
    asm mov di,0x80+4
    asm mov word ptr ds:[di],offset keyboard_int
    asm add di,2
    asm mov word ptr ds:[di],seg keyboard_int
    asm pop di
    asm pop ax
}

static VOID open_keyboard_int(VOID)
{
    BYTE reg = 0;

    asm push ax
    asm in al,0x21
    asm mov reg,al
    asm pop ax

    reg = reg & 0xfd;

    asm push ax
    asm mov al,reg
    asm out 0x21,al

    asm in al,0x60
    asm pop ax
}

static VOID set_curse_pos(BYTE row, BYTE col)
{
    WORD pos = 80*row+col;

    asm push ax
    asm push dx
    asm push bx
    asm db 66H
    asm mov bx,pos
    asm mov al,0fH
    asm mov dx,03d4H
    asm out dx,al
    asm nop
    asm db 66H
    asm mov ax,bx
    asm db 66H
    asm and ax,00ffH
    asm db 00H,00H
    asm mov dx,03d5H
    asm db 66H
    asm out dx,ax
    asm nop
    asm db 66H
    asm mov ax,000eH
    asm db 00H,00H
    asm mov dx,03d4H
    asm db 66H
    asm out dx,ax
    asm nop
    asm db 66H
    asm shr bx,1
    asm db 66H
    asm shr bx,1
    asm db 66H
    asm shr bx,1
    asm db 66H
    asm shr bx,1
    asm db 66H
    asm shr bx,1
    asm db 66H
    asm shr bx,1
    asm db 66H
    asm shr bx,1
    asm db 66H
    asm shr bx,1
    asm db 66H
    asm and bx,00ffH
    asm db 00H,00H
    asm db 66H
    asm mov ax,bx
    asm mov dx,03d5H
    asm db 66H
    asm out dx,ax
    asm nop
    asm pop bx
    asm pop dx
    asm pop ax
}

static VOID select_os(VOID)
{
#if 0
    BYTE str[] = "select os(1/2):";
    call far ptr SCOPY@
#endif

    os_8259a();

    modify_keyboard_int();

    open_keyboard_int();

    print_string("select os(1/2):", 0x0f, OS_CHOOSE_MSG_POS, 0);

    set_curse_pos(OS_CHOOSE_MSG_POS, 17);

    asm sti

    while (!cos32_version) {
        asm hlt;
    }

    asm cli

    bios_8259a();
}

static VOID get_disk_para(VOID)
{
    unsigned char ch_para;
    unsigned char cl_para;
    unsigned char dl_para;
    unsigned char dh_para;

    asm mov ah,08
    asm mov dl,DISK_DIRVER_TYPE
    asm int 13H
    asm mov ch_para,ch
    asm mov cl_para,cl
    asm mov dh_para,dh
    asm mov dl_para,dl

    disk_para_table.cylinder = ch_para|((((unsigned int)cl_para)<<2)&0x00ff);
    disk_para_table.head = dh_para;
    disk_para_table.sector = cl_para&0x3f;
    disk_para_table.driver = dl_para;

    print_string("disk cylinder:", 0x0f, DISK_CYLINDER_MSG_POS, 0);
    print_hex(disk_para_table.cylinder, 0x0f, DISK_CYLINDER_MSG_POS, 16);

    print_string("disk head:", 0x0f, DISK_HEAD_MSG_POS, 0);
    print_hex(disk_para_table.head, 0x0f, DISK_HEAD_MSG_POS, 11);

    print_string("disk sector:", 0x0f, DISK_SECTOR_MSG_POS, 0);
    print_hex(disk_para_table.sector, 0x0f, DISK_SECTOR_MSG_POS, 13);

    print_string("disk driver:", 0x0f, DISK_DRIVER_MSG_POS, 0);
    print_hex(disk_para_table.driver, 0x0f, DISK_DRIVER_MSG_POS, 13);
}

VOID load_os(VOID)
{
    open_disk_drive();

    get_disk_para();

    get_disk_bsp();

    select_os();

    load_asm32();

    load_cos32();

    close_disk_drive();
}

