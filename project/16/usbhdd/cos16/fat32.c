
/* not support big-endian, long file name, asynchronism */

#include "typedef.h"
#include "fat32.h"
#include "lib.h"

VOID move_1m(DWORD src_addr, DWORD dst_addr, DWORD data_len);

BYTE DISK_DRIVER_TYPE;

static struct disk_para {
    WORD cylinder;
    BYTE head;
    BYTE sector;

    BYTE driver;
    BYTE rsvd[3];
} disk_para_table;

static struct bpb_para {
    /* 每簇扇区数 */
    BYTE sec_per_clus;
    /* 备份fat表数量 */
    BYTE num_fats;
    /* 保留扇区数 */
    WORD rsvd_sec_cnt;
    /* 隐藏扇区数 */
    BYTE hiddsec[4];
    /* fat表所占的扇区数 */
    DWORD fatsz32;
    /* 每扇区字节数 */
    WORD byts_per_sec;
} bpb_para_table;

typedef struct {
    BYTE DIR_Name[FAT_DIR_NAME_SIZE];
    BYTE DIR_Attr;
    BYTE DIR_NTRes;
    BYTE DIR_CrtTimeTeenth;
    WORD DIR_CrtTime;
    WORD DIR_CrtDate;
    WORD DIR_LastAccDate;
    WORD DIR_FstClusHI;
    WORD DIR_WrtTime;
    WORD DIR_WrtDate;
    WORD DIR_FstClusLO;
    DWORD DIR_FileSize;
} fat32_dir_item;

/* fat起始扇区数 */
static DWORD fat_start_sector = 0;

/* 目录表起始扇区号 */
static DWORD dir_start_sector = 0;

/* 文本模式下，每行最大字符数 */
#define SCREEN_COL 80

/* 打印位置记录 */
static WORD fat_col = 0;
static WORD fat_row = 11;

static VOID fat_print_dec(WORD num, BYTE flag)
{
    WORD i;
    WORD temp;
    BYTE fg = 1;

    /* 65536 */
    for (i=10000; i>1; i=i/10) {
        temp = num/i;
        if ((0 != temp)||(0 == fg)) {
            fg = 0;
            num -= temp*i;

            print_char('0'+temp, 0xf, fat_row, fat_col);
            fat_col++;
        }
    }

    print_char('0'+num, 0xf, fat_row, fat_col);
    fat_col++;

    /* 换行标志 */
    if (flag) {
        while (SCREEN_COL - 1 < fat_col) {
            fat_col -= SCREEN_COL;
            fat_row++;
        }

        fat_row++;
        fat_col = 0;
    }
}

static VOID fat_print_hex(BYTE num, BYTE flag)
{
    /* color is white */
    print_hex(num, 0xf, fat_row, fat_col);

    fat_col = fat_col + 2;

    /* 换行标志 */
    if (flag) {
        while (SCREEN_COL - 1 < fat_col) {
            fat_col -= SCREEN_COL;
            fat_row++;
        }

        fat_row++;
        fat_col = 0;
    }
}

static VOID fat_print_dword_hex(DWORD num, BYTE flag)
{
    /* color is white */
    print_32bit_hex(num, 0xf, fat_row, fat_col);

    fat_col = fat_col + 8;

    /* 换行标志 */
    if (flag) {
        while (SCREEN_COL - 1 < fat_col) {
            fat_col -= SCREEN_COL;
            fat_row++;
        }

        fat_row++;
        fat_col = 0;
    }
}

static VOID fat_print_char(BYTE c, BYTE flag)
{
    print_char(c, 0xf, fat_row, fat_col);

    fat_col++;

    /* 换行标志 */
    if (flag) {
        while (SCREEN_COL - 1 < fat_col) {
            fat_col -= SCREEN_COL;
            fat_row++;
        }

        fat_row++;
        fat_col = 0;
    }
}

static VOID fat_print_str(BYTE *str, BYTE flag)
{
    while (0 != *str) {
        print_char(*str, 0xf, fat_row, fat_col);
        str++;
        fat_col++;
    }

    /* 换行标志 */
    if (flag) {
        while (SCREEN_COL - 1 < fat_col) {
            fat_col -= SCREEN_COL;
            fat_row++;
        }

        fat_row++;
        fat_col = 0;
    }
}

