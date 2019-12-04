/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : kdm.c
 * version     : 1.0
 * description : (key) kernel dynamic memory, 基于变长内存块.
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include "kdm.h"

/***************************************************************
 global variable declare
 ***************************************************************/
/* 动态内存配置信息, 4K align */
LOCALD const struct kdm_cfg_item kdm_cfg[KDMB_COUNT] = {
    { 0x100,    0x1000 },
    { 0x200,    0x800 },
    { 0x400,    0x400 },
    { 0x800,    0x200 },
    { 0x1000,   0x100 },
    { 0x2000,   0x100 },
    { 0x4000,   0x80 },
    { 0x8000,   0x40 },
    { 0x10000,  0x40 },
    { 0x20000,  0x20 },
    { 0x40000,  0x20 },
    { 0x80000,  0x20 },
    { 0x100000, 0x10 },
    { 0x200000, 0x10 },
    { 0x400000, 0x8 },
    { 0x800000, 0x8 }
};

/* 内存控制块信息 */
LOCALD struct kdmcb_info kdmcb_info[KDMB_COUNT] = { 0 };
LOCALD spinlock_t kdmcb_lock;

/* 内存控制块起始地址 */
LOCALD os_u32 kdmcb_addr = 0;

/* 内核动态内存起始地址 */
LOCALD os_u32 kdm_addr = 0;

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dump_kdm_info(os_void)
{
    os_u32 i, j, k, n;
    struct kdmcb *block;
#define MAX_KDM_STA_CNT 0x10
struct kdm_statistics {
    const os_u8 *file;
    os_u32 line;

    os_uint cnt;
} statistics[MAX_KDM_STA_CNT];
    os_uint search_index;
    os_u32 size;

    print("kdm addr: %x %x\n", kdmcb_addr, kdm_addr);
    for (i = 0; i < array_size(kdmcb_info); i++) {
        print("[%x, %x]", kdmcb_info[i].size, kdmcb_info[i].idle_num);
        if (0 == kdmcb_info[i].idle_num) {
            size = kdmcb_info[i].size;
        }
        block = kdmcb_info[i].idle_block;
        while (block) {
            //flog("kdm addr: %x", block);
            //flog("addr: %x\n", block->addr);
            block = block->next;
        }
    } print("\n");

    n = 0;
    for (i = 0; i < KDMB_COUNT; i++) {
        n += kdm_cfg[i].count;
    };

    for (search_index = 0, i = 0, block = (struct kdmcb *) kdmcb_addr; i < n; i++, block++) {
        if (KDM_STATUS_BUSY == block->status) {
            if (size == kdmcb_info[block->kdmcb_info_index].size) {
                print("%s-%d-%d\n", block->file, block->line, size);
            }
        }
    } print("\n");

    mem_set(statistics, 0, MAX_KDM_STA_CNT * sizeof(struct kdm_statistics));
    for (search_index = 0, i = 0, block = (struct kdmcb *) kdmcb_addr; i < n; i++, block++) {
        if ((0 == block->kdmcb_info_index)
         && (KDM_STATUS_BUSY == block->status)) {
            for (j = 1; j < MAX_KDM_STA_CNT; j++) {
                if ((0 == statistics[j].cnt)
                 || ((block->file == statistics[j].file) && (block->line == statistics[j].line))) {
                    break;
                }
            }
            if ((0 == search_index) && (MAX_KDM_STA_CNT <= j)) {
                search_index = i;
            }
            if (0 == statistics[j].cnt) {
                statistics[j].file = block->file;
                statistics[j].line = block->line;
                statistics[j].cnt = 1;
            } else {
                statistics[j].cnt++;
            }
        }
    }

    for (; search_index < n;) {
        block = (struct kdmcb *) kdmcb_addr + search_index;

        statistics[0].cnt = 1;
        statistics[0].file = block->file;
        statistics[0].line = block->line;

        /* filter repeat */
        for (j = 1; j < MAX_KDM_STA_CNT; j++) {
            if ((statistics[0].file == statistics[j].file)
             && (statistics[0].line == statistics[j].line)) {
                break;
            }
        }
        if (MAX_KDM_STA_CNT > j) {
            search_index++;
            continue;
        }

        for (k = 1, i = search_index; i < n; i++, block++) {
            if ((0 == block->kdmcb_info_index)
             && (KDM_STATUS_BUSY == block->status)) {
                if ((block->file == statistics[0].file) && (block->line == statistics[0].line)) {
                    statistics[0].cnt++;
                } else if (k) {
                    k = 0;
                    search_index = i;
                }
            }
        }
        if (k) search_index++;

        for (k = 0, i = statistics[0].cnt, j = 1; j < MAX_KDM_STA_CNT; j++) {
            if (statistics[j].cnt < i) {
                k = j;
                i = statistics[j].cnt;
            }
        }
        if (k) {
            statistics[k].cnt = statistics[0].cnt;
            statistics[k].file = statistics[0].file;
            statistics[k].line = statistics[0].line;
        }
    }

    for (j = 1; j < MAX_KDM_STA_CNT; j++) {
        if (statistics[j].cnt) {
            print("%s-%d-%d\n", statistics[j].file, statistics[j].line, statistics[j].cnt);
        }
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void show_kdm_num(os_void)
{
    flog("kdm num: %x\n", kdmcb_info[0].idle_num);
}

LOCALD os_u8 kdm_debug_name[] = { "kdm" };
LOCALD struct dump_info kdm_debug = {
    kdm_debug_name,
    dump_kdm_info
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void OS_API *alloc_kdm(os_u32 size, os_u32 align, os_u32 line_no, IN os_u8 *file_name)
{
    os_u32 i;
    struct kdmcb_info *t;
    struct kdmcb **node;

    spin_lock(&kdmcb_lock);

    for (i = 0, t = kdmcb_info; i < array_size(kdmcb_info); i++, t++) {
        /* 找到第一个就停止 */
        if (t->size >= size) {
            if (0 < t->idle_num) {
                rmb();
                for (node = &t->idle_block; OS_NULL != *node; node = &(*node)->next) {
                    struct kdmcb *curr;
                    curr = *node;
                    if ((*node)->align >= align) {
                        /* delete */
                        *node = curr->next;

                        t->idle_num--;
                        curr->next = OS_NULL;
                        curr->status = KDM_STATUS_BUSY; /* 内存控制块已使用 */
                        curr->line = line_no; /* 记录行号 */
                        curr->file = file_name;

                        spin_unlock(&kdmcb_lock);
                        return curr->addr;
                    }
                }
            }

            spin_unlock(&kdmcb_lock);

            dump_kdm_info();
            print("kmalloc fail size %x %s %d\n", size, file_name, line_no);
            dump_stack(print);
            return OS_NULL; /* 符合块大小的内存块分配完毕 */
        }
    }

    spin_unlock(&kdmcb_lock);
    return OS_NULL; /* 块大小尺寸不合法 */
}

/***************************************************************
 * description : break mem
 * history     :
 ***************************************************************/
LOCALC os_void clear_kdm(os_u8 *addr, os_u32 len)
{
    mem_set(addr, 0xcc, len);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void OS_API *free_kdm(INOUT os_void **addr, os_u32 line)
{
    struct kdm_head *kdm_head = OS_NULL;
    struct kdmcb *temp_kdmcb = OS_NULL;

    /* 避免重复释放内存 */
    if (OS_NULL != *addr) {
        kdm_head = *addr - sizeof(struct kdm_head);

        /* 检查校验位 */
        cassert(KDM_CRC == kdm_head->check);
        if (KDM_CRC == kdm_head->check) {
            temp_kdmcb = kdm_head->kdmcb;

            /* 内存控制块是否空闲 */
            if (KDM_STATUS_IDLE != temp_kdmcb->status) {
#ifdef _DEBUG_VERSION_
                /* break memory first */
                clear_kdm(*addr, kdmcb_info[temp_kdmcb->kdmcb_info_index].size);
#endif
                spin_lock(&kdmcb_lock);

                /* 置控制块状态 */
                temp_kdmcb->status = KDM_STATUS_IDLE;

                /* 清行号 */
                temp_kdmcb->line = 0;
                temp_kdmcb->file = OS_NULL;
                temp_kdmcb->next = kdmcb_info[temp_kdmcb->kdmcb_info_index].idle_block;

                kdmcb_info[temp_kdmcb->kdmcb_info_index].idle_block = temp_kdmcb;
                kdmcb_info[temp_kdmcb->kdmcb_info_index].idle_num++;
                wmb();

                *addr = OS_NULL;

                spin_unlock(&kdmcb_lock);

                return kdm_head + 1;
            }
        }
    }

    /* 避免重复释放内存 | 校验位失败, 不是动态内存块 | 空闲内存控制块不再释放 */
    flog("free kdm fail, line %d\n", line);
    return OS_NULL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_u32 calculate_mem_align(os_u32 mem)
{
    os_u32 align;
    os_u8 i;

    align = 1;
    for (i = 0; i < 32; i++) {
        if (mem & align) {
            return align;
        }
        align = align << 1;
    }
    cassert(OS_FALSE);
    return 0;
}

/***************************************************************
 * description : 初始化内存控制块
 * history     :
 ***************************************************************/
os_void init_kdm(os_void)
{
    os_u32 i = 0;
    os_u32 j = 0;
    os_u32 kdmcb_num = 0;
    os_u32 kdm_size = 0;
    struct kdm_head *kdm_head_addr = OS_NULL;
    struct kdmcb *temp_kdmcb_addr = OS_NULL;

    /* 计算内存控制块的数量和内存块的大小 */
    for (i = 0; i < KDMB_COUNT; i++) {
        kdmcb_num += kdm_cfg[i].count;
        kdm_size += kdm_cfg[i].count * (sizeof(struct kdm_head) + kdm_cfg[i].size);
    }

    /* 分配内存控制块静态内存 */
    kdmcb_addr = alloc_ksm(kdmcb_num * sizeof(struct kdmcb));
    cassert(0 != kdmcb_addr);

    /* 分配动态内存空间 */
    kdm_addr = alloc_ksm(kdm_size);
    cassert(0 != kdm_addr);

    temp_kdmcb_addr = (struct kdmcb *) kdmcb_addr;

    kdm_head_addr = (struct kdm_head *) kdm_addr;

    for (i = 0; i < KDMB_COUNT; i++) {
        /* 内存块大小 */
        kdmcb_info[i].size = kdm_cfg[i].size;
        /* 内存块数量 */
        kdmcb_info[i].idle_num = kdm_cfg[i].count;
        /* 内存空闲块链表 */
        kdmcb_info[i].idle_block = temp_kdmcb_addr;

        for (j = 0; j < kdm_cfg[i].count; j++) {
            /* 内存块头部分 */
            kdm_head_addr->check = KDM_CRC;
            kdm_head_addr->kdmcb = temp_kdmcb_addr;
            temp_kdmcb_addr->kdmcb_info_index = i;

            /* 内存控制块状态 */
            temp_kdmcb_addr->status = KDM_STATUS_IDLE;

            temp_kdmcb_addr->line = 0;
            temp_kdmcb_addr->file = OS_NULL;

            /* 设置内存块地址 */
            temp_kdmcb_addr->addr = (os_u8 *) kdm_head_addr + sizeof(struct kdm_head);
            temp_kdmcb_addr->align = calculate_mem_align((os_u32) temp_kdmcb_addr->addr);

            kdm_head_addr = (struct kdm_head *)(temp_kdmcb_addr->addr + kdm_cfg[i].size);

            /* 设置内存控制块地址 */
            temp_kdmcb_addr->next = temp_kdmcb_addr + 1;

            temp_kdmcb_addr++;
        }

        temp_kdmcb_addr--;

        /* 链表收尾 */
        temp_kdmcb_addr->next = OS_NULL;

        temp_kdmcb_addr++;
    }

    init_spinlock(&kdmcb_lock);

    if (OS_SUCC != register_dump(&kdm_debug)) {
        flog("kdm register dump fail\n");
    }
}

