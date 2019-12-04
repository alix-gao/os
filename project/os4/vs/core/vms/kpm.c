/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : kpm.c
 * version     : 1.0
 * description : (key) kernel page memory, first fit.
 * author      : gaocheng
 * date        : 2011-04-22
 ***************************************************************/

#include <core.h>
#include <kpm.h>

/* 数据页首地址 */
LOCALD os_u32 kpm_base = 0;

/* 页数量0x1000, 内存大小为16M */
#define KPM_PAGE_NUM 0x1000

/***************************************************************
 * description :
 ***************************************************************/
enum kpm_status {
    KPM_STATUS_UD, /* undefined */
    KPM_STATUS_IDLE,
    KPM_STATUS_BUSY
};

/***************************************************************
 * description :
 ***************************************************************/
struct kpmcb_struct {
    enum kpm_status status;
    os_u32 prev;
    os_u32 next;
    os_u32 line; /* record line no */
};
LOCALD struct kpmcb_struct kpm_cb[KPM_PAGE_NUM] = {0};

LOCALD spinlock_t kpm_lock;

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC __inline__ os_u32 get_align_mask(os_u32 align)
{
    os_u32 mask;

    for (mask = 1; 0 != mask; mask = mask << 1) {
        if (mask & align) {
            return (mask - 1);
        }
    }
    cassert(OS_FALSE);
    return 0;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC __inline__ os_u32 get_align_offset(os_u32 offset, os_u32 align_mask)
{
    os_u32 address;

    address = offset << KPM_ALIGN;
    if (align_mask & address) {
        address = (address & (~align_mask)) + align_mask + 1;
    }
    offset = address >> KPM_ALIGN;
    if (KPM_PAGE_NUM > offset) {
        return offset;
    }
    return KPM_PAGE_NUM;
}

/* handle align */
#define split_page(offset, i) \
    do { \
        if (i != offset) { \
            kpm_cb[offset].status = KPM_STATUS_IDLE; \
            kpm_cb[offset].prev = i; \
            kpm_cb[offset].next = kpm_cb[i].next; \
            kpm_cb[i].status = KPM_STATUS_IDLE; \
            kpm_cb[i].next = offset; \
            kpm_cb[i].line = 0; \
        } \
    } while (0)

/***************************************************************
 * description : 分配低4G的内存空间
 * history     :
 ***************************************************************/
os_void OS_API *alloc_coherent_mem(os_u32 size, os_u32 align, os_u32 line)
{
    os_u32 i;
    os_u32 page_num;
    os_u32 times;
    os_u32 mask;

    if (0 != size) {
        mask = get_align_mask(align);
        page_num = (size >> KPM_ALIGN) + ((size & ((1 << KPM_ALIGN) - 1)) ? (1) : (0));

        spin_lock(&kpm_lock);
        for (i = 0, times = 0; (KPM_PAGE_NUM > i) && (KPM_PAGE_NUM > times); i = kpm_cb[i].next, times++) {
            if (KPM_STATUS_IDLE == kpm_cb[i].status) {
                os_u32 offset;
                offset = get_align_offset(i, mask);
                if (KPM_PAGE_NUM != offset) {
                    if ((offset + page_num) < kpm_cb[i].next) { // insert new point
                        split_page(offset, i);

                        if (KPM_PAGE_NUM > kpm_cb[offset].next) {
                            kpm_cb[kpm_cb[offset].next].prev = offset + page_num;
                        }

                        kpm_cb[offset + page_num].status = KPM_STATUS_IDLE;
                        kpm_cb[offset + page_num].next = kpm_cb[offset].next;
                        kpm_cb[offset + page_num].prev = offset; /* used to be merge */

                        kpm_cb[offset].status = KPM_STATUS_BUSY;
                        kpm_cb[offset].next = offset + page_num;
                        kpm_cb[offset].line = line;

                        spin_unlock(&kpm_lock);
                        return (os_void *)(kpm_base + (offset << KPM_ALIGN));
                    } else if ((offset + page_num) == kpm_cb[i].next) {
                        split_page(offset, i);

                        kpm_cb[offset].status = KPM_STATUS_BUSY;
                        kpm_cb[offset].line = line;

                        spin_unlock(&kpm_lock);
                        return (os_void *)(kpm_base + (offset << KPM_ALIGN));
                    } else {}
                }
            }
        }
        spin_unlock(&kpm_lock);
    }
    return OS_NULL;
}

/***************************************************************
 * description : break mem
 * history     :
 ***************************************************************/
LOCALC os_void clear_kpm(os_u32 offset, os_u32 num)
{
    mem_set((kpm_base + (offset << KPM_ALIGN)), 0xcc, (1 << KPM_ALIGN) * num);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void OS_API *free_coherent_mem(os_void **addr, os_u32 line)
{
    os_u32 offset;
    struct kpmcb_struct *kpm;

    if ((0 == ((os_u32)(*addr) & ((1 << KPM_ALIGN) - 1))) && ((os_u32) *addr >= kpm_base) && ((os_u32) *addr < (kpm_base + KPM_PAGE_NUM * (1 << PAGE_ALIGN)))) {
        /* caculate offset */
        offset = ((os_u32) *addr - kpm_base) >> KPM_ALIGN;

        if (KPM_STATUS_BUSY == kpm_cb[offset].status) {
#ifdef _DEBUG_VERSION_
            /* debug, break memory first, and then free it. */
            clear_kpm(offset, kpm_cb[offset].next - offset);
#endif
            spin_lock(&kpm_lock);

            kpm_cb[offset].status = KPM_STATUS_IDLE;

            /* merge with the prev */
            if (0 != offset) {
                kpm = &kpm_cb[kpm_cb[offset].prev];
                if (KPM_STATUS_IDLE == kpm->status) {
                    os_u32 temp;

                    if (KPM_PAGE_NUM > kpm_cb[offset].next) {
                        kpm_cb[kpm_cb[offset].next].prev = kpm_cb[offset].prev;
                    }
                    kpm->next = kpm_cb[offset].next;

                    temp = kpm_cb[offset].prev;
                    kpm_cb[offset].prev = KPM_PAGE_NUM;
                    kpm_cb[offset].next = KPM_PAGE_NUM;
                    kpm_cb[offset].status = KPM_STATUS_UD;
                    kpm_cb[offset].line = 0;
                    offset = temp; // let offset is the last last position
                }
            }

            /* merge with the next */
            if (KPM_PAGE_NUM > kpm_cb[offset].next) {
                kpm = &kpm_cb[kpm_cb[offset].next];
                if (KPM_STATUS_IDLE == kpm->status) {
                    kpm_cb[offset].next = kpm->next;
                    kpm_cb[kpm->next].prev = offset;

                    kpm->prev = KPM_PAGE_NUM;
                    kpm->next = KPM_PAGE_NUM;
                    kpm->status = KPM_STATUS_UD;
                    kpm->line = 0;
                }
            }

            *addr = OS_NULL;

            spin_unlock(&kpm_lock);
        }
    } else {
        flog("kfree mem, %d input %x error\n", line, *addr);
    }

    return OS_NULL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void show_kpm_info(os_void)
{
    os_u32 i;
    os_u32 offset;

    offset = 0;
    while (KPM_PAGE_NUM > offset) {
        switch (kpm_cb[offset].status) {
        case KPM_STATUS_IDLE:
            print("(%d,%d)", offset, kpm_cb[offset].next - offset);
            break;

        case KPM_STATUS_BUSY:
            print("[%d,%d,%d]", offset, kpm_cb[offset].next - offset, kpm_cb[offset].line);
            break;

        default:
            cassert(OS_FALSE);
            break;
        }

        /* check error */
        for (i = offset + 1; i < kpm_cb[offset].next; i++) {
            if (KPM_STATUS_UD != kpm_cb[i].status) {
                print("kpm %d is broken!\n", i);
            }
        }

        /* next position */
        offset = kpm_cb[offset].next;
    } flog("\n");
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dump_kpm_info(os_void)
{
    print("kpm addr: %x %x\n", kpm_cb, kpm_base);
    show_kpm_info();
    //return;

    do {
        os_u8 *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8;
        p1 = cmalloc(8192 * 8, 4);
        p2 = cmalloc(8192 * 8, 4);
        p3 = cmalloc(8192 * 8, 4);
        p4 = cmalloc(8192 * 8, 4);
        p5 = cmalloc(8192 * 8, 4);
        p6 = cmalloc(8192 * 8, 4);
        p7 = cmalloc(8192 * 8, 4);
        p8 = cmalloc(8192 * 8, 4);
        cfree(p1);
        cfree(p2);
        cfree(p3);
        cfree(p4);
        cfree(p5);
        cfree(p6);
        cfree(p7);
        cfree(p8);
    } while (0);

    do {
        os_u8 *p1, *p2, *p3;
        p1 = cmalloc(0x1000, 0x100);
        p2 = cmalloc(0x2000, 0x100);
        p3 = cmalloc(0x1000, 0x100);
        cfree(p2);
        p2 = cmalloc(0x1000, 0x100);
        p1 = cmalloc(0x1000, 0x100);
        cfree(p2);
        show_kpm_info();
        cfree(p3);
        show_kpm_info();
    } while (0);

    do {
        os_u8 *p1, *p2, *p3;
        p1 = cmalloc(0x5000, 0x4000);
        p2 = cmalloc(0x200, 0x8000);
        p3 = cmalloc(0x8000, 0x100);
        show_kpm_info();
        cfree(p1);
        show_kpm_info();
        cfree(p3);
        show_kpm_info();
        cfree(p2);
        show_kpm_info();
    } while (0);
}

LOCALD os_u8 kpm_debug_name[] = { "kpm" };
LOCALD struct dump_info kpm_debug = {
    kpm_debug_name,
    dump_kpm_info
};

/***************************************************************
 * description : 位于低4G空间
 * history     :
 ***************************************************************/
os_void init_kpm(os_void)
{
    os_u32 i;
    os_u64 kpm_addr;
    os_uint len;

    len = KPM_PAGE_NUM * (1 << PAGE_ALIGN);

    /* 分配数据页内存 */
    kpm_addr = alloc_ksm(len);
    cassert(0 != kpm_addr);

    kpm_base = kpm_addr;

    init_spinlock(&kpm_lock);

    /* 初始化控制块 */
    for (i = 0; i < KPM_PAGE_NUM; i++) {
        kpm_cb[i].status = KPM_STATUS_UD;
        kpm_cb[i].prev = KPM_PAGE_NUM;
        kpm_cb[i].next = KPM_PAGE_NUM;
        kpm_cb[i].line = 0;
    }

    kpm_cb[0].status = KPM_STATUS_IDLE;
    kpm_cb[0].next = KPM_PAGE_NUM;

    if (OS_SUCC != register_dump(&kpm_debug)) {
        flog("kpm register dump fail\n");
    }
}