#define tc_assert(condition) \
 do { \
    if (condition) { \
    } else { \
        fat_print_str("assert!!! ", 0); \
        fat_print_dec(__LINE__, 1); \
        abort(); \
    } \
 } while (0)

static struct global_ram_file {
    /* 磁盘参数表 */
    BYTE ns; /* 每磁道扇区数 */
    BYTE nh; /* 最大磁头数 */
    WORD nc; /* 最大磁道号 */

    BYTE bsp_flag; /* 未记录0, 已记录1 */
    BYTE rsvd;
    BYTE ss; /* 起始磁道号,起始为1 */
    BYTE sh; /* 起始磁头数,起始为0 */
    WORD sc; /* 起始磁道号,起始为0 */
} *grf;

#define GRF_ADDR 0x90000

static VOID get_disk_bsp(VOID)
{
    init_32p(grf, GRF_ADDR);
}

/* read disk by chs format, 20bit addr is plat address */
static VOID read_chs_disk(chs *para, WORD sector_num, DWORD addr_20bit)
{
    WORD int13_ax = 0;
    WORD int13_bx = 0;
    WORD int13_cx = 0;
    WORD int13_dx = 0;
    WORD int13_es = 0;

#define INT13_AX_FUNC 0x2

    int13_ax = (INT13_AX_FUNC<<8)|sector_num;
    int13_cx = ((para->track & 0xff)<<8) | (((para->track & 0x300)>>2) | (para->sector_no & 0x3f));
    int13_dx = (para->head<<8) | DISK_DRIVER_TYPE;
    int13_bx = addr_20bit & 0x0f;
    int13_es = ((WORD) addr_20bit)>>4;
    int13_es = int13_es|(((WORD)(addr_20bit>>16))<<12);

    do {
        asm push es
        asm push dx
        asm push cx
        asm push bx
        asm push ax

        asm push int13_ax
        asm push int13_bx
        asm push int13_cx
        asm push int13_dx
        asm push int13_es
        asm pop ax
        asm mov es,ax
        asm pop dx
        asm pop cx
        asm pop bx
        asm pop ax
        asm int 13H

        asm pop ax
        asm pop bx
        asm pop cx
        asm pop dx
        asm pop es
    } while (0);
}

/* 20bit addr is plat address */
static VOID read_lba_disk(DWORD lba, WORD sector_num, DWORD addr_20bit)
{
    chs para;
    DWORD quotient, remainder;
    WORD curr_sector_num;
    DWORD offset;
    WORD i;

    /* para.head = (lba / disk_para_table.sector) % disk_para_table.head;
       para.track = lba / disk_para_table.sector / disk_para_table.head;
       para.sector_no = lba % disk_para_table.sector; */

    /* 计算三维几何参数 */
    tc_32bit_div(&quotient, &remainder, lba, disk_para_table.sector);
    para.sector_no = remainder + grf->ss;
    tc_32bit_div(&quotient, &remainder, quotient, disk_para_table.head);
    para.head = remainder + grf->sh;
    para.track = quotient + grf->sc;
    if (0xff == para.head) { /* bugfix: grf->sh为0xff(-1), para.head为0xff. 此时磁道号被借位 */
        para.track--;
    }

    while (sector_num) {
        /* 本次读取的扇区数 */
        if (disk_para_table.sector < sector_num) {
            curr_sector_num = disk_para_table.sector - para.sector_no + grf->ss;
        } else {
            curr_sector_num = sector_num;
        }
        /* 计算剩余扇区数 */
        sector_num -= curr_sector_num;

        read_chs_disk(&para, curr_sector_num, addr_20bit);

        /* 下移chs参数 */
        para.sector_no = grf->ss;
        para.head++;
        if ((disk_para_table.head - 1) < para.head) {
            para.head = grf->sh;
            para.track++;
        }

        tc_32bit_mul(&offset, curr_sector_num, bpb_para_table.byts_per_sec);
        tc_32bit_add(&addr_20bit, addr_20bit, offset);
    }
}

/* 20bit addr is plat address */
static VOID read_clus_disk(DWORD clus, DWORD addr_20bit)
{
    DWORD clus_num;
    DWORD start_sector;

    tc_assert(-1 != tc_32bit_cmp(clus, ROOT_DIR_CLUS_NO));

    tc_32bit_sub(&clus_num, clus, ROOT_DIR_CLUS_NO);

    tc_32bit_mul(&start_sector, clus_num, bpb_para_table.sec_per_clus);
    tc_32bit_add(&start_sector, dir_start_sector, start_sector);
    read_lba_disk(start_sector, bpb_para_table.sec_per_clus, addr_20bit);
}

