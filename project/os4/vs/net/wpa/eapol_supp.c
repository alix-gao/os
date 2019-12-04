
#include <lib.h>
#include <core.h>
#include "defs.h"
#include "common.h"
#include "eap_defs.h"
#include "wpa_common.h"
#include "eapol_common.h"
#include "wpa_ie.h"

os_void wpa_hexdump(os_uint level, os_u8 *title, os_u8 *buf, os_uint len)
{
    os_uint i;

    wpa_dbg("%s\n", title);
    for (i = 0; i < len; i++) {
        wpa_dbg("%x ", buf[i]);
    } wpa_dbg("\n");
}

static u8 * wpa_alloc_eapol(u8 type,
			    u8 *data, u16 data_len,
			    u32 *msg_len, void **data_pos)
{
	struct ieee802_1x_hdr *hdr;

	*msg_len = sizeof(*hdr) + data_len;
	hdr = kmalloc(*msg_len);
	if (hdr == NULL)
		return NULL;

	hdr->version = DEFAULT_EAPOL_VERSION;
	hdr->type = type;
	hdr->length = net_host_16(data_len);

	if (data)
		mem_cpy(hdr + 1, data, data_len);
	else
		mem_set(hdr + 1, 0, data_len);

	if (data_pos)
		*data_pos = hdr + 1;

	return (u8 *) hdr;
}

/**
 * wpa_supplicant_eapol_send - Send IEEE 802.1X EAPOL packet to Authenticator
 * @ctx: Pointer to wpa_supplicant data (wpa_s)
 * @type: IEEE 802.1X packet type (IEEE802_1X_TYPE_*)
 * @buf: EAPOL payload (after IEEE 802.1X header)
 * @len: EAPOL payload length
 * Returns: >=0 on success, <0 on failure
 *
 * This function adds Ethernet and IEEE 802.1X header and sends the EAPOL frame
 * to the current Authenticator.
 */
static int wpa_supplicant_eapol_send(struct wpa_dev *dev, int type, u8 *buf, u32 len)
{
    u8 *msg, *dst;
    u32 msglen;
    int res;

    cassert(OS_NULL != dev);

    msg = wpa_alloc_eapol(type, buf, len, &msglen, NULL);
    if (msg == NULL)
    	return -1;

// TODO:    res = wpa_ether_send(dst, ETH_P_EAPOL, msg, msglen);
    res = issue_eapol_data(dev->dev, msg, msglen);
    kfree(msg);
    return res;
}

/**
 * wpa_eapol_key_send - Send WPA/RSN EAPOL-Key message
 * @sm: Pointer to WPA state machine data from wpa_sm_init()
 * @kck: Key Confirmation Key (KCK, part of PTK)
 * @ver: Version field from Key Info
 * @dest: Destination address for the frame
 * @proto: Ethertype (usually ETH_P_EAPOL)
 * @msg: EAPOL-Key message
 * @msg_len: Length of message
 * @key_mic: Pointer to the buffer to which the EAPOL-Key MIC is written
 */
void wpa_eapol_key_send(struct wpa_dev *dev, u8 *kck,
        int ver, u8 *dest, u16 proto,
        u8 *msg, size_t msg_len, u8 *key_mic)
{
    if (is_zero_ether_addr(dest)) {
        /*
         * Association event was not yet received; try to fetch
         * BSSID from the driver.
         */
        wpa_dbg("WPA: Failed to read BSSID for EAPOL-Key destination address");
    }

    if (key_mic && wpa_eapol_key_mic(kck, ver, msg, msg_len, key_mic)) {
        wpa_dbg("WPA: Failed to generate EAPOL-Key version %d MIC", ver);
        goto out;
    }
    wpa_hexdump(MSG_DEBUG, "WPA: KCK", kck, 16);
    wpa_hexdump(MSG_DEBUG, "WPA: Derived Key MIC", key_mic, 16);
    wpa_hexdump(MSG_MSGDUMP, "WPA: TX EAPOL-Key", msg, msg_len);
// TODO:    wpa_sm_ether_send(sm, dest, proto, msg, msg_len);
// TODO:   eapol_sm_notify_tx_eapol_key(sm->eapol);
    issue_eapol_data(dev->dev, msg, msg_len);
out:
    kfree(msg);
}

void eapol_sm_txStart(void)
{
    wpa_supplicant_eapol_send(OS_NULL, IEEE802_1X_TYPE_EAPOL_START, (u8 *) "", 0);
}

