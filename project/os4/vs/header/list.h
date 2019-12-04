/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : list.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2012-04-22
 ***************************************************************/

#ifndef __LIST_H__
#define __LIST_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 * description :
 ***************************************************************/
struct list_node {
    struct list_node *prev;
    struct list_node *next;
};

/* read only, travel forward */
#define loop_list(pos, head) for ((pos) = (head)->next; (pos) != (head); (pos) = (pos)->next)

/* read and delete one (not safe for twice and more), travel forward */
#define loop_del_list(del, _n, head) for ((del) = (head)->next, (_n) = (del)->next; (del) != (head); (del) = (_n), (_n) = (_n)->next)

/* read only, travel backward */
#define loop_r_list(pos, head) for ((pos) = (head)->prev; (pos) != (head); (pos) = (pos)->prev)

/* read and delete one (not safe for twice and more), travel backward */
#define loop_r_del_list(del, _n, head) for ((del) = (head)->prev, (_n) = (del)->prev; (del) != (head); (del) = (_n), (_n) = (del)->prev)

#define list_addr(ptr, type, member) ((type *)((pointer)(ptr) - (pointer)(&((type *) 0)->member)))

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC inline void add_list_head(struct list_node *head, struct list_node *new)
{
    cassert((NULL != new) && (NULL != head));

    head->next->prev = new;
    new->next = head->next;
    new->prev = head;
    head->next = new;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC inline void add_list_tail(struct list_node *head, struct list_node *new)
{
    cassert((NULL != new) && (NULL != head));

    head->prev->next = new;
    new->prev = head->prev;
    new->next = head;
    head->prev = new;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC inline void del_list(struct list_node *node)
{
    cassert((NULL != node) && (NULL != node->prev) && (NULL != node->next));
    node->next->prev = node->prev;
    node->prev->next = node->next;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC inline void del_init_list(struct list_node *node)
{
    cassert((NULL != node) && (NULL != node->prev) && (NULL != node->next));
    node->next->prev = node->prev;
    node->prev->next = node->next;
    node->next = node->prev = node;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC inline uint list_empty(struct list_node *head)
{
    cassert(NULL != head);
    return head->next == head;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC inline void init_list_head(struct list_node *head)
{
    cassert(NULL != head);
    head->next = head->prev = head;
}

#pragma pack()

#endif /* end of header */