/* 使用扩展int13H读磁盘扇区 */
static VOID extend_read_disk(BYTE head, WORD track, WORD sector_no, WORD sector_num, DWORD addr_20bit)
{
/* dpa */
struct DiskAddressPacket {
    BYTE PacketSize;
    BYTE Reserved;
    WORD BlockCount;
    DWORD BufferAddr;
    QWORD BlockNum;
};

    WORD int13_ax = 0;
    WORD int13_bx = 0;
    WORD int13_cx = 0;
    WORD int13_dx = 0;
    WORD int13_es = 0;

#define INT13_AX_FUNC 0x2

    int13_ax = (INT13_AX_FUNC<<8)|sector_num;
    int13_cx = ((track&0xff)<<8)|(((track&0x300)>>2)|(sector_no&0x3f));
    int13_dx = (head<<8)|DISK_DRIVER_TYPE;
    int13_bx = addr_20bit&0x0f;
    int13_es = ((WORD)addr_20bit)>>4;
    int13_es = int13_es|(((WORD)(addr_20bit>>16))<<12);

    do {
        asm push es
        asm push dx
        asm push cx
        asm push bx
        asm push ax

        asm push int13_ax
        asm push int13_bx
        asm push int13_cx
        asm push int13_dx
        asm push int13_es
        asm pop ax
        asm mov es,ax
        asm pop dx
        asm pop cx
        asm pop bx
        asm pop ax
        asm int 13H

        asm pop ax
        asm pop bx
        asm pop cx
        asm pop dx
        asm pop es
    } while (0);
}

static BYTE test_extend13H(VOID)
{
    asm push ax
    asm push bx
    asm push cx

    asm mov ah,41H
    asm mov al,00H
    asm mov bx,55aaH
    asm mov cx,0000H

    asm pop cx
    asm pop bx
    asm pop ax
}

static VOID get_disk_para(VOID)
{
    BYTE ch_para;
    BYTE cl_para;
    BYTE dl_para;
    BYTE dh_para;

    do {
        asm push ax
        asm push cx
        asm push dx

        asm mov ah,08
        asm mov dl,DISK_DRIVER_TYPE
        asm int 13H
        asm mov ch_para,ch
        asm mov cl_para,cl
        asm mov dh_para,dh
        asm mov dl_para,dl

        asm pop dx
        asm pop cx
        asm pop ax
    } while (0);

    /* DH
       logical last index of heads = number_of - 1 (because index starts with 0)
       CX[7:6][15:8]
       logical last index of cylinders = number_of - 1 (because index starts with 0)
       CX[5:0]
       logical last index of sectors per track = number_of (because index starts with 1) */
    disk_para_table.cylinder = ch_para | ((((WORD) cl_para) << 2) & 0x300);
    disk_para_table.cylinder++;
    disk_para_table.head = dh_para;
    disk_para_table.head++;
    disk_para_table.sector = cl_para & 0x3f;
    disk_para_table.driver = dl_para;

    fat_print_str(" .disk cylinder: ", 0);
    fat_print_hex((BYTE)(disk_para_table.cylinder >> 8), 0);
    fat_print_hex((BYTE) disk_para_table.cylinder, 0);

    fat_print_str(" .disk head: ", 0);
    fat_print_hex(disk_para_table.head, 0);

    fat_print_str(" .disk sector: ", 0);
    fat_print_hex(disk_para_table.sector, 0);

    fat_print_str(" .disk driver: ", 0);
    fat_print_hex(disk_para_table.driver, 0);

    fat_print_char('', 1);
}

/* max clus no of disk */
static DWORD disk_max_clus_no = 0;

