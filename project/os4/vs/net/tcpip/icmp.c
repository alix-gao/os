/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : icmp.c
 * version     : 1.0
 * description :
 * author      : sicui
 * date        : 2016-4-8
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <lib.h>
#include <os.h>
#include <core.h>
#include <net.h>
#include <ip_layer.h>
#include <icmp.h>
#include <swab.h>

/***************************************************************
 global variable declare
 ***************************************************************/

os_u16 g_sendIcmpSeqNo = 0;

typedef struct
{
    os_u16 seq_no;
    os_u32 ip_addr;
    os_void (*OnRcvReply)(os_void* msg);
}ICMP_REPLY_FUC;
typedef struct
{
    ICMP_REPLY_FUC rply_func;
    struct list_node node;
}ICMP_REPLY_FUC_LIST;

ICMP_REPLY_FUC_LIST g_icmp_reply_fuc_list;

typedef struct
{
    os_u32 task_flag;
    os_u32 dst_ip;
    os_u32 src_ip;
    os_u32 try_times;
    os_u32 recv_packet;
    os_u32 send_packet;
    os_u32 send_seq;
    HTIMER timer;
}PING_TASK_INFO;
PING_TASK_INFO g_ping_task;

#define icmp_dbg(fmt, arg...) flog(fmt, ##arg)

LOCALC os_void _OnRecvIcmpEchoReply(os_void *msg);

/***************************************************************
 * description :
 * history     : sicui 2016-4-9
 ***************************************************************/
LOCALC os_void _RegIcmpReplyFunc(ICMP_REPLY_FUC *fuc)
{
    ICMP_REPLY_FUC_LIST *new_fuc = OS_NULL;
    if (fuc == OS_NULL) {
        return;
    }
    new_fuc = kmalloc(sizeof(ICMP_REPLY_FUC_LIST));
    if (new_fuc == OS_NULL) {
        return;
    }
    mem_cpy(&new_fuc->rply_func, fuc, sizeof(ICMP_REPLY_FUC));
    add_list_tail(&g_icmp_reply_fuc_list.node, &new_fuc->node);
}

/***************************************************************
 * description :
 * history     : sicui 2016-4-9
 ***************************************************************/
LOCALC os_void _DeRegIcmpReplyFunc(os_u32 seq_no)
{
    struct list_node *i, *_save;
    ICMP_REPLY_FUC_LIST *fuc = OS_NULL;
    loop_del_list(i, _save, &g_icmp_reply_fuc_list.node) {
        fuc = list_addr(i, ICMP_REPLY_FUC_LIST, node);
        if (fuc->rply_func.seq_no == seq_no)
        {
            del_list(&fuc->node);
            kfree(fuc);
            return;
        }
    }
}

/***************************************************************
 * description :
 * history     : sicui 2016-4-9
 ***************************************************************/
LOCALC os_void _OnRecvIcmpEcho(os_void* msg, os_u16 len)
{
    IP_HEAD_STRU *ip_head = OS_NULL;
    ICMP_ECHO_STRU *icmp_head = OS_NULL;
    os_u32 tmp = 0;
    os_u8 * resp = OS_NULL;

    resp = kmalloc(len);
    if (resp == OS_NULL) {
        return;
    }

    mem_cpy(resp, msg, len);
    ip_head = (IP_HEAD_STRU *)resp;
    tmp = ip_head->dst_ip;
    ip_head->dst_ip = ip_head->src_ip;
    ip_head->src_ip = tmp;
    ip_head->h_crc = 0;
    icmp_head = (ICMP_ECHO_STRU*)(ip_head + 1);
    icmp_head->type = ECHO_REPLY;
    icmp_head->check_sum = 0;
    icmp_head->check_sum = ipCksum((os_u16*)icmp_head, (len-20));
    icmp_dbg("send icmp echo reply,resp=%d, len=%d", resp, len);
    IP_SendPacket(resp, SINGLE_CAST);
    kfree(resp);
}

/***************************************************************
 * description :
 * history     : sicui 2016-4-9
 ***************************************************************/
