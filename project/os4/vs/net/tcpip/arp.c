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

/***************************************************************
 include header file
 ***************************************************************/
#include <lib.h>
#include <os.h>
#include <core.h>
#include <net.h>
#include <arp.h>
#include <swab.h>

/***************************************************************
 global variable declare
 ***************************************************************/
#define MAX_ARP_TABLE_SIZE (256)
#define DEFAULT_ARP_AGE    (300)
ARP_INFO_STRU   g_arpTable[MAX_ARP_TABLE_SIZE];

#define arp_dbg(fmt, arg...) flog(fmt, ##arg)

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
LOCALC ARP_INFO_STRU* _GetArpInfoByMacAddr(os_u8 mac_addr[6])
{
    os_u32 i;
    for (i = 0; i < MAX_ARP_TABLE_SIZE; i++){
        if (g_arpTable[i].state == NORMAL && mem_cmp(mac_addr, g_arpTable[i].mac_addr, MAC_ADDR_LEN)) {
            return &g_arpTable[i];
        }
    }

    return OS_NULL;
}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
os_void RefreshArpTableInfo(os_u32 port_no, os_u8 mac_addr[6], os_u8 ip_addr[4], os_u32 state, os_u32 age, os_u32 type)
{
    ARP_REQUEST_CONTENT_STUR *arp_req = OS_NULL;
    os_u32 empty_index = MAX_ARP_TABLE_SIZE;
    os_u32 i;
    os_u8 brocast_mac[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    for (i = 0; i < MAX_ARP_TABLE_SIZE; i++){
        if (mem_cmp(g_arpTable[i].mac_addr, mac_addr, MAC_ADDR_LEN)) {
            mem_cpy(g_arpTable[i].ip_addr, ip_addr, IPV4_ADDR_LEN);
            g_arpTable[i].port_no = port_no;
            g_arpTable[i].age = age;
            g_arpTable[i].state = state;
            break;
        }

        if (g_arpTable[i].state == EMPTY && empty_index == MAX_ARP_TABLE_SIZE){
            empty_index = i;
        }
    }

    if (i >= MAX_ARP_TABLE_SIZE && empty_index < MAX_ARP_TABLE_SIZE) {
        print("Add Arp Table\n");
        mem_cpy(g_arpTable[empty_index].mac_addr, mac_addr, MAC_ADDR_LEN);
        mem_cpy(g_arpTable[empty_index].ip_addr, ip_addr, IPV4_ADDR_LEN);
        g_arpTable[empty_index].port_no = port_no;
        g_arpTable[empty_index].age   = age;
        g_arpTable[empty_index].state = state;
        g_arpTable[empty_index].type  = type;
    }

    /*Send Free Arp Msg*/
    if (type == STATIC_TABLE) {
        Arp_SendPacket(mac_addr, ip_addr, brocast_mac, ip_addr, ARP_REQUEST);
    }
}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
LOCALC os_void _OnRecvArpRequest(os_u32 port_no, os_void* msg)
{
    ARP_INFO_STRU *arp_info = OS_NULL;
    ARP_REQUEST_CONTENT_STUR *arp_req = OS_NULL;

    /* Refresh the dynamic arp table*/
    arp_req = (ARP_REQUEST_CONTENT_STUR *)msg;
    RefreshArpTableInfo(port_no, arp_req->src_mac_addr, arp_req->src_ip_addr, NORMAL, DEFAULT_ARP_AGE, DYNAMIC_TABLE);

    /*if the requset src IP is your own localIP, do nothing*/
    arp_info = Arp_GetArpInfoByIPAddr(arp_req->src_ip_addr);
    if (arp_info == OS_NULL || arp_info->type == STATIC_TABLE){
        return;
    }

    /* if the request dest IP is on of your localIp, you need send a resp*/
    arp_info = Arp_GetArpInfoByIPAddr(arp_req->dst_ip_addr);
    if (arp_info == OS_NULL || arp_info->type != STATIC_TABLE){
        return;
    }

    /*Send Msg*/
    Arp_SendPacket(arp_info->mac_addr,arp_info->ip_addr, arp_req->src_mac_addr, arp_req->src_ip_addr, ARP_RESP);
}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
LOCALC os_void _OnRecvArpResp(os_u32 port_no, os_void* msg)
{
    ARP_RESP_CONTENT_STUR *arp_resp = (ARP_RESP_CONTENT_STUR *)msg;
    arp_dbg("_OnRecvArpResp\n");
    arp_dbg("src IP:%x-%x-%x-%x\n", arp_resp->src_ip_addr[0], arp_resp->src_ip_addr[1], arp_resp->src_ip_addr[2], arp_resp->src_ip_addr[3]);
    arp_dbg("src mac: %x:%x:%x:%x:%x:%x\n", arp_resp->src_mac_addr[0], arp_resp->src_mac_addr[1], arp_resp->src_mac_addr[2], arp_resp->src_mac_addr[3], arp_resp->src_mac_addr[4], arp_resp->src_mac_addr[5]);
    RefreshArpTableInfo(port_no, arp_resp->src_mac_addr, arp_resp->src_ip_addr, NORMAL, DEFAULT_ARP_AGE, DYNAMIC_TABLE);
}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
LOCALC os_void _OnRecvRarpRequest()
{
}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
LOCALC os_void _OnRecvRarpResp()
{
}

LOCALC os_ret OS_CALLBACK _Arp_TimeOut(os_u32 event_id)
{
    os_u32 i;
    static os_u32 timeoutTimes = 0;
    ARP_REQUEST_CONTENT_STUR *arp_req = OS_NULL;
    os_u8 brocast_mac[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    timeoutTimes++;
    for (i = 0; i < MAX_ARP_TABLE_SIZE; i++) {
        if (g_arpTable[i].type == DYNAMIC_TABLE && g_arpTable[i].state == NORMAL) {
            if (g_arpTable[i].age == 0) {
                mem_set(&g_arpTable[i], EMPTY, sizeof(g_arpTable[i]));
            }
            else {
                g_arpTable[i].age--;
            }
        }
        else if (g_arpTable[i].type == STATIC_TABLE && g_arpTable[i].state == NORMAL && timeoutTimes % 300 == 0)
        {
            arp_dbg("Send Free Arp:mac:0x%x-0x%x-0x%x-0x%x-0x%x-0x%x, IP:0x%x",g_arpTable[i].mac_addr[0],g_arpTable[i].mac_addr[1],g_arpTable[i].mac_addr[2],
                 g_arpTable[i].mac_addr[3],g_arpTable[i].mac_addr[4],g_arpTable[i].mac_addr[5], *(os_u32*)g_arpTable[i].ip_addr);
            Arp_SendPacket(g_arpTable[i].mac_addr, g_arpTable[i].ip_addr, brocast_mac, g_arpTable[i].ip_addr, ARP_REQUEST);
        }
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dump_arp(os_void)
{
    os_u32 i;
    os_u8 *mac_addr = OS_NULL;
    os_u8  ip_addr[4]={192, 168, 0, 4};

  //  mac_addr = Eth_GetItfMacAddr(1);
   // print("Get mac: %x:%x:%x:%x:%x:%x\n",mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  //  RefreshArpTableInfo(1, mac_addr, ip_addr, NORMAL, DEFAULT_ARP_AGE, STATIC_TABLE);

    for (i = 0; i < MAX_ARP_TABLE_SIZE; i++) {
        if (g_arpTable[i].state == NORMAL) {
            print("arp[%d]:port_no:%d, state:%d, type:%d, age:%d\n", i, g_arpTable[i].port_no, g_arpTable[i].state, g_arpTable[i].type, g_arpTable[i].age);
            print("IP:%x-%x-%x-%x\n", g_arpTable[i].ip_addr[0], g_arpTable[i].ip_addr[1], g_arpTable[i].ip_addr[2], g_arpTable[i].ip_addr[3]);
            print("mac: %x:%x:%x:%x:%x:%x\n", g_arpTable[i].mac_addr[0], g_arpTable[i].mac_addr[1], g_arpTable[i].mac_addr[2], g_arpTable[i].mac_addr[3], g_arpTable[i].mac_addr[4], g_arpTable[i].mac_addr[5]);
        }
    }
}

LOCALD os_u8 arp_debug_name[] = { "arp" };
LOCALD struct dump_info arp_debug = {
    arp_debug_name,
    dump_arp
};

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
os_void Arp_Init()
{
    os_u32 reg;
    HTIMER timer;
    mem_set(g_arpTable, EMPTY, sizeof(g_arpTable));
    timer = set_timer_callback(reg, 100, _Arp_TimeOut, TIMER_MODE_LOOP);
    (void)register_dump(&arp_debug);
}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
os_void Arp_RecvPacket(os_u32 port_no, os_void* l2msg)
{
    ARP_HEAD_STRU *arp_msg = OS_NULL;

    if (OS_NULL == l2msg) {
        return;
    }

    arp_msg = (ARP_HEAD_STRU *)((ETH_HEAD_STRU *) l2msg + 1);
    arp_dbg("Recv Arp Packet:%d\n", ntoh16(arp_msg->op_type));
    if (arp_msg->hrd_addr_len != MAC_ADDR_LEN || arp_msg->pro_addr_len != IPV4_ADDR_LEN) {
        return;
    }
    arp_msg->op_type = ntoh16(arp_msg->op_type);

    switch(arp_msg->op_type){
        case ARP_REQUEST:
            _OnRecvArpRequest(port_no, arp_msg + 1);
            break;
        case ARP_RESP:
            _OnRecvArpResp(port_no, arp_msg + 1);
            break;
        case RARP_REQUEST:
            //_OnRecvRarpRequest((ARP_HEAD_STRU *)msg + 1);
            break;
        case RARP_RESP:
            //_OnRecvRarpResp((ARP_HEAD_STRU *)msg + 1);
            break;
        default:
            break;
    }

    return;
}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
ARP_INFO_STRU* Arp_GetArpInfoByIPAddr(os_u8 ip_addr[4])
{
    os_u32 i;
    for (i = 0; i < MAX_ARP_TABLE_SIZE; i++){
        if (g_arpTable[i].state == NORMAL && mem_cmp(ip_addr, g_arpTable[i].ip_addr, IPV4_ADDR_LEN)) {
            return &g_arpTable[i];
        }
    }
    return OS_NULL;
}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
os_ret Arp_SendPacket(os_u8 src_mac[6], os_u8 src_ip[4], os_u8 dst_mac[6], os_u8 dst_ip[4], os_u16 op_type)
{
    os_u8* msg = OS_NULL;
    ETH_HEAD_STRU *eth_head = OS_NULL;
    ARP_HEAD_STRU *arp_head = OS_NULL;
    ARP_CONTENT_STUR *arp_content = OS_NULL;
    os_u32 len = sizeof(ARP_HEAD_STRU) + sizeof(ARP_CONTENT_STUR) + sizeof(ETH_HEAD_STRU);

    msg = (os_u8*)kmalloc(len);
    if (msg == OS_NULL) {
        return OS_FAIL;
    }

    eth_head = (ETH_HEAD_STRU *)msg;
    mem_cpy(eth_head->dst_mac, dst_mac, MAC_ADDR_LEN);
    mem_cpy(eth_head->src_mac, src_mac, MAC_ADDR_LEN);
    eth_head->type = hton16(PRO_ARP);

    arp_head = (ARP_HEAD_STRU*)(eth_head + 1);
    arp_head->hrd_type = hton16(1);
    arp_head->pro_type = hton16(PRO_IP);
    arp_head->hrd_addr_len = MAC_ADDR_LEN;
    arp_head->pro_addr_len = IPV4_ADDR_LEN;
    arp_head->op_type = hton16(op_type);

    arp_content = (ARP_CONTENT_STUR*)(arp_head + 1);
    mem_cpy(arp_content->src_mac_addr, src_mac, MAC_ADDR_LEN);
    mem_cpy(arp_content->src_ip_addr, src_ip, IPV4_ADDR_LEN);
    mem_cpy(arp_content->dst_mac_addr, dst_mac, MAC_ADDR_LEN);
    mem_cpy(arp_content->dst_ip_addr, dst_ip, IPV4_ADDR_LEN);

    arp_dbg("Send Arp packet%d:\n", op_type);
    arp_dbg("src IP:%x-%x-%x-%x\n", src_ip[0], src_ip[1], src_ip[2], src_ip[3]);
    arp_dbg("src mac: %x:%x:%x:%x:%x:%x\n", src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]);
    arp_dbg("dst IP:%x-%x-%x-%x\n", dst_ip[0], dst_ip[1], dst_ip[2], dst_ip[3]);
    arp_dbg("dst mac: %x:%x:%x:%x:%x:%x\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
    Eth_SendMsgToPhy(msg, len);
    kfree(msg);
}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
os_ret add_static_arp_item(os_u32 port_no, os_u8 mac_addr[6], os_u8 ip_addr[4])
{
    RefreshArpTableInfo(port_no, mac_addr, ip_addr, NORMAL, DEFAULT_ARP_AGE, STATIC_TABLE);
}