static VOID get_fat32_para(VOID)
{
#define BYTSPERSEC_OFFSET 11
#define SECPERCLUS_OFFSET 13
#define RSVDSECCNT_OFFSET 14
#define NUMFATS_OFFSET 16
#define HIDDSEC_OFFSET 28
/* this field is the fat32 32-bit count of sectors occupied by one fat. */
#define FATSZ32_OFFSET 36

    BYTE i;
    DWORD dbr_start_sector = 0;
    BYTE *buffer;

    fat_print_str("init fat32", 1);

    get_disk_para();

    get_disk_bsp();

    /* dbr, 0 head, 0 track, 1 sector */
    read_lba_disk(DBR_LBA, 1, BUFFER_ADDR);

    /* bpb_sec_per_clus = *(unsigned char huge*)(0x10000+i);
       bug, turbo c不能将立即数强制类型转换为地址, 因为内存不是平坦模式. */
    init_32p(buffer, BUFFER_ADDR);

    /* 地址为1000:0000 */
    bpb_para_table.sec_per_clus = *(BYTE huge*)(buffer + SECPERCLUS_OFFSET);
    bpb_para_table.rsvd_sec_cnt = *(WORD *)(buffer + RSVDSECCNT_OFFSET);
    bpb_para_table.num_fats = *(BYTE *)(buffer + NUMFATS_OFFSET);
    bpb_para_table.byts_per_sec = *(WORD *)(buffer + BYTSPERSEC_OFFSET);

    fat_print_str(" .sectors per clus: ", 0);
    fat_print_hex(bpb_para_table.sec_per_clus, 0);
    fat_print_str(" .rsvd sectors: ", 0);
    fat_print_hex(bpb_para_table.rsvd_sec_cnt, 0);
    fat_print_str(" .fat num: ", 0);
    fat_print_hex(bpb_para_table.num_fats, 0);

    /* min unit is cluster!!! */
    tc_assert(-1 != tc_32bit_cmp(FAT_LEN, bpb_para_table.sec_per_clus * bpb_para_table.byts_per_sec));
    tc_assert(-1 != tc_32bit_cmp(FAT_DIR_LEN, bpb_para_table.sec_per_clus * bpb_para_table.byts_per_sec));

    /* bpb_fatsz32 = *(unsigned long *)(BUFFER_ADDR*0x1000+FATSZ32_OFFSET); */
    set_32bit_value(&bpb_para_table.fatsz32, *(DWORD *)(buffer + FATSZ32_OFFSET));

    fat_print_str(" .fat size: ", 0);
    fat_print_dword_hex(bpb_para_table.fatsz32, 0);

    /* can not get hidden sector from dbr */
#if 0
    bpb_para_table.hiddsec[0] = *(BYTE *)(BUFFER_ADDR*0x1000+HIDDSEC_OFFSET+0);
    bpb_para_table.hiddsec[1] = *(BYTE *)(BUFFER_ADDR*0x1000+HIDDSEC_OFFSET+1);
    bpb_para_table.hiddsec[2] = *(BYTE *)(BUFFER_ADDR*0x1000+HIDDSEC_OFFSET+2);
    bpb_para_table.hiddsec[3] = *(BYTE *)(BUFFER_ADDR*0x1000+HIDDSEC_OFFSET+3);
    fat_print_hex(bpb_para_table.hiddsec[0],0);
    fat_print_hex(bpb_para_table.hiddsec[1],0);
    fat_print_hex(bpb_para_table.hiddsec[2],0);
    fat_print_hex(bpb_para_table.hiddsec[3],0);
    fat_start_sector = bpb_para_table.rsvd_sec_cnt + *(DWORD *)bpb_para_table.hiddsec;
#endif

    fat_print_char('', 1);

    /* bugfix */
#if 0
    dbr_start_sector = grf->ns * (BYTE)(grf->sh + 1);
    fat_start_sector = dbr_start_sector + bpb_para_table.rsvd_sec_cnt;
#else
    /* dbr, 0 head, 0 track, 1 sector */
    dbr_start_sector = disk_para_table.sector;
    fat_start_sector = dbr_start_sector + bpb_para_table.rsvd_sec_cnt;
#endif
    fat_print_str(" .fat start sector: ", 0);
    fat_print_dword_hex(fat_start_sector, 0);

    /* RootDirSector = ((BPB_RootEntCnt * 32) + (BPB_BytePerSec - 1)) / BPB_BytePerSec; */
    /* FirstDataSector = BPB_ResvedSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors; */
    /* for fat32, RootDirSector = 0 */
    tc_32bit_mul(&dir_start_sector, bpb_para_table.num_fats, bpb_para_table.fatsz32);
    tc_32bit_add(&dir_start_sector, dir_start_sector, fat_start_sector);

    fat_print_str(" .dir start sector: ", 0);
    fat_print_dword_hex(dir_start_sector, 0);

    fat_print_char('', 1);

    do {
        DWORD quotient;
        DWORD remainder;

        /* disk_max_clus_no = (bpb_para_table.fatsz32 / 4) * bpb_para_table.byts_per_sec; */
        tc_32bit_div(&quotient, &remainder, bpb_para_table.byts_per_sec, sizeof(DWORD));
        tc_32bit_mul(&disk_max_clus_no, quotient, bpb_para_table.fatsz32);
    } while (0);
}

