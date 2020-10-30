/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : fat32.c
 * version     : 1.0
 * description : (key) mbr.dbr.reserved.fat1.fat2.[rootdir][3][4]
 * author      : gaocheng
 * date        : 2011-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <vds.h>
#include <vfs.h>
#include "fat32.h"

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret fat32_short_file_name(struct fat32_dir_item *dir, os_u16 *file_name)
{
    os_u32 i, cnt;
    os_u8 tmp;

    cnt = 0;
    for (i = 0; i < FAT_DIR_NAME_SIZE; i++) {
        tmp = dir->DIR_Name[i];
        if (FAT_INVALID_NAME_CHAR != tmp) {
            if (8 == i) { // test, if folder's length is 9, it becomes longfilename and the last 3 is 0x20.
                file_name[cnt] = '.';
                cnt++;
            }

            if (('A' <= tmp) && ('Z' >= tmp)) {
                file_name[cnt] = tmp + ('a' - 'A');
            } else {
                file_name[cnt] = tmp;
            }
            cnt++;
        }
    }
    file_name[cnt] = '\0';
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC inline os_ret get_long_file_name(os_u16 *name, os_u32 *pos, os_u16 *src, os_u32 len)
{
    os_u32 i;
    os_u16 c;

    for (i = 0; i < len; i++) {
        c = src[i];
        if (!long_name_is_terminated(c)) {
            if (FAT32_MAX_FILE_NAME_LEN <= *pos) {
                name[*(pos - 1)] = '\0';
                return OS_FAIL;
            }
            name[*pos] = c;
            (*pos)++;
        }
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret fat32_long_file_name(struct fat32_ldir_item *ldir, os_u16 *file_name)
{
    os_u32 cnt;

    cnt = 0;
    do {
        /* Names are also NUL terminated and padded with 0xffff characters ... */
        if ((OS_SUCC != get_long_file_name(file_name, &cnt, ldir->LDIR_Name1, LDIR_NAME1_LEN))
         || (OS_SUCC != get_long_file_name(file_name, &cnt, ldir->LDIR_Name2, LDIR_NAME2_LEN))
         || (OS_SUCC != get_long_file_name(file_name, &cnt, ldir->LDIR_Name3, LDIR_NAME3_LEN))) {
            fat_dbg("write_long_file_name fail\n");
            return OS_FAIL;
        }
        if (long_dir_is_end(ldir)) {
            /* last item */
            file_name[cnt] = '\0';
            return OS_SUCC;
        }
        ldir--;
    } while ((!dir_is_del(ldir)) && (!dir_is_end(ldir)) && (dir_is_long_name_attr(ldir)));
    return OS_FAIL;
}

/***************************************************************
 * description : Returns an unsigned byte checksum computed on an unsigned byte
 *               array. The array must be 11 bytes long and is assumed to contain
 *               a name stored in the format of a MS-DOS directory entry.
 * history     :
 ***************************************************************/
LOCALC os_u8 ChkSum(os_u8 *pFcbName)
{
    os_s16 FcbNameLen;
    os_u8 Sum;

    Sum = 0;
    for (FcbNameLen=11; FcbNameLen!=0; FcbNameLen--) {
        // NOTE: The operation is an unsigned char rotate right
        Sum = ((Sum & 1) ? 0x80 : 0) + (Sum >> 1) + *pFcbName++;
    }
    return (Sum);
}

/***************************************************************
 * description : Second, an 8-bit checksum is computed on the name contained in the short directory entry at the time the short and long directory entries are created.
 *               All 11 characters of the name in the short entry are used in the checksum calculation.
 *               The check sum is placed in every long entry.
 *               If any of the check sums in the set of long entries do not agree with the computed checksum of the name contained in the short entry,
 *               then the long entries are treated as orphans.
 * history     :
 ***************************************************************/
LOCALC os_ret fat32_file_checksum(struct fat32_dir_item *dir)
{
    struct fat32_ldir_item *ldir;
    os_u8 checksum;

    checksum = ChkSum(dir->DIR_Name);

    ldir = (struct fat32_ldir_item *)(dir - 1);
    if ((!dir_is_del(ldir)) && (!dir_is_end(ldir)) && (dir_is_long_name_attr(ldir))) {
        if (checksum != ldir->LDIR_Chksum) { // compare one item
            return OS_FAIL;
        }
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret fat32_file_name(struct fat32_dir_item *dir, os_u16 *file_name)
{
    struct fat32_ldir_item *ldir;

    ldir = (struct fat32_ldir_item *)(dir - 1);
    if ((dir_is_del(ldir)) || (dir_is_end(ldir)) || (!dir_is_long_name_attr(ldir))) {
        return fat32_short_file_name(dir, file_name);
    } else {
        os_ret ret;
        ret = fat32_file_checksum(dir);
        if (OS_SUCC != ret) {
            fat_dbg("file name checksum fail %x %d\n", *(os_u32 *) dir, __LINE__);
            return OS_FAIL;
        }
        return fat32_long_file_name(ldir, file_name);
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_bool cmp_file_name(struct fat32_dir_item *dir, IN os_u8 *file_name)
{
    os_u16 *item_name;
    os_u32 i;
    os_ret ret;

    item_name = kmalloc(sizeof(os_u16) * (1 + FAT32_MAX_FILE_NAME_LEN));
    if (OS_NULL == item_name) {
        fat_dbg("cmp file name, alloc mem fail\n");
        return OS_FALSE;
    }

    ret = fat32_file_name(dir, item_name);
    if (OS_SUCC != ret) {
        fat_dbg("cmp file name, get file name fail\n");
        ret = OS_FALSE;
        goto end;
    }

    /* compare */
    ret = OS_TRUE;
    for (i = 0; i < (FAT32_MAX_FILE_NAME_LEN + 1); i++) {
        if (file_name[i] != item_name[i]) {
            ret = OS_FALSE;
            goto end;
        }
        if ('\0' == file_name[i]) {
            break; // file_name[i] == item_name[i]
        }
    }
    if ((FAT32_MAX_FILE_NAME_LEN + 1) == i) {
        ret = OS_FALSE;
    }
  end:
    kfree(item_name);
    return ret;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC inline os_u32 get_next_clus(os_u32 clus, struct fat32_cache *fat_cache)
{
    os_u32 n;
    /* fat_cache->fat_table[0] 第0簇, 表示介质类型
       fat_cache->fat_table[1] 第1簇, reserved
       fat_cache->fat_table[2] 第2簇, 存储根目录 */
    spin_lock(&fat_cache->fat_lock);
    n = fat_cache->fat_table[clus] & END_OF_CLUS;
    spin_unlock(&fat_cache->fat_lock);
    return n;
}

/***************************************************************
 * description : read/write clus
 * history     :
 ***************************************************************/
LOCALC os_ret handle_clus(os_u32 clus, os_u8 *buffer, os_u32 device_id, struct fat32_cache *fat_cache,
                          os_ret OS_API (*operation)(os_u32 device_id, os_u32 start_sector, os_u16 num, OUT os_u8 *buffer))
{
    os_u32 start_sector;

    if (ROOT_DIR_CLUS_NO > clus) {
        return OS_FAIL;
    }

    /* 目录表是第一个数据簇, fat32的第二个 */
    start_sector = fat_cache->dir_sector + (clus - ROOT_DIR_CLUS_NO) * fat_cache->bpb.BPB_SecPerClus;

    return operation(device_id, start_sector, fat_cache->bpb.BPB_SecPerClus, buffer);
}

/***************************************************************
 * description : file includes fat, dir and file
 * history     :
 ***************************************************************/
LOCALC os_ret travel_file_by_clus(os_u32 clus, klm_handle data,
                                  os_u32 device_id, struct fat32_cache *fat_cache,
                                  os_ret OS_API (*operation)(os_u32 device_id, os_u32 start_sector, os_u16 num, OUT os_u8 *buffer))
{
    os_u32 offset;
    os_u8 *buffer;
    os_u32 size;
    os_ret ret;

    if (ROOT_DIR_CLUS_NO <= clus) {
        size = fat_cache->bpb.BPB_BytsPerSec * fat_cache->bpb.BPB_SecPerClus;
        offset = 0;
        do {
            /* 是否越界 */
            if (fat_cache->fat_max_clus_no <= clus) {
                if (END_OF_CLUS != clus) {
                    fat_dbg("fat32 overrun max clus %d\n", __LINE__);
                    return OS_FAIL;
                }
            }

            buffer = klm_addr(data, offset, size);
            if (OS_NULL == buffer) {
                fat_dbg("handle file by clus, get klm addr fail\n");
                return OS_FAIL;
            }
            ret = handle_clus(clus, buffer, device_id, fat_cache, operation);
            if (OS_SUCC != ret) {
                fat_dbg("handle clus %d fail\n", clus);
                return OS_FAIL;
            }

            offset += size;

            /* 获取文件的下一簇 */
            clus = get_next_clus(clus, fat_cache);
        } while (END_OF_CLUS != clus);
    }
    return OS_SUCC;
}

#define read_file_by_clus(clus, data, device_id, fat_cache) \
    travel_file_by_clus(clus, data, device_id, fat_cache, read_disk)
#define write_file_by_clus(clus, data, device_id, fat_cache) \
    travel_file_by_clus(clus, data, device_id, fat_cache, write_disk)

#define get_clus_no(dir) (os_u32)((dir)->DIR_FstClusLO + (os_u32)(((os_u32) (dir)->DIR_FstClusHI) << 16))

#define set_clus_no(dir, clus_no) \
    do { \
        (dir)->DIR_FstClusLO = (os_u16)(clus_no); \
        (dir)->DIR_FstClusHI = (os_u16)((clus_no) >> 16); \
    } while (0)

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC struct fat32_dir_item *get_file_dir_item(IN os_u8 *file_name, klm_handle dir)
{
    struct fat32_dir_item *item;
    os_u8 *buffer;
    os_u32 buffer_size;
    os_u32 item_cnt;
    os_u32 i;
    os_ret ret;

    for (buffer = OS_NULL, buffer = loop_klm(dir, buffer, &buffer_size);
         OS_NULL != buffer;
         buffer = loop_klm(dir, buffer, &buffer_size)) {
        item_cnt = buffer_size / sizeof(struct fat32_dir_item);
        item = (struct fat32_dir_item *) buffer;
        for (i = 0; i < item_cnt; i++, item++) {
            /* 目录表查找完毕 */
            if (dir_is_end(item)) {
                break;
            }

            /* 文件是否被删除 */
            if (dir_is_del(item)) {
                continue;
            }

            if (!dir_is_long_name_attr(item)) {
                ret = cmp_file_name(item, file_name);
                if (OS_TRUE == ret) {
                    return item;
                }
            }
        }
    }
    return OS_NULL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret clear_fat_clus(os_u32 clus_no, struct fat32_cache *fat_cache)
{
    spin_lock(&fat_cache->fat_lock);
    fat_cache->fat_table[clus_no] &= ~END_OF_CLUS;
    spin_unlock(&fat_cache->fat_lock);
    return OS_SUCC;
}

/***************************************************************
 * description : file includes fat, dir and file
 * history     :
 ***************************************************************/
LOCALC os_ret free_fat(struct fat32_cache *fat_cache, os_u32 clus_no)
{
    os_u32 tmp;

    if (ROOT_DIR_CLUS_NO < clus_no) {
        do {
            if (fat_cache->fat_max_clus_no <= clus_no) {
                if (END_OF_CLUS != clus_no) {
                    fat_dbg("fat32 overrun max clus %d\n", __LINE__);
                    return OS_FAIL;
                }
            }
            tmp = clus_no;
            clus_no = get_next_clus(clus_no, fat_cache);
            clear_fat_clus(tmp, fat_cache);
        } while (END_OF_CLUS != clus_no);
    } else {
        /* cassert(OS_FALSE); */
    }
    return OS_SUCC;
}

/***************************************************************
 * description : 第0簇, 表示介质类型; 第1簇, reserved; 第2簇, 存储根目录;
 * history     :
 ***************************************************************/
LOCALC os_u32 alloc_fat(struct fat32_cache *fat_cache, os_u32 file_size)
{
    os_u32 i;
    os_u32 block_size;
    os_u32 clus_cnt;
    os_u32 first_clus;
    os_u32 last_clus;

    /* caculate the clus count */
    block_size = fat_cache->bpb.BPB_BytsPerSec * fat_cache->bpb.BPB_SecPerClus;
    cassert(0 != block_size);
    clus_cnt = divl_cell(file_size, block_size);

    /* find free items from fat */
    last_clus = first_clus = 0;
    spin_lock(&fat_cache->fat_lock);
    for (i = ROOT_DIR_CLUS_NO + 1; i < fat_cache->fat_max_clus_no; i++) {
        if (0 == clus_cnt) {
            break;
        }
        if (0 == (END_OF_CLUS & fat_cache->fat_table[i])) { /* fat is free */
            clus_cnt--;

            if (0 != last_clus) {
                fat_cache->fat_table[last_clus] += i;
            } else { /* first one */
                first_clus = i;
            }
            last_clus = i;
        }
    }
    if (0 != last_clus) { /* last one */
        fat_cache->fat_table[last_clus] += END_OF_CLUS;
    }
    spin_unlock(&fat_cache->fat_lock);
    return first_clus;
}

/***************************************************************
 * description : 获取目录表占用的簇数
 * history     :
 ***************************************************************/
LOCALC os_u32 get_file_clus_cnt(os_u32 clus_no, struct fat32_cache *fat_cache)
{
    os_u32 num = 0;

    if (ROOT_DIR_CLUS_NO <= clus_no) {
        do {
            /* 是否越界 */
            if (fat_cache->fat_max_clus_no <= clus_no) {
                if (END_OF_CLUS != clus_no) {
                    fat_dbg("fat32 overrun max clus %x %x %d\n", clus_no, fat_cache->fat_max_clus_no, __LINE__);
                    dump_stack(print);
                    return num;
                }
            }
            num++;

            /* 获取文件的下一簇 */
            clus_no = get_next_clus(clus_no, fat_cache);
        } while (END_OF_CLUS != clus_no);
    }

    return num;
}

/***************************************************************
 * description :
 ***************************************************************/
struct file_name_info {
    const os_u8 *name;
    os_u32 name_len;
    const os_u8 *ext;
    os_u32 ext_len;
};

/***************************************************************
 * description : 解析文件名
 * history     :
 ***************************************************************/
LOCALC os_ret parse_file_name(IN os_u8 *file_name, struct file_name_info *result)
{
    os_u32 i;
    os_u32 len;

    len = str_len(file_name);
    if (0 != len) {
        for (i = len - 1; 0 != i; i--) {
            if ('.' == file_name[i]) {
                result->ext = &file_name[i + 1];
                result->ext_len = len - (i + 1);
                result->name = file_name;
                result->name_len = i;
                return OS_SUCC;
            }
        }
    }
    return OS_FAIL;
}

/* Date Format. A FAT directory entry date stamp is a 16-bit field that is basically a date relative to the MS-DOS epoch of 01/01/1980. Here is the format (bit 0 is the LSB of the 16-bit word, bit 15 is the MSB of the 16-bit word):
   Bits 0-4: Day of month, valid value range 1-31 inclusive.
   Bits 5-8: Month of year, 1 = January, valid value range 1-12 inclusive.
   Bits 9-15: Count of years from 1980, valid value range 0-127 inclusive (1980-2107). */
#define file_year(DIR_WrtDate) (1980 + (((DIR_WrtDate) & 0x0fe00) >> 9))
#define file_month(DIR_WrtDate) (((DIR_WrtDate) & 0x1e0) >> 5)
#define file_date(DIR_WrtDate) ((DIR_WrtDate) & 0x1f)
LOCALC inline os_ret set_file_date(os_u16 *DIR_WrtDate, os_u16 y, os_u16 m, os_u16 d)
{
    y -= 1980;
    if (y < 128) {
        if ((1 <= m) && (12 >= m)) {
            if ((1 <= d) && (31 >= d)) {
                *DIR_WrtDate = (y << 9) | ((m << 5) & ((1 << 9) - 1)) | (d & ((1 << 5) - 1));
                return OS_SUCC;
            } else {
                fat_dbg("invalid day: %d\n", d);
            }
        } else {
            fat_dbg("invalid month: %d\n", m);
        }
    } else {
        fat_dbg("invalid year: %d\n", y);
    }
    return OS_FAIL;
}

/* Time Format. A FAT directory entry time stamp is a 16-bit field that has a granularity of 2 seconds. Here is the format (bit 0 is the LSB of the 16-bit word, bit 15 is the MSB of the 16-bit word).
   Bits 0-4: 2-second count, valid value range 0-29 inclusive (0 - 58 seconds).
   Bits 5-10: Minutes, valid value range 0-59 inclusive.
   Bits 11-15: Hours, valid value range 0-23 inclusive. */
#define file_hour(DIR_WrtTime) (((DIR_WrtTime) & 0x0f800) >> 11)
#define file_minute(DIR_WrtTime) (((DIR_WrtTime) & 0x7e0) >> 5)
#define file_second(DIR_WrtTime) (((DIR_WrtTime) & 0x1f) * 2)
LOCALC inline os_ret set_file_time(os_u16 *DIR_WrtTime, os_u16 h, os_u16 m, os_u16 s)
{
    s /= 2;
    if (h < 24) {
        if (m < 60) {
            if (s < 30) {
                *DIR_WrtTime = (h << 11) | ((m << 5) & ((1 << 11) - 1)) | (s & ((1 << 5) - 1));
                return OS_SUCC;
            } else {
                fat_dbg("invalid second: %d\n", s);
            }
        } else {
            fat_dbg("invalid minute: %d\n", m);
        }
    } else {
        fat_dbg("invalid hour: %d\n", h);
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void format_fat32_file_name(os_u8 *name)
{
    os_u32 i;

    for (i = 0; i < FAT_DIR_NAME_SIZE; i++) {
        if (('a' <= name[i]) && ('z' >= name[i])) {
            name[i] -= ('a' - 'A');
        }
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_bool enough_free_item(struct fat32_dir_item *item, os_u32 count)
{
    os_u32 i;

    for (i = 0; i < count; i++) {
        if ((!dir_is_del(&item[i])) && (!dir_is_end(&item[i]))) {
            return OS_FALSE;
        }
    }
    return OS_TRUE;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_u32 set_ldir_name(os_u16 *LDIR_Name, os_u32 dst_len, IN os_u8 *file_name, os_u32 index, os_u32 total_len)
{
    os_u32 i;
    os_u32 len;

    if (total_len >= index + dst_len) {
        len = dst_len;
    } else {
        len = total_len - index;
    }
    for (i = 0; i < len; i++) {
        LDIR_Name[i] = file_name[index + i];
    }
    for (i = len; i < dst_len; i++) {
        LDIR_Name[i] = LONG_NAME_PAD;
    }
    return index + len;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret del_dir_item(struct fat32_dir_item *item)
{
    if (dir_is_long_name_attr(item)) {
        do {
            del_dir(item);
            item--;
        } while ((!long_dir_is_end(item)) && (!dir_is_end(item)) && (!dir_is_del(item)));
    }
    if ((!dir_is_end(item)) && (!dir_is_del(item))) {
        del_dir(item);
    } else {
        fat_dbg("error long or short dir\n");
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret fat32_destory_file(os_u32 device_id, IN HDIR dir, IN os_u8 *file_name)
{
    struct disk_device *disk;
    struct fat32_cache *fat_cache;
    struct fat32_dir_item *item;

    cassert(OS_NULL != file_name);

    disk = get_disk_device(device_id);
    if (OS_NULL == disk) {
        fat_dbg("get disk device fail\n");
        return OS_FAIL;
    }

    /* lookup from current directory */
    fat_cache = disk->vfs_cache;
    item = get_file_dir_item(file_name, fat_cache->curr_dir);
    if (OS_NULL != item) {
        /* free fat */
        free_fat(fat_cache, get_clus_no(item));
        /* free dir item */
        del_dir_item(item);
        /* write fat and dir */
        write_disk(device_id, fat_cache->fat_sector, fat_cache->bpb.BPB_FATSz32, (OUT os_u8 *) fat_cache->fat_table);
        write_file_by_clus(fat_cache->curr_dir_clus_no, fat_cache->curr_dir, device_id, fat_cache);
    } else {
        fat_dbg("can not find file\n");
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description : new file
 * history     :
 ***************************************************************/
LOCALC os_ret fat32_create_file(os_u32 device_id, IN HDIR dir, IN os_u8 *file_name)
{
    struct file_name_info info;
    struct fat32_dir_item *dir_item;
    struct disk_device *disk;
    struct fat32_cache *fat_cache;
    struct fat32_dir_item *free_item;
    os_u32 item_cnt;
    os_u32 name_len;
    os_u32 count; /* item count */
    os_u32 size;
    os_u8 *buffer;
    os_u32 i;

    cassert(OS_NULL != file_name);

    name_len = str_m_len(file_name, FAT32_MAX_FILE_NAME_LEN + 1);
    if (name_len > FAT32_MAX_FILE_NAME_LEN) {
        fat_dbg("create file, name is too long\n");
        return OS_FAIL;
    }
    if (OS_SUCC != parse_file_name(file_name, &info)) {
        fat_dbg("parse file name fail\n");
        return OS_FAIL;
    }
    if ((info.ext_len > 3) || (info.name_len > 8)) { /* long file name */
        count = divl_cell(name_len, LDIR_NAME3_LEN + LDIR_NAME2_LEN + LDIR_NAME1_LEN);
    } else {
        count = 0;
    }

    disk = get_disk_device(device_id);
    if (OS_NULL == disk) {
        fat_dbg("get disk device fail\n");
        return OS_FAIL;
    }

    /* lookup from current directory */
    fat_cache = disk->vfs_cache;

    free_item = OS_NULL;
    for (buffer = OS_NULL, buffer = loop_klm(fat_cache->curr_dir, buffer, &size);
        OS_NULL != buffer;
        buffer = loop_klm(fat_cache->curr_dir, buffer, &size)) {
        item_cnt = size / sizeof(struct fat32_dir_item);
        dir_item = (struct fat32_dir_item *) buffer;
        for (i = 0; i < item_cnt; i++) {
            if ((dir_is_end(&dir_item[i])) || (dir_is_del(&dir_item[i]))) {
                if (((i + count) < item_cnt) && (enough_free_item(&dir_item[i], count + 1))) {
                    /* long name and short name */
                    free_item = &dir_item[i + count];
                    goto lookup_end;
                }
                if (dir_is_end(&dir_item[i])) {
                    /* fill with delete */
                    del_dir(&dir_item[i]);
                    fat_dbg("count: %d, i: %d\n", count, i);
                    /* goto lookup_end; */
                }
            } else {
                /* do not create same file */
                if ((!dir_is_long_name_attr(&dir_item[i])) && (OS_TRUE == cmp_file_name(&dir_item[i], file_name))) {
                    fat_dbg("same file has exist\n");
                    return OS_FAIL;
                }
            }
        }
    }

  lookup_end:
    if (OS_NULL == free_item) {
        os_u32 file_clus_no;
        os_u32 new_dir_len;
        os_ret ret;

        /* do not split file name items between two dir, just discard the last dir. */
        fat_dbg("no free dir item\n");

        new_dir_len = fat_cache->bpb.BPB_BytsPerSec * fat_cache->bpb.BPB_SecPerClus;

        if (name_len > ((LDIR_NAME3_LEN + LDIR_NAME2_LEN + LDIR_NAME1_LEN) * (new_dir_len / sizeof(struct fat32_dir_item)))) {
            fat_dbg("file name is too long\n");
            return OS_FAIL;
        }

        /* clear the dir clus */
        ret = add_klm(fat_cache->curr_dir, new_dir_len, __LINE__);
        if (OS_SUCC != ret) {
            fat_dbg("alloc klm mem fail %d\n", __LINE__);
            return OS_FAIL;
        }
        buffer = klm_addr(fat_cache->curr_dir, klm_size(fat_cache->curr_dir) - new_dir_len, new_dir_len);
        cassert(OS_NULL != buffer);
        mem_set(buffer, 0, new_dir_len);

        /* alloc new dir */
        fat_dbg("clus no: %d %x %x", fat_cache->curr_dir_clus_no, new_dir_len, klm_size(fat_cache->curr_dir));
        free_fat(fat_cache, fat_cache->curr_dir_clus_no);
        file_clus_no = alloc_fat(fat_cache, klm_size(fat_cache->curr_dir));
        if (0 == file_clus_no) {
            destroy_klm(fat_cache->curr_dir);
            fat_dbg("alloc fat fail\n");
            return OS_FAIL;
        }
        fat_dbg("clus no: %d", file_clus_no);
        fat_cache->curr_dir_clus_no = file_clus_no;
        /* write fat */
        write_disk(device_id, fat_cache->fat_sector, fat_cache->bpb.BPB_FATSz32, (OUT os_u8 *) fat_cache->fat_table);

        /* find the last address. */
        dir_item = (struct fat32_dir_item *) buffer;
        free_item = &dir_item[count];
    }

    /* generate short name */
    do {
        mem_set(free_item->DIR_Name, FAT_INVALID_NAME_CHAR, FAT_DIR_NAME_SIZE);
        mem_cpy(free_item->DIR_Name, info.name, min(info.name_len, 8));
        mem_cpy(&free_item->DIR_Name[8], info.ext, min(info.ext_len, 3));
        /* translate into capital */
        format_fat32_file_name(free_item->DIR_Name);
        free_item->DIR_Attr = ATTR_ARCHIVE; /* ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID */
        /* Reserved for use by Windows NT. Set value to 0 when a file is created and never modify or look at it after that. */
        free_item->DIR_NTRes = 0;
        /* Many FAT file systems do not support Date/Time other than DIR_WrtTime and DIR_WrtDate.
           For this reason, DIR_CrtTimeMil, DIR_CrtTime, DIR_CrtDate, and DIR_LstAccDate are actually optional fields.
           DIR_WrtTime and DIR_WrtDate must be supported, however.
           If the other date and time fields are not supported, they should be set to 0 on file create and ignored on other file operations. */
        free_item->DIR_CrtTimeTeenth = 0;
        free_item->DIR_CrtTime = 0;
        free_item->DIR_CrtDate = 0;
        free_item->DIR_LastAccDate = 0;
        set_file_time(&free_item->DIR_WrtTime, 0, 0, 0);
        set_file_date(&free_item->DIR_WrtDate, 2013, 05, 29);
        /* not alloc fat here */
        set_clus_no(free_item, 0);
        free_item->DIR_FileSize = 0;
    } while (0);

    /* long name */
    if (0 != count) {
        struct fat32_ldir_item *litem;
        os_u32 index;
        os_u8 chksum;

        index = 0;
        name_len++; /* Names are also NUL terminated */
        chksum = ChkSum(free_item->DIR_Name);
        for (i = 0, litem = (struct fat32_ldir_item *)(free_item - 1); i < count; i++, litem--) {
            litem->LDIR_Attr = ATTR_LONG_NAME;
            litem->LDIR_Chksum = chksum;
            /* Must be ZERO. This is an artifact of the FAT "first cluster" and must be zero for compatibility with existing disk utilities.  It's meaningless in the context of a long dir entry. */
            litem->LDIR_FstClusLO = 0;
            litem->LDIR_Ord = i + 1;
            /* If zero, indicates a directory entry that is a sub-component of a long name.  NOTE: Other values reserved for future extensions.
               Non-zero implies other dirent types. */
            litem->LDIR_Type = 0;
            index = set_ldir_name(litem->LDIR_Name1, LDIR_NAME1_LEN, file_name, index, name_len);
            index = set_ldir_name(litem->LDIR_Name2, LDIR_NAME2_LEN, file_name, index, name_len);
            index = set_ldir_name(litem->LDIR_Name3, LDIR_NAME3_LEN, file_name, index, name_len);
        }
        (litem + 1)->LDIR_Ord |= LAST_LONG_ENTRY;
    }

    /* write dir file to disk */
    write_file_by_clus(fat_cache->curr_dir_clus_no, fat_cache->curr_dir, device_id, fat_cache);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret fat32_rename_file(os_u32 device_id, IN HDIR dir, IN os_u8 *file_name, IN os_u8 *new_file_name)
{
    return OS_FAIL;
}

/***************************************************************
 * description : parse full file path, return file clus no.
 * history     :
 ***************************************************************/
LOCALC os_u32 parse_file_path(os_u8 *name, struct fat32_cache *fat_cache, os_u32 device_id)
{
    os_u8 *n, *m;
    os_u32 size;
    os_u8 *t;
    struct fat32_dir_item *item;
    klm_handle dir;
    os_u32 clus;
    os_ret ret;

    n = OS_NULL;
    dir = OS_NULL;
    clus = INVLAID_CLUS_NO;

    size = str_len(name) + 1;
    n = kmalloc(size);
    if (n) {
        mem_cpy(n, name, size);
        m = n;

        if ('\\' == m[0]) {
            clus = ROOT_DIR_CLUS_NO;
            m = m + 1;
        } else {
            clus = fat_cache->curr_dir_clus_no;
        }

        dir = OS_NULL;
        for (;;) {
            t = str_brk(&m, "\\");
            if (OS_NULL == t) {
                break;
            }
            ret = create_klm(&dir, __LINE__);
            if (OS_SUCC != ret) {
                fat_dbg("create klm fail\n");
                clus = INVLAID_CLUS_NO;
                goto end;
            }
            ret = add_klm(dir, get_file_clus_cnt(clus, fat_cache) * fat_cache->bpb.BPB_BytsPerSec * fat_cache->bpb.BPB_SecPerClus, __LINE__); /* 分配簇的整数倍 */
            if (OS_SUCC != ret) {
                fat_dbg("alloc klm mem fail %d\n", __LINE__);
                clus = INVLAID_CLUS_NO;
                goto end;
            }
            ret = read_file_by_clus(clus, dir, device_id, fat_cache);
            if (OS_SUCC != ret) {
                fat_dbg("read dir fail\n");
                clus = INVLAID_CLUS_NO;
                goto end;
            }
            item = get_file_dir_item(t, dir);
            if (item) {
                clus = get_clus_no(item);
            } else {
                clus = INVLAID_CLUS_NO;
                goto end;
            }

            destroy_klm(dir);
            dir = OS_NULL;
        }
    }
  end:
    if (dir) destroy_klm(dir);
    if (n) kfree(n);
    return clus;
}

/***************************************************************
 * description : 创建文件资源
 * history     :
 ***************************************************************/
LOCALC HFILE fat32_open_file(os_u32 device_id, IN HDIR dir, IN os_u8 *file_name)
{
    struct vfs_handle *handle;
    struct disk_device *disk;
    struct fat32_dir_item *item;
    struct fat32_cache *fat_cache;
    os_u32 clus;
    os_ret ret;

    cassert(OS_NULL != file_name);

    disk = get_disk_device(device_id);
    if (OS_NULL == disk) {
        fat_dbg("get disk device fail\n");
        return OS_NULL;
    }

    /* 分配文件句柄空间 */
    handle = kmalloc(sizeof(struct vfs_handle));
    if (OS_NULL == handle) {
        return OS_NULL;
    }
    handle->total_size = 0;
    ret = create_klm(&handle->data, __LINE__);
    if (OS_SUCC != ret) {
        fat_dbg("create klm fail\n");
        goto fail1;
    }
    handle->file_pointer = 0;
    handle->file_wflag = OS_FALSE;

    if (MAX_FILE_NAME_LEN < str_m_len(file_name, MAX_FILE_NAME_LEN + 1)) {
        goto fail1;
    }
    str_ncpy(handle->file_name, file_name, MAX_FILE_NAME_LEN);
    handle->file_name[MAX_FILE_NAME_LEN] = '\0';

    handle->device_id = device_id;

    /* lookup from current directory */
    fat_cache = disk->vfs_cache;
    item = get_file_dir_item(file_name, fat_cache->curr_dir);
    if (OS_NULL != item) {
        ret = add_klm(handle->data,
                      get_file_clus_cnt(get_clus_no(item), fat_cache) * fat_cache->bpb.BPB_BytsPerSec * fat_cache->bpb.BPB_SecPerClus,
                      __LINE__); /* 分配簇的整数倍 */
        if (OS_SUCC != ret) {
            fat_dbg("alloc klm mem fail %d\n", __LINE__);
            goto fail1;
        }
        handle->total_size = item->DIR_FileSize;
        /* here, alloc enough buffer */
        ret = read_file_by_clus(get_clus_no(item), handle->data, handle->device_id, fat_cache);
        if (OS_SUCC != ret) {
            fat_dbg("read file by clus fail\n");
            goto fail2;
        }
        return handle;
    } else {
        fat_dbg("can not find file\n");
        goto fail1;
    }

  fail2:
    destroy_klm(handle->data);
  fail1:
    kfree(handle);
    return OS_NULL;
}

#define FAT_TRY_TIMES 4

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret fat32_close_file(HFILE file_handle)
{
    struct vfs_handle *handle;
    struct fat32_cache *fat_cache;
    struct disk_device *disk;
    struct fat32_dir_item *item;
    os_u32 file_clus_no; /* start clus no */
    os_u32 i;
    os_ret ret;

    cassert(OS_NULL != file_handle);

    handle = file_handle;
    if (OS_TRUE == handle->file_wflag) {
        rmb();

        disk = get_disk_device(handle->device_id);
        if ((OS_NULL == disk) || (OS_NULL == disk->vfs_cache)) {
            fat_dbg("close file, get disk device fail\n");
            return OS_FAIL;
        }
        fat_cache = disk->vfs_cache;

        item = get_file_dir_item(handle->file_name, fat_cache->curr_dir);
        if (OS_NULL != item) {
            /* lock allocate fat */
            free_fat(fat_cache, get_clus_no(item));
            file_clus_no = alloc_fat(fat_cache, handle->total_size);
            /* dir: 0x203f, fat: 0x1a33 */
            if (0 != file_clus_no) {
                for (i = 0; i < FAT_TRY_TIMES; i++) {
                    /* 1. write data */
                    ret = write_file_by_clus(file_clus_no, handle->data, handle->device_id, fat_cache);
                    if (OS_FAIL == ret) {
                        print("write data fail\n");
                        continue;
                    }
                    /* 2. modify dir */
                    set_clus_no(item, file_clus_no);
                    item->DIR_FileSize = handle->total_size;
                    ret = write_file_by_clus(fat_cache->curr_dir_clus_no, fat_cache->curr_dir, handle->device_id, fat_cache);
                    if (OS_FAIL == ret) {
                        print("write directory fail\n");
                        continue;
                    }
                    /* 3. lock write fat */
                    ret = write_disk(handle->device_id, fat_cache->fat_sector, fat_cache->bpb.BPB_FATSz32, (OUT os_u8 *) fat_cache->fat_table);
                    if (OS_SUCC == ret) {
                        break;
                    }
                    print("write fat fail(%s)\n", handle->file_name);
                    delay_ms(1000);
                }
                if (OS_SUCC != ret) {
                    print("write fat fail %x %x\n", fat_cache->fat_sector, fat_cache->bpb.BPB_FATSz32);
                }
            } else {
                print("close file, alloc fat fail\n");
            }
        } else {
            print("close file, get item fail\n");
        }
    }
    destroy_klm(handle->data);
    kfree(handle);
    return OS_SUCC;
}

/***************************************************************
 * description : 读内存文件
 * history     :
 ***************************************************************/
LOCALC os_ret fat32_read_file(HFILE file_handle, os_u8 *buffer, os_u32 len)
{
    struct vfs_handle *handle;
    os_ret ret;

    cassert((OS_NULL != file_handle) && (OS_NULL != buffer) && (0 != len));

    handle = file_handle;

    if ((!add_u32_overflow(len, handle->file_pointer))
     && (handle->total_size >= (len + handle->file_pointer))) {
        ret = read_klm(handle->data, handle->file_pointer, buffer, len);
        if (OS_SUCC == ret) {
            handle->file_pointer += len;
            return OS_SUCC;
        }
    }
    fat_dbg("fat32 read file fail %d \%dn", len, handle->total_size);
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret fat32_write_file(HFILE file_handle, os_u8 *buffer, os_u32 len)
{
    struct vfs_handle *handle;

    cassert(OS_NULL != file_handle);

    handle = file_handle;

    if (!add_u32_overflow(len, handle->file_pointer)) {
        if (klm_size(handle->data) < (len + handle->file_pointer)) {
            struct disk_device *disk;
            struct fat32_cache *fat_cache;
            os_u32 block_size;
            os_u32 block_cnt;

            disk = get_disk_device(handle->device_id);
            if (OS_NULL == disk) {
                fat_dbg("write fat32, get disk device fail\n");
                return OS_FAIL;
            }

            /* lookup from current directory */
            fat_cache = disk->vfs_cache;

            /* caculate the number of block */
            do {
                os_u32 temp;
                block_size = fat_cache->bpb.BPB_BytsPerSec * fat_cache->bpb.BPB_SecPerClus;
                cassert(0 != block_size);
                temp = (len + handle->file_pointer) - klm_size(handle->data);
                block_cnt = divl_cell(temp, block_size);
            } while (0);
            add_klm(handle->data, block_cnt * block_size, __LINE__);
        }
        write_klm(handle->data, handle->file_pointer, buffer, len);
        if (handle->total_size < (len + handle->file_pointer)) {
            handle->total_size = len + handle->file_pointer;
        }
        handle->file_pointer += len;
        wmb();
        handle->file_wflag = OS_TRUE;
        return OS_SUCC;
    }
    return OS_FAIL;
}

/***************************************************************
 * description : list current directory
 * history     :
 ***************************************************************/
LOCALC os_ret fat32_list_file(os_u32 device_id, IN HDIR dir)
{
    struct disk_device *disk;
    struct fat32_cache *fat_cache;
    struct fat32_dir_item *dir_item;
    os_u16 *file_name;
    os_u8 *buffer;
    os_u32 size;
    os_u32 dir_cnt;
    os_u32 i;
    os_ret ret;

    disk = get_disk_device(device_id);
    if ((OS_NULL == disk) || (OS_NULL == disk->vfs_cache)) {
        return OS_FAIL;
    }

    fat_cache = disk->vfs_cache;

    file_name = kmalloc(sizeof(os_u16) * (1 + FAT32_MAX_FILE_NAME_LEN));
    if (OS_NULL == file_name) {
        fat_dbg("list fat32, alloc mem fail\n");
        return OS_FAIL;
    }

    for (buffer = OS_NULL, buffer = loop_klm(fat_cache->curr_dir, buffer, &size);
        OS_NULL != buffer;
        buffer = loop_klm(fat_cache->curr_dir, buffer, &size)) {
        dir_cnt = size / sizeof(struct fat32_dir_item);
        dir_item = (struct fat32_dir_item *) buffer;
        for (i = 0; i < dir_cnt; i++, dir_item++) {
            if (dir_is_end(dir_item)) {
                /* 目录表结束 */
                break;
            }

            if (dir_is_del(dir_item)) {
                /* 如果文件被删除 */
                continue;
            }

            /* 处理长文件名和短文件名 */
            if (!dir_is_long_name_attr(dir_item)) {
                /* 打印文件创建日期, 起始时间1980年 */
                fat_dbg("%d-%d-%d ", file_year(dir_item->DIR_WrtDate),
                                     file_month(dir_item->DIR_WrtDate),
                                     file_date(dir_item->DIR_WrtDate));
                /* 打印文件创建时间 */
                fat_dbg("%d:%d:%d ", file_hour(dir_item->DIR_WrtTime),
                                     file_minute(dir_item->DIR_WrtTime),
                                     file_second(dir_item->DIR_WrtTime));
                if (ATTR_DIRECTORY == dir_item->DIR_Attr) {
                    fat_dbg("<dir> ");
                } else {
                    /* 打印文件大小 */
                    fat_dbg("%d ", dir_item->DIR_FileSize);
                }
                /* 打印文件名 */
                ret = fat32_file_name(dir_item, file_name);
                if (OS_SUCC == ret) {
                    os_u32 j;
                    for (j = 0; j < FAT32_MAX_FILE_NAME_LEN; j++) {
                        if ('\0' == file_name[j]) {
                            break;
                        }
                        fat_dbg("%c", file_name[j]);
                    }
                } fat_dbg("\n");
            }
        }
    }
    kfree(file_name);
    return OS_SUCC;
}

/***************************************************************
 * description : 文件定位
 * history     :
 ***************************************************************/
LOCALC os_ret fat32_seek_file(HFILE file_handle, os_s32 offset, enum file_seek pos)
{
    struct vfs_handle *handle;

    cassert((OS_NULL != file_handle) && (SEEK_POS_BUT > pos));

    handle = file_handle;

    switch (pos) {
    /* 文件开始位置 */
    case SEEK_POS_SET:
        if ((handle->total_size <= offset) || (0 > offset)) {
            return OS_FAIL;
        }
        handle->file_pointer = offset;
        break;

    /* 文件当前位置 */
    case SEEK_POS_CUR:
        if ((handle->total_size <= (offset + handle->file_pointer)) || (0 > (offset + handle->file_pointer))) {
            return OS_FAIL;
        }
        handle->file_pointer += offset;
        break;

    /* 文件结尾位置 */
    case SEEK_POS_END:
        if (0 < offset) {
            return OS_FAIL;
        }
        handle->file_pointer = handle->total_size + offset;
        break;

    default:
        return OS_FAIL;
        break;
    }

    return OS_SUCC;
}

/***************************************************************
 * description : eof
 * history     :
 ***************************************************************/
LOCALC os_bool fat32_eof(HFILE file_handle)
{
    struct vfs_handle *handle;

    cassert(OS_NULL != file_handle);

    handle = file_handle;
    if (handle->file_pointer >= handle->total_size) {
        return OS_FALSE;
    } else {
        return OS_TRUE;
    }
}

/***************************************************************
 * description : size of file
 * history     :
 ***************************************************************/
LOCALC os_u32 fat32_size(HFILE file_handle)
{
    cassert(OS_NULL != file_handle);
    return ((struct vfs_handle *) file_handle)->total_size;
}

/***************************************************************
 * description : create directory of fat32
 * history     :
 ***************************************************************/
LOCALC os_ret fat32_create_directory(os_u32 device_id, IN HDIR dir, IN os_u8 *dir_name)
{
}

/***************************************************************
 * description : change working directory
 * history     :
 ***************************************************************/
LOCALC os_ret fat32_destroy_directory(os_u32 device_id, IN HDIR dir, IN os_u8 *dir_name)
{
}

/***************************************************************
 * description : change working directory
 * history     :
 ***************************************************************/
LOCALC os_ret fat32_change_directory(os_u32 device_id, IN os_u8 *full_path)
{
    os_u8 *dir_name;
    struct disk_device *disk;
    struct fat32_cache *fat_cache;
    struct fat32_dir_item *dir_item;
    os_u8 *buffer;
    os_u32 size;
    os_u32 i;
    os_u32 dir_cnt;
    os_u32 clus_no;
    os_ret ret;

    cassert(OS_NULL != dir_name);

    disk = get_disk_device(device_id);
    if ((OS_NULL == disk) || (OS_NULL == disk->vfs_cache)) {
        return OS_FAIL;
    }
    fat_cache = disk->vfs_cache;

    clus_no = parse_file_path((os_u8 *) full_path, fat_cache, device_id);
    if (INVLAID_CLUS_NO == clus_no) {
        return OS_FAIL;
    }

    destroy_klm(fat_cache->curr_dir);
    create_klm(&fat_cache->curr_dir, __LINE__);
    /* caculate current directory */
    ret = add_klm(fat_cache->curr_dir, get_file_clus_cnt(clus_no, fat_cache) * fat_cache->bpb.BPB_BytsPerSec * fat_cache->bpb.BPB_SecPerClus, __LINE__); /* 分配簇的整数倍 */
    if (OS_SUCC != ret) {
        fat_dbg("alloc klm mem fail %d\n", __LINE__);
        return OS_FAIL;
    }
    /* read directory cluster */
    ret = read_file_by_clus(clus_no, fat_cache->curr_dir, device_id, fat_cache);
    if (OS_SUCC != ret) {
        fat_dbg("cd read dir fail\n");
        destroy_klm(fat_cache->curr_dir);
        return OS_FAIL;
    }
    /* update current directory */
    fat_cache->curr_dir_clus_no = clus_no;
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret unmount_fat32(os_u32 device_id)
{
    struct disk_device *disk;
    struct fat32_cache *fat_cache;

    fat_dbg("unmount fat32\n");

    disk = get_disk_device(device_id);
    if (OS_NULL == disk) {
        fat_dbg("%d not fat32 device\n", device_id);
        return OS_FAIL;
    }

    fat_cache = disk->vfs_cache;
    if (OS_NULL != fat_cache) {
        kfree(fat_cache->fat_table);
        destroy_klm(fat_cache->curr_dir);
        kfree(fat_cache);
    }

    return OS_SUCC;
}

LOCALC os_ret check_fat32(os_u32 device_id);

LOCALD const struct vfs_operations fat32_operations = {
    check_fat32,
    unmount_fat32,
    fat32_change_directory,
    fat32_create_directory,
    fat32_destroy_directory,
    OS_NULL,
    OS_NULL,
    fat32_list_file,
    fat32_create_file,
    fat32_destory_file,
    fat32_rename_file,
    fat32_open_file,
    fat32_seek_file,
    fat32_read_file,
    fat32_write_file,
    fat32_eof,
    fat32_size,
    fat32_close_file
};

LOCALD const struct vfs fat32_info = {
    "fat32",
    FS_FAT32,
    &fat32_operations
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret check_fat32(os_u32 device_id)
{
    struct disk_device *disk;
    struct disk_para para;
    os_u8 *buffer;
    os_u8 *dpt;
    os_u8 bootflag;
    os_u32 dbrstartSector;
    struct fat32_bpb *bpb;
    os_u32 RootDirSector;
    os_u32 FirstDataSector;
    os_u32 fat_sector;
    struct fat32_cache *fat_cache;
    os_ret ret;

    disk = get_disk_device(device_id);
    if (OS_NULL == disk) {
        fat_dbg("check fat32, disk no exist\n");
        return OS_FAIL;
    }

    /* 查询磁盘参数 */
    ret = query_disk(device_id, &para);
    if (OS_SUCC != ret) {
        fat_dbg("check fat32, disk para no exist\n");
        return OS_FAIL;
    }
    buffer = kmalloc(para.bytes_per_sec);
    if (OS_NULL == buffer) {
        fat_dbg("malloc fail %d\n", __LINE__);
        return OS_FAIL;
    }
    /* 读取mbr */
    ret = read_disk(device_id, 0, 1, buffer);
    if ((OS_SUCC != ret) || (0x55 != buffer[510]) || (0xaa != buffer[511])) {
        goto error_1;
    }
    /* 第一个磁盘分区表only */
    dpt = buffer + 0x1be;
    /* 分区表类型 */
    if ((0xc != dpt[4]) && (0xb != dpt[4])) {
        /* 非fat32, 不支持 */
        goto error_1;
    }
    bootflag = dpt[0];

    /* dbr的相对相对扇区数 */
    dbrstartSector = *(os_u32 *)(&dpt[8]);
    ret = read_disk(device_id, dbrstartSector, 1, buffer);
    if ((OS_SUCC != ret) || (0x55 != buffer[510]) || (0xaa != buffer[511])) {
        fat_dbg("read dbr fail\n");
        goto error_1;
    }

    bpb = (struct fat32_bpb *) buffer;
    /* BPB_RsvdSecCnt:
       Number of reserved sectors in the reserved region of the volume
       starting at the first sector of the volume. this field must not be 0.
       for FAT12 and FAT16 volumes, this value should never be
       anything other than 1. for FAT32 volumes, this value is typically
       32. there is a lot of FAT code in the world "hard wired" to 1
       reserved sector for FAT12 and FAT16 volumes and that doesn't
       bother to check this field to make sure it is 1. microsoft operating
       systems will proberly support any non-zero value in this field.
       BPB_RootEntCnt:
       for FAT12 and FAT16 volumes, this field contains the count of 32-
       byte directory entries in the root directory. for FAT32 volumes,
       this field must be set to 0. for FAT12 and FAT16 volumes, this
       value should always specify a count that when multiplied by 32
       results in an even multiple of BPB_BytsPerSec. for maximum
       compatibility, FAT16 volumes should use the value 512. */
    if ((1 == bpb->BPB_RsvdSecCnt) || (0 != bpb->BPB_RootEntCnt)) {
        fat_dbg("this file system is not fat32\n");
        goto error_1;
    }

    /* 实际每扇区扇区字节数能力与bpb中记录是否一致 */
    if (bpb->BPB_BytsPerSec != para.bytes_per_sec) {
        fat_dbg("byte per sector is not match\n");
        goto error_1;
    }

    if (0x80 == bootflag) {
        /* set boot disk as current */
        set_disk_device_id(device_id);
    }

    /* 校验结束 */
    fat_dbg("device:%d fat32 check ok\n", device_id);

    /* 初始化fat32信息 */
    // RootDirSector = ((BPB_RootEntCnt * 32) + (BPB_BytePerSec - 1)) / BPB_BytePerSec;
    // FirstDataSector = BPB_ResvedSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors;
    RootDirSector = ((bpb->BPB_RootEntCnt * 32) + (bpb->BPB_BytsPerSec - 1)) / bpb->BPB_BytsPerSec; // RootDirSector = 0;
    /* 此处计算要加上dbr的起始扇区 */
    FirstDataSector = dbrstartSector + RootDirSector + bpb->BPB_RsvdSecCnt + (bpb->BPB_NumFATs * bpb->BPB_FATSz32);
    fat_sector = dbrstartSector + bpb->BPB_RsvdSecCnt;

    fat_cache = kmalloc(sizeof(struct fat32_cache));
    if (OS_NULL == fat_cache) {
        fat_dbg("alloc fat cache fail %d\n", __LINE__);
        goto error_2;
    }
    fat_cache->dbrstartSector = dbrstartSector;
    fat_cache->RootDirSector = RootDirSector;
    fat_cache->fat_sector = fat_sector;
    fat_cache->dir_sector = FirstDataSector;
    mem_cpy(&fat_cache->bpb, bpb, sizeof(struct fat32_bpb));

    /* 读取整个fat表 */
    fat_cache->fat_len = para.bytes_per_sec * fat_cache->bpb.BPB_FATSz32;
    fat_cache->fat_max_clus_no = 1 + fat_cache->fat_len / sizeof(os_u32);
    fat_cache->fat_table = kmalloc(fat_cache->fat_len);
    if (OS_NULL == fat_cache->fat_table) {
        fat_dbg("alloc fat cluster(len:%x) fail %d\n", fat_cache->fat_len, __LINE__);
        goto error_2;
    }
    init_spinlock(&fat_cache->fat_lock);
    ret = read_disk(device_id, fat_sector, fat_cache->bpb.BPB_FATSz32, (os_u8 *) fat_cache->fat_table);
    if (OS_SUCC != ret) {
        fat_dbg("read fat fail\n");
        goto error_3;
    }
    fat_dbg("fat addr: %x %x\n", fat_sector, fat_cache->fat_table);

    /* default root directory */
    fat_cache->curr_dir_clus_no = ROOT_DIR_CLUS_NO;
    ret = create_klm(&fat_cache->curr_dir, __LINE__);
    if (OS_SUCC != ret) {
        fat_dbg("create klm fail\n");
        goto error_3;
    }
    ret = add_klm(fat_cache->curr_dir, get_file_clus_cnt(ROOT_DIR_CLUS_NO, fat_cache) * para.bytes_per_sec * fat_cache->bpb.BPB_SecPerClus, __LINE__); /* 分配簇的整数倍 */
    if (OS_SUCC != ret) {
        fat_dbg("alloc klm mem fail %d\n", __LINE__);
        goto error_4;
    }
    ret = read_file_by_clus(ROOT_DIR_CLUS_NO, fat_cache->curr_dir, device_id, fat_cache);
    if (OS_SUCC != ret) {
        fat_dbg("read dir fail\n");
        goto error_4;
    }
    fat_dbg("dir addr: %x %x %x\n", FirstDataSector, klm_addr(fat_cache->curr_dir, 0, 1), klm_size(fat_cache->curr_dir));
    print("fat addr %x, len %x\n", fat_cache->fat_sector, fat_cache->bpb.BPB_FATSz32);
    disk->vfs_cache = fat_cache;
    disk->vfs = &fat32_info;
    kfree(buffer);
    return OS_SUCC;
  error_4:
    destroy_klm(fat_cache->curr_dir);
  error_3:
    kfree(fat_cache->fat_table);
  error_2:
    kfree(fat_cache);
  error_1:
    kfree(buffer);
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void test_fat32(os_void)
{
    os_uint i;
    os_ret ret;
    struct disk_device *disk;
    struct fat32_cache *fat_cache;

    disk = get_disk_device(curr_disk_device_id());
    if (OS_NULL == disk) {
        fat_dbg("disk no exist\n");
        return;
    }
    fat_cache = disk->vfs_cache;
    for (i = 0; i < 0x10; i++) {
        ret = write_disk(curr_disk_device_id(), fat_cache->fat_sector, fat_cache->bpb.BPB_FATSz32, (OUT os_u8 *) fat_cache->fat_table);
        if (OS_SUCC == ret) {
            print("test write fat succ\n");
        } else {
            print("test write fat fail\n");
        }
    }
}
LOCALD os_u8 test_fat32_name[] = { "fat" };
LOCALD struct dump_info fat32_debug = {
    test_fat32_name,
    test_fat32
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_fat32(os_void)
{
    os_ret ret;

    /* 注册到vfs */
    ret = register_fs(&fat32_info);
    cassert(OS_SUCC == ret);

    ret = register_dump(&fat32_debug);
    cassert(OS_SUCC == ret);
}
device_init_func(init_fat32);

