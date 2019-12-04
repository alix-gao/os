/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : arp.h
 * version     : 1.0
 * description :
 * author      : sicui
 * date        : 2016-3-25
 ***************************************************************/

#ifndef __ARP_H__
#define __ARP_H__
#include <eth_interface.h>

#define MAC_ADDR_LEN         (6)
#define IPV4_ADDR_LEN        (4)

enum ARP_TABLE_STATE_ENUM
{
    EMPTY = 0,
    NORMAL,
    WAITE_ARP_RESP,
    WAITE_RARP_RESP
};

enum ARP_TABLE_TYPE_ENUM
{
    STATIC_TABLE = 1,
    DYNAMIC_TABLE = 2
};

typedef struct
{
    os_u32 port_no;
    os_u8  mac_addr[6];
    os_u8  ip_addr[4];
    os_u32  age;
    os_u32  state;
    os_u32  type;
}ARP_INFO_STRU;



enum ARP_TYPE
{
    ARP_REQUEST  = 1,
    ARP_RESP     = 2,
    RARP_REQUEST = 3,
    RARP_RESP    = 4,
    ARP_TYPE_NULL
};

typedef struct
{
    os_u8 src_mac_addr[MAC_ADDR_LEN];
    os_u8 src_ip_addr[IPV4_ADDR_LEN];
    os_u8 dst_mac_addr[MAC_ADDR_LEN];
    os_u8 dst_ip_addr[IPV4_ADDR_LEN];
}ARP_REQUEST_CONTENT_STUR, ARP_RESP_CONTENT_STUR,ARP_CONTENT_STUR;

typedef struct
{
    os_u16 hrd_type;
    os_u16 pro_type;
    os_u8  hrd_addr_len;
    os_u8  pro_addr_len;
    os_u16 op_type;
}ARP_HEAD_STRU;

os_void Arp_Init();
os_void Arp_RecvPacket(os_u32 port_no, os_void* msg);
os_ret Arp_SendPacket(os_u8 src_mac[6], os_u8 src_ip[4], os_u8 dst_mac[6], os_u8 dst_ip[4], os_u16 op_type);
ARP_INFO_STRU* Arp_GetArpInfoByIPAddr(os_u8 ip_addr[4]);
os_void RefreshArpTableInfo(os_u32 port_no, os_u8 mac_addr[6], os_u8 ip_addr[4], os_u32 state, os_u32 age, os_u32 type);
#endif