/* clus no of current fat table buffer */
static DWORD curr_min_clus_no = 0;
static DWORD curr_max_clus_no = 0;

#define clus_is_valid(clus_no) ((1 != tc_32bit_cmp(curr_min_clus_no, clus_no)) && (1 == tc_32bit_cmp(curr_max_clus_no, clus_no)))

/* init fat32 talble */
static VOID read_fat32_table(VOID)
{
    DWORD fat_sector_num;
    DWORD temp;

    curr_min_clus_no = 0;
    tc_32bit_div(&fat_sector_num, &temp, FAT_LEN, bpb_para_table.byts_per_sec);
    if (TRUE != mul_is_overflow(fat_sector_num, bpb_para_table.byts_per_sec / sizeof(DWORD))) {
        tc_32bit_mul(&curr_max_clus_no, fat_sector_num, (bpb_para_table.byts_per_sec / sizeof(DWORD)));
    } else {
        fat_print_str("max clus no overflow", 0);
        abort();
    }

    /* read fat table */
    read_lba_disk(fat_start_sector, fat_sector_num, FAT_ADDR);

    fat_print_str(" .max clus no: ", 0);
    fat_print_dword_hex(curr_max_clus_no, 0);
    fat_print_char('', 1);
}

static VOID reread_fat_table(DWORD clus)
{
    DWORD temp;
    DWORD sector_num;
    DWORD item_num;
    DWORD buffer_num;

    tc_32bit_div(&sector_num, &temp, FAT_LEN, bpb_para_table.byts_per_sec);

    /* how many items in FAT_LEN */
    tc_32bit_mul(&item_num, sector_num, (bpb_para_table.byts_per_sec / sizeof(DWORD)));

    /* n * FAT_LEN */
    tc_32bit_div(&buffer_num, &temp, clus, item_num);

    /* min & max */
    tc_32bit_mul(&temp, buffer_num, item_num);
    set_32bit_value(&curr_min_clus_no, temp);
    tc_32bit_add(&curr_max_clus_no, item_num, curr_min_clus_no);

    /* fat sector no. */
    tc_32bit_mul(&temp, buffer_num, sector_num);
    tc_32bit_add(&temp, temp, fat_start_sector);
    read_lba_disk(temp, sector_num, FAT_ADDR);
}

static VOID get_next_clus(DWORD *clus)
{
    DWORD *fat;
    DWORD temp;

    tc_assert(clus_is_valid(*clus));

    init_32p(fat, FAT_ADDR);
#if 0
    /* 解析fat */
    fat = (DWORD *)p32addr(FAT_ADDR);
    /* 指针使用es寄存器 */
    push_ps(FAT_ADDR, tmp_reg);
#endif

    tc_32bit_sub(&temp, *clus, curr_min_clus_no);
    fat += temp;
    set_32bit_value(clus, *fat);

#if 0
    /* 恢复es寄存器 */
    pop_ss(tmp_reg);
#endif
}

#define DIR_NULL 0x20

/* 不比较扩展名 */
BYTE file_name_is_same(fat32_dir_item *dir, BYTE *file_name)
{
    WORD i;
    BYTE tmp;

    for (i = 0; i < FAT_DIR_NAME_SIZE - 3; i++) {
        if (('a' <= file_name[i]) && ('z' >= file_name[i])) {
            tmp = file_name[i] - ('a' - 'A');
        } else {
            tmp = file_name[i];
        }

        if (tmp != dir->DIR_Name[i]) {
            if ((DIR_NULL == dir->DIR_Name[i])
             && (('.' == file_name[i]) || ('\0' == file_name[i]))) { /* file and folder */
                return TRUE;
            } else {
                return FALSE;
            }
        }
    }
    return TRUE;
}

#define clus_is_end(clus) (END_OF_CLUS == (clus))

#define dir_is_end(dir_name) (0x00 == (dir_name)[0])
#define dir_is_del(dir_name) (DIR_DEL == (dir_name)[0])
#define dir_is_longfilename(dir_attr) (0x0f == (dir_attr))

