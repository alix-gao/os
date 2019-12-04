
/*******************************************************************************
this memory is start at 0x400000, lenth is 1m.
it used for timer, message, semaphores.
*******************************************************************************/
#include <graph_disp.h>
#include <ipc_msg.h>

//msg memory setup, u can modify it for memory alloc.
msg_mem_setup_node_stru msg_mem_setup[]=
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
msg_mem_map_node_stru msg_mem_map[msg_mem_map_num];

int msg_mem_use_block_size;

/* kernel memory inquery */
void msg_mem_inquery()
{
     /* clear */

     print_int_graph_x(msg_mem_use_block_size, 550, 20, 3);
}

/*******************
kernel init function
 *******************/
void msg_mem_init()
{
     os_uint32 ulLoop_1 = 0;
     os_uint32 ulLoop_2 = 0;
     os_uint32 ulcount = 0;

     /* base addr, kernel mem = 0x800000, msg mem = 0x600000 */
     os_uint32 ulAddr = msg_mem_start_addr;

     for (ulLoop_1 = 0; ulLoop_1 < msg_mem_setup_num; ulLoop_1++)
     {
         for (ulLoop_2 = 0; ulLoop_2 < msg_mem_setup[ulLoop_1].num; ulLoop_2++)
         {
             msg_mem_map[ulLoop_2 + ulcount].use_or_no = enum_msg_mem_not_use;/* no use */
             msg_mem_map[ulLoop_2 + ulcount].p_addr = (os_uint8 *)ulAddr;
             msg_mem_map[ulLoop_2 + ulcount].size = msg_mem_setup[ulLoop_1].size;
             ulAddr = ulAddr + msg_mem_setup[ulLoop_1].size;
         }

         ulcount = ulcount + msg_mem_setup[ulLoop_1].num;
     }

     /* use size is 0 */
     msg_mem_use_block_size = 0;

     print_int_graph_x(ulAddr, 280, 180, 3);

     msg_mem_inquery();
}

void msg_mem_setup_flag(int num_count, int start_pos)
{
     int ulLoop = 0;

     /* start pos */
     msg_mem_map[start_pos].use_or_no = enum_msg_mem_start_no_end;
     msg_mem_use_block_size = msg_mem_use_block_size + msg_mem_map[start_pos].size;

     /* mid */
     for (ulLoop = 1; ulLoop < num_count - 1; ulLoop++)
     {
         msg_mem_map[ulLoop + start_pos].use_or_no = enum_msg_mem_mid;
         msg_mem_use_block_size = msg_mem_use_block_size + msg_mem_map[ulLoop + start_pos].size;
     }

     /* end */
     msg_mem_map[start_pos + num_count -1].use_or_no = enum_msg_mem_end;
     msg_mem_use_block_size = msg_mem_use_block_size + msg_mem_map[start_pos + num_count -1].size;
}

/* kernel memory alloc */
void *msg_malloc(os_uint32 ulsize)
{
     os_uint32 ulLoop = 0;
     os_uint32 size_count = 0;
     os_uint32 num_count = 0;

     /* lookup */
     for (ulLoop = 0; ulLoop < msg_mem_map_num; ulLoop++)
     {
         if ((enum_msg_mem_not_use == msg_mem_map[ulLoop].use_or_no)
           &&(ulsize <= msg_mem_map[ulLoop].size))
         {
                /* set flag */
                msg_mem_map[ulLoop].use_or_no = enum_msg_mem_start_end;

                /* how many mem used. */
                msg_mem_use_block_size = msg_mem_use_block_size + msg_mem_map[ulLoop].size;

                return msg_mem_map[ulLoop].p_addr;
         }
     }

     /* merge */
     for (ulLoop = msg_mem_map_num-1 ; ulLoop >= 0; ulLoop--)
     {
         /* not use, add count */
         if (enum_msg_mem_not_use == msg_mem_map[ulLoop].use_or_no)
         {
               /* size is enough */
               if (ulsize <= (size_count + msg_mem_map[ulLoop].size))
               {
                   num_count++;

                   /* start set flag */
                   msg_mem_setup_flag(num_count, ulLoop);

                   return msg_mem_map[ulLoop].p_addr;
               }
               /* size is less */
               else
               {
                   size_count = size_count + msg_mem_map[ulLoop].size;
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

void msg_mem_free(void *pointer)
{
     os_uint32 ulLoop = 0;
     os_uint32 ulLoop_1 = 0;

     for (ulLoop = 0; ulLoop < msg_mem_map_num; ulLoop++)
     {
         if (pointer == msg_mem_map[ulLoop].p_addr)
         break;
     }
     print_int_x_no_pos(ulLoop);
     print_int_x_no_pos(msg_mem_map[ulLoop].use_or_no);
     /* no chain */
     if (enum_msg_mem_start_end == msg_mem_map[ulLoop].use_or_no)
     {
                            msg_mem_map[ulLoop].use_or_no = enum_msg_mem_not_use;
                            msg_mem_use_block_size = msg_mem_use_block_size - msg_mem_map[ulLoop].size;
                            return;
     }

     /* chain */
     if (enum_msg_mem_start_no_end == msg_mem_map[ulLoop].use_or_no)
     {
         msg_mem_map[ulLoop].use_or_no = enum_msg_mem_not_use;
         msg_mem_use_block_size = msg_mem_use_block_size - msg_mem_map[ulLoop].size;

         for (ulLoop_1 = ulLoop + 1; ulLoop < msg_mem_map_num; ulLoop++)
         {
             if ((enum_msg_mem_mid != msg_mem_map[ulLoop_1].use_or_no)
               &&(enum_msg_mem_end != msg_mem_map[ulLoop_1].use_or_no))
             return;

             msg_mem_map[ulLoop_1].use_or_no = enum_msg_mem_not_use;
             msg_mem_use_block_size = msg_mem_use_block_size - msg_mem_map[ulLoop_1].size;
         }
     }

     return;
}

