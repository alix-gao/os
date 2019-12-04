/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : klm.c
 * version     : 1.0
 * description : (key) kernel link memory
 * author      : gaocheng
 * date        : 2013-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <lib.h>
#include <klm.h>

struct klm_node {
    struct list_node node;
    os_u32 line;
    os_u32 len;
    os_u8 addr[0]; /* ignored by sizeof() */
};

#define KLM_CRC UINT32_C(0x55aa55aa)
struct klm_head {
    os_u32 crc;
    os_u32 line;
    os_u32 totol_size;
    spinlock_t lock;
    struct list_node node;
};

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description : os_u32 array[];
 * history     :
 ***************************************************************/
os_ret OS_API create_klm(klm_handle *handle, os_u32 line_no)
{
    struct klm_head *klm;

    cassert(OS_NULL != handle);

    klm = (struct klm_head *) kmalloc(sizeof(struct klm_head));
    if (OS_NULL == klm) {
        flog("create klm kmalloc fail\n");
        return OS_FAIL;
    }
    init_list_head(&klm->node);
    klm->crc = KLM_CRC;
    klm->line = line_no;
    klm->totol_size = 0;
    init_spinlock(&klm->lock);

    *handle = klm;
    return OS_SUCC;
}

/***************************************************************
 * description : len = sizeof(array);
 * history     :
 ***************************************************************/
os_u32 OS_API klm_size(IN klm_handle handle)
{
    struct klm_head *klm;

    cassert(OS_NULL != handle);

    klm = handle;
    if (KLM_CRC == klm->crc) {
        return klm->totol_size;
    }
    return 0;
}

/***************************************************************
 * description : array[] = remalloc(more);
 * history     :
 ***************************************************************/
os_ret OS_API add_klm(IN klm_handle handle, os_u32 size, os_u32 line_no)
{
    struct klm_head *klm;

    cassert(OS_NULL != handle);

    klm = handle;
    if (KLM_CRC == klm->crc) {
        struct klm_node *node;

        spin_lock(&klm->lock);
        cassert(!add_u32_overflow(sizeof(struct klm_node), size));
        node = (struct klm_node *) kmalloc(sizeof(struct klm_node) + size);
        if (OS_NULL == node) {
            spin_unlock(&klm->lock);
            flog("add klm, alloc node fail\n");
            return OS_FAIL;
        }
        init_list_head(&node->node);
        node->len = size;
        node->line = line_no;

        add_list_tail(&klm->node, &node->node);
        cassert(!add_u32_overflow(klm->totol_size, size));
        klm->totol_size += size;
        spin_unlock(&klm->lock);
        return OS_SUCC;
    }
    return OS_FAIL;
}

/***************************************************************
 * description : array[i] = data;
 * history     :
 ***************************************************************/
os_ret OS_API write_klm(IN klm_handle handle, os_u32 offset, os_u8 *data, os_u32 size)
{
    struct klm_head *klm;

    cassert((OS_NULL != handle) && (OS_NULL != data));

    klm = handle;
    if (KLM_CRC == klm->crc) {
        cassert(!add_u32_overflow(offset, size));
        if (klm->totol_size >= (offset + size)) {
            struct list_node *i;
            struct klm_node *node;
            os_u32 i_klm;
            os_u32 i_data;
            os_u32 cp_len;

            i_klm = i_data = 0;
            spin_lock(&klm->lock);
            loop_list(i, &klm->node) {
                node = list_addr(i, struct klm_node, node);

                if (i_klm >= offset) {
                    cp_len = min(node->len, size - i_data);
                    mem_cpy(&node->addr[0], &data[i_data], cp_len);
                    if (node->len >= (size - i_data)) {
                        spin_unlock(&klm->lock);
                        return OS_SUCC;
                    }
                    i_data += cp_len;
                } else {
                    if (node->len > (offset - i_klm)) {
                        cp_len = min(node->len - (offset - i_klm), size);
                        mem_cpy(&node->addr[offset - i_klm], &data[0], cp_len);
                        if (cp_len >= size) {
                            spin_unlock(&klm->lock);
                            return OS_SUCC;
                        }
                        i_data += cp_len;
                    }
                }
                i_klm += node->len;
            }
            spin_unlock(&klm->lock);
            cassert(OS_FALSE);
        } /* space is not enough */
    } /* check fail */
    return OS_FAIL;
}

/***************************************************************
 * description : data = array[i];
 * history     :
 ***************************************************************/
os_ret OS_API read_klm(IN klm_handle handle, os_u32 offset, os_u8 *data, os_u32 size)
{
    struct klm_head *klm;

    cassert((OS_NULL != handle) && (OS_NULL != data));

    klm = handle;
    if (KLM_CRC == klm->crc) {
        cassert(!add_u32_overflow(offset, size));
        if (klm->totol_size >= (offset + size)) {
            struct list_node *i;
            struct klm_node *node;
            os_u32 i_klm;
            os_u32 i_data;
            os_u32 cp_len;

            i_klm = i_data = 0;
            spin_lock(&klm->lock);
            loop_list(i, &klm->node) {
                node = list_addr(i, struct klm_node, node);

                if (i_klm >= offset) {
                    cp_len = min(node->len, size - i_data);
                    mem_cpy(&data[i_data], &node->addr[0], cp_len);
                    if (node->len >= (size - i_data)) {
                        spin_unlock(&klm->lock);
                        return OS_SUCC;
                    }
                    i_data += cp_len;
                } else {
                    if (node->len > (offset - i_klm)) {
                        cp_len = min(node->len - (offset - i_klm), size);
                        mem_cpy(&data[0], &node->addr[offset - i_klm], cp_len);
                        if (cp_len >= size) {
                            spin_unlock(&klm->lock);
                            return OS_SUCC;
                        }
                        i_data += cp_len;
                    }
                }
                i_klm += node->len;
            }
            spin_unlock(&klm->lock);
            cassert(OS_FALSE);
        } /* space is not enough */
    } /* check fail */
    return OS_FAIL;
}