LOCALC os_void _SendIcmpEcho(os_u32 dst_ip, os_u32 src_ip)
{
    ICMP_ECHO_STRU *icmp_msg = OS_NULL;
    IP_HEAD_STRU   *ip_msg = OS_NULL;
    ICMP_REPLY_FUC  reply_fuc;

    ip_msg = (IP_HEAD_STRU*)kmalloc(sizeof(IP_HEAD_STRU) + sizeof(ICMP_ECHO_STRU));
    if (OS_NULL == ip_msg) {
        return;
    }
    mem_set(ip_msg, 0, sizeof(IP_HEAD_STRU));
    ip_msg->ver = 4;
    ip_msg->hlen = (sizeof(IP_HEAD_STRU)/4);
    ip_msg->tos = 48;
    ip_msg->length = hton16(sizeof(IP_HEAD_STRU) + sizeof(ICMP_ECHO_STRU));
    ip_msg->dst_ip = hton32(dst_ip);
    ip_msg->src_ip = hton32(src_ip);
    ip_msg->ttl = 128;
    ip_msg->pro = PRO_ICMP;

    g_sendIcmpSeqNo++;

    icmp_msg = (ICMP_ECHO_STRU *)(ip_msg + 1);
    icmp_msg->type = ECHO;
    icmp_msg->code = 0;
    icmp_msg->id = 0x1234;
    icmp_msg->seq = hton16(g_sendIcmpSeqNo);
    icmp_msg->check_sum = 0;
    // mem_set(icmp_msg->data, 0, sizeof(icmp_msg->data));
    icmp_msg->check_sum = ipCksum((os_u16*)icmp_msg, sizeof(ICMP_ECHO_STRU));
    IP_SendPacket(ip_msg, SINGLE_CAST);
    kfree(ip_msg);

    reply_fuc.seq_no  = g_sendIcmpSeqNo;
    reply_fuc.ip_addr = dst_ip;
    reply_fuc.OnRcvReply = _OnRecvIcmpEchoReply;
    _RegIcmpReplyFunc(&reply_fuc);
    g_ping_task.send_seq = g_sendIcmpSeqNo;
    g_ping_task.send_packet++;
}

/***************************************************************
 * description :
 * history     : sicui 2016-4-9
 ***************************************************************/
LOCALC os_void _OnRecvIcmpEchoReply(os_void *msg)
{
     ICMP_ECHO_STRU *icmp_echo = OS_NULL;
    if (OS_NULL == msg || 0 == g_ping_task.task_flag) {
        print("No Ping Task\n");
        return;
    }

    icmp_echo = (ICMP_ECHO_STRU *)msg;
    print("recv reply from %x: seq=%d(wait seq=%d)\n",g_ping_task.dst_ip, ntoh16(icmp_echo->seq), g_ping_task.send_seq);
    if (ntoh16(icmp_echo->seq) != g_ping_task.send_seq) {
        return;
    }

    g_ping_task.recv_packet++;
    if (g_ping_task.send_packet == g_ping_task.try_times){
        print("Send %d packet, recv %d packet, ping finished!\n", g_ping_task.send_packet, g_ping_task.recv_packet);
        kill_timer(g_ping_task.timer);
        g_ping_task.task_flag = 0;
    }
    else {
        //_SendIcmpEcho(g_ping_task.dst_ip);
        modify_timer(g_ping_task.timer, PING_INTERVAL);
        reset_timer(g_ping_task.timer);
    }
}

/***************************************************************
 * description :
 * history     : sicui 2016-4-9
 ***************************************************************/
LOCALC os_void _OnRecvIcmpReply(os_void *msg)
{
    struct list_node *i, *_save;
    ICMP_REPLY_FUC_LIST *fuc = OS_NULL;
    IP_HEAD_STRU *ip_head = (IP_HEAD_STRU *)msg;
    ICMP_COMM_REPLY_HEAD *rply_head = OS_NULL;
    if (msg == OS_NULL) {
        return;
    }
    rply_head = (ICMP_COMM_REPLY_HEAD *)(ip_head + 1);
    if (rply_head->code != ECHO_REPLY && rply_head->code != TIME_STAMP_REPLY && rply_head->code != INFO_REPLY) {
        return;
    }

    print("_OnRecvIcmpReply:srcip:%x, code=%d, seq=%x\n ", ntoh32(ip_head->src_ip), rply_head->code, ntoh16(rply_head->seq));
    loop_del_list(i, _save, &g_icmp_reply_fuc_list.node) {
        fuc = list_addr(i, ICMP_REPLY_FUC_LIST, node);
        if (fuc->rply_func.ip_addr == ntoh32(ip_head->src_ip) && fuc->rply_func.seq_no == ntoh16(rply_head->seq) && fuc->rply_func.OnRcvReply != OS_NULL)
        {
            fuc->rply_func.OnRcvReply(rply_head);
            del_list(&fuc->node);
            kfree(fuc);
            return;
        }
    }

    icmp_dbg("Recv UnKnow Msg!");
}