struct ieee802_1x_eapol_key {
	u8 type;
	u16 key_length;
	u8 replay_counter[8]; /* does not repeat within the life of the keying
			       * material used to encrypt the Key field;
			       * 64-bit NTP timestamp MAY be used here */
	u8 key_iv[16]; /* cryptographically random number */
	u8 key_index; /* key flag in the most significant bit:
		       * 0 = broadcast (default key),
		       * 1 = unicast (key mapping key); key index is in the
		       * 7 least significant bits */
	u8 key_signature[16]; /* HMAC-MD5 message integrity check computed with
			       * MS-MPPE-Send-Key as the key */

	/* followed by key: if packet body length = 44 + key length, then the
	 * key field (of key_length bytes) contains the key in encrypted form;
	 * if packet body length = 44, key field is absent and key_length
	 * represents the number of least significant octets from
	 * MS-MPPE-Send-Key attribute to be used as the keying material;
	 * RC4 key used in encryption = Key-IV + MS-MPPE-Recv-Key */
};

static int wpa_derive_ptk(struct wpa_dev *dev, const unsigned char *src_addr,
                          struct wpa_eapol_key *key,
                          struct wpa_ptk *ptk, os_uint ptk_len)
{
    wpa_pmk_to_ptk(dev->pmk, dev->pmk_len, "Pairwise key expansion",
                   dev->station_addr, dev->bssid, dev->snonce, key->key_nonce,
                    (u8 *) ptk, ptk_len);
	return 0;
}

int random_get_bytes(u8 *buf, size_t len)
{
    os_uint i;

    for (i = 0; i < len; i++) {
        buf[i] = '0' + i;
    }
    return OS_SUCC;
}

/**
 * wpa_supplicant_send_2_of_4 - Send message 2 of WPA/RSN 4-Way Handshake
 * @sm: Pointer to WPA state machine data from wpa_sm_init()
 * @dst: Destination address for the frame
 * @key: Pointer to the EAPOL-Key frame header
 * @ver: Version bits from EAPOL-Key Key Info
 * @nonce: Nonce value for the EAPOL-Key frame
 * @wpa_ie: WPA/RSN IE
 * @wpa_ie_len: Length of the WPA/RSN IE
 * @ptk: PTK to use for keyed hash and encryption
 * Returns: 0 on success, -1 on failure
 */
int wpa_supplicant_send_2_of_4(struct wpa_dev *dev, unsigned char *dst,
            struct wpa_eapol_key *key,
            int ver, u8 *nonce,
            u8 *wpa_ie, size_t wpa_ie_len,
            struct wpa_ptk *ptk)
{
    u32 rlen;
    struct wpa_eapol_key *reply;
    u8 *rbuf;

    if (wpa_ie == NULL) {
        wpa_dbg("WPA: No wpa_ie set - cannot generate msg 2/4");
        return -1;
    }

    wpa_hexdump(MSG_DEBUG, "WPA: WPA IE for msg 2/4", wpa_ie, wpa_ie_len);

    rbuf = wpa_alloc_eapol(IEEE802_1X_TYPE_EAPOL_KEY,
            NULL, sizeof(*reply) + wpa_ie_len,
            &rlen, (void *) &reply);
    if (rbuf == NULL) {
        return -1;
    }

    reply->type = key->type == WPA_PROTO_RSN ? EAPOL_KEY_TYPE_RSN : EAPOL_KEY_TYPE_WPA;
    WPA_PUT_BE16(reply->key_info, ver | WPA_KEY_INFO_KEY_TYPE | WPA_KEY_INFO_MIC);
    if (key->type == WPA_PROTO_RSN)
        WPA_PUT_BE16(reply->key_length, 0);
    else
        mem_cpy(reply->key_length, key->key_length, 2);
    mem_cpy(reply->replay_counter, key->replay_counter, WPA_REPLAY_COUNTER_LEN);
    wpa_hexdump(MSG_DEBUG, "WPA: Replay Counter", reply->replay_counter, WPA_REPLAY_COUNTER_LEN);

    WPA_PUT_BE16(reply->key_data_length, wpa_ie_len);
    mem_cpy(reply + 1, wpa_ie, wpa_ie_len);

    mem_cpy(reply->key_nonce, nonce, WPA_NONCE_LEN);

    wpa_dbg("WPA: Sending EAPOL-Key 2/4");
    wpa_eapol_key_send(dev, ptk->kck, ver, dst, ETH_P_EAPOL,
        rbuf, rlen, reply->key_mic);

    return 0;
}

