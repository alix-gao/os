
#include "typedef.h"
#include "lib.h"

VOID time_int(VOID);
VOID os_8259a(VOID);
VOID bios_8259a(VOID);

#define RMHO_MSG_POS 6

#define P1_MSG_POS 7

#define P2_MSG_POS 8

typedef struct regs_struct {
    WORD es;
    WORD ds;
    WORD di;
    WORD si;
    WORD bp;
    WORD bx;
    WORD dx;
    WORD cx;
    WORD ax;
    WORD ss;
    WORD sp;
} regs_struct;

typedef struct task_struct {
    WORD        pid;
    regs_struct regs;
    WORD        stack[1024];
} task_struct;

static task_struct process_table[3];
static VOID (*p)();
static BYTE msg_info='s';
static WORD no;

WORD temp_reg_sp;
WORD temp_reg_ss;
WORD temp_reg;

/* dword */
BYTE *sche_context;

static WORD process_1;
static WORD process_2;

VOID time_int_func(VOID)
{
    print_char(msg_info, 15, RMHO_MSG_POS, 0);
    msg_info++;

    /* schel */
    no++;
    no = no%3;

    /* mov word ptr ds:0x63b0,es & mov word ptr ds:0x63ae,bx */
    sche_context = (BYTE *) &(process_table[no].regs);
}

static VOID modify_time_int(VOID)
{
    asm push ax
    asm push di
    asm mov ax,0
    asm mov ds,ax
    asm mov di,0x80
    asm mov word ptr ds:[di],offset time_int
    asm add di,2
    asm mov word ptr ds:[di],seg time_int
    asm pop di
    asm pop ax
}

static VOID open_time_int(VOID)
{
    BYTE reg = 0;

    asm push ax
    asm in al,0x21
    asm mov reg,al
    asm pop ax

    reg = reg&0xfe;

    asm push ax
    asm mov al,reg
    asm out 0x21,al
    asm pop ax
}

static VOID time_delay(WORD time)
{
    BYTE t;
    BYTE save;
    WORD i,j;

    for (i=0; i<100*time; i++)
    for (j=0; j<10; j++) {
        asm push ax
        asm push dx
        asm mov dx,0x61
        asm in al,dx
        asm mov t,al
        asm pop dx
        asm pop ax
        t = t&0x10;
        save = t;
        while (1) {
            asm push ax
            asm push dx
            asm mov dx,0x61
            asm in al,dx
            asm mov t,al
            asm pop dx
            asm pop ax
            t = t&0x10;
            if (save!=t)
                break;
        }
    }
}

static VOID p1(VOID)
{
    WORD i;

    for (i=0; i<80; i++) {
        print_char('p', 15, P1_MSG_POS, i);
        time_delay(7);
    }

    process_1 = 1;

    while (1) ;
}

static VOID p2(VOID)
{
    WORD i;

    for (i=0; i<80; i++) {
        print_char('s', 15, P2_MSG_POS, i);
        time_delay(7);
    }

    process_2 = 1;

    while (1) ;
}

static VOID init_pt(VOID)
{
    BYTE *sche_stack;

    process_table[1].regs.es = 0;
    process_table[1].regs.ds = 0;

    sche_stack = (BYTE *) &(process_table[1].stack[1024-3]);
    asm push ax
    asm mov ax,word ptr sche_stack+2
    asm mov temp_reg_ss,ax
    asm mov ax,word ptr sche_stack
    asm mov temp_reg_sp,ax
    asm pop ax
    process_table[1].regs.sp = temp_reg_sp;
    process_table[1].regs.ss = temp_reg_ss;

    process_table[1].stack[1024-1] = 0x202;
    sche_stack = (BYTE *) p1;
    asm push ax
    asm mov ax,word ptr sche_stack+2
    asm mov temp_reg_ss,ax
    asm mov ax,word ptr sche_stack
    asm mov temp_reg_sp,ax
    asm pop ax
    process_table[1].stack[1024-2] = temp_reg_ss;
    process_table[1].stack[1024-3] = temp_reg_sp;

    process_table[2].regs.es = 0;
    process_table[2].regs.ds = 0;

    sche_stack = (BYTE *) &(process_table[2].stack[1024-3]);
    asm push ax
    asm mov ax,word ptr sche_stack+2
    asm mov temp_reg_ss,ax
    asm mov ax,word ptr sche_stack
    asm mov temp_reg_sp,ax
    asm pop ax
    process_table[2].regs.sp = temp_reg_sp;
    process_table[2].regs.ss = temp_reg_ss;

    process_table[2].stack[1024-1] = 0x202;
    sche_stack = (BYTE *) p2;
    asm push ax
    asm mov ax,word ptr sche_stack+2
    asm mov temp_reg_ss,ax
    asm mov ax,word ptr sche_stack
    asm mov temp_reg_sp,ax
    asm pop ax
    process_table[2].stack[1024-2] = temp_reg_ss;
    process_table[2].stack[1024-3] = temp_reg_sp;
}

static VOID init_flag(VOID)
{
    process_1 = 0;
    process_2 = 0;
}

static VOID task_switch(VOID)
{
    BYTE msg='h';

    init_flag();

    print_char(msg, 4, RMHO_MSG_POS, 1);
    p = print_char;

    modify_time_int();

    process_table[0].pid = 0xffff;
    process_table[1].pid = 0x5555;
    process_table[2].pid = 0xaaaa;

    no = 0;
    sche_context = (BYTE *) &(process_table[no].regs);

    open_time_int();

    init_pt();

    asm sti

    while (0==process_1 && 0==process_2)
        asm hlt;

    asm cli;

    print_char('o', 4, RMHO_MSG_POS, 2);
}

VOID rmho(VOID)
{
    os_8259a();

    task_switch();

    bios_8259a();
}

