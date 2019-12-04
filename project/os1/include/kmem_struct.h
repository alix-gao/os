
#include <os_type.h>

#ifndef _kmem_struct_h
#define _kmem_struct_h

/* kernel memory start addr */
#define kmem_start_addr 0x1300000

#define kmem_map_num 2*(2*2*2*2*2*2*2*2*2*2*2-1)+2

#define kmem_setup_num 13

enum
{
    enum_kmem_not_use,
    enum_kmem_start_end,
    enum_kmem_start_no_end,
    enum_kmem_mid,
    enum_kmem_end,
    enum_kmem_butt
};

/* kernel memory */
typedef struct kernel_mem_stru
{
       os_uint8 *p_addr;

       /* 0 is not use. 1 is begin & end. 2 is begin & noend. 3 is mid. 4 is end. */
       os_uint32 use_or_no:8;

       /* size less 24bit, 10 0000 = 1m, so 24bit = 16m */
       os_uint32 size:24;
}kmem_map_node_stru;

typedef struct kernel_mem_setup_stru
{
        os_uint32 size;
        os_uint32 num;
}kernel_mem_setup_node_stru;

extern kernel_mem_setup_node_stru kernel_mem_setup[];
extern kmem_map_node_stru kmem_map[];
extern int kmem_use_block_size;

#endif

