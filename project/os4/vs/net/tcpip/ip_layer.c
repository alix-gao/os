/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : ip_layer.c
 * version     : 1.0
 * description :
 * author      : sicui
 * date        : 2016-3-25
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <lib.h>
#include <os.h>
#include <core.h>
#include <ip_layer.h>
#include <icmp.h>
#include <arp.h>
#include <net.h>

/***************************************************************
 global variable declare
 ***************************************************************/
typedef struct
{
   os_u32  protocol_type;
   os_void (*init)();
   os_void (*OnRcvPacket)(os_void* msg, os_u16 len);
   os_u32  rcv_pack_num;
   os_u32  send_pack_num;
}IP_UPER_PROTOCOL_STRU;

IP_UPER_PROTOCOL_STRU g_Ip_upper_protocol[] =
{
    { PRO_ICMP,  ICMP_Init, ICMP_RecvPacket, 0, 0},
    { PRO_IGMP,  OS_NULL, OS_NULL,    0, 0},
    { PRO_TCP,  OS_NULL, OS_NULL,     0, 0},
    { PRO_UDP,  OS_NULL, OS_NULL,     0, 0},
    { PRO_OSPF,  OS_NULL, OS_NULL,    0, 0},
};

typedef struct
{
    os_u32 dev_no;
    os_u32 ip_addr;
    os_u32 ip_mask;
}ITF_IP_STRU;

typedef struct
{
    ITF_IP_STRU Ip_info;
    struct list_node node;
    spinlock_t lock;
}ITF_IP_LIST;
ITF_IP_LIST g_itf_ip_list;


LOCALC os_void dump_iplayer(os_void)
{
    struct list_node *i;
    ITF_IP_LIST *new_ip = OS_NULL;

    loop_list(i, &g_itf_ip_list.node) {
        new_ip = list_addr(i, ITF_IP_LIST, node);
        if (new_ip != OS_NULL) {
            print("devNo:%x, ip=%x, mask=%x\n", new_ip->Ip_info.dev_no, new_ip->Ip_info.ip_addr, new_ip->Ip_info.ip_mask);
        }
    }
}

LOCALD os_u8 iplayer_debug_name[] = { "iplayer" };
LOCALD struct dump_info iplayer_debug = {
    iplayer_debug_name,
    dump_iplayer
};

os_u16 g_ip_identify = 0;
#define iplayer_dbg(fmt, arg...) flog(fmt, ##arg)

/***************************************************************
 * description :
 * history     : sicui 2016-4-9
 ***************************************************************/
os_u16 ipCksum(os_u16 *addr,int len)
{
    os_u16 cksum;
    os_u32 sum=0;
    while(len>1) {
        sum+=*addr++;
        len-=2;
    }
    if(len==1)
        sum+=*(os_u8*)addr;
    sum=(sum>>16)+(sum&0xffff);  //把高位的进位，加到低八位，其实是32位加法
    sum+=(sum>>16);  //add carry
    cksum=~sum;   //取反
    return (cksum);
}

/***************************************************************
 * description :
 * history     : sicui 2016-4-9
 ***************************************************************/
