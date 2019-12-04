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
#include <arp.h>
#include <ip_layer.h>
#include <eth_interface.h>
#include <net.h>

/***************************************************************
 global variable declare
 ***************************************************************/
typedef struct
{
   os_u32  protocol_type;
   os_void (*init)(os_void);
   os_void (*OnRcvPacket)(os_u32 port_no, os_void* l2msg);
   os_u32  rcv_pack_num;
   os_u32  send_pack_num;
}ETH_UPER_PROTOCOL_STRU;

os_void init_eapol(os_void);
os_void recv_eapol_packet(os_u32 dev_id, os_void* msg);

ETH_UPER_PROTOCOL_STRU g_eth_upper_protocol[] =
{
    { PRO_ARP, Arp_Init, Arp_RecvPacket, 0, 0 },
    { PRO_IP,  IP_Init,  IP_RecvPacket,  0, 0 },
    { PRO_EAPOL, init_eapol, recv_eapol_packet, 0, 0 }
};

#define MAX_MSG_QUEUE_LEN  1024
#define MAX_PACKET_LEN     1500

typedef struct
{
    os_u32 len;
    os_u8  content[MAX_PACKET_LEN];
}MSG_STRU;

typedef struct
{
    os_u32 head;
    os_u32 tail;
    MSG_STRU msgQ[MAX_MSG_QUEUE_LEN];
}MSG_QUEUE;

typedef struct
{
    ETH_DEV_INFO_STRU dev_info;
    MSG_QUEUE msg_queue;
    struct list_node node;
    /* double locks */
    spinlock_t rx_lock;
    spinlock_t tx_lock;
}ETH_DEVINFO_LIST;

ETH_DEVINFO_LIST g_eth_phydev_list;
LOCALD HEVENT Eth_sem_id = 0;

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dump_ethItf(os_void)
{
    struct list_node *node;
    ETH_DEVINFO_LIST *device;

    loop_list(node, &g_eth_phydev_list.node) {
        device = list_addr(node, ETH_DEVINFO_LIST, node);
        if (device != OS_NULL) {
            print("devNo:%x\n", device->dev_info.dev_no);
            print("mac: %x:%x:%x:%x:%x:%x\n", device->dev_info.mac_addr[0],device->dev_info.mac_addr[1],device->dev_info.mac_addr[2],device->dev_info.mac_addr[3],device->dev_info.mac_addr[4],device->dev_info.mac_addr[5]);
        }
    }
}

