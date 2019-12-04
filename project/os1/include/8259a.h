
#ifndef _8259a_h
#define _8259a_h

#define cli() __asm__("cli")
#define sti() __asm__("sti");

#define send_master_8259a_eoi() outb(0x20,0x20)
#define send_slave_8259a_eoi() {outb(0xa0,0x20);\
                                outb(0x20,0x20);}

#endif

