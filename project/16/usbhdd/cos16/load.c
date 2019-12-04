
#include "typedef.h"
#include "lib.h"
#include "vfs.h"

VOID keyboard_int(VOID);
VOID move_1m(DWORD src_addr, DWORD dst_addr, DWORD data_len);

#define DISK_CACHE_ADDR 0x40000

#define ASM32_ADDR 0x132000

#define COS32_ADDR 0x1000000

#define SECTOR_NO_PER_TRACK 0x20

#define INT13_AX_FUNC 0x2

#define INT13_CX_ORG_SECTOR 0x1

#define INT13_DX 0x0

#define OS_CHOOSE_MSG_POS 9

#define OS2_START_HEAD 8
#define OS2_END_HEAD 12
#define OS2_START_TRACK 0
#define OS2_END_TRACK 1

static WORD cos32_version = 0;

static VOID open_disk_drive(VOID)
{
    asm push ax
    asm xor ax,ax
    asm int 13H
    asm pop ax
}

static VOID read_disk_track(BYTE head, WORD track, DWORD addr_20bit)
{
    WORD int13_ax = 0;
    WORD int13_bx = 0;
    WORD int13_cx = 0;
    WORD int13_dx = 0;
    WORD int13_es = 0;

    int13_ax = (INT13_AX_FUNC<<8)|SECTOR_NO_PER_TRACK;
    int13_cx = ((track&0xff)<<8)|(((track&0x300)>>2)|(INT13_CX_ORG_SECTOR&0x3f));
    int13_dx = (head<<8)|INT13_DX;
    int13_bx = addr_20bit&0x0f;
    int13_es = ((WORD)addr_20bit)>>4;
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

static VOID system_hlt(VOID)
{
    while (1) asm hlt;
}

static VOID load_asm32(VOID)
{
    WORD result;

    result = load_file("asm32.bin", ASM32_ADDR);
    if (SUCC != result) {
        system_hlt();
    }

#if 0
    read_disk_track(3, 0, 0x2000);
#endif
}

static VOID load_os1(VOID)
{
    WORD result;

    result = load_file("os1.bin", COS32_ADDR);
    if (SUCC != result) {
        system_hlt();
    }
#if 0
    read_disk_track(4, 0, DISK_CACHE_ADDR+0x0000);
    read_disk_track(5, 0, DISK_CACHE_ADDR+0x4000);
    read_disk_track(6, 0, DISK_CACHE_ADDR+0x8000);
    read_disk_track(7, 0, DISK_CACHE_ADDR+0xc000);
#endif
}

static VOID load_os2(VOID)
{
    WORD result;

    result = load_file("os2.bin", COS32_ADDR);
    if (SUCC != result) {
        system_hlt();
    }
#if 0
    #if 1
    DWORD addr;
    WORD track;
    BYTE head;

    addr = DISK_CACHE_ADDR;

    for (track=OS2_START_TRACK; track<OS2_END_TRACK; track++)
    for (head=OS2_START_HEAD; head<OS2_END_HEAD; head++)
    {
        read_disk_track(head, track, addr);
#define DISK_TRACK_DATA_LEN 0x00004000
        tc_32bit_add(&addr, addr, DISK_TRACK_DATA_LEN);
    }
    #else
    read_disk_track(8, 0, DISK_CACHE_ADDR+0x0000);
    read_disk_track(9, 0, DISK_CACHE_ADDR+0x4000);
    read_disk_track(10, 0, DISK_CACHE_ADDR+0x8000);
    read_disk_track(11, 0, DISK_CACHE_ADDR+0xc000);
    #endif
#endif
}

static VOID load_os3(VOID)
{
    WORD result;

    result = load_file("os3.bin", COS32_ADDR);
    if (SUCC != result) {
        system_hlt();
    }
}

static BYTE osx_name[] = {"osx.bin"};

#define DEFAULT_OS_VERSION 4

static VOID load_cos32(VOID)
{
    WORD result;

    if (0 == cos32_version) {
        cos32_version = DEFAULT_OS_VERSION;
        print_char('0' + cos32_version, 0x0f, OS_CHOOSE_MSG_POS, 17);
    }
    osx_name[2] = '0' + cos32_version;
    result = load_file(osx_name, COS32_ADDR);
    if (SUCC != result) {
        print_char('x', 0x0f, OS_CHOOSE_MSG_POS, 17);
        system_hlt();
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

#if 0
VOID move_asm32(VOID)
{
#define ASM32_LEN 0x2000
    move_1m(DISK_CACHE_ADDR, ASM32_ADDR, ASM32_LEN);
}

VOID move_cos32(VOID)
{
#define COS32_LEN 0x10000
    move_1m(DISK_CACHE_ADDR, COS32_ADDR, COS32_LEN);
}
#endif

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
        stop_rmho();
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

VOID select_os(VOID)
{
#if 0
    BYTE str[] = "select os(1/2):";
    call far ptr SCOPY@
#endif

    modify_keyboard_int();

    open_keyboard_int();

    print_string("select os(1/n):", 0x0f, OS_CHOOSE_MSG_POS, 0);

    set_curse_pos(OS_CHOOSE_MSG_POS, 16);
}

VOID show_vfs(VOID)
{
    open_disk_drive();

    /* fat32 info */
    init_vfs();
    change_directory("system");
    list_vfs();
}

VOID load_os(VOID)
{
    load_asm32();
#if 0
    move_asm32();
#endif

    load_cos32();
#if 0
    move_cos32();
#endif

    /* close_disk_drive(); */
}