LOCALC os_void wpa_supplicant_process_1_of_4(struct wpa_dev *dev, unsigned char *src_addr, struct wpa_eapol_key *key, u16 ver)
{
    struct wpa_eapol_ie_parse ie;
    u8 buf[8];
    int res;

    wpa_dbg("WPA: RX message 1 of 4-Way Handshake from " MACSTR " (ver=%d)\n", MAC2STR(src_addr), ver);

    mem_set(&ie, 0, sizeof(ie));

    if (key->type == WPA_PROTO_RSN) {
        /* RSN: msg 1/4 should contain PMKID for the selected PMK */
        u8 *_buf = (u8 *) (key + 1);
        size_t len = WPA_GET_BE16(key->key_data_length);
        wpa_hexdump(MSG_DEBUG, "RSN: msg 1/4 key data", _buf, len);
        wpa_supplicant_parse_ies(_buf, len, &ie);
        if (ie.pmkid) {
            wpa_dbg("RSN: PMKID from Authenticator", ie.pmkid, PMKID_LEN);
        }
    }

    if (random_get_bytes(dev->snonce, WPA_NONCE_LEN)) {
        wpa_dbg("WPA: Failed to get random data for SNonce\n");
        goto failed;
    }
    wpa_hexdump(MSG_DEBUG, "WPA: Renewed SNonce", dev->snonce, WPA_NONCE_LEN);

    /* Calculate PTK which will be stored as a temporary PTK until it has been verified when processing message 3/4. */
    wpa_derive_ptk(dev, src_addr, key,
                   &dev->ptk, ver == WPA_KEY_INFO_TYPE_HMAC_SHA1_AES ? 48 : 64);
    /* Supplicant: swap tx/rx Mic keys */
    mem_cpy(buf, dev->ptk.u.auth.tx_mic_key, 8);
    mem_cpy(dev->ptk.u.auth.tx_mic_key, dev->ptk.u.auth.rx_mic_key, 8);
    mem_cpy(dev->ptk.u.auth.rx_mic_key, buf, 8);
extern os_u8 security_ie[0x100];
extern os_uint security_ie_len;
    if (wpa_supplicant_send_2_of_4(dev, dev->bssid, key, ver, dev->snonce,
            security_ie, security_ie_len,
            &dev->ptk))
        goto failed;

    return;
failed:
    return;// TODO: wpa_sm_deauthenticate(sm, WLAN_REASON_UNSPECIFIED);
}

LOCALC os_void handle_eap_packet(struct wpa_dev *dev, os_u8 *payload, os_uint len)
{
    struct eap_hdr *hdr;
    os_u8 id_response[0x10];

    hdr = (struct eap_hdr *) payload;
    wpa_dbg(" code: %d\n id: %d\n len: %d\n type: %d\n", hdr->code, hdr->identifier, net_host_16(hdr->length), hdr->type);
    switch (hdr->type) {
    case EAP_TYPE_IDENTITY:
        mem_cpy(id_response, hdr, sizeof(struct eap_hdr));
        hdr = (struct eap_hdr *) id_response;
        hdr->code = EAP_CODE_RESPONSE;
#define EAP_ID_NAME "tempdev"
        mem_cpy(id_response + sizeof(struct eap_hdr), EAP_ID_NAME, str_len(EAP_ID_NAME) + 1);
        hdr->length += str_len(EAP_ID_NAME) + 1;
        wpa_supplicant_eapol_send(dev, IEEE802_1X_TYPE_EAP_PACKET, id_response, len + str_len(EAP_ID_NAME));
        break;
    default:
        wpa_dbg("type: %x\n", hdr->type);
        break;
    }
}

