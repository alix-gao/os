
//-----io.h-----
#ifndef _io_h
#define _io_h

//byte
#define inb(port,value)\
__asm__ __volatile__("inb %%dx,%%al":"=a"(value):"d"(port))

#define outb(port,value)\
__asm__ __volatile__("outb %%al,%%dx"::"a"(value),"d"(port))

#define inb_p(port,value)\
__asm__ volatile("inb %%dx,%%al\n"\
               "\tjmp 1f\n"\
               "1:\tjmp 1f\n"\
               "1:":"=a"(value):"d"(port))

#define outb_p(port,value)\
__asm__ volatile("outb %%al,%%dx\n"\
                "\tjmp 1f\n"\
                "1:\tjmp 1f\n"\
                "1:"::"a"(value),"d"(port))

//word
#define inw(port,value)\
__asm__ __volatile__("inw %%dx,%%ax":"=a"(value):"d"(port))

#define outw(port,value)\
__asm__ __volatile__("outw %%ax,%%dx"::"a"(value),"d"(port))

#define inw_p(port,value)\
__asm__ volatile("inw %%dx,%%ax\n"\
                 "\tjmp 1f\n"\
                 "1:\tjmp 1f\n"\
                 "1:":"=a"(value):"d"(port))

#define outw_p(port,value)\
__asm__ volatile("outw %%ax,%%dx\n"\
                 "\tjmp 1f\n"\
                 "1:\tjmp 1f\n"\
                 "1:"::"a"(value),"d"(port))

//double word
#define inl(port,value)\
__asm__ __volatile__("inl %%dx,%%eax":"=a"(value):"d"(port))

#define outl(port,value)\
__asm__ __volatile__("outl %%eax,%%dx"::"a"(value),"d"(port))

#define inl_p(port,value)\
__asm__ volatile("inl %%dx,%%eax\n"\
               "\tjmp 1f\n"\
               "1:\tjmp 1f\n"\
               "1:":"=a"(value):"d"(port))

#define outl_p(port,value)\
__asm__ volatile("outl %%eax,%%dx\n"\
                "\tjmp 1f\n"\
                "1:\tjmp 1f\n"\
                "1:"::"a"(value),"d"(port))

#endif