os_ret str2ipv4(os_u8 *str, os_u32 *ip)
{
    os_u8 *s = ".";
    os_u8 ip_buff[3*4 + 3 + 1];
    os_uint i;
    os_u32 m;
    os_u8 *t;

    mem_cpy(ip_buff, str, sizeof(ip_buff));
    str = ip_buff;
    m = 0;
    for (i = 0; i < 3; i++) {
        t = str_brk(&str, s);
        if ((OS_NULL == t) || (str2num(t) > 0xff)) {
            print("invalid IP addr!\n");
            return OS_FAIL;
        }
        m |= (str2num(t) << ((3-i)*8));
    }
    m |= str2num(str);
    *ip = m;
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
os_void IP_Init()
{
    os_u32 i;
    for (i = 0; i < array_size(g_Ip_upper_protocol); i++) {
        if (g_Ip_upper_protocol[i].init != OS_NULL) {
            g_Ip_upper_protocol[i].init();
        }
    }

    init_list_head(&g_itf_ip_list.node);
    init_spinlock(&g_itf_ip_list.lock);

    (void)register_dump(&iplayer_debug);
}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
LOCALC os_ret _FillIPAndEthHeadByDstIPAddr(os_u32 ip_addr, IP_HEAD_STRU *ip_head, ETH_HEAD_STRU *eth_head)
{
    struct list_node *i;
    ITF_IP_LIST *itf_ip = OS_NULL;
    ARP_INFO_STRU* arp_info = OS_NULL;
    os_u8* mac_addr = OS_NULL;
    os_u32 itf_addr = 0;
    os_u8 broadcast_mac[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

    if (eth_head == OS_NULL || OS_NULL == ip_head) {
        return OS_FAIL;
    }

    spin_lock(&g_itf_ip_list.lock);
    loop_list(i, &g_itf_ip_list.node) {
        itf_ip = list_addr(i, ITF_IP_LIST, node);

        if (itf_ip != OS_NULL) {
            if ((ip_head->src_ip != 0xFFFFFFFF  && itf_ip->Ip_info.ip_addr == ntoh32(ip_head->src_ip))
                || (ip_head->src_ip == 0xFFFFFFFF  && (itf_ip->Ip_info.ip_addr & itf_ip->Ip_info.ip_mask) == (ip_addr & itf_ip->Ip_info.ip_mask))) {

                ip_head->src_ip = hton32(itf_ip->Ip_info.ip_addr);

                mac_addr = Eth_GetItfMacAddr(itf_ip->Ip_info.dev_no);
                if (mac_addr == OS_NULL) {
                    print("Eth_GetItfMacAddr fail:ip=%x\n", itf_ip->Ip_info.ip_addr);
                    continue;
                }

                ip_addr = hton32(ip_addr);
                arp_info = Arp_GetArpInfoByIPAddr((os_u8*)&ip_addr);
                if (arp_info == OS_NULL) {
                    print("Arp_GetArpInfoByIPAddr fail:ip=%x\n", hton32(ip_addr));
                    itf_addr = hton32(itf_ip->Ip_info.ip_addr);
                    Arp_SendPacket(mac_addr, (os_u8*)&itf_addr, broadcast_mac, (os_u8*)&ip_addr, ARP_REQUEST);
                    continue;
                }
                else {
                    mem_cpy(eth_head->dst_mac, arp_info->mac_addr, MAC_ADDR_LEN);
                    mem_cpy(eth_head->src_mac, mac_addr, MAC_ADDR_LEN);
                    spin_unlock(&g_itf_ip_list.lock);
                    return OS_SUCC;
                }
            }
        }
    }
    spin_unlock(&g_itf_ip_list.lock);
    print("_FillEthHeadByIPAddr fail:ip=%x\n", ip_addr);

    return OS_FAIL;
}
/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
os_void IP_SendPacket(os_void* msg, os_u32 msg_type)
{
    struct list_node *i;
    ITF_IP_LIST *itf_ip = OS_NULL;
    os_u8* mac_addr = OS_NULL;
    os_u32 next_hop = 0;
    os_u8 broadcast_mac[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    IP_HEAD_STRU *ip_head = OS_NULL;
    ETH_HEAD_STRU *eth_head = OS_NULL;
    os_u8 *packet = OS_NULL;

    if (msg == OS_NULL) {
        return;
    }

    ip_head = (IP_HEAD_STRU *)msg;
    packet = kmalloc(ntoh16(ip_head->length) + sizeof(ETH_HEAD_STRU));
    if (packet == OS_NULL) {
        return;
    }
    mem_cpy(packet + sizeof(ETH_HEAD_STRU), msg, ntoh16(ip_head->length));
    eth_head  = (ETH_HEAD_STRU*)packet;
    eth_head->type = hton16(PRO_IP);

    ip_head  = (IP_HEAD_STRU *)(eth_head + 1);
    ip_head->idendify = hton16(g_ip_identify);
    g_ip_identify++;

    /*send brocast packet*/
    if (BROAD_CAST == msg_type) {
        mem_cpy(eth_head->dst_mac, broadcast_mac, MAC_ADDR_LEN);
        spin_lock(&g_itf_ip_list.lock);
        loop_list(i, &g_itf_ip_list.node) {
            itf_ip = list_addr(i, ITF_IP_LIST, node);
            if (itf_ip->Ip_info.ip_addr & ip_head->dst_ip == itf_ip->Ip_info.ip_addr) {
                ip_head->src_ip = hton32(itf_ip->Ip_info.ip_addr);
                mac_addr = Eth_GetItfMacAddr(itf_ip->Ip_info.ip_addr);
                if (mac_addr != OS_NULL) {
                    mem_cpy(eth_head->src_mac, mac_addr, MAC_ADDR_LEN);
                    iplayer_dbg("BROAD_CAST:IP_SendPacket:srcip=%x, dstip=%x, len=%x\n", ntoh32(ip_head->src_ip), ntoh32(ip_head->dst_ip), ntoh16(ip_head->length));
                    ip_head->h_crc = ipCksum((os_u16 * )ip_head, 20);
                    Eth_SendMsgToPhy(packet, ntoh16(ip_head->length) + sizeof(ETH_HEAD_STRU));
                    kfree(packet);
                }
            }
        }
        spin_unlock(&g_itf_ip_list.lock);
    }
    else {
        /*dst ip is the same net of local ip*/
        if (OS_SUCC == _FillIPAndEthHeadByDstIPAddr(ntoh32(ip_head->dst_ip), ip_head, eth_head))
        {
            iplayer_dbg("IP_SendPacket:srcip=%x, dstip=%x, len=%x\n", ntoh32(ip_head->src_ip), ntoh32(ip_head->dst_ip), ntoh16(ip_head->length));
            ip_head->h_crc = ipCksum((os_u16 * )ip_head, 20);
            Eth_SendMsgToPhy(packet, ntoh16(ip_head->length) + sizeof(ETH_HEAD_STRU));
            kfree(packet);
            return;
        }

        /*find next hop*/

        /*search arp table*/
    }

}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
os_void IP_RecvPacket(os_u32 port_no, os_void* l2msg)
{
    os_u32 j = 0;
    IP_HEAD_STRU *ip_head = OS_NULL;
    struct list_node *i;
    ITF_IP_LIST *itf_ip = OS_NULL;

    if (l2msg == OS_NULL) {
        return;
    }

    ip_head = (IP_HEAD_STRU *)((ETH_HEAD_STRU *) l2msg + 1);

    /*check ttl, if ttl ==0, discard the packet*/
    if (ip_head->ttl == 0) {
        return;
    }
    print("IP_RecvPacket:pro=%x, len=%d, dst_ip=%x\n", ip_head->pro, ntoh16(ip_head->length), ntoh32(ip_head->dst_ip));

    spin_lock(&g_itf_ip_list.lock);
    loop_list(i, &g_itf_ip_list.node) {
        itf_ip = list_addr(i, ITF_IP_LIST, node);
        /* if dst ip is your local ip just deal the msg*/
        if ((itf_ip->Ip_info.ip_addr == ntoh32(ip_head->dst_ip))
         || (itf_ip->Ip_info.ip_addr & ntoh32(ip_head->dst_ip) == itf_ip->Ip_info.ip_addr)) {
            for (j = 0; j < sizeof(g_Ip_upper_protocol)/sizeof(IP_UPER_PROTOCOL_STRU); j++) {
                   if (ip_head->pro == g_Ip_upper_protocol[j].protocol_type && g_Ip_upper_protocol[j].OnRcvPacket != OS_NULL) {
                       g_Ip_upper_protocol[j].OnRcvPacket(ip_head, ntoh16(ip_head->length));
                       g_Ip_upper_protocol[j].rcv_pack_num++;
                       spin_unlock(&g_itf_ip_list.lock);
                       return;
                   }
           }
        }
    }
    spin_unlock(&g_itf_ip_list.lock);
}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
os_void IP_AddLocalIPAddr(os_u32 dev_no, os_u32 ip_addr, os_u32 ip_mask)
{
    os_u8 *mac_addr = OS_NULL;
    ITF_IP_LIST *new_ip = OS_NULL;

    new_ip = kmalloc(sizeof(ITF_IP_LIST));
    if (new_ip == OS_NULL)
    {
        return;
    }
    new_ip->Ip_info.dev_no = dev_no;
    new_ip->Ip_info.ip_addr = ip_addr;
    new_ip->Ip_info.ip_mask = ip_mask;
    spin_lock(&g_itf_ip_list.lock);
    add_list_tail(&g_itf_ip_list.node, &new_ip->node);
    spin_unlock(&g_itf_ip_list.lock);

    ip_addr = hton32(ip_addr);
    mac_addr = Eth_GetItfMacAddr(dev_no);
    RefreshArpTableInfo(dev_no, mac_addr, (os_u8*)&ip_addr, NORMAL, 255, STATIC_TABLE);
}

/***************************************************************
 * description :
 * history     : gaocheng 2018-01-01
 ***************************************************************/
os_void IP_del_local_IPAddr(os_u32 dev_no, os_u32 ip_addr)
{
    struct list_node *i;
    ITF_IP_LIST *ip;

    spin_lock(&g_itf_ip_list.lock);
    loop_list(i, &g_itf_ip_list.node) {
        ip = list_addr(i, ITF_IP_LIST, node);
        /* if dst ip is your local ip just deal the msg*/
        if ((ip->Ip_info.dev_no == dev_no)
         && (ip->Ip_info.ip_addr == ip_addr)) {
            del_list(&ip->node);
            kfree(ip);
            spin_unlock(&g_itf_ip_list.lock);
            return;
        }
    }
    spin_unlock(&g_itf_ip_list.lock);
}

/***************************************************************
 * description :
 * history     : gaocheng 2019-01-01
 ***************************************************************/
os_ret get_ipv4_address(os_u32 dev_no, os_u32 *ip, os_u32 *mask)
{
    struct list_node *i;
    ITF_IP_LIST *ip_node;

    spin_lock(&g_itf_ip_list.lock);
    loop_list(i, &g_itf_ip_list.node) {
        ip_node = list_addr(i, ITF_IP_LIST, node);
        if (ip_node->Ip_info.dev_no == dev_no) {
            *ip = ip_node->Ip_info.ip_addr;
            *mask = ip_node->Ip_info.ip_mask;
            spin_unlock(&g_itf_ip_list.lock);
            return OS_SUCC;
        }
    }
    spin_unlock(&g_itf_ip_list.lock);
    return OS_FAIL;
}