/**
 * set_key - Configure encryption key
 * @ifname: Interface name (for multi-SSID/VLAN support)
 * @priv: private driver interface data
 * @alg: encryption algorithm (%WPA_ALG_NONE, %WPA_ALG_WEP,
 *	%WPA_ALG_TKIP, %WPA_ALG_CCMP, %WPA_ALG_IGTK, %WPA_ALG_PMK);
 *	%WPA_ALG_NONE clears the key.
 * @addr: Address of the peer STA (BSSID of the current AP when setting
 *	pairwise key in station mode), ff:ff:ff:ff:ff:ff for
 *	broadcast keys, %NULL for default keys that are used both for
 *	broadcast and unicast; when clearing keys, %NULL is used to
 *	indicate that both the broadcast-only and default key of the
 *	specified key index is to be cleared
 * @key_idx: key index (0..3), usually 0 for unicast keys; 0..4095 for
 *	IGTK
 * @set_tx: configure this key as the default Tx key (only used when
 *	driver does not support separate unicast/individual key
 * @seq: sequence number/packet number, seq_len octets, the next
 *	packet number to be used for in replay protection; configured
 *	for Rx keys (in most cases, this is only used with broadcast
 *	keys and set to zero for unicast keys); %NULL if not set
 * @seq_len: length of the seq, depends on the algorithm:
 *	TKIP: 6 octets, CCMP: 6 octets, IGTK: 6 octets
 * @key: key buffer; TKIP: 16-byte temporal key, 8-byte Tx Mic key,
 *	8-byte Rx Mic Key
 * @key_len: length of the key buffer in octets (WEP: 5 or 13,
 *	TKIP: 32, CCMP: 16, IGTK: 16)
 *
 * Returns: 0 on success, -1 on failure
 *
 * Configure the given key for the kernel driver. If the driver
 * supports separate individual keys (4 default keys + 1 individual),
 * addr can be used to determine whether the key is default or
 * individual. If only 4 keys are supported, the default key with key
 * index 0 is used as the individual key. STA must be configured to use
 * it as the default Tx key (set_tx is set) and accept Rx for all the
 * key indexes. In most cases, WPA uses only key indexes 1 and 2 for
 * broadcast keys, so key index 0 is available for this kind of
 * configuration.
 *
 * Please note that TKIP keys include separate TX and RX MIC keys and
 * some drivers may expect them in different order than wpa_supplicant
 * is using. If the TX/RX keys are swapped, all TKIP encrypted packets
 * will trigger Michael MIC errors. This can be fixed by changing the
 * order of MIC keys by swapping te bytes 16..23 and 24..31 of the key
 * in driver_*.c set_key() implementation, see driver_ndis.c for an
 * example on how this can be done.
 */
static inline int wpa_sm_set_key(struct wpa_dev *dev,
    enum wpa_alg alg,
    const u8 *addr, int key_idx, int set_tx,
    const u8 *seq, size_t seq_len,
    const u8 *key, size_t key_len)
{
    cfg80211_rtw_add_key(dev->dev, alg, addr, key_idx, seq, seq_len, key, key_len);
    return 0;
}

