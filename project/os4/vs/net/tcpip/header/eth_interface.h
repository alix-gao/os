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

#ifndef __ETH_INTERFACE_H__
#define __ETH_INTERFACE_H__

#define PRO_ARP (0x0806)
#define PRO_IP  (0x0800)
#define PRO_EAPOL 0x888e

typedef struct
{
    os_u8 dst_mac[6];
    os_u8 src_mac[6];
    os_u16 type;
}ETH_HEAD_STRU;

os_void Eth_Init();
os_ret Eth_RecvMsgFromPhy(os_u32 port_no, os_void* msg, os_u32 len);
os_ret Eth_SendMsgToPhy(os_void* msg, os_u32 len);
os_u8* Eth_GetItfMacAddr(os_u32 devce_no);

#endif

