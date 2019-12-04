/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : net.h
 * version     : 1.0
 * description :
 * author      : sicui
 * date        : 2016-3-25
 ***************************************************************/
#ifndef __NET_H__
#define __NET_H__

#include <swab.h>

#define hton16(n) ___constant_swab16(n)
#define hton32(n) ___constant_swab32(n)
#define ntoh16(n) ___constant_swab16(n)
#define ntoh32(n) ___constant_swab32(n)

#define eth_dbg(fmt, arg...) flog(fmt, ##arg)

struct eth_mac_header {
    os_u8 dst[6];
    os_u8 src[6];
    os_u16 type;
    os_u8 payload[0];
};

typedef struct {
    os_u32 dev_no;
    os_u8  mac_addr[6];

    os_void *dedicated;

    os_ret (*connect)(os_void *user, os_u8 *ssid, os_u8 *pass);
    os_ret (*SendPacket)(os_void *user, os_void* msg, os_u32 len);
    os_ret (*disconnect)(os_void *user);
} ETH_DEV_INFO_STRU;

os_ret Eth_RegDevice(ETH_DEV_INFO_STRU *device);
os_void Eth_DeRegDevice(os_u32 device_no);
os_void process_packet(os_u32 dev_no, os_u8 *l2packet, os_uint len);

os_void ping(os_u8* dst_ip, os_u8* src_ip, os_u32 times);

os_u16 ipCksum(os_u16 *addr,int len);
os_ret str2ipv4(os_u8 *str, os_u32 *ip);

os_ret gen_ie_by_parse_scan(os_u8 *buf, os_uint len, os_u8 *beacon_ie, os_uint beacon_ie_len);

#endif

