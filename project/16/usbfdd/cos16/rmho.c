
#include "typedef.h"
#include "lib.h"

VOID time_int(VOID);

#define RMHO_MSG_POS 6

#define TASK1_MSG_POS 7

#define TASK2_MSG_POS 8

struct register_set {
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
};

struct task_entity {
    WORD pid;
    struct register_set regs;
    WORD stack[1024];
};

static struct task_entity process_table[3];
static VOID (*p)();
static BYTE msg_info='s';
static WORD no;

WORD temp_reg_sp;
WORD temp_reg_ss;
WORD temp_reg;

static WORD tick = 0;

/* dword */
BYTE *sche_context;

static WORD task1_running;
static WORD task2_running;

VOID time_int_func(VOID)
{
    print_char(msg_info, 15, RMHO_MSG_POS, 0);
    msg_info++;

    tick++;

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
    WORD t;
    WORD i;

    for (i = 0; i < time; i++) {
        t = tick;
        while (0x4 > (tick - t)) {
            ;
        }
    }
}

static VOID task1(VOID)
{
    WORD i;

    for (i=0; i<80; i++) {
        print_char('c', 15, TASK1_MSG_POS, i);
        time_delay(4);
        if (0 == task1_running) {
            break;
        }
    }

    task1_running = 0;

    while (1) ;
}

static VOID task2(VOID)
{
    WORD i;

    for (i=0; i<80; i++) {
        print_char('s', 15, TASK2_MSG_POS, i);
        time_delay(4);
        if (0 == task2_running) {
            break;
        }
    }

    task2_running = 0;

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
    sche_stack = (BYTE *) task1;
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
    sche_stack = (BYTE *) task2;
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
    task1_running = 1;
    task2_running = 1;
}

static VOID ori_task(VOID)
{
    init_flag();

    print_char('h', 4, RMHO_MSG_POS, 1);
    p = print_char;

    process_table[0].pid = 0xffff;
    process_table[1].pid = 0x5555;
    process_table[2].pid = 0xaaaa;

    no = 0;
    sche_context = (BYTE *) &(process_table[no].regs);

    init_pt();

    modify_time_int();
    open_time_int();

    asm sti

    while (task1_running && task2_running)
        asm hlt;

    asm cli;

    print_char('o', 4, RMHO_MSG_POS, 2);
}

VOID rmho(VOID)
{
    ori_task();
}

VOID stop_rmho(VOID)
{
    task1_running = task2_running = 0;
}