/***************************************************************
 * description :
 * history     : sicui 2016-4-9
 ***************************************************************/
os_void ICMP_Init()
{
    init_list_head(&g_icmp_reply_fuc_list.node);
    mem_set(&g_ping_task, 0, sizeof(g_ping_task));
}

/***************************************************************
 * description :
 * history     : sicui 2016-4-9
 ***************************************************************/
os_void ICMP_RecvPacket(os_void* msg, os_u16 len)
{
    IP_HEAD_STRU *ip_head = OS_NULL;
    ICMP_COM_HEAD *icmp_head = OS_NULL;
    if (OS_NULL == msg) {
         return;
    }

    ip_head = (IP_HEAD_STRU *)msg;
    icmp_head = (ICMP_COM_HEAD*)(ip_head + 1);
    icmp_dbg("Recv Icmp Packet:type:%d, code:%d, len=%d", icmp_head->type, icmp_head->code, len);

    switch(icmp_head->type){
        case ECHO_REPLY:
        case TIME_STAMP_REPLY:
        case INFO_REPLY:
            _OnRecvIcmpReply(msg);
            break;
        case DEST_UNRECH:
            break;
        case SOURCE_QUENCE:
            break;
        case REDITECT:
            break;
        case ECHO:
            _OnRecvIcmpEcho(msg, len);
        case TIME_EXCEED:
        case TIME_STAMP:
        case INFO_REQ:
        default:
            break;
    }

    return;

}

/***************************************************************
 * description :
 * history     : sicui 2016-4-9
 ***************************************************************/
LOCALC os_ret OS_CALLBACK _ping_TimeOut(os_u32 event_id)
{
    os_u32 reg;

    print("_ping_TimeOut");
    if (0 == g_ping_task.task_flag) {
        return FAIL;
    }

    _DeRegIcmpReplyFunc(g_sendIcmpSeqNo);

    if (g_ping_task.send_packet == g_ping_task.try_times) {
        print("Send %d packet, recv %d packet, ping finished!\n", g_ping_task.send_packet, g_ping_task.recv_packet);
        kill_timer(g_ping_task.timer);
        g_ping_task.task_flag = 0;
    }
    else {
        modify_timer(g_ping_task.timer, PING_TIMEOUT);
        _SendIcmpEcho(g_ping_task.dst_ip, g_ping_task.src_ip);
    }
}

/***************************************************************
 * description :
 * history     : sicui 2016-4-9
 ***************************************************************/
os_void ping(os_u8* dst_ip, os_u8* src_ip, os_u32 times)
{
    os_u8 i;
    os_u32 reg;
    os_u8* p = OS_NULL;
    os_u8 *s = ".";
    os_u8 ip_buff[3*4 + 3 + 1];
    os_u8 *ip_str;
    os_ret ret;

    if (0 != g_ping_task.task_flag) {
        print("Other Ping Task is on going, please try again leater!\n");
        return;
    }
    mem_set(&g_ping_task, 0, sizeof(g_ping_task));

    ret = str2ipv4(dst_ip, &g_ping_task.dst_ip);
    if (OS_FAIL == ret) {
        return;
    }

    if (src_ip == NULL) {
         g_ping_task.src_ip = 0xFFFFFFFF;
    } else {
        ret = str2ipv4(src_ip, &g_ping_task.src_ip);
        if (OS_FAIL == ret) {
            return;
        }
    }

    print("ping %s, with 32 byte data:\n", dst_ip);
    g_ping_task.task_flag= 1;
    g_ping_task.try_times = times;
    g_ping_task.timer = set_timer_callback(reg, PING_TIMEOUT, _ping_TimeOut, TIMER_MODE_LOOP);
    _SendIcmpEcho(g_ping_task.dst_ip, g_ping_task.src_ip);
}

/***************************************************************
 * description :
 * history     : sicui 2016-4-9
 ***************************************************************/
os_void ping_status(os_void)
{
    print("send = %d, receive = %d, lost = %d\n", g_ping_task.send_packet, g_ping_task.recv_packet, g_ping_task.send_packet - g_ping_task.recv_packet);
}

