
/*******************************************************************************
this memory is start at 0x400000, lenth is 1m.
it used for timer, message, semaphores.
*******************************************************************************/
#include <graph_disp.h>
#include <timer.h>
#include <rtc.h>

//timer memory setup, u can modify it for memory alloc.
timer_mem_setup_node_stru timer_mem_setup[]=
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
timer_mem_map_node_stru timer_mem_map[timer_mem_map_num];

int timer_mem_use_block_size;

/* kernel memory inquery */
void timer_mem_inquery()
{
     /* clear */

     print_int_graph_x(timer_mem_use_block_size, 550, 20, 3);
}

/*******************
kernel init function
 *******************/
void timer_mem_init()
{
     os_uint32 ulLoop_1 = 0;
     os_uint32 ulLoop_2 = 0;
     os_uint32 ulcount = 0;

     /* base addr, kernel mem = 0x800000, timer mem = 0x700000 */
     os_uint32 ulAddr = timer_mem_start_addr;

     for (ulLoop_1 = 0; ulLoop_1 < timer_mem_setup_num; ulLoop_1++)
     {
         for (ulLoop_2 = 0; ulLoop_2 < timer_mem_setup[ulLoop_1].num; ulLoop_2++)
         {
             timer_mem_map[ulLoop_2 + ulcount].use_or_no = enum_timer_mem_not_use;/* no use */
             timer_mem_map[ulLoop_2 + ulcount].p_addr = (os_uint8 *)ulAddr;
             timer_mem_map[ulLoop_2 + ulcount].size = timer_mem_setup[ulLoop_1].size;
             ulAddr = ulAddr + timer_mem_setup[ulLoop_1].size;
         }

         ulcount = ulcount + timer_mem_setup[ulLoop_1].num;
     }

     /* use size is 0 */
     timer_mem_use_block_size = 0;

     print_int_graph_x(ulAddr, 280, 180, 3);

     timer_mem_inquery();
}

void timer_mem_setup_flag(int num_count, int start_pos)
{
     int ulLoop = 0;

     /* start pos */
     timer_mem_map[start_pos].use_or_no = enum_timer_mem_start_no_end;
     timer_mem_use_block_size = timer_mem_use_block_size + timer_mem_map[start_pos].size;

     /* mid */
     for (ulLoop = 1; ulLoop < num_count - 1; ulLoop++)
     {
         timer_mem_map[ulLoop + start_pos].use_or_no = enum_timer_mem_mid;
         timer_mem_use_block_size = timer_mem_use_block_size + timer_mem_map[ulLoop + start_pos].size;
     }

     /* end */
     timer_mem_map[start_pos + num_count -1].use_or_no = enum_timer_mem_end;
     timer_mem_use_block_size = timer_mem_use_block_size + timer_mem_map[start_pos + num_count -1].size;
}

/* kernel memory alloc */
void *timer_malloc(os_uint32 ulsize)
{
     os_uint32 ulLoop = 0;
     os_uint32 size_count = 0;
     os_uint32 num_count = 0;

     /* lookup */
     for (ulLoop = 0; ulLoop < timer_mem_map_num; ulLoop++)
     {
         if ((enum_timer_mem_not_use == timer_mem_map[ulLoop].use_or_no)
           &&(ulsize <= timer_mem_map[ulLoop].size))
         {
                /* set flag */
                timer_mem_map[ulLoop].use_or_no = enum_timer_mem_start_end;

                /* how many mem used. */
                timer_mem_use_block_size = timer_mem_use_block_size + timer_mem_map[ulLoop].size;

                return timer_mem_map[ulLoop].p_addr;
         }
     }

     /* merge */
     for (ulLoop = timer_mem_map_num-1 ; ulLoop >= 0; ulLoop--)
     {
         /* not use, add count */
         if (enum_timer_mem_not_use == timer_mem_map[ulLoop].use_or_no)
         {
               /* size is enough */
               if (ulsize <= (size_count + timer_mem_map[ulLoop].size))
               {
                   num_count++;

                   /* start set flag */
                   timer_mem_setup_flag(num_count, ulLoop);

                   return timer_mem_map[ulLoop].p_addr;
               }
               /* size is less */
               else
               {
                   size_count = size_count + timer_mem_map[ulLoop].size;
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

void timer_mem_free(void *pointer)
{
     os_uint32 ulLoop = 0;
     os_uint32 ulLoop_1 = 0;

     for (ulLoop = 0; ulLoop < timer_mem_map_num; ulLoop++)
     {
         if (pointer == timer_mem_map[ulLoop].p_addr)
         break;
     }
     print_int_x_no_pos(ulLoop);
     print_int_x_no_pos(timer_mem_map[ulLoop].use_or_no);
     /* no chain */
     if (enum_timer_mem_start_end == timer_mem_map[ulLoop].use_or_no)
     {
                            timer_mem_map[ulLoop].use_or_no = enum_timer_mem_not_use;
                            timer_mem_use_block_size = timer_mem_use_block_size - timer_mem_map[ulLoop].size;
                            return;
     }

     /* chain */
     if (enum_timer_mem_start_no_end == timer_mem_map[ulLoop].use_or_no)
     {
         timer_mem_map[ulLoop].use_or_no = enum_timer_mem_not_use;
         timer_mem_use_block_size = timer_mem_use_block_size - timer_mem_map[ulLoop].size;

         for (ulLoop_1 = ulLoop + 1; ulLoop < timer_mem_map_num; ulLoop++)
         {
             if ((enum_timer_mem_mid != timer_mem_map[ulLoop_1].use_or_no)
               &&(enum_timer_mem_end != timer_mem_map[ulLoop_1].use_or_no))
             return;

             timer_mem_map[ulLoop_1].use_or_no = enum_timer_mem_not_use;
             timer_mem_use_block_size = timer_mem_use_block_size - timer_mem_map[ulLoop_1].size;
         }
     }

     return;
}

