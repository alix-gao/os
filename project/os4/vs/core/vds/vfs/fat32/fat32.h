/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : fat32.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __FAT32_H__
#define __FAT32_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(1)

/***************************************************************
 macro define
 ***************************************************************/

#define fat_dbg(fmt, arg...) do { if (1) { print(fmt, ##arg); } else { flog(fmt, ##arg); } } while (0)

#define END_OF_CLUS UINT32_C(0x0fffffff)

/* 根目录的起始簇号 */
#define ROOT_DIR_CLUS_NO UINT32_C(2)

#define INVLAID_CLUS_NO UINT32_C(1)

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/

/***************************************************************
 * description : 一字节对齐
 ***************************************************************/
struct fat32_bpb {
    os_u8 BS_jmpBoot[3];
    os_u8 BS_OEMName[8];
    os_u16 BPB_BytsPerSec;
    os_u8 BPB_SecPerClus;
    os_u16 BPB_RsvdSecCnt;
    os_u8 BPB_NumFATs;
    os_u16 BPB_RootEntCnt;
    os_u16 BPB_TotSec16;
    os_u8 BPB_Media;
    os_u16 BPB_FATSz16;
    os_u16 BPB_SecPerTrk;
    os_u16 BPB_NumHeads;
    os_u32 BPB_HiddSec;
    os_u32 BPB_TotSec32;

    /* only for fat32 */
    os_u32 BPB_FATSz32; // this field is the fat32 32-bit count of sectors occupied by one fat.
    os_u16 BPB_ExtFlags;
    os_u16 BPB_FSVer;
    os_u32 BPB_RootClus;
    os_u16 BPB_FSInfo;
    os_u16 BPB_BkBootSec;
    os_u8 BPB_Reserved[12];
    os_u8 BPB_DrvNum;
    os_u8 BPB_Reserved1[1];
    os_u8 BPB_BootSig;
    os_u32 BPB_VolId;
    os_u8 BPB_VolLab[11];
    os_u8 BPB_FilSysType[8];
};

#define FAT_DIR_NAME_SIZE 11

/* DIR_Attr */
#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10 /* 文件夹 */
#define ATTR_ARCHIVE 0x20 /* 存档属性 */
#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

#define FAT_INVALID_NAME_CHAR 0x20

/***************************************************************
 * description :
 ***************************************************************/
struct fat32_dir_item {
    os_u8 DIR_Name[FAT_DIR_NAME_SIZE];
    os_u8 DIR_Attr; /* same with (struct fat32_ldir_item) : LDIR_Attr */
    os_u8 DIR_NTRes;
    os_u8 DIR_CrtTimeTeenth;
    os_u16 DIR_CrtTime;
    os_u16 DIR_CrtDate;
    os_u16 DIR_LastAccDate;
    os_u16 DIR_FstClusHI;
    os_u16 DIR_WrtTime;
    os_u16 DIR_WrtDate;
    os_u16 DIR_FstClusLO;
    os_u32 DIR_FileSize;
};

#define LDIR_NAME1_LEN 5
#define LDIR_NAME2_LEN 6
#define LDIR_NAME3_LEN 2

#define LAST_LONG_ENTRY 0x40

#define FAT32_MAX_FILE_NAME_LEN (LAST_LONG_ENTRY * (LDIR_NAME3_LEN + LDIR_NAME2_LEN + LDIR_NAME1_LEN))

/***************************************************************
 * description :
 ***************************************************************/
struct fat32_ldir_item {
    os_u8 LDIR_Ord;
    os_u16 LDIR_Name1[LDIR_NAME1_LEN];
    os_u8 LDIR_Attr; /* same with (struct fat32_dir_item) : DIR_Attr */
    os_u8 LDIR_Type;
    os_u8 LDIR_Chksum; /* Checksum of name in the short dir entry at the end of the long dir set. */
    os_u16 LDIR_Name2[LDIR_NAME2_LEN];
    os_u16 LDIR_FstClusLO;
    os_u16 LDIR_Name3[LDIR_NAME3_LEN];
};

#define DIR_DEL 0xe5
#define del_dir(item) (((struct fat32_dir_item *) item)->DIR_Name[0] = DIR_DEL)
#define dir_is_del(item) (DIR_DEL == ((struct fat32_dir_item *) item)->DIR_Name[0])

#define dir_is_end(item) (0x00 == ((struct fat32_dir_item *) item)->DIR_Name[0])

#define dir_is_long_name_attr(item) (ATTR_LONG_NAME == ((struct fat32_ldir_item *) item)->LDIR_Attr)

/* If masked with 0x40 (LAST_LONG_ENTRY), this indicates the entry is the last long dir entry in a set of long dir entries. All valid sets of long dir entries must begin with an entry having this mask. */
#define long_dir_is_end(litem) (LAST_LONG_ENTRY & ((struct fat32_ldir_item *) litem)->LDIR_Ord)
/* Names are also NUL terminated and padded with 0xFFFF characters in order to detect corruption of long name fields by errant disk utilities. */
#define LONG_NAME_PAD 0xffff
#define long_name_is_terminated(c) (('\0' == (c)) || (LONG_NAME_PAD == (c)))

/***************************************************************
 * description :
 ***************************************************************/
struct fat32_cache {
    struct fat32_bpb bpb;
    os_u8 rsvd[2];

    os_u32 dbrstartSector; /* dbr的起始扇区 */
    os_u32 fat_sector; /* fat表的起始扇区 */
    os_u32 RootDirSector; /* 根目录的扇区数 */
    os_u32 dir_sector; /* 根目录的起始扇区(起始簇号ROOT_DIR_CLUS_NO) */

    os_u32 fat_len;
    os_u32 *fat_table; /* fat表cache */
    spinlock_t fat_lock;
    os_u32 fat_max_clus_no;

    os_u32 curr_dir_clus_no; /* 当前目录的起始簇号 */
    klm_handle curr_dir;
};

/***************************************************************
 extern function
 ***************************************************************/

#pragma pack()

#endif

