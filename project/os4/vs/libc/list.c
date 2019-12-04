/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : list.c
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <typedef.h>
#include <list.h>

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description : 链表的示例代码
 * history     :
 ***************************************************************/
LOCALC void list_example_code(void)
{
    struct example_list {
        u32 info;
        struct list_node node;
        u32 data;
    };

    struct example_list head;

    struct example_list *new;
    struct example_list mem;

    struct list_node *i;
    struct example_list *qnode;

    /* init */
    init_list_head(&head.node);

    /* create new node */
    new = &mem; /* usually we use malloc */
    new->data = 0;

    /* insert */
    add_list_head(&head.node, &new->node);

    /* query */
    loop_list(i, &head.node) {
        qnode = list_addr(i, struct example_list, node);
        /* deal with qnode */
    }

    /* delete */
    del_list(&new->node);
}

