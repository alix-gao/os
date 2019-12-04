
#include <kmem_struct.h>

//kernel memory setup, u can modify it for memory alloc.
kernel_mem_setup_node_stru kernel_mem_setup[]=
{
        0x20,   2*2*2*2*2*2*2*2*2*2*2, /* 32 */
        0x40,   2*2*2*2*2*2*2*2*2*2,   /* 64 */
        0x80,   2*2*2*2*2*2*2*2*2,     /* 128 */
        0x100,  2*2*2*2*2*2*2*2,
        0x200,  2*2*2*2*2*2*2,
        0x400,  2*2*2*2*2*2,
        0x800,  2*2*2*2*2,
        0x1000, 2*2*2*2,
        0x2000, 2*2*2,
        0x4000, 2*2,
        0x8000, 2,
        0x10000,1, /* 64k */
        0x40000,1, /* 256k */
};

/* Sn = a1(1-q^n)/(1-q) */
kmem_map_node_stru kmem_map[kmem_map_num];

int kmem_use_block_size;

