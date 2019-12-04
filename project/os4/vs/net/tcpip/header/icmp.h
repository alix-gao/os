/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : icmp.h
 * version     : 1.0
 * description :
 * author      : sicui
 * date        : 2016-4-8
 ***************************************************************/

#ifndef __ICMP_H__
#define __ICMP_H__

#define PING_INTERVAL OS_HZ
#define PING_TIMEOUT (4*OS_HZ)

enum ICMP_TYPE
{
    ECHO_REPLY = 0,
    DEST_UNRECH = 3,
    SOURCE_QUENCE = 4,
    REDITECT = 5,
    ECHO = 8,
    TIME_EXCEED = 11,
    PARA_ERR = 12,
    TIME_STAMP = 13,
    TIME_STAMP_REPLY = 14,
    INFO_REQ = 15,
    INFO_REPLY = 16
};


enum DEST_UNREACH_CODE
{
    NET_UNREACH = 0,
    HOST_UNREACH,
    PROTOCOL_UNREACH,
    PORT_UNREACH,
    FRAG_DF,
    SOURCE_ROUTE_FAIL,
};

enum TIME_EXCEED_CODE
{
    TTL_EXCEED = 0,
    FRAG_FAIL = 1,
};

typedef struct
{
    os_u8 type;
    os_u8 code;
}ICMP_COM_HEAD;

typedef struct
{
    os_u8 type;
    os_u8 code;
    os_u16 check_sum;
    os_u32 unused;
    os_u8  data[48];
}ICMP_SOURCE_QUENCH_STRU,ICMP_DEST_UNREACH_STRU,ICMP_TIME_OUT_STRU;

typedef struct
{
    os_u8 type;
    os_u8 code;
    os_u16 check_sum;
    os_u8 point;
    os_u8 unused[3];
    os_u8  data[48];
}ICMP_PARA_ERR_STRU;

enum REDIRECT_CODE
{
    RED_NET = 0,
    RED_HOST = 1,
    RED_SEV_NET = 2,
    RED_SEV_HOST = 3
};

typedef struct
{
    os_u8 type;
    os_u8 code;
    os_u16 check_sum;
    os_u32 gate_ip;
    os_u8  data[48];
}ICMP_REDIRECT_STRU;

typedef struct
{
    os_u8 type;
    os_u8 code;
    os_u16 check_sum;
    os_u16 id;
    os_u16 seq;
    os_u8  data[32];
}ICMP_ECHO_STRU;


typedef struct
{
    os_u8 type;
    os_u8 code;
    os_u16 id;
    os_u16 seq;
    os_u32  org_time;
    os_u32  rcv_time;
    os_u32  tx_time;
}ICMP_TIME_STAMP_STRU;

typedef struct
{
    os_u8 type;
    os_u8 code;
    os_u16 check_sum;
    os_u16 id;
    os_u16 seq;
}ICMP_INFOMATION_STRU, ICMP_COMM_REPLY_HEAD;

os_void ICMP_Init();
os_void ICMP_RecvPacket(os_void* msg, os_u16 len);

#endif

