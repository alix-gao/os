/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : sbuffer.c
 * version     : 1.0
 * description : (key) only one read and one write, no lock, no encapuslation and no depend mem.
 * author      : gaocheng
 * date        : 2012-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <typedef.h>
#include <arch.h>
#include <assert.h>
#include <memory.h>
#include <sbuffer.h>

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 * description : SBUFFER_HANDLE
 ***************************************************************/
struct sbuffer {
    u32 sbuffer_item_len;
    u32 sbuffer_item_num;
    ALLOC_FUNCPTR alloc;
    FREE_FUNCPTR free;

    u32 head; /* read */
    u8 cache_line[64]; /* clear false sharing */
    u32 tail; /* write */
    pointer buffer;
};

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description :
 * history     :
 ***************************************************************/
SBUFFER_HANDLE create_sbuffer(u32 sbuffer_item_len, u32 sbuffer_item_num, ALLOC_FUNCPTR alloc, FREE_FUNCPTR free)
{
    struct sbuffer *handle;

    if ((0 == sbuffer_item_num) || (0 == sbuffer_item_len) || (NULL == alloc) || (NULL == free)) {
        return NULL;
    }

    handle = alloc(sizeof(struct sbuffer) + sbuffer_item_num * sbuffer_item_len);
    if (NULL == handle) {
        return NULL;
    }

    handle->alloc = alloc;
    handle->free = free;
    handle->sbuffer_item_len = sbuffer_item_len;
    handle->sbuffer_item_num = sbuffer_item_num;

    handle->buffer = (pointer)(handle + 1);
    handle->head = 0;
    handle->tail = 0;

    return handle;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
uint destroy_sbuffer(IN SBUFFER_HANDLE handle)
{
    struct sbuffer *temp;

    temp = handle;
    if ((NULL != temp) && (0 != temp->free)) {
        temp->free(temp);
        return SUCC;
    }
    return FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
BOOL sbuffer_empty(IN SBUFFER_HANDLE handle)
{
    struct sbuffer *temp;

    if (NULL != handle) {
        temp = handle;
        if (temp->head != temp->tail) {
            rmb();
            return FALSE;
        }
    }
    /* handle is null, return true also. */
    return TRUE;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
BOOL sbuffer_full(IN SBUFFER_HANDLE handle)
{
    struct sbuffer *temp;

    if (NULL != handle) {
        temp = handle;
        if (((temp->tail + 1) % temp->sbuffer_item_num) != temp->head) {
            rmb();
            return FALSE;
        }
    }
    /* handle is null, return true also. */
    return TRUE;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
uint push_sbuffer(IN SBUFFER_HANDLE handle, IN void *item)
{
    struct sbuffer *temp;

    if (NULL != handle) {
        if (FALSE == sbuffer_full(handle)) {
            temp = handle;
            mem_cpy(temp->buffer + (temp->tail * temp->sbuffer_item_len), item, temp->sbuffer_item_len);
            wmb();
            temp->tail = (temp->tail + 1) % temp->sbuffer_item_num;
            return SUCC;
        }
    }
    return FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
uint pop_sbuffer(IN SBUFFER_HANDLE handle, OUT void *item)
{
    struct sbuffer *temp;

    if (NULL != handle) {
        if (FALSE == sbuffer_empty(handle)) {
            temp = handle;
            mem_cpy(item, temp->buffer + temp->head * temp->sbuffer_item_len, temp->sbuffer_item_len);
            wmb();
            temp->head = (temp->head + 1) % temp->sbuffer_item_num;
            return SUCC;
        }
    }
    return FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
void sbuffer_example(void)
{
    SBUFFER_HANDLE handle;
    struct example_item {
        int a;
        int b;
    };
    struct example_item data;

    LOCALC void *alloc_sbuffer(u32 len);
    LOCALC void *free_sbuffer(void *addr);

    /* count is 0x10 */
    handle = create_sbuffer(4, sizeof(struct example_item), alloc_sbuffer, free_sbuffer);
    if (NULL == handle) {
        flog("create sbuffer fail\n");
    }

    /* add item */
    data.a = 1;
    data.b = 2;
    push_sbuffer(handle, &data);

    data.a = data.b = 0;
    pop_sbuffer(handle, &data);

    /* clear */
    destroy_sbuffer(handle);
}
LOCALC void *alloc_sbuffer(u32 len)
{
    LOCALD u8 buffer[0x20];

    return buffer;
}
LOCALC void *free_sbuffer(void *addr)
{
    return NULL;
}