static int wpa_supplicant_install_ptk(struct wpa_dev *dev, struct wpa_eapol_key *key)
{
    int keylen, rsclen;
    enum wpa_alg alg;
    u8 *key_rsc;
    u8 null_rsc[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    wpa_dbg("WPA: Installing PTK to the driver\n");

    switch (dev->wpa_cfg.pairwise_cipher) {
    case WPA_CIPHER_CCMP:
        alg = WPA_ALG_CCMP;
        keylen = 16;
        rsclen = 6;
        break;
    case WPA_CIPHER_TKIP:
        alg = WPA_ALG_TKIP;
        keylen = 32;
        rsclen = 6;
        break;
    case WPA_CIPHER_NONE:
        wpa_dbg("WPA: Pairwise Cipher Suite: NONE - do not use pairwise keys\n");
        return 0;
    default:
        wpa_dbg("WPA: Unsupported pairwise cipher %d\n", dev->wpa_cfg.pairwise_cipher);
        return -1;
    }

    if (dev->wpa_cfg.proto == WPA_PROTO_RSN) {
        key_rsc = null_rsc;
    } else {
        key_rsc = key->key_rsc;
        wpa_hexdump(MSG_DEBUG, "WPA: RSC", key_rsc, rsclen);
    }

    if (wpa_sm_set_key(dev, alg, dev->bssid, 0, 1, key_rsc, rsclen, (u8 *) dev->ptk.tk1, keylen) < 0) {
        wpa_dbg("WPA: Failed to set PTK to the driver (alg=%d keylen=%d bssid=" MACSTR ")\n", alg, keylen, MAC2STR(dev->bssid));
        return -1;
    }

    return 0;
}

struct wpa_gtk_data {
    enum wpa_alg alg;
    int tx, key_rsc_len, keyidx;
    u8 gtk[32];
    int gtk_len;
};

static int wpa_supplicant_gtk_tx_bit_workaround(struct wpa_dev *dev, int tx)
{
    if (tx && dev->wpa_cfg.pairwise_cipher != WPA_CIPHER_NONE) {
        /* Ignore Tx bit for GTK if a pairwise key is used. One AP
         * seemed to set this bit (incorrectly, since Tx is only when
         * doing Group Key only APs) and without this workaround, the
         * data connection does not work because wpa_supplicant
         * configured non-zero keyidx to be used for unicast. */
        wpa_dbg("WPA: Tx bit set for GTK, but pairwise keys are used - ignore Tx bit");
        return 0;
    }
    return tx;
}

static int wpa_supplicant_check_group_cipher(int group_cipher, int keylen, int maxkeylen, int *key_rsc_len, enum wpa_alg *alg)
{
    int ret = 0;

    switch (group_cipher) {
    case WPA_CIPHER_CCMP:
        if (keylen != 16 || maxkeylen < 16) {
            ret = -1;
            break;
        }
        *key_rsc_len = 6;
        *alg = WPA_ALG_CCMP;
        break;
    case WPA_CIPHER_TKIP:
        if (keylen != 32 || maxkeylen < 32) {
            ret = -1;
            break;
        }
        *key_rsc_len = 6;
        *alg = WPA_ALG_TKIP;
        break;
    case WPA_CIPHER_WEP104:
        if (keylen != 13 || maxkeylen < 13) {
            ret = -1;
            break;
        }
        *key_rsc_len = 0;
        *alg = WPA_ALG_WEP;
        break;
    case WPA_CIPHER_WEP40:
        if (keylen != 5 || maxkeylen < 5) {
            ret = -1;
            break;
        }
        *key_rsc_len = 0;
        *alg = WPA_ALG_WEP;
        break;
    default:
        wpa_dbg("WPA: Unsupported Group Cipher %d", group_cipher);
        return -1;
    }

    if (ret < 0 ) {
        wpa_dbg("WPA: Unsupported %s Group Cipher key length %d (%d)", wpa_cipher_txt(group_cipher), keylen, maxkeylen);
    }

    return ret;
}

static int wpa_supplicant_install_gtk(struct wpa_dev *dev, struct wpa_gtk_data *gd, u8 *key_rsc)
{
    u8 *_gtk = gd->gtk;
    u8 gtk_buf[32];

    wpa_hexdump(MSG_DEBUG, "WPA: Group Key", gd->gtk, gd->gtk_len);
    wpa_dbg("WPA: Installing GTK to the driver (keyidx=%d tx=%d len=%d)", gd->keyidx, gd->tx, gd->gtk_len);
    wpa_hexdump(MSG_DEBUG, "WPA: RSC", key_rsc, gd->key_rsc_len);
    if (dev->wpa_cfg.group_cipher == WPA_CIPHER_TKIP) {
        /* Swap Tx/Rx keys for Michael MIC */
        mem_cpy(gtk_buf, gd->gtk, 16);
        mem_cpy(gtk_buf + 16, gd->gtk + 24, 8);
        mem_cpy(gtk_buf + 24, gd->gtk + 16, 8);
        _gtk = gtk_buf;
    }
    if (dev->wpa_cfg.pairwise_cipher == WPA_CIPHER_NONE) {
        if (wpa_sm_set_key(dev, gd->alg, NULL, gd->keyidx, 1, key_rsc, gd->key_rsc_len, _gtk, gd->gtk_len) < 0) {
            wpa_dbg("WPA: Failed to set GTK to the driver (Group only)");
            return -1;
        }
    } else if (wpa_sm_set_key(dev, gd->alg, broadcast_ether_addr, gd->keyidx, gd->tx, key_rsc, gd->key_rsc_len, _gtk, gd->gtk_len) < 0) {
        wpa_dbg("WPA: Failed to set GTK to the driver (alg=%d keylen=%d keyidx=%d)", gd->alg, gd->gtk_len, gd->keyidx);
        return -1;
    }

    return 0;
}

static int wpa_supplicant_pairwise_gtk(struct wpa_dev *dev,
                struct wpa_eapol_key *key,
                u8 *gtk, size_t gtk_len,
                int key_info)
{
#ifndef CONFIG_NO_WPA2
    struct wpa_gtk_data gd;

    /*
     * IEEE Std 802.11i-2004 - 8.5.2 EAPOL-Key frames - Figure 43x
     * GTK KDE format:
     * KeyID[bits 0-1], Tx [bit 2], Reserved [bits 3-7]
     * Reserved [bits 0-7]
     * GTK
     */

    mem_set(&gd, 0, sizeof(gd));
    wpa_hexdump(MSG_DEBUG, "RSN: received GTK in pairwise handshake", gtk, gtk_len);

    if (gtk_len < 2 || gtk_len - 2 > sizeof(gd.gtk))
        return -1;

    gd.keyidx = gtk[0] & 0x3;
    gd.tx = wpa_supplicant_gtk_tx_bit_workaround(dev, !!(gtk[0] & BIT(2)));
    gtk += 2;
    gtk_len -= 2;

    mem_cpy(gd.gtk, gtk, gtk_len);
    gd.gtk_len = gtk_len;

    if (wpa_supplicant_check_group_cipher(dev->wpa_cfg.group_cipher, gtk_len, gtk_len, &gd.key_rsc_len, &gd.alg)
     || wpa_supplicant_install_gtk(dev, &gd, key->key_rsc)) {
        wpa_dbg("RSN: Failed to install GTK");
        return -1;
    }

    return 0;
#else /* CONFIG_NO_WPA2 */
    return -1;
#endif /* CONFIG_NO_WPA2 */
}

/**
 * wpa_supplicant_send_4_of_4 - Send message 4 of WPA/RSN 4-Way Handshake
 * @sm: Pointer to WPA state machine data from wpa_sm_init()
 * @dst: Destination address for the frame
 * @key: Pointer to the EAPOL-Key frame header
 * @ver: Version bits from EAPOL-Key Key Info
 * @key_info: Key Info
 * @kde: KDEs to include the EAPOL-Key frame
 * @kde_len: Length of KDEs
 * @ptk: PTK to use for keyed hash and encryption
 * Returns: 0 on success, -1 on failure
 */
int wpa_supplicant_send_4_of_4(struct wpa_dev *dev, unsigned char *dst,
        struct wpa_eapol_key *key,
        u16 ver, u16 key_info,
        u8 *kde, size_t kde_len,
        struct wpa_ptk *ptk)
{
    os_u32 rlen;
    struct wpa_eapol_key *reply;
    u8 *rbuf;

    if (kde)
        wpa_hexdump(MSG_DEBUG, "WPA: KDE for msg 4/4", kde, kde_len);

    rbuf = wpa_alloc_eapol(IEEE802_1X_TYPE_EAPOL_KEY, NULL, sizeof(*reply) + kde_len, &rlen, (void *) &reply);
    if (rbuf == NULL)
        return -1;

    reply->type = dev->wpa_cfg.proto == WPA_PROTO_RSN ? EAPOL_KEY_TYPE_RSN : EAPOL_KEY_TYPE_WPA;
    key_info &= WPA_KEY_INFO_SECURE;
    key_info |= ver | WPA_KEY_INFO_KEY_TYPE | WPA_KEY_INFO_MIC;
    WPA_PUT_BE16(reply->key_info, key_info);
    if (dev->wpa_cfg.proto == WPA_PROTO_RSN)
        WPA_PUT_BE16(reply->key_length, 0);
    else
        mem_cpy(reply->key_length, key->key_length, 2);
    mem_cpy(reply->replay_counter, key->replay_counter, WPA_REPLAY_COUNTER_LEN);

    WPA_PUT_BE16(reply->key_data_length, kde_len);
    if (kde)
        mem_cpy(reply + 1, kde, kde_len);

    wpa_dbg("WPA: Sending EAPOL-Key 4/4");
    wpa_eapol_key_send(dev, ptk->kck, ver, dst, ETH_P_EAPOL, rbuf, rlen, reply->key_mic);

    return 0;
}

extern struct wpa_ie_data wpa_cfg;
static void wpa_supplicant_process_3_of_4(struct wpa_dev *dev, struct wpa_eapol_key *key, u16 ver)
{
    u16 key_info, keylen, len;
    u8 *pos;
    struct wpa_eapol_ie_parse ie;

    wpa_dbg("WPA: RX message 3 of 4-Way Handshake from " MACSTR " (ver=%d)", MAC2STR(dev->bssid), ver);

    key_info = WPA_GET_BE16(key->key_info);

    pos = (u8 *) (key + 1);
    len = WPA_GET_BE16(key->key_data_length);
    wpa_hexdump(MSG_DEBUG, "WPA: IE KeyData", pos, len);
    wpa_supplicant_parse_ies(pos, len, &ie);
    if (ie.gtk && !(key_info & WPA_KEY_INFO_ENCR_KEY_DATA)) {
        wpa_dbg("WPA: GTK IE in unencrypted key data");
        goto failed;
    }
#ifdef CONFIG_IEEE80211W
    if (ie.igtk && !(key_info & WPA_KEY_INFO_ENCR_KEY_DATA)) {
        wpa_msg(sm->ctx->msg_ctx, MSG_WARNING, "WPA: IGTK KDE in unencrypted key data");
        goto failed;
    }

    if (ie.igtk && ie.igtk_len != sizeof(struct wpa_igtk_kde)) {
        wpa_msg(sm->ctx->msg_ctx, MSG_WARNING, "WPA: Invalid IGTK KDE length %lu", (unsigned long) ie.igtk_len);
        goto failed;
    }
#endif /* CONFIG_IEEE80211W */

#if 0 // mod by alic
    if (wpa_supplicant_validate_ie(dev, dev->bssid, &ie) < 0)
        goto failed;
#endif

    if (mem_cmp(dev->anonce, key->key_nonce, WPA_NONCE_LEN) != TRUE) {
        wpa_dbg("WPA: ANonce from message 1 of 4-Way Handshake differs from 3 of 4-Way Handshake - drop packet (src=" MACSTR ")", MAC2STR(dev->bssid));
        wpa_hexdump(MSG_DEBUG, "dev anonce", dev->anonce, WPA_NONCE_LEN);
        wpa_hexdump(MSG_DEBUG, "key anonce", key->key_nonce, WPA_NONCE_LEN);
        goto failed;
    }

    keylen = WPA_GET_BE16(key->key_length);
    mem_cpy(&dev->wpa_cfg, &wpa_cfg, sizeof(struct wpa_ie_data));
    switch (dev->wpa_cfg.pairwise_cipher) {
    case WPA_CIPHER_CCMP:
        if (keylen != 16) {
            wpa_dbg("WPA: Invalid CCMP key length %d (src=" MACSTR ")", keylen, MAC2STR(dev->bssid));
            goto failed;
        }
        break;
    case WPA_CIPHER_TKIP:
        if (keylen != 32) {
            wpa_dbg("WPA: Invalid TKIP key length %d (src=" MACSTR ")", keylen, MAC2STR(dev->bssid));
            goto failed;
        }
        break;
    }

    if (wpa_supplicant_send_4_of_4(dev, dev->bssid, key, ver, key_info, NULL, 0, &dev->ptk)) {
        goto failed;
    }

    /* SNonce was successfully used in msg 3/4, so mark it to be renewed
     * for the next 4-Way Handshake. If msg 3 is received again, the old
     * SNonce will still be used to avoid changing PTK. */
    //sm->renew_snonce = 1;

    if (key_info & WPA_KEY_INFO_INSTALL) {
        if (wpa_supplicant_install_ptk(dev, key)) {
            wpa_dbg("ptk install fail\n");
            goto failed;
        }
    }

    if (key_info & WPA_KEY_INFO_SECURE) {
        //wpa_sm_mlme_setprotection(sm, sm->bssid, MLME_SETPROTECTION_PROTECT_TYPE_RX, MLME_SETPROTECTION_KEY_TYPE_PAIRWISE);
    }

    if (ie.gtk && wpa_supplicant_pairwise_gtk(dev, key, ie.gtk, ie.gtk_len, key_info) < 0) {
        wpa_dbg("RSN: Failed to configure GTK");
        goto failed;
    }
#if 0 // mod by alic
    if (ieee80211w_set_keys(sm, &ie) < 0) {
        wpa_dbg("RSN: Failed to configure IGTK");
        goto failed;
    }
#endif
    print("eapol 4/4 finished\n");
    return;

failed:
    return; // TODO:wpa_sm_deauthenticate(sm, WLAN_REASON_UNSPECIFIED);
}

/* Decrypt RSN EAPOL-Key key data (RC4 or AES-WRAP) */
static int wpa_supplicant_decrypt_key_data(struct wpa_dev *dev,
            struct wpa_eapol_key *key, u16 ver)
{
    u16 keydatalen = WPA_GET_BE16(key->key_data_length);

    wpa_hexdump(MSG_DEBUG, "RSN: encrypted key data", (u8 *) (key + 1), keydatalen);

    /* Decrypt key data here so that this operation does not need
     * to be implemented separately for each message type. */
    if (ver == WPA_KEY_INFO_TYPE_HMAC_MD5_RC4) {
        u8 ek[32];
        mem_cpy(ek, key->key_iv, 16);
        mem_cpy(ek + 16, dev->ptk.kek, 16);
        if (rc4_skip(ek, 32, 256, (u8 *) (key + 1), keydatalen)) {
            wpa_dbg("WPA: RC4 failed");
            return -1;
        }
    } else if (ver == WPA_KEY_INFO_TYPE_HMAC_SHA1_AES ||
           ver == WPA_KEY_INFO_TYPE_AES_128_CMAC) {
        u8 *buf;
        if (keydatalen % 8) {
            wpa_dbg("WPA: Unsupported AES-WRAP len %d", keydatalen);
            return -1;
        }
        keydatalen -= 8; /* AES-WRAP adds 8 bytes */
        buf = kmalloc(keydatalen);
        if (buf == NULL) {
            wpa_dbg("WPA: No memory for AES-UNWRAP buffer");
            return -1;
        }
        if (aes_unwrap(dev->ptk.kek, keydatalen / 8, (u8 *) (key + 1), buf)) {
            kfree(buf);
            wpa_dbg("WPA: AES unwrap failed - could not decrypt EAPOL-Key key data");
            return -1;
        }
        mem_cpy(key + 1, buf, keydatalen);
        kfree(buf);
        WPA_PUT_BE16(key->key_data_length, keydatalen);
    } else {
        wpa_dbg("WPA: Unsupported key_info type %d", ver);
        return -1;
    }
    wpa_hexdump(MSG_DEBUG, "WPA: decrypted EAPOL-Key key data", (u8 *) (key + 1), keydatalen);
    return 0;
}

os_void recv_eapol(struct wpa_dev *dev, os_u8 *src_addr, os_u8 *buf)
{
    struct ieee802_1x_hdr *hdr;
    struct wpa_eapol_key *key;
    os_u16 plen, data_len, key_data_len;
    os_u16 key_info, ver;

    hdr = (struct ieee802_1x_hdr *) buf;
    plen = net_host_16(hdr->length);
    data_len = plen + sizeof(*hdr);

    // no wps supported

    switch (hdr->type) {
    case IEEE802_1X_TYPE_EAP_PACKET:
        wpa_dbg("EAPOL: EAPOL-packet frame received\n");
        handle_eap_packet(dev, (os_u8 *)(hdr + 1), net_host_16(hdr->length));
        break;
    case IEEE802_1X_TYPE_EAPOL_KEY:
        if (plen < sizeof(*key)) {
            wpa_dbg("EAPOL: invalid EAPOL-Key frame received\n");
            break;
        }
        key = (struct wpa_eapol_key *) (hdr + 1);
        if ((key->type != EAPOL_KEY_TYPE_WPA) && (key->type != EAPOL_KEY_TYPE_RSN)) {
            /* WPA Supplicant takes care of this frame. */
            wpa_dbg("unknown WPA EAPOL-Key frame, type %d\n", key->type);
            break;
        }
        key_info = WPA_GET_BE16(key->key_info);
        ver = key_info & WPA_KEY_INFO_TYPE_MASK;
        if ((ver != WPA_KEY_INFO_TYPE_HMAC_MD5_RC4)
         && (ver != WPA_KEY_INFO_TYPE_AES_128_CMAC)
         && (ver != WPA_KEY_INFO_TYPE_HMAC_SHA1_AES)) {
            wpa_dbg("WPA: Unsupported EAPOL-Key descriptor version %d\n", ver);
            break;
        }

        if (!(key_info & (WPA_KEY_INFO_ACK | WPA_KEY_INFO_SMK_MESSAGE))) {
            wpa_dbg("WPA: No Ack bit in key_info\n");
            break;
        }
        if (key_info & WPA_KEY_INFO_REQUEST) {
            wpa_dbg("WPA: EAPOL-Key with Request bit - dropped");
            break;
        }

        key_data_len = WPA_GET_BE16(key->key_data_length);
        wpa_dbg("proto %d, %d\n", dev->wpa_cfg.proto, key_info & WPA_KEY_INFO_ENCR_KEY_DATA);
        if (dev->wpa_cfg.proto == WPA_PROTO_RSN && (key_info & WPA_KEY_INFO_ENCR_KEY_DATA)) {
            if (wpa_supplicant_decrypt_key_data(dev, key, ver))
                return;
                key_data_len = WPA_GET_BE16(key->key_data_length);
            }
        if (key_info & WPA_KEY_INFO_KEY_TYPE) {
            if (key_info & WPA_KEY_INFO_KEY_INDEX_MASK) {
                wpa_dbg("WPA: Ignored EAPOL-Key (Pairwise) with non-zero key index\n");
                break;
            }
            if (key_info & WPA_KEY_INFO_MIC) {
                /* 3/4 4-Way Handshake */
                wpa_supplicant_process_3_of_4(dev, key, ver);
            } else {
                mem_cpy(dev->anonce, key->key_nonce, WPA_NONCE_LEN);
                /* 1/4 4-Way Handshake */
                wpa_supplicant_process_1_of_4(dev, src_addr, key, ver);
            }
        } else if (key_info & WPA_KEY_INFO_SMK_MESSAGE) {
            /* PeerKey SMK Handshake */
            wpa_dbg("peerkey_rx_eapol_smk");
        } else {
            if (key_info & WPA_KEY_INFO_MIC) {
                /* 1/2 Group Key Handshake */
                wpa_dbg("group key handshake\n");
                // TODO: wpa_supplicant_process_1_of_2(sm, src_addr, key, extra_len, ver);
            } else {
                wpa_dbg("WPA: EAPOL-Key (Group) without Mic bit - dropped\n");
            }
        }
        break;
    default:
        wpa_dbg("EAPOL: Received unknown EAPOL type %d", hdr->type);
        break;
    }
}

