/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : config.c
 * version     : 1.0
 * description : wpa2 = rsn, wpa1 = tkip
 *               WPA2 ¡ªThe trade name for an implementation of the 802.11i standard, including AES and CCMP.
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#include <core.h>
#include <net.h>

#include "common.h"
#include "defs.h"
#include "wpa_common.h"
#include "eapol_common.h"

struct wpa_dev *dbg_wpa = NULL;

/**
 * wpa_config_update_psk - Update WPA PSK based on passphrase and SSID
 * @ssid: Pointer to network configuration data
 *
 * This function must be called to update WPA PSK when either SSID or the
 * passphrase has changed for the network configuration.
 */
LOCALC void wpa_config_update_psk(os_void)
{
    uint i;

    /* personal mode, psk = pmk */
    pbkdf2_sha1("cs52941451",
                "shenlanxia", 10,
                4096, dbg_wpa->pmk, 32);

    for (i = 0; i < 32; i++) {
        flog("%x ", dbg_wpa->pmk[i]);
    } flog("psk end\n");
}

LOCALC struct wpa_dev *add_new_wpa_dev(struct net_device *net_dev, u8 *sta_addr, u8 *bssid, u8 *ssid, uint ssid_len, char *passphrase)
{
    struct wpa_dev *dev;

    dev = NULL;

    if ((NULL == sta_addr) || (NULL == bssid)) {
        goto error;
    }

    dev = kmalloc(sizeof(struct wpa_dev));
    if (NULL == dev) {
        wpa_dbg("add new wpa dev fail\n");
        goto error;
    }
    mem_set(dev, 0, sizeof(struct wpa_dev));

    dev->dev = net_dev;
    mem_cpy(dev->station_addr, sta_addr, ETH_ALEN);
    mem_cpy(dev->bssid, bssid, ETH_ALEN);
    if (ssid_len > sizeof(dev->ssid)) {
        goto error;
    }
    mem_cpy(dev->ssid, ssid, ssid_len);
    dev->ssid_len = ssid_len;
    pbkdf2_sha1(passphrase, ssid, ssid_len,
                4096, dev->pmk, PMK_LEN);
    dev->pmk_len = PMK_LEN;
    return dev;
  error:
    if (dev) kfree(dev);
    return NULL;
}

void create_dbg_wpa_dev(struct net_device *dev, u8 *sta_addr, u8 *bssid, u8 *ssid, uint ssid_len, char *passphrase)
{
    wpa_dbg("sta:"MACSTR", bssid:"MACSTR", ssid: %s, pass: %s\n", MAC2STR(sta_addr), MAC2STR(bssid), ssid, passphrase);
    dbg_wpa = add_new_wpa_dev(dev, sta_addr, bssid, ssid, ssid_len, passphrase);
}

os_void init_eapol(os_void)
{
}

os_void recv_eapol_packet(os_u32 dev_id, os_void* l2msg)
{
    os_uint i;
    struct eth_mac_header *hdr;

    hdr = (struct eth_mac_header *) l2msg;
    flog("type:%x\n", hdr->type);
    recv_eapol(dbg_wpa, hdr->src, hdr->payload);
}