/* current directory cluster no */
static DWORD dir_clus_no = 0;

#define file_clus_no(dir) (DWORD)((dir)->DIR_FstClusLO + (DWORD)((DWORD) (dir)->DIR_FstClusHI << 16))

/* find file or folder in current directory */
static WORD get_file_clus_no(BYTE *name, DWORD *clus)
{
    DWORD curr_clus;
    DWORD item_cnt;
    DWORD i;
    fat32_dir_item *dir;
    fat32_dir_item *fat_dir;
    WORD j;

    set_32bit_value(&curr_clus, dir_clus_no);
    do {
        /* read one cluster for dir */
        read_clus_disk(curr_clus, FAT_DIR_ADDR);

        /* lookup name */
        init_32p(fat_dir, FAT_DIR_ADDR);
        item_cnt = bpb_para_table.sec_per_clus * (bpb_para_table.byts_per_sec / sizeof(fat32_dir_item));
        for (i = 0; -1 == tc_32bit_cmp(i, item_cnt); i++) {
            dir = &fat_dir[i];

            /* dir is over */
            if (dir_is_end(dir->DIR_Name)) {
                set_32bit_value(clus, 0);
                return FAIL;
            }

            /* 文件是否被删除 */
            if (dir_is_del(dir->DIR_Name)) continue;

            /* 不支持长文件名 */
            if (dir_is_longfilename(dir->DIR_Attr)) continue;

            if (file_name_is_same(dir, name)) { /* find the file or folder */
                if (ATTR_DIRECTORY == dir->DIR_Attr) {
                    if (0 == file_clus_no(dir)) {
                        /* the dotdot entry points to the starting cluster of the parent of this directroy (which is 0 if this
                           directory parent is the root directory). */
                        if (0 == cmp_str("..", name)) {
                            set_32bit_value(clus, ROOT_DIR_CLUS_NO);
                            return SUCC;
                        } else {
                            fat_print_str("cd clus no is zero", 0);
                            abort();
                            return FAIL;
                        }
                    }
                }
                set_32bit_value(clus, file_clus_no(dir));
                return SUCC;
            }
        }

        /* get next clus of directory info */
        get_next_clus(&curr_clus);
    } while (!clus_is_end(curr_clus));

    set_32bit_value(clus, 0);
    return FAIL;
}

WORD fat32_change_directory(BYTE *dir_name)
{
    fat32_dir_item *dir;
    WORD i, j;
    DWORD dir_cnt, dir_clus_cnt;
    WORD ret;
    DWORD clus_no;

    if (NULL == dir_name) {
        return FAIL;
    }

    ret = get_file_clus_no(dir_name, &clus_no);
    if (SUCC == ret) {
        set_32bit_value(&dir_clus_no, clus_no);
        return SUCC;
    } else {
        fat_print_str("dir not exist", 0);
        return FAIL;
    }
}

static WORD read_file_by_clus_no(DWORD clus, DWORD addr)
{
    WORD cnt;

    cnt = 0;
    do {
        /* curr_max_clus_no >= clus */
        if (!clus_is_valid(clus)) {
            reread_fat_table(clus);
        }

        read_clus_disk(clus, BUFFER_ADDR);
        cnt++;

        /* 将磁盘缓存的内容转移到4G内存中 */
        move_1m(BUFFER_ADDR, addr, bpb_para_table.sec_per_clus * bpb_para_table.byts_per_sec);

        tc_32bit_add(&addr, addr, bpb_para_table.sec_per_clus * bpb_para_table.byts_per_sec);

        /* 获取文件的下一簇 */
        get_next_clus(&clus);

        /* cheat complier to use stack */
        while (0) {
            fat_print_hex(clus, 0);
            fat_print_hex(addr, 0);
        }
    } while (!clus_is_end(clus));

    if (0) {
WORD crc16(DWORD address, DWORD length);
        DWORD crc_len;
        WORD crc_value;
        static BYTE time = 0;

        fat_col = 0;
        fat_row = 1;
        fat_print_str("used-cluster: ", 0);
        fat_print_dec(cnt, 0);
        fat_print_str("last addr: ", 0);
        fat_print_dword_hex(addr, 0);

        tc_32bit_mul(&crc_len, cnt, bpb_para_table.sec_per_clus * bpb_para_table.byts_per_sec);
        fat_print_str("len:", 0);
        /* set file length, not disk space. 0x5d0000, 0x5d1400 */
        set_32bit_value(&crc_len, 6099064);
        fat_print_dword_hex(crc_len, 0);
        fat_print_str("crc:", 0);
        crc_value = crc16(0x1000000, crc_len);
        fat_print_hex((crc_value & 0xff00) >> 8, 0);
        fat_print_hex(crc_value & 0xff, 0);
        if (time++) abort();
    }

    return SUCC;
}

