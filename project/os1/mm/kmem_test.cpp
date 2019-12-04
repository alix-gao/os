
#include <kmemory.h>
#include <kmem_struct.h>
#include <graph_disp.h>

void kmem_test()
{
     char *p;

     p = (char *)kmalloc(0x20);
     if (0 == p)
     {
           ;
     }

     print_int_x_no_pos((int)p);
     kfree(p);

     p = (char *)kmalloc(0x40001);
     if (0 == p)
     {
           ;
     }
     kfree(p);
     print_int_x_no_pos(0xffffffff);
     print_int_x_no_pos((int)p);
     print_int_x_no_pos((int)(kmem_map[kmem_map_num-4].p_addr));
     print_int_x_no_pos((int)(kmem_map[kmem_map_num-4].use_or_no));
     print_int_x_no_pos((int)(kmem_map[kmem_map_num-3].p_addr));
     print_int_x_no_pos((int)(kmem_map[kmem_map_num-3].use_or_no));
     print_int_x_no_pos((int)(kmem_map[kmem_map_num-2].p_addr));
     print_int_x_no_pos((int)(kmem_map[kmem_map_num-2].use_or_no));
     print_int_x_no_pos((int)(kmem_map[kmem_map_num-1].p_addr));
     print_int_x_no_pos((int)(kmem_map[kmem_map_num-1].use_or_no));

     print_int_x_no_pos((int)p);

     p = (char *)kmalloc(0x10001);
     if (0 == p)
     {
           ;
     }
     kfree(p);
     print_int_x_no_pos(0xffffffff);
     print_int_x_no_pos((int)p);
     print_int_x_no_pos((int)(kmem_map[kmem_map_num-5].p_addr));
     print_int_x_no_pos((int)(kmem_map[kmem_map_num-5].use_or_no));
     print_int_x_no_pos((int)(kmem_map[kmem_map_num-4].p_addr));
     print_int_x_no_pos((int)(kmem_map[kmem_map_num-4].use_or_no));
     print_int_x_no_pos((int)(kmem_map[kmem_map_num-3].p_addr));
     print_int_x_no_pos((int)(kmem_map[kmem_map_num-3].use_or_no));
     print_int_x_no_pos((int)(kmem_map[kmem_map_num-2].p_addr));
     print_int_x_no_pos((int)(kmem_map[kmem_map_num-2].use_or_no));
     print_int_x_no_pos((int)(kmem_map[kmem_map_num-1].p_addr));
     print_int_x_no_pos((int)(kmem_map[kmem_map_num-1].use_or_no));

     kmem_inquery();
}

