/****************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name  : fat32.h
 * version    : 1.0
 * description:
 * author     : gaocheng
 * date       : 2009-04-22
 ****************************************************************/

#ifndef __FAT32_H__
#define __FAT32_H__

/****************************************************************
 include head file
 ****************************************************************/

/****************************************************************
 macro define
 ****************************************************************/
#define FAT_ADDR 0x20000
#define FAT_LEN 0x10000

#define FAT_DIR_ADDR 0x30000
#define FAT_DIR_LEN 0x10000

#define BUFFER_ADDR 0x40000
/* Number of sectors per allocation unit.
   This value must be a power of 2 that is greater than 0.
   The legal values are 1, 2, 4, 8, 16, 32, 64, and 128.
   Note however, that a value should never be used that results in a "bytes per cluster" value (BPB_BytsPerSec * BPB_SecPerClus) greater than 32K (32 * 1024).
   There is a misconception that values greater than this are OK.
   Values that cause a cluster size greater than 32K bytes do not work properly;
   do not try to define one.
   Some versions of some systems allow 64K bytes per cluster value.
   Many application setup programs will not work correctly on such a FAT volume. */
#define BUFFER_LEN 0x40000

#define DBR_LBA 0x3f

#define FAT_DIR_NAME_SIZE 11
#define DIR_DEL 0xe5
#define ATTR_DIRECTORY 0x10
#define END_OF_CLUS 0x0fffffff
#define ROOT_DIR_CLUS_NO 2

/****************************************************************
 enum define
 ****************************************************************/

/****************************************************************
 struct define
 ****************************************************************/
typedef struct {
    BYTE head;
    WORD track;
    WORD sector_no;
} chs;

/****************************************************************
 extern function
 ****************************************************************/
VOID init_fat32(VOID);
WORD fat32_load_file(BYTE *file_name, DWORD addr);
WORD fat32_change_directory(BYTE *dir_name);
VOID list_fat32(VOID);

#endif

