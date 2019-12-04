
#include <kmem_struct.h>
#include <graph_disp.h>
#include <kmemory.h>

extern void kmem_test();

/*******************
kernel init function
 *******************/
void kmem_init()
{
     os_uint32 ulLoop_1 = 0;
     os_uint32 ulLoop_2 = 0;
     os_uint32 ulcount = 0;

     /* base addr */
     os_uint32 ulAddr = kernel_mem_start_addr;

     for (ulLoop_1 = 0; ulLoop_1 < kmem_setup_num; ulLoop_1++)
     {
         for (ulLoop_2 = 0; ulLoop_2 < kernel_mem_setup[ulLoop_1].num; ulLoop_2++)
         {
             kmem_map[ulLoop_2 + ulcount].use_or_no = enum_kmem_not_use;/* no use */
             kmem_map[ulLoop_2 + ulcount].p_addr = (os_uint8 *)ulAddr;
             kmem_map[ulLoop_2 + ulcount].size = kernel_mem_setup[ulLoop_1].size;
             ulAddr = ulAddr + kernel_mem_setup[ulLoop_1].size;
         }

         ulcount = ulcount + kernel_mem_setup[ulLoop_1].num;
     }

     /* use size is 0 */
     kmem_use_block_size = 0;

     print_int_graph_x(ulAddr, 180, 180, 3);
     kmem_test();
}

