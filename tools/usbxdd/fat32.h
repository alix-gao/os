
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef signed char s8;
typedef signed short s16;
typedef signed long s32;

#define info(fmt, arg...) do { if (1) { printf(fmt, ##arg); } } while (0)

/* ��0�ֽ�     �Ƿ�Ϊ�����,����Ϊ80H,����Ϊ00H
   ��1�ֽ�     �÷�����ʼ��ͷ��
   ��2�ֽ�     �÷�����ʼ������(��6λ)����ʼ�����(��2λ)
   ��3�ֽ�     �÷�����ʼ����ŵĵ�8λ
   ��4�ֽ�     ϵͳ��־,00H��÷���δʹ��,06H��߰汾DOSϵͳ,05HչDOS����,65H��Netwear����
   ��5�ֽ�     �÷���������ͷ��
   ��6�ֽ�     �÷�������������(��6λ)�ͽ��������(��2λ)
   ��7�ֽ�     �÷�����������ŵĵ�8λ
   ��8~11�ֽ�  ���������,�÷�����ʼ������߼�������,��λ�ں��λ��ǰ
   ��12~15�ֽ� �÷�������������,��λ�ں�,��λ��ǰ */

#define DPT_LEN 16
#define DPT_0_OFFSET 0x1be
#define DPT_1_OFFSET (DPT_0_OFFSET + DPT_LEN)
#define DPT_2_OFFSET (DPT_1_OFFSET + DPT_LEN)
#define DPT_3_OFFSET (DPT_2_OFFSET + DPT_LEN)

/* dpt start */
#define BOOT_FLAG_OFFSET 0
#define DBR_HEAD_OFFSET 1
#define DBR_SECTOR_OFFSET 2
#define DBR_CYLINDER_OFFSET 3
#define FS_TYPE_OFFSET 4
/* dpt end */

#define FS_TYPE_FAT32 0x01
#define FS_TYPE_WIN95_FAT32_1 0x0b
#define FS_TYPE_WIN95_FAT32_2 0x0c
#define FS_TYPE_WIN95_FAT16 0x0e

/* relative sector */
#define DPT_POS_OFFSET 8

#pragma pack(1)
struct fat32_bpb {
    u8 BS_jmpBoot[3];
    u8 BS_OEMName[8];
    u16 BPB_BytsPerSec;
    u8 BPB_SecPerClus;
    u16 BPB_RsvdSecCnt;
    u8 BPB_NumFATs;
    u16 BPB_RootEntCnt;
    u16 BPB_TotSec16;
    u8 BPB_Media;
    u16 BPB_FATSz16;
    u16 BPB_SecPerTrk;
    u16 BPB_NumHeads;
    u32 BPB_HiddSec;
    u32 BPB_TotSec32;

    /* only for fat32 */
    u32 BPB_FATSz32; // this field is the fat32 32-bit count of sectors occupied by one fat.
    u16 BPB_ExtFlags;
    u16 BPB_FSVer;
    u32 BPB_RootClus;
    u16 BPB_FSInfo;
    u16 BPB_BkBootSec;
    u8 BPB_Reserved[12];
    u8 BPB_DrvNum;
    u8 BPB_Reserved1[1];
    u8 BPB_BootSig;
    u32 BPB_VolId;
    u8 BPB_VolLab[11];
    u8 BPB_FilSysType[8];
};
#pragma pack()

