
#include <os_type.h>

#ifndef _kmemory_h
#define _kmemory_h

/* kernel memory addr start : 0x1300000 */
#define kernel_mem_start_addr 0x1300000

extern void *kmalloc(os_uint32 ulsize);
extern void kfree(void *pointer);
extern void kmem_inquery();

#endif