/***************************************************************
 * description : if (last_addr == NULL) { from the first one; }
 * history     :
 ***************************************************************/
os_u8 OS_API *loop_klm(IN klm_handle handle, IN os_u8 *last_addr, os_u32 *next_size)
{
    struct klm_head *klm;
    struct klm_node *node;

    cassert((OS_NULL != handle) && (OS_NULL != next_size));

    klm = handle;
    if (KLM_CRC == klm->crc) {
        if (OS_NULL == last_addr) {
            spin_lock(&klm->lock);
            /* return the first one */
            if (&klm->node != klm->node.next) {
                node = list_addr(klm->node.next, struct klm_node, node);
                *next_size = node->len;
                spin_unlock(&klm->lock);
                return node->addr;
            }
            spin_unlock(&klm->lock);
        } else {
            spin_lock(&klm->lock);
            node = list_addr(last_addr, struct klm_node, addr);
            if (&klm->node != node->node.next) {
                node = list_addr(node->node.next, struct klm_node, node);
                *next_size = node->len;
                spin_unlock(&klm->lock);
                return node->addr;
            }
            spin_unlock(&klm->lock);
        }
    }
    return OS_NULL;
}

/***************************************************************
 * description : os_u32 *p = &array[offset];
 * history     :
 ***************************************************************/
os_u8 OS_API *klm_addr(IN klm_handle handle, os_u32 offset, os_u32 size)
{
    struct klm_head *klm;

    cassert(OS_NULL != handle);

    klm = handle;
    if (KLM_CRC == klm->crc) {
        cassert(!add_u32_overflow(offset, size));
        if (klm->totol_size >= (offset + size)) {
            struct list_node *i;
            struct klm_node *node;
            os_u32 pos;

            pos = 0;
            spin_lock(&klm->lock);
            loop_list(i, &klm->node) {
                node = list_addr(i, struct klm_node, node);

                cassert(!add_u32_overflow(pos, node->len));
                if ((pos + node->len) > offset) {
                    /* overflow check or memory limit check */
                    cassert(!add_u32_overflow((os_u32) node->addr, offset - pos));
                    if ((node->len - (offset - pos)) >= size) {
                        spin_unlock(&klm->lock);
                        return &node->addr[offset - pos];
                    } else {
                        spin_unlock(&klm->lock);
                        return OS_NULL;
                    }
                }
                pos += node->len;
            }
            spin_unlock(&klm->lock);
            cassert(OS_FALSE);
        } /* space is not enough */
    } /* check fail */
    return OS_NULL;
}

/***************************************************************
 * description : free(array);
 * history     :
 ***************************************************************/
os_ret OS_API destroy_klm(IN klm_handle handle)
{
    struct klm_head *klm;

    cassert(OS_NULL != handle);

    klm = handle;
    if (KLM_CRC == klm->crc) {
        struct list_node *i, *_save;
        struct klm_node *node;

        spin_lock(&klm->lock);
        loop_del_list(i, _save, &klm->node) {
            node = list_addr(i, struct klm_node, node);
            del_list(i);
            kfree(node);
        }
        spin_unlock(&klm->lock);
        kfree(klm);
        return OS_SUCC;
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void test_klm(os_void)
{
    klm_handle handle;
    os_u8 *buffer;
    os_u8 arr[0x10] = { 0 };
    os_u32 i;
    os_ret ret;

    ret = create_klm(&handle, __LINE__);
    if (OS_SUCC != ret) {
        flog("test, create klm fail\n");
        return;
    }

    ret = add_klm(handle, 8, __LINE__);
    if (OS_SUCC != ret) {
        flog("test, add klm fail\n");
        return;
    }

    ret = add_klm(handle, 4, __LINE__);
    if (OS_SUCC != ret) {
        flog("test, add klm fail\n");
        return;
    }

    buffer = klm_addr(handle, 0, 8);
    if (OS_NULL == buffer) {
        flog("test, get klm addr fail\n");
        return;
    }
    buffer[0] = 1;
    buffer[1] = 2;
    buffer[2] = 3;
    buffer[3] = 4;
    buffer[4] = 5;
    buffer[5] = 6;
    buffer[6] = 7;
    buffer[7] = 8;

    buffer = klm_addr(handle, 9, 1);
    if (OS_NULL == buffer) {
        flog("test, get klm addr fail\n");
        return;
    }
    buffer[0] = 10;
    buffer[1] = 11;

    ret = read_klm(handle, 8, arr, 3);
    if (OS_SUCC != ret) {
        flog("test, read klm fail\n");
        return;
    }

    ret = destroy_klm(handle);
    if (OS_SUCC != ret) {
        flog("test, destroy klm fail\n");
        return;
    }

    for (i = 0; i < 0x10; i++) {
        flog("%d ", arr[i]);
    } flog("\n");
}

