
#include <os_type.h>
#include <kmem_struct.h>
#include <graph_disp.h>

void kmem_setup_flag(int num_count, int start_pos)
{
     int ulLoop = 0;

     /* start pos */
     kmem_map[start_pos].use_or_no = enum_kmem_start_no_end;
     kmem_use_block_size = kmem_use_block_size + kmem_map[start_pos].size;

     /* mid */
     for (ulLoop = 1; ulLoop < num_count - 1; ulLoop++)
     {
         kmem_map[ulLoop + start_pos].use_or_no = enum_kmem_mid;
         kmem_use_block_size = kmem_use_block_size + kmem_map[ulLoop + start_pos].size;
     }

     /* end */
     kmem_map[start_pos + num_count -1].use_or_no = enum_kmem_end;
     kmem_use_block_size = kmem_use_block_size + kmem_map[start_pos + num_count -1].size;
}

/* kernel memory alloc */
void *kmalloc(os_uint32 ulsize)
{
     os_uint32 ulLoop = 0;
     os_uint32 size_count = 0;
     os_uint32 num_count = 0;

     /* lookup */
     for (ulLoop = 0; ulLoop < kmem_map_num; ulLoop++)
     {
         if ((enum_kmem_not_use == kmem_map[ulLoop].use_or_no)
           &&(ulsize <= kmem_map[ulLoop].size))
         {
                /* set flag */
                kmem_map[ulLoop].use_or_no = enum_kmem_start_end;

                /* how many mem used. */
                kmem_use_block_size = kmem_use_block_size + kmem_map[ulLoop].size;

                return kmem_map[ulLoop].p_addr;
         }
     }

     /* merge */
     for (ulLoop = kmem_map_num-1 ; ulLoop >= 0; ulLoop--)
     {
         /* not use, add count */
         if (enum_kmem_not_use == kmem_map[ulLoop].use_or_no)
         {
               /* size is enough */
               if (ulsize <= (size_count + kmem_map[ulLoop].size))
               {
                   num_count++;

                   /* start set flag */
                   kmem_setup_flag(num_count, ulLoop);

                   return kmem_map[ulLoop].p_addr;
               }
               /* size is less */
               else
               {
                   size_count = size_count + kmem_map[ulLoop].size;
                   num_count++;
               }
         }
         /* lookup use mem block, clear count */
         else
         {
                               size_count = 0;
                               num_count = 0;
         }
     }

     return os_null;
}

void kfree(void *pointer)
{
     os_uint32 ulLoop = 0;
     os_uint32 ulLoop_1 = 0;

     for (ulLoop = 0; ulLoop < kmem_map_num; ulLoop++)
     {
         if (pointer == kmem_map[ulLoop].p_addr)
         break;
     }
     print_int_x_no_pos(ulLoop);
     print_int_x_no_pos(kmem_map[ulLoop].use_or_no);
     /* no chain */
     if (enum_kmem_start_end == kmem_map[ulLoop].use_or_no)
     {
                            kmem_map[ulLoop].use_or_no = enum_kmem_not_use;
                            kmem_use_block_size = kmem_use_block_size - kmem_map[ulLoop].size;
                            return;
     }

     /* chain */
     if (enum_kmem_start_no_end == kmem_map[ulLoop].use_or_no)
     {
         kmem_map[ulLoop].use_or_no = enum_kmem_not_use;
         kmem_use_block_size = kmem_use_block_size - kmem_map[ulLoop].size;

         for (ulLoop_1 = ulLoop + 1; ulLoop < kmem_map_num; ulLoop++)
         {
             if ((enum_kmem_mid != kmem_map[ulLoop_1].use_or_no)
               &&(enum_kmem_end != kmem_map[ulLoop_1].use_or_no))
             return;

             kmem_map[ulLoop_1].use_or_no = enum_kmem_not_use;
             kmem_use_block_size = kmem_use_block_size - kmem_map[ulLoop_1].size;
         }
     }

     return;
}

/* kernel memory inquery */
void kmem_inquery()
{
     /* clear */

     print_int_graph_x(kmem_use_block_size, 460, 20, 3);
}

