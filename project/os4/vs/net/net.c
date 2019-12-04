
#include <lib.h>
#include <os.h>
#include <core.h>
#include <net.h>

os_void process_packet(os_u32 dev_no, os_u8 *l2packet, os_uint len)
{
    os_uint i;
    struct eth_mac_header *hdr;

    if ((OS_NULL == l2packet) || (0 == len)) {
        return;
    }

    hdr = (struct eth_mac_header *) l2packet;
    flog("process_packet:%x\n", hdr->type);
    Eth_RecvMsgFromPhy(dev_no, hdr, len);
}

