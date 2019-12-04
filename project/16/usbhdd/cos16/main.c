
#include "typedef.h"
#include "lib.h"
#include "rmho.h"
#include "load.h"

VOID goto_asm32(VOID);

#define COS16_MSG_POS 5

#define iodelay() \
    do { \
        asm nop; \
        asm nop; \
        asm nop; \
        asm nop; \
    } while (0)

static VOID os_8259a(VOID)
{
    asm push ax
    /* icw1.边沿触发、多片8259a且最后需要发送icw4 */
    asm mov al,11H
    asm out 20H,al
    iodelay();
    asm out 0a0H,al
    iodelay();
    /* icw2.中断向量号起始为0x20 */
    asm mov al,020H
    asm out 21H,al
    iodelay();
    /* icw2.中断向量号起始为0x28 */
    asm mov al,28H
    asm out 0a1H,al
    iodelay();
    /* icw3.主芯片的ir2引脚连接一个从芯片 */
    asm mov al,04H
    asm out 21H,al
    iodelay();
    /* icw3.从芯片连接到主芯片的ir2引脚 */
    asm mov al,02H
    asm out 0a1H,al
    iodelay();
    /* 普通全嵌套、非缓冲、非自动结束中断方式，用于8086及其兼容系统 */
    asm mov al,01H
    asm out 21H,al
    iodelay();
    asm out 0a1H,al
    iodelay();
    asm mov al,11111111B
    asm out 21H,al
    iodelay();
    asm mov al,11111111B
    asm out 0a1H,al
    iodelay();
    asm pop ax
}

static VOID bios_8259a(VOID)
{
    asm push ax
    asm mov al,0x11
    asm out 0x20,al
    iodelay();
    asm out 0xa0,al
    iodelay();
    asm mov al,0x08
    asm out 0x21,al
    iodelay();
    asm mov al,0x70
    asm out 0xa1,al
    iodelay();
    asm mov al,0x04
    asm out 0x21,al
    iodelay();
    asm mov al,0x02
    asm out 0xa1,al
    iodelay();
    asm mov al,0x01
    asm out 0x21,al
    iodelay();
    asm out 0xa1,al
    iodelay();
    asm mov al,0xb8
    asm out 0x21,al
    iodelay();
    asm mov al,0x9f
    asm out 0xa1,al
    iodelay();
    asm pop ax
}

static VOID cos16_info(VOID)
{
    print_string("16 bit cos", 0x0f, COS16_MSG_POS, 0);
}

static VOID debug_mem(WORD addr_16bit)
{
    WORD i = 0;
    BYTE *p = (BYTE *)(addr_16bit);

    for (i=0; i<0x20; i=i+2) {
        print_hex(*p, 0x0f, 20, i);
        p++;
    }
}

static VOID get_type(VOID)
{
    print_hex(sizeof(char), 0x0f, 20, 0);
    print_hex(sizeof(int), 0x0f, 20, 4);
    print_hex(sizeof(long), 0x0f, 20, 8);
    print_hex(sizeof(unsigned char), 0x0f, 20, 12);
    print_hex(sizeof(unsigned int), 0x0f, 20, 16);
    print_hex(sizeof(unsigned long), 0x0f, 20, 20);
}

#define synci()

VOID main(VOID)
{
    cos16_info();

    show_vfs();

    os_8259a();

    select_os();

    rmho();

    bios_8259a();

    load_os();

    synci();

    goto_asm32();
}

