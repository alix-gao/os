/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : ip_layer.h
 * version     : 1.0
 * description :
 * author      : sicui
 * date        : 2016-3-25
 ***************************************************************/

#ifndef __IP_LAYER_H__
#define __IP_LAYER_H__
#include <eth_interface.h>
enum IP_PACKET_TYPE
{
    SINGLE_CAST = 0,
    BROAD_CAST  = 1,
};

enum IP_RPOTOCOL_TYPE
{
    PRO_ICMP = 1,
    PRO_IGMP = 2,
    PRO_TCP  = 6,
    PRO_UDP  = 17,
    PRO_OSPF = 89,
};
#ifdef LITTLE_ENDIAN
typedef struct
{
    os_u8 hlen:4;
    os_u8 ver:4;
    os_u8 tos;
    os_u16 length;
    os_u16 idendify;
    os_u16  frag_offset:13;
    os_u16  frag_flag:3;
    os_u8 ttl;
    os_u8 pro;
    os_u16 h_crc;
    os_u32 src_ip;
    os_u32 dst_ip;
}IP_HEAD_STRU;
#else
typedef struct
{
    os_u8 ver:4;
    os_u8 hlen:4;
    os_u8 tos;
    os_u16 length;
    os_u16 idendify;
    os_u16  frag_flag:3;
    os_u16  frag_offset:13;
    os_u8 ttl;
    os_u8 pro;
    os_u16 h_crc;
    os_u32 src_ip;
    os_u32 dst_ip;
}IP_HEAD_STRU;
#endif
os_void IP_Init();
os_void IP_RecvPacket(os_u32 port_no, os_void* l2msg);
os_void IP_AddLocalIPAddr(os_u32 dev_no, os_u32 ip_addr, os_u32 ip_mask);
os_void IP_SendPacket(os_void* msg, os_u32 msg_type);

#endif

