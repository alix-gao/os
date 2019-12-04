/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : io.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __IO_H__
#define __IO_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
/* i/o byte */
#define inb(port,value) __asm__ __volatile__("inb %%dx,%%al":"=a"(value):"d"(port))

#define outb(port,value) __asm__ __volatile__("outb %%al,%%dx"::"a"(value),"d"(port))

#define inb_p(port,value) __asm__ __volatile__("inb %%dx,%%al\n"\
                                               "\tjmp 1f\n"\
                                               "1:\tjmp 1f\n"\
                                               "1:":"=a"(value):"d"(port))

#define outb_p(port,value) __asm__ __volatile__("outb %%al,%%dx\n"\
                                                "\tjmp 1f\n"\
                                                "1:\tjmp 1f\n"\
                                                "1:"::"a"(value),"d"(port))

/* i/o word */
#define inw(port,value) __asm__ __volatile__("inw %%dx,%%ax":"=a"(value):"d"(port))

#define outw(port,value) __asm__ __volatile__("outw %%ax,%%dx"::"a"(value),"d"(port))

#define inw_p(port,value) __asm__ __volatile__("inw %%dx,%%ax\n"\
                                               "\tjmp 1f\n"\
                                               "1:\tjmp 1f\n"\
                                               "1:":"=a"(value):"d"(port))

#define outw_p(port,value) __asm__ __volatile__("outw %%ax,%%dx\n"\
                                                "\tjmp 1f\n"\
                                                "1:\tjmp 1f\n"\
                                                "1:"::"a"(value),"d"(port))

/* i/o double word */
#define inl(port,value) __asm__ __volatile__("inl %%dx,%%eax":"=a"(value):"d"(port))

#define outl(port,value) __asm__ __volatile__("outl %%eax,%%dx"::"a"(value),"d"(port))

#define inl_p(port,value) __asm__ __volatile__("inl %%dx,%%eax\n"\
                                               "\tjmp 1f\n"\
                                               "1:\tjmp 1f\n"\
                                               "1:":"=a"(value):"d"(port))

#define outl_p(port,value) __asm__ __volatile__("outl %%eax,%%dx\n"\
                                                "\tjmp 1f\n"\
                                                "1:\tjmp 1f\n"\
                                                "1:"::"a"(value),"d"(port))

#define insl(port, buf, nr) __asm__ ("cld;rep;insl\n\t"::"d"(port), "D"(buf), "c"(nr))

#define outsl(buf, nr, port) __asm__ ("cld;rep;outsl\n\t"::"d"(port), "S" (buf), "c" (nr))

/* io ram for ia32, notice the key word volatile */
#define _io_virt(x) ((os_void *)(x))
#define read_byte(addr) (*(volatile os_u8 *) _io_virt(addr))
#define write_byte(addr, value) (*(volatile os_u8 *) _io_virt(addr) = (value))
#define read_word(addr) (*(volatile os_u16 *) _io_virt(addr))
#define write_word(addr, value) (*(volatile os_u16 *) _io_virt(addr) = (value))
#define read_dword(addr) (*(volatile os_u32 *) _io_virt(addr))
#define write_dword(addr, value) (*(volatile os_u32 *) _io_virt(addr) = (value))

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/

/***************************************************************
 extern function
 ***************************************************************/

#pragma pack()

#endif /* end of header */