LOCALD os_u8 eth_debug_name[] = { "ethitf" };
LOCALD struct dump_info EthItf_debug = {
    eth_debug_name,
    dump_ethItf
};
/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
LOCALC os_ret _Eth_DispatchMsg(os_u32 dev_no, os_void* msg)
{
    os_u32 i;
    ETH_HEAD_STRU *eth_head = OS_NULL;
    if (msg == OS_NULL) {
        return OS_FAIL;
    }

    eth_head = (ETH_HEAD_STRU *)msg;
    eth_dbg("Eth RecvMsgFromPhy:type=%x\n", ntoh16(eth_head->type));

    for (i = 0; i < sizeof(g_eth_upper_protocol)/sizeof(ETH_UPER_PROTOCOL_STRU); i++) {
        if (ntoh16(eth_head->type) == g_eth_upper_protocol[i].protocol_type && g_eth_upper_protocol[i].OnRcvPacket != OS_NULL) {
            g_eth_upper_protocol[i].OnRcvPacket(dev_no, eth_head);
            g_eth_upper_protocol[i].rcv_pack_num++;
            return OS_SUCC;
        }
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
LOCALC os_ret OS_CALLBACK _Eth_RecvTaskProc(os_u32 arg1, os_u32 arg2, os_u32 arg3, os_u32 arg4, os_u32 arg5, os_u32 arg6, os_u32 arg7)
{
    os_void *msg = OS_NULL;
    struct list_node *i;
    ETH_DEVINFO_LIST *device = OS_NULL;
    MSG_QUEUE *msg_q = OS_NULL;

    /* ÏûÏ¢Ñ­»· */
    while (1) {
        wait_event(Eth_sem_id, 0);
        spin_lock(&g_eth_phydev_list.rx_lock);
        loop_list(i, &g_eth_phydev_list.node) {
            device = list_addr(i, ETH_DEVINFO_LIST, node);
            msg_q = &device->msg_queue;
            while (msg_q->head != msg_q->tail) {
                msg = (os_void*)msg_q->msgQ[msg_q->head].content;
                _Eth_DispatchMsg(device->dev_info.dev_no, msg);
                msg_q->head = (msg_q->head + 1) % MAX_MSG_QUEUE_LEN;
                eth_dbg("_ETH_GetMsg:port_no:%d, MsgQ head = 0x%x\n", device->dev_info.dev_no, msg_q->head);
            }
        }
        spin_unlock(&g_eth_phydev_list.rx_lock);
    }

    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_wifi(os_u32 argc, os_u8 *argv[])
{
    struct list_node *i;
    ETH_DEVINFO_LIST *device;
    os_u32 id;
    os_u32 ip, mask;
    os_ret ret;

    if (0 == argc) {
        print("command example:\n");
        print("    wifi <id> conn <ssid> <password>\n");
        print("    wifi <id> disc\n");
        print("    wifi <id> ip <xxx.xxx.xxx.xxx> <255.255.255.0>\n");
        print("    wifi devices:\n");

        spin_lock(&g_eth_phydev_list.tx_lock);
        spin_lock(&g_eth_phydev_list.rx_lock);
        loop_list(i, &g_eth_phydev_list.node) {
            device = list_addr(i, ETH_DEVINFO_LIST, node);
            print("      device id:%d, mac:%x:%x:%x:%x:%x:%x ", device->dev_info.dev_no, device->dev_info.mac_addr[0], device->dev_info.mac_addr[1], device->dev_info.mac_addr[2], device->dev_info.mac_addr[3], device->dev_info.mac_addr[4], device->dev_info.mac_addr[5]);
            if (OS_SUCC == get_ipv4_address(device->dev_info.dev_no, &ip, &mask)) {
                print("ip: %d.%d.%d.%d, %d.%d.%d.%d ", (ip >> 24) & 0xff, (ip >> 16) & 0xff, (ip >> 8) & 0xff, (ip >> 0) & 0xff, (mask >> 24) & 0xff, (mask >> 16) & 0xff, (mask >> 8) & 0xff, (mask >> 0) & 0xff);
            } else {
                print("ip: none ");
            }
            print("\n");
        }
        spin_unlock(&g_eth_phydev_list.tx_lock);
        spin_unlock(&g_eth_phydev_list.rx_lock);
        return OS_SUCC;
    }

    id = str2num(argv[0]);
    if ((0 == strcmp(argv[1], "ip")) && (4 == argc)) {
        /* parse ip address */
        ret = str2ipv4(argv[2], &ip);
        if (OS_FAIL == ret) {
            return OS_FAIL;
        }
        /* parse ip mask */
        ret = str2ipv4(argv[3], &mask);
        if (OS_FAIL == ret) {
            return OS_FAIL;
        }
        IP_AddLocalIPAddr(id, ip, mask);
        return OS_SUCC;
    }

    if ((0 == strcmp(argv[1], "conn")) && (4 == argc)) {
        spin_lock(&g_eth_phydev_list.tx_lock);
        spin_lock(&g_eth_phydev_list.rx_lock);
        loop_list(i, &g_eth_phydev_list.node) {
            device = list_addr(i, ETH_DEVINFO_LIST, node);
            if (id == device->dev_info.dev_no) {
                device->dev_info.connect(device->dev_info.dedicated, argv[2], argv[3]);
                spin_unlock(&g_eth_phydev_list.tx_lock);
                spin_unlock(&g_eth_phydev_list.rx_lock);
                return OS_SUCC;
            }
        }
        spin_unlock(&g_eth_phydev_list.tx_lock);
        spin_unlock(&g_eth_phydev_list.rx_lock);
    }

    if (0 == strcmp(argv[1], "disc")) {
        spin_lock(&g_eth_phydev_list.tx_lock);
        spin_lock(&g_eth_phydev_list.rx_lock);
        loop_list(i, &g_eth_phydev_list.node) {
            device = list_addr(i, ETH_DEVINFO_LIST, node);
            if (id == device->dev_info.dev_no) {
                device->dev_info.disconnect(device->dev_info.dedicated);
                spin_unlock(&g_eth_phydev_list.tx_lock);
                spin_unlock(&g_eth_phydev_list.rx_lock);
                return OS_SUCC;
            }
        }
        spin_unlock(&g_eth_phydev_list.tx_lock);
        spin_unlock(&g_eth_phydev_list.rx_lock);
    }
    print("wrong parameters\n");
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
os_void Eth_Init(void)
{
    HTASK handle;
    os_u32 i;

    for (i = 0; i < array_size(g_eth_upper_protocol); i++) {
        if (g_eth_upper_protocol[i].init != OS_NULL) {
            g_eth_upper_protocol[i].init();
        }
    }
    init_list_head(&g_eth_phydev_list.node);
    init_spinlock(&g_eth_phydev_list.rx_lock);
    init_spinlock(&g_eth_phydev_list.tx_lock);
    (void)register_dump(&EthItf_debug);

    Eth_sem_id = create_event_handle(EVENT_INVALID, "eth", __LINE__);
    cassert(OS_NULL != Eth_sem_id);

    handle = create_task("protocol", _Eth_RecvTaskProc, TASK_PRIORITY_3,  0, 1, 0, 0, 4, 1, 2);
    cassert(OS_NULL != handle);

    register_cmd("wifi", cmd_wifi);
}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
os_ret Eth_RecvMsgFromPhy(os_u32 port_no, os_void* l2msg, os_u32 len)
{
    MSG_QUEUE *msg_q = OS_NULL;
    struct list_node *i;
    ETH_DEVINFO_LIST *device = OS_NULL;

    flog("Eth_RecvMsgFromPhy\n");
    spin_lock(&g_eth_phydev_list.rx_lock);
    loop_list(i, &g_eth_phydev_list.node) {
        device = list_addr(i, ETH_DEVINFO_LIST, node);
        if (device->dev_info.dev_no == port_no)
        {
            msg_q = &device->msg_queue;
            break;
        }
    }

    if ((OS_NULL == l2msg) || (OS_NULL == msg_q) || (len > MAX_PACKET_LEN)) {
        flog("msg is null %d %d %d\n", l2msg, msg_q, len);
        spin_unlock(&g_eth_phydev_list.rx_lock);
        return OS_FAIL;
    }

    if((msg_q->tail + 1) % MAX_MSG_QUEUE_LEN == msg_q->head){
        eth_dbg("port_no:%d, Msg Q is Full!", port_no);
        spin_unlock(&g_eth_phydev_list.rx_lock);
        return OS_FAIL;
    }

    msg_q->msgQ[msg_q->tail].len = len;
    mem_cpy(&msg_q->msgQ[msg_q->tail].content, l2msg, len);
    msg_q->tail = (msg_q->tail + 1) % MAX_MSG_QUEUE_LEN;
    notify_event(Eth_sem_id, __LINE__);
    spin_unlock(&g_eth_phydev_list.rx_lock);

    eth_dbg("RecvMsgFromPhy:port_no:%d,  MsgQ tail = 0x%x\n", port_no, msg_q->tail);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
os_ret Eth_SendMsgToPhy(os_void* msg, os_u32 len)
{
    struct list_node *i;
    ETH_DEVINFO_LIST *device = OS_NULL;
    ETH_HEAD_STRU *eth_head = OS_NULL;

    if (msg == OS_NULL || len == 0) {
        return OS_FAIL;
    }
    eth_head = (ETH_HEAD_STRU *)msg;
    /* choose the sendout device */
    spin_lock(&g_eth_phydev_list.tx_lock);
    loop_list(i, &g_eth_phydev_list.node) {
        device = list_addr(i, ETH_DEVINFO_LIST, node);
        if (mem_cmp(eth_head->src_mac, device->dev_info.mac_addr, sizeof(device->dev_info.mac_addr))) {
            eth_dbg("Eth_SendMsgToPhy:pro_type(%d),len(%d)", ntoh16(eth_head->type), len);
            eth_dbg("dst_mac: %x:%x:%x:%x:%x:%x\n", eth_head->dst_mac[0],eth_head->dst_mac[1],eth_head->dst_mac[2],eth_head->dst_mac[3],eth_head->dst_mac[4],eth_head->dst_mac[5]);
            eth_dbg("src_mac: %x:%x:%x:%x:%x:%x\n", eth_head->src_mac[0],eth_head->src_mac[1],eth_head->src_mac[2],eth_head->src_mac[3],eth_head->src_mac[4],eth_head->src_mac[5]);
            print("send packet id %d\n", device->dev_info.dev_no);
            device->dev_info.SendPacket(device->dev_info.dedicated, msg, len);
            spin_unlock(&g_eth_phydev_list.tx_lock);
            return OS_SUCC;
        }
    }
    spin_unlock(&g_eth_phydev_list.tx_lock);
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
LOCALC os_bool config_is_equal(os_u8 *dst, os_u8 *src)
{
    os_uint i;

    for (i = 0; dst[i] == src[i]; i++) {
        if ('\0' == src[i]) {
            return OS_TRUE;
        }
    }
    if (('\0' == src[i]) && (('\n' == dst[i]) || ('\r' == dst[i]))) {
        /* equal */
        return OS_TRUE;
    }
    return OS_FALSE;
}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
LOCALC os_bool copy_config(os_u8 *dst, os_u8 *src)
{
    os_uint i;

    i = 0;
    while ((src[i] != '\0') && (src[i] != '\n') && (src[i] != '\r')) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

#define CFG_FLAG "{item}"

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
LOCALC os_ret parse_config(os_u8 *file_name, os_u8 *key, os_u8 *value, os_uint count, ...)
{
    os_ret ret;
    os_u32 id;
    HFILE fp;
    os_u8 *buff, *item;
    os_u32 size;
    va_list args;
    s32 start_next[sizeof(CFG_FLAG)];
    s32 *n;

    ret = OS_SUCC;
    buff = OS_NULL;
    fp = OS_NULL;

    id = curr_disk_device_id();
    fp = open_file(id, file_name);
    if (OS_NULL == fp) {
        ret = OS_FAIL;
        goto fail;
    }
    seek_file(fp, 0, SEEK_POS_SET);
    size = size_of_file(fp);
    buff = kmalloc(size + 1);
    if (OS_NULL == buff) {
        ret = OS_FAIL;
        goto fail;
    }
    read_file(fp, buff, size);
    buff[size] = '\0';
    item = buff;
    kmp_next(CFG_FLAG, start_next);
    for (;;) {
        /* lookup start flag */
        ret = kmp(item, CFG_FLAG, start_next);
        if (-1 == ret) {
            eth_dbg("no item\n");
            ret = OS_FAIL;
            goto fail;
        }
        item += (ret + sizeof(CFG_FLAG));

        /* lookup main key */
        size = strlen(key);
        n = kmalloc(size * sizeof(s32));
        if (!n) {
            ret = OS_FAIL;
            goto fail;
        }
        kmp_next(key, n);
        ret = kmp(item, key, n);
        kfree(n);
        if (-1 == ret) {
            eth_dbg("no %s key\n", key);
            continue;
        }
        item += (ret + size);
        if (config_is_equal(item, value)) {
            va_start(args, count);
            while (count--) {
                os_u8 *k, *d;
                os_u32 pos;
                k = (os_u8 *) va_arg(args, os_u32);
                d = (os_u8 *) va_arg(args, os_u32);
                ret = lookup_string(item, k, &pos);
                if (OS_FAIL == ret) {
                    break;
                }
                copy_config(d, item + pos + str_len(k));
            }
            va_end(args);
        }
    }

  fail:
    if (buff) kfree(buff);
    if (fp) close_file(fp);
    return ret;
}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
os_ret Eth_RegDevice(ETH_DEV_INFO_STRU *device)
{
    ETH_DEVINFO_LIST *new_dev = OS_NULL;
    os_ret ret;
    os_u8 mac[6*2+5+1];

    if (device == OS_NULL)
    {
        return OS_FAIL;
    }
    new_dev = kmalloc(sizeof(ETH_DEVINFO_LIST));
    if (new_dev == OS_NULL)
    {
        return OS_FAIL;
    }
    new_dev->msg_queue.head = 0;
    new_dev->msg_queue.tail = 0;
    mem_cpy(&new_dev->dev_info, device, sizeof(ETH_DEV_INFO_STRU));
    spin_lock(&g_eth_phydev_list.rx_lock);
    spin_lock(&g_eth_phydev_list.tx_lock);
    add_list_tail(&g_eth_phydev_list.node, &new_dev->node);
    spin_unlock(&g_eth_phydev_list.tx_lock);
    spin_unlock(&g_eth_phydev_list.rx_lock);

    /* check configuration */
    do {
        os_u32 ip, mask;
        os_u8 ip_buf[12+3+1];
        os_u8 mask_buf[12+3+1];

        mac[array_size(mac) - 1] = '\0';
        snprintf(mac, array_size(mac), "%2x:%2x:%2x:%2x:%2x:%2x", device->mac_addr[0],device->mac_addr[1],device->mac_addr[2],device->mac_addr[3],device->mac_addr[4],device->mac_addr[5]);
        parse_config("ipconfig.txt", "[mac]", mac, 2, "<ip>", ip_buf, "<mask>", mask_buf);
        ret = str2ipv4(ip_buf, &ip);
        if (OS_FAIL == ret) {
            return OS_FAIL;
        }
        ret = str2ipv4(mask_buf, &mask);
        if (OS_FAIL == ret) {
            return OS_FAIL;
        }
        IP_AddLocalIPAddr(device->dev_no, ip, mask);
    } while (0);

    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     : sicui 2016-3-27
 ***************************************************************/
os_void Eth_DeRegDevice(os_u32 device_no)
{
    struct list_node *i;
    ETH_DEVINFO_LIST *device = OS_NULL;

    spin_lock(&g_eth_phydev_list.rx_lock);
    spin_lock(&g_eth_phydev_list.tx_lock);
    loop_list(i, &g_eth_phydev_list.node) {
        device = list_addr(i, ETH_DEVINFO_LIST, node);
        if (device->dev_info.dev_no == device_no)
        {
            del_list(&device->node);
            spin_unlock(&g_eth_phydev_list.tx_lock);
            spin_unlock(&g_eth_phydev_list.rx_lock);
            kfree(device);
            return;
        }
    }
    spin_unlock(&g_eth_phydev_list.tx_lock);
    spin_unlock(&g_eth_phydev_list.rx_lock);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_u8* Eth_GetItfMacAddr(os_u32 device_no)
{
    struct list_node *i;
    ETH_DEVINFO_LIST *device = OS_NULL;

    spin_lock(&g_eth_phydev_list.rx_lock);
    spin_lock(&g_eth_phydev_list.tx_lock);
    loop_list(i, &g_eth_phydev_list.node) {
        device = list_addr(i, ETH_DEVINFO_LIST, node);
        if (device != OS_NULL && device->dev_info.dev_no == device_no)
        {
            spin_unlock(&g_eth_phydev_list.tx_lock);
            spin_unlock(&g_eth_phydev_list.rx_lock);
            return device->dev_info.mac_addr;
        }
    }
    spin_unlock(&g_eth_phydev_list.tx_lock);
    spin_unlock(&g_eth_phydev_list.rx_lock);
    return OS_NULL;
}

