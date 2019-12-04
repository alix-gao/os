
#ifndef _ipc_msg_h
#define _ipc_msg_h

/******************************************************************************
 include
 ******************************************************************************/
#include <kmem_struct.h>

/******************************************************************************
 macro
 ******************************************************************************/
#define msg_mem_map_num 2*(2*2*2*2*2*2*2*2*2*2*2-1)+2

#define msg_mem_setup_num 13

#define msg_mem_setup_node_stru kernel_mem_setup_node_stru

#define msg_mem_map_node_stru kmem_map_node_stru

/* ipc message memory addr start : 0x1100000 */
#define msg_mem_start_addr 0x1100000

/******************************************************************************
 enum
 ******************************************************************************/
enum
{
    enum_msg_mem_not_use,
    enum_msg_mem_start_end,
    enum_msg_mem_start_no_end,
    enum_msg_mem_mid,
    enum_msg_mem_end,
    enum_msg_mem_butt
};

/******************************************************************************
 struct
 ******************************************************************************/

#endif