/* 将文件读取到一个32位地址空间中 */
WORD fat32_load_file(BYTE *file_name, DWORD addr)
{
    DWORD file_clus_no = 0x123456;
    WORD ret;

    if (NULL != file_name) {
        /* start cluster no of file */
        ret = get_file_clus_no(file_name, &file_clus_no);
        if ((SUCC == ret) && (0 != file_clus_no)) {
            /* 按簇读磁盘并将磁盘缓存的内容转移到4G内存中 */
            return read_file_by_clus_no(file_clus_no, addr);
        } else {
            fat_print_str("can not find file", 1);
        }
    }
    return FAIL;
}

/* show one cluster dir, so it is part. */
VOID list_fat32(VOID)
{
    fat32_dir_item *dir;
    fat32_dir_item *fat_dir;
    DWORD item_cnt;
    WORD i, j;

    read_clus_disk(dir_clus_no, FAT_DIR_ADDR);

    init_32p(fat_dir, FAT_DIR_ADDR);

    item_cnt = bpb_para_table.sec_per_clus * (bpb_para_table.byts_per_sec / sizeof(fat32_dir_item));
    for (i = 0; i < item_cnt; i++) {
        /* ...
           mov word ptr [bp-20],es
           mov word ptr [bp-22],bx */
        dir = &fat_dir[i];

        if (dir_is_end(dir->DIR_Name)) return;

        if (dir_is_del(dir->DIR_Name)) continue;

        if (dir_is_longfilename(dir->DIR_Attr)) continue;

        /* 打印文件创建日期 */
        fat_print_dec(((dir->DIR_WrtDate & 0x0fe00) >> 9) + 1980, 0);
        fat_print_char('-', 0);
        fat_print_dec((dir->DIR_WrtDate & 0x1e0) >> 5, 0);
        fat_print_char('-', 0);
        fat_print_dec(dir->DIR_WrtDate & 0x1f, 0);

        fat_print_char(' ', 0);

        /* 打印文件创建时间 */
        fat_print_dec((dir->DIR_WrtTime & 0x0f800) >> 11, 0);
        fat_print_char(':', 0);
        fat_print_dec((dir->DIR_WrtTime & 0x7e0) >> 5, 0);
        fat_print_char(':', 0);
        fat_print_dec((dir->DIR_WrtTime & 0x1f)*2, 0);

        fat_print_char(' ', 0);

        /* 打印文件大小, 属性 */
        if (ATTR_DIRECTORY == dir->DIR_Attr) {
            fat_print_str("<dir>", 0);
        } else {
            fat_print_dword_hex(dir->DIR_FileSize, 0);
        }

        fat_print_char(' ', 0);

        /* 打印文件名 */
        for (j = 0; j < FAT_DIR_NAME_SIZE; j++) {
            if (8 == j) fat_print_char('.', 0);
            if (0x20 != dir->DIR_Name[j]) fat_print_char(dir->DIR_Name[j], 0);
        }

        fat_print_char('', 1);
    }
}

static VOID read_fat32_root_dir(VOID)
{
    fat_print_str("root dir", 1);

    /* 第一簇是2号 */
    set_32bit_value(&dir_clus_no, ROOT_DIR_CLUS_NO);

    /* list_fat32(); */
}

static VOID read_fat32_data(VOID)
{
    WORD i;
    BYTE *addr;

    /* 第二簇是3号 */
    read_clus_disk(ROOT_DIR_CLUS_NO + 1, BUFFER_ADDR);

    init_32p(addr, BUFFER_ADDR);
    for (i=0; i<0x20; i++) {
        fat_print_hex(addr[i], 0);
    }
}

VOID init_fat32(VOID)
{
#include "../media.inc"

    get_fat32_para();

    read_fat32_table();

    read_fat32_root_dir();

    /* read_fat32_data(); */
}

