/******************************************************************************
 *
 * Copyright(c) 2007 - 2013 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifndef __OSDEP_LINUX_SERVICE_H_
#define __OSDEP_LINUX_SERVICE_H_

//#define snprintf(buf, n, fmt, arg...) snprint(buf, n, fmt, ##arg)
#define sprintf(buf, fmt, arg...) snprintf(buf, UINT32_MAX, fmt, ##arg)

#define EXPORT_SYMBOL(a)
/* for debug error end */

/* for link error star */
#define undefined_function() \
    do { \
        flog("undefined function %s\n", __FUNCTION__); \
    } while (0)

#define printk(fmt, arg...) flog(fmt, ##arg)
#define seq_printf(fmt, arg...) flog(fmt, ##arg),OS_TRUE

#define mtx_lock(o) spin_lock(o)
#define mtx_unlock(o) spin_unlock(o)
#define spin_lock_bh(o) spin_lock(o)
#define spin_unlock_bh(o) spin_unlock(o)
#define spin_lock_irqsave(o,t) spin_lock(o)
#define spin_unlock_irqsave(o,t) spin_unlock(o)
#define spin_lock_irqrestore(o,t) spin_lock(o)
#define spin_unlock_irqrestore(o,t) spin_unlock(o)
#define spin_lock_init(plock) init_spinlock(plock)
#define sema_init(sema, init_val) \
    do { \
        *(sema) = create_mevent(init_val, "rtw_csem", __LINE__); \
    } while (0)
#define init_MUTEX(sema) \
    do { \
        *(sema) = create_event_handle(EVENT_VALID, "MUTEX", __LINE__); \
    } while (0)

#define netdev_priv(a) (a)
#define SET_NETDEV_DEV(net, pdev) (net)->priv = (pdev)

/* return value is meanless */
#define del_timer_sync(ptimer) do { kill_timer(ptimer->handle); ptimer->handle = OS_NULL; } while (0)

#define KERN_WARNING
#define KERN_ERR
#define WARN_ON(condition) do {} while (0)

#define list_del_init(l) del_init_list(l)
#define list_add(plist, phead) add_list_head(phead, plist)
#define list_add_tail(plist, phead) add_list_tail(phead, plist)
#define INIT_LIST_HEAD(n) init_list_head(n)

#define likely(c) c
#define unlikely(c) c

static __inline__ int atomic_add_return(int i, atomic_t * v)
{
    atomic_inc(v);
    return v->counter;
}
static __inline__ int atomic_sub_return(int i, atomic_t * v)
{
    atomic_dec(v);
    return v->counter;
}
#define atomic_dec_return(v)		atomic_sub_return(1, (v))
#define atomic_inc_return(v)		atomic_add_return(1, (v))

#define mdelay(m) delay_ms(m)
#define udelay(n) delay_us(n)

#define yield schedule

#define up(sema) notify_mevent(*(sema), __LINE__)
#define down_interruptible(sema) wait_mevent(*(sema), 0)

#define free_netdev(d)

#define do_div(n,base) ({					\
	u32 __base = (base);				\
	u32 __rem;						\
	__rem = ((u64)(n)) % __base;			\
	(n) = ((u64)(n)) / __base;				\
	__rem;							\
 })

static inline struct usb_device *
usb_get_dev(struct usb_device *dev)
{
        return dev;
}

static inline void
usb_put_dev(struct usb_device *dev)
{
        return;
}

#define usb_register(d) register_usb_driver(d)

/* Odd : get (world access), even : set (root access) */
#define IW_IS_SET(cmd)	(!((cmd) & 0x1))
#define IW_IS_GET(cmd)	((cmd) & 0x1)

#define typecheck(a,b) OS_TRUE
#define time_after_eq(a,b)	\
	(typecheck(unsigned long, a) && \
	 typecheck(unsigned long, b) && \
	 ((long)(a) - (long)(b) >= 0))
#define time_before_eq(a,b)	time_after_eq(b,a)

#define random32() 0

struct kref {
	atomic_t refcount;
};
#define atomic_dec_and_test(v)		(atomic_sub_return(1, (v)) == 0)

#define _irqlevel_changed_(a,b)

#define daemonize(fmt, arg...) flog(fmt, ##arg)

#define csum_partial(a,b,c) 0
#define csum_ipv6_magic(a,b,c,d,e) 0

#define init_completion(a)
#define complete_and_exit(a,b)
#define complete(a)
#define tasklet_hi_schedule(a)
#define schedule_work(a)
#define INIT_WORK(a,b)
#define cancel_work_sync(a)
#define __netif_subqueue_stopped(ndev, i) 1
#define find_vpid(pid)
#define kill_pid(pid,sig,v)

/* for link error end */

#if 0 // alic
	#include <linux/version.h>
	#include <linux/spinlock.h>
	#include <linux/compiler.h>
	#include <linux/kernel.h>
	#include <linux/errno.h>
	#include <linux/init.h>
	#include <linux/slab.h>
	#include <linux/module.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,5))
	#include <linux/kref.h>
#endif
	//#include <linux/smp_lock.h>
	#include <linux/netdevice.h>
	#include <linux/skbuff.h>
	#include <linux/circ_buf.h>
	#include <asm/uaccess.h>
	#include <asm/byteorder.h>
	#include <asm/atomic.h>
	#include <asm/io.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
	#include <asm/semaphore.h>
#else
	#include <linux/semaphore.h>
#endif
	#include <linux/sem.h>
	#include <linux/sched.h>
	#include <linux/etherdevice.h>
	#include <linux/wireless.h>
	#include <net/iw_handler.h>
	#include <linux/if_arp.h>
	#include <linux/rtnetlink.h>
	#include <linux/delay.h>
	#include <linux/interrupt.h>	// for struct tasklet_struct
	#include <linux/ip.h>
	#include <linux/kthread.h>
	#include <linux/list.h>
	#include <linux/vmalloc.h>

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,41))
	#include <linux/tqueue.h>
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0))
	#include <uapi/linux/limits.h>
#else
	#include <linux/limits.h>
#endif

#ifdef RTK_DMP_PLATFORM
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12))
	#include <linux/pageremap.h>
#endif
	#include <asm/io.h>
#endif

	/* Monitor mode */
	#include <net/ieee80211_radiotap.h>

#ifdef CONFIG_IOCTL_CFG80211
/*	#include <linux/ieee80211.h> */
	#include <net/cfg80211.h>
#endif //CONFIG_IOCTL_CFG80211

#ifdef CONFIG_TCP_CSUM_OFFLOAD_TX
	#include <linux/in.h>
	#include <linux/udp.h>
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	#include <linux/earlysuspend.h>
#endif //CONFIG_HAS_EARLYSUSPEND

#ifdef CONFIG_EFUSE_CONFIG_FILE
	#include <linux/fs.h>
#endif //CONFIG_EFUSE_CONFIG_FILE

#ifdef CONFIG_USB_HCI
	#include <linux/usb.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21))
	#include <linux/usb_ch9.h>
#else
	#include <linux/usb/ch9.h>
#endif
#endif
#else

#define   KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
/* emulate a modern version */
#define LINUX_VERSION_CODE KERNEL_VERSION(2, 6, 35)

#define list_head list_node

    typedef struct {
        HTIMER handle;
        TIMER_FUNC_PTR function;
        void *data;
    } _timer;

typedef u32 size_t;
typedef s32 ssize_t;
typedef u32 loff_t;

#define __u8 u8
#define __u16 u16
#define __u32 u32
#define __s8 s8
#define __s16 s16
#define __s32 s32

#undef IN
#define IN

#define __user

#define HZ OS_HZ

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#if 0 // alic mod for scan
#define container_of(p,t,n) (t*)((p)-&(((t*)0)->n))
#else
#define container_of(p,t,n) (t*)((u32)(p) - (u32) &(((t*)0)->n))
#endif

#define	MAX_SCHEDULE_TIMEOUT UINT32_MAX

#define strsep(str, c) str_brk((u8 **)(str), (const u8 *)(c))

#include <linux/wireless.h>

enum {
    EFAULT = 1,
    EINVAL,
    EPERM,
    EPIPE,
    ENODEV,
    ESHUTDOWN,
    ENOENT,
    EPROTO,
    EILSEQ,
    ETIME,
    ECOMM,
    EOVERFLOW,
    EINPROGRESS,
    ECONNRESET,
    ENOMEM,
    EOPNOTSUPP,
    E2BIG,
    EBUSY,
    EAGAIN
};

#define __init
#define __exit

/* This equals 0, but use constants in case they ever change */
#define GFP_NOWAIT 0
/* GFP_ATOMIC means both !wait (__GFP_WAIT not set) and use emergency pool */
#define GFP_ATOMIC 0
#define GFP_NOIO 0
#define GFP_NOFS 0
#define GFP_KERNEL 0
#define GFP_USER 0
#define GFP_HIGHUSER 0

typedef unsigned gfp_t;

typedef struct pm_message {
	int event;
} pm_message_t;

#ifndef IW_CUSTOM_MAX
#define IW_CUSTOM_MAX 256
#endif

#define SIGHUP		 1
#define SIGINT		 2
#define SIGQUIT		 3
#define SIGILL		 4
#define SIGTRAP		 5
#define SIGABRT		 6
#define SIGIOT		 6
#define SIGBUS		 7
#define SIGFPE		 8
#define SIGKILL		 9
#define SIGUSR1		10
#define SIGSEGV		11
#define SIGUSR2		12
#define SIGPIPE		13
#define SIGALRM		14
#define SIGTERM		15
#define SIGSTKFLT	16
#define SIGCHLD		17
#define SIGCONT		18
#define SIGSTOP		19
#define SIGTSTP		20
#define SIGTTIN		21
#define SIGTTOU		22
#define SIGURG		23
#define SIGXCPU		24
#define SIGXFSZ		25
#define SIGVTALRM	26
#define SIGPROF		27
#define SIGWINCH	28
#define SIGIO		29
#define SIGPOLL		SIGIO

/* Packet types */
#define PACKET_HOST		0		/* To us		*/
#define PACKET_BROADCAST	1		/* To all		*/
#define PACKET_MULTICAST	2		/* To group		*/
#define PACKET_OTHERHOST	3		/* To someone else 	*/
#define PACKET_OUTGOING		4		/* Outgoing of any type */
/* These ones are invisible by user level */
#define PACKET_LOOPBACK		5		/* MC/BRD frame looped back */
#define PACKET_FASTROUTE	6		/* Fastrouted frame	*/

/* ARP protocol HARDWARE identifiers. */
#define ARPHRD_NETROM	0		/* from KA9Q: NET/ROM pseudo	*/
#define ARPHRD_ETHER 	1		/* Ethernet 10Mbps		*/
#define	ARPHRD_EETHER	2		/* Experimental Ethernet	*/
#define	ARPHRD_AX25	3		/* AX.25 Level 2		*/
#define	ARPHRD_PRONET	4		/* PROnet token ring		*/
#define	ARPHRD_CHAOS	5		/* Chaosnet			*/
#define	ARPHRD_IEEE802	6		/* IEEE 802.2 Ethernet/TR/TB	*/
#define	ARPHRD_ARCNET	7		/* ARCnet			*/
#define	ARPHRD_APPLETLK	8		/* APPLEtalk			*/
#define ARPHRD_DLCI	15		/* Frame Relay DLCI		*/
#define ARPHRD_ATM	19		/* ATM 				*/
#define ARPHRD_METRICOM	23		/* Metricom STRIP (new IANA id)	*/
#define	ARPHRD_IEEE1394	24		/* IEEE 1394 IPv4 - RFC 2734	*/
#define ARPHRD_EUI64	27		/* EUI-64                       */
#define ARPHRD_INFINIBAND 32		/* InfiniBand			*/

/* Dummy types for non ARP hardware */
#define ARPHRD_SLIP	256
#define ARPHRD_CSLIP	257
#define ARPHRD_SLIP6	258
#define ARPHRD_CSLIP6	259
#define ARPHRD_RSRVD	260		/* Notional KISS type 		*/
#define ARPHRD_ADAPT	264
#define ARPHRD_ROSE	270
#define ARPHRD_X25	271		/* CCITT X.25			*/
#define ARPHRD_HWX25	272		/* Boards with X.25 in firmware	*/
#define ARPHRD_CAN	280		/* Controller Area Network      */
#define ARPHRD_PPP	512
#define ARPHRD_CISCO	513		/* Cisco HDLC	 		*/
#define ARPHRD_HDLC	ARPHRD_CISCO
#define ARPHRD_LAPB	516		/* LAPB				*/
#define ARPHRD_DDCMP    517		/* Digital's DDCMP protocol     */
#define ARPHRD_RAWHDLC	518		/* Raw HDLC			*/

#define ARPHRD_TUNNEL	768		/* IPIP tunnel			*/
#define ARPHRD_TUNNEL6	769		/* IP6IP6 tunnel       		*/
#define ARPHRD_FRAD	770             /* Frame Relay Access Device    */
#define ARPHRD_SKIP	771		/* SKIP vif			*/
#define ARPHRD_LOOPBACK	772		/* Loopback device		*/
#define ARPHRD_LOCALTLK 773		/* Localtalk device		*/
#define ARPHRD_FDDI	774		/* Fiber Distributed Data Interface */
#define ARPHRD_BIF      775             /* AP1000 BIF                   */
#define ARPHRD_SIT	776		/* sit0 device - IPv6-in-IPv4	*/
#define ARPHRD_IPDDP	777		/* IP over DDP tunneller	*/
#define ARPHRD_IPGRE	778		/* GRE over IP			*/
#define ARPHRD_PIMREG	779		/* PIMSM register interface	*/
#define ARPHRD_HIPPI	780		/* High Performance Parallel Interface */
#define ARPHRD_ASH	781		/* Nexus 64Mbps Ash		*/
#define ARPHRD_ECONET	782		/* Acorn Econet			*/
#define ARPHRD_IRDA 	783		/* Linux-IrDA			*/
/* ARP works differently on different FC media .. so  */
#define ARPHRD_FCPP	784		/* Point to point fibrechannel	*/
#define ARPHRD_FCAL	785		/* Fibrechannel arbitrated loop */
#define ARPHRD_FCPL	786		/* Fibrechannel public loop	*/
#define ARPHRD_FCFABRIC	787		/* Fibrechannel fabric		*/
	/* 787->799 reserved for fibrechannel media types */
#define ARPHRD_IEEE802_TR 800		/* Magic type ident for TR	*/
#define ARPHRD_IEEE80211 801		/* IEEE 802.11			*/
#define ARPHRD_IEEE80211_PRISM 802	/* IEEE 802.11 + Prism2 header  */
#define ARPHRD_IEEE80211_RADIOTAP 803	/* IEEE 802.11 + radiotap header */
#define ARPHRD_IEEE802154	  804

#define ARPHRD_PHONET	820		/* PhoNet media type		*/
#define ARPHRD_PHONET_PIPE 821		/* PhoNet pipe header		*/
#define ARPHRD_CAIF	822		/* CAIF media type		*/

#define ARPHRD_VOID	  0xFFFF	/* Void type, nothing is known */
#define ARPHRD_NONE	  0xFFFE	/* zero header length */

/* ARP protocol opcodes. */
#define	ARPOP_REQUEST	1		/* ARP request			*/
#define	ARPOP_REPLY	2		/* ARP reply			*/
#define	ARPOP_RREQUEST	3		/* RARP request			*/
#define	ARPOP_RREPLY	4		/* RARP reply			*/
#define	ARPOP_InREQUEST	8		/* InARP request		*/
#define	ARPOP_InREPLY	9		/* InARP reply			*/
#define	ARPOP_NAK	10		/* (ATM)ARP NAK			*/

enum usb_device_speed {
    USB_SPEED_UNKNOWN = 0,            /* enumerating */
    USB_SPEED_LOW, USB_SPEED_FULL,        /* usb 1.1 */
    USB_SPEED_HIGH,                /* usb 2.0 */
    USB_SPEED_WIRELESS,            /* wireless (usb 2.5) */
    USB_SPEED_SUPER,            /* usb 3.0 */
};

/*
 * Endpoints
 */
#define USB_ENDPOINT_NUMBER_MASK    0x0f    /* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK        0x80

#define USB_ENDPOINT_SYNCTYPE        0x0c
#define USB_ENDPOINT_SYNC_NONE        (0 << 2)
#define USB_ENDPOINT_SYNC_ASYNC        (1 << 2)
#define USB_ENDPOINT_SYNC_ADAPTIVE    (2 << 2)
#define USB_ENDPOINT_SYNC_SYNC        (3 << 2)

#define USB_ENDPOINT_XFERTYPE_MASK    0x03    /* in bmAttributes */
#define USB_ENDPOINT_XFER_CONTROL    0
#define USB_ENDPOINT_XFER_ISOC        1
#define USB_ENDPOINT_XFER_BULK        2
#define USB_ENDPOINT_XFER_INT        3
#define USB_ENDPOINT_MAX_ADJUSTABLE    0x80

static inline int usb_autopm_get_interface(struct usb_device *intf)
{ return 0; }

#define SIOCDEVPRIVATE	0x89F0	/* to 89FF */

struct file {};
struct seq_file {};
struct work_struct {};
struct completion {};
struct tasklet_struct {};

#define tasklet_init(a,b,c)
#define tasklet_kill(a)
#define tasklet_schedule(a)

struct ifreq {};
struct net_device_stats {
	unsigned long	rx_packets;
	unsigned long	tx_packets;
	unsigned long	rx_bytes;
	unsigned long	tx_bytes;
	unsigned long	rx_errors;
	unsigned long	tx_errors;
	unsigned long	rx_dropped;
	unsigned long	tx_dropped;
	unsigned long	multicast;
	unsigned long	collisions;
	unsigned long	rx_length_errors;
	unsigned long	rx_over_errors;
	unsigned long	rx_crc_errors;
	unsigned long	rx_frame_errors;
	unsigned long	rx_fifo_errors;
	unsigned long	rx_missed_errors;
	unsigned long	tx_aborted_errors;
	unsigned long	tx_carrier_errors;
	unsigned long	tx_fifo_errors;
	unsigned long	tx_heartbeat_errors;
	unsigned long	tx_window_errors;
	unsigned long	rx_compressed;
	unsigned long	tx_compressed;
};
struct iw_statistics {};
struct proc_dir_entry {};
struct device {
};
struct usb_interface {
    struct device dev;
    struct usb_device *udev;
    void *data;
};
static inline void *usb_get_intfdata(struct usb_device *usb)
{
    return get_usb_dedicated(usb);
}
static inline void usb_set_intfdata(struct usb_device *usb, void *data)
{
    set_usb_dedicated(usb, data);
}
static inline struct usb_device *interface_to_usbdev(struct usb_device *usb)
{
    return usb;
}

struct urb {
    int status;
    void *context;
    u32 actual_length;
};

struct urb *usb_alloc_urb(int iso_packets, gfp_t mem_flags);

#define MODULE_DEVICE_TABLE(a,b)
#define module_param(a,b,c)
#define MODULE_LICENSE(a)
#define MODULE_DESCRIPTION(a)
#define MODULE_AUTHOR(a)
#define MODULE_VERSION(a)
#define MODULE_PARM_DESC(a,b)

struct notifier_block {
    int (*notifier_call)(struct notifier_block * nb, unsigned long state, void *ptr);
};

struct _curr_struct {
    u8 *comm;
    os_u32 pid;
};
extern struct _curr_struct *current;
extern os_u32 jiffies;

#endif

#ifdef CONFIG_USB_HCI
	typedef struct urb *  PURB;
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,22))
#ifdef CONFIG_USB_SUSPEND
#define CONFIG_AUTOSUSPEND	1
#endif
#endif
#endif

	typedef HEVENT _sema;
	typedef	spinlock_t	_lock;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	typedef struct mutex 		_mutex;
#else
	typedef HEVENT	_mutex;
#endif
	//typedef struct timer_list _timer;

	struct	__queue	{
		struct	list_head	queue;
		_lock	lock;
	};

	typedef	struct sk_buff	_pkt;
	typedef unsigned char	_buffer;

	typedef struct	__queue	_queue;
	typedef struct	list_head	_list;
	typedef	int	_OS_STATUS;
	//typedef u32	_irqL;
	typedef unsigned long _irqL;
	typedef	struct	net_device * _nic_hdl;

	typedef void*		_thread_hdl_;
	typedef int		thread_return;
	typedef void*	thread_context;

	#define thread_exit() complete_and_exit(NULL, 0)

	typedef void timer_hdl_return;
	typedef void* timer_hdl_context;

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,5,41))
	typedef struct work_struct _workitem;
#else
	typedef struct tq_struct _workitem;
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
	#define DMA_BIT_MASK(n) (((n) == 64) ? ~0ULL : ((1ULL<<(n))-1))
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22))
// Porting from linux kernel, for compatible with old kernel.
static inline unsigned char *skb_tail_pointer(const struct sk_buff *skb)
{
	return skb->tail;
}

static inline void skb_reset_tail_pointer(struct sk_buff *skb)
{
	skb->tail = skb->data;
}

static inline void skb_set_tail_pointer(struct sk_buff *skb, const int offset)
{
	skb->tail = skb->data + offset;
}

static inline unsigned char *skb_end_pointer(const struct sk_buff *skb)
{
	return skb->end;
}
#endif

__inline static _list *get_next(_list	*list)
{
	return list->next;
}

__inline static _list	*get_list_head(_queue	*queue)
{
	return (&(queue->queue));
}


#define LIST_CONTAINOR(ptr, type, member) \
        ((type *)((char *)(ptr)-(SIZE_T)(&((type *)0)->member)))


__inline static void _enter_critical(_lock *plock, _irqL *pirqL)
{
	spin_lock_irqsave(plock, *pirqL);
}

__inline static void _exit_critical(_lock *plock, _irqL *pirqL)
{
	spin_unlock_irqrestore(plock, *pirqL);
}

__inline static void _enter_critical_ex(_lock *plock, _irqL *pirqL)
{
	spin_lock_irqsave(plock, *pirqL);
}

__inline static void _exit_critical_ex(_lock *plock, _irqL *pirqL)
{
	spin_unlock_irqrestore(plock, *pirqL);
}

__inline static void _enter_critical_bh(_lock *plock, _irqL *pirqL)
{
	spin_lock_bh(plock);
}

__inline static void _exit_critical_bh(_lock *plock, _irqL *pirqL)
{
	spin_unlock_bh(plock);
}

__inline static int _enter_critical_mutex(_mutex *pmutex, _irqL *pirqL)
{
	int ret = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	//mutex_lock(pmutex);
	ret = mutex_lock_interruptible(pmutex);
#else
	ret = down_interruptible(pmutex);
#endif
	return ret;
}


__inline static void _exit_critical_mutex(_mutex *pmutex, _irqL *pirqL)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
		mutex_unlock(pmutex);
#else
		up(pmutex);
#endif
}

__inline static void rtw_list_delete(_list *plist)
{
	list_del_init(plist);
}

#define RTW_TIMER_HDL_ARGS void *FunctionContext

__inline static void _init_timer(_timer *ptimer,_nic_hdl nic_hdl,void *pfunc,void* cntx)
{
#if 0 // alic
	//setup_timer(ptimer, pfunc,(u32)cntx);
	ptimer->function = pfunc;
	ptimer->data = (unsigned long)cntx;
	init_timer(ptimer);
#else
    ptimer->function = pfunc;
    ptimer->data = cntx;
    ptimer->handle = OS_NULL;
#endif
}

__inline static void _set_timer(_timer *ptimer,u32 delay_time)
{
#if 0 // alic
	mod_timer(ptimer , (jiffies+(delay_time*HZ/1000)));
#else
    if (ptimer->handle) {
        kill_timer(ptimer->handle);
    }
    ptimer->handle = set_timer_callback((u32) ptimer->data, divl_cell(delay_time, 1000/OS_HZ), ptimer->function, TIMER_MODE_NOT_LOOP);
#endif
}

__inline static void _cancel_timer(_timer *ptimer,u8 *bcancelled)
{
#if 0 // alic
	del_timer_sync(ptimer);
#else
    kill_timer(ptimer->handle);
#endif
	*bcancelled=  _TRUE;//TRUE ==1; FALSE==0
}


__inline static void _init_workitem(_workitem *pwork, void *pfunc, PVOID cntx)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))
	INIT_WORK(pwork, pfunc);
#elif (LINUX_VERSION_CODE > KERNEL_VERSION(2,5,41))
	INIT_WORK(pwork, pfunc,pwork);
#else
	INIT_TQUEUE(pwork, pfunc,pwork);
#endif
}

__inline static void _set_workitem(_workitem *pwork)
{
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,5,41))
	schedule_work(pwork);
#else
	schedule_task(pwork);
#endif
}

__inline static void _cancel_workitem_sync(_workitem *pwork)
{
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,22))
	cancel_work_sync(pwork);
#elif (LINUX_VERSION_CODE > KERNEL_VERSION(2,5,41))
	flush_scheduled_work();
#else
	flush_scheduled_tasks();
#endif
}
//
// Global Mutex: can only be used at PASSIVE level.
//

#define ACQUIRE_GLOBAL_MUTEX(_MutexCounter)                              \
{                                                               \
	while (atomic_inc_return((atomic_t *)&(_MutexCounter)) != 1)\
	{                                                           \
		atomic_dec((atomic_t *)&(_MutexCounter));        \
		msleep(10);                          \
	}                                                           \
}

#define RELEASE_GLOBAL_MUTEX(_MutexCounter)                              \
{                                                               \
	atomic_dec((atomic_t *)&(_MutexCounter));        \
}

static inline int rtw_netif_queue_stopped(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
	return (netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 0)) &&
		netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 1)) &&
		netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 2)) &&
		netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 3)) );
#else
	return netif_queue_stopped(pnetdev);
#endif
}

static inline void netif_tx_wake_all_queues(struct net_device *dev)
{ undefined_function(); }
static inline void netif_tx_stop_all_queues(struct net_device *dev)
{ undefined_function(); }

static inline void rtw_netif_wake_queue(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
	netif_tx_wake_all_queues(pnetdev);
#else
	netif_wake_queue(pnetdev);
#endif
}

static inline void rtw_netif_start_queue(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
	netif_tx_start_all_queues(pnetdev);
#else
	netif_start_queue(pnetdev);
#endif
}

static inline void rtw_netif_stop_queue(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
	netif_tx_stop_all_queues(pnetdev);
#else
	netif_stop_queue(pnetdev);
#endif
}

static inline void rtw_merge_string(char *dst, int dst_len, char *src1, char *src2)
{
	int	len = 0;
	len += snprintf(dst+len, dst_len - len, "%s", src1);
	len += snprintf(dst+len, dst_len - len, "%s", src2);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
#define rtw_signal_process(pid, sig) kill_pid(find_vpid((pid)),(sig), 1)
#else //(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
#define rtw_signal_process(pid, sig) kill_proc((pid), (sig), 1)
#endif //(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))


// Suspend lock prevent system from going suspend
#ifdef CONFIG_WAKELOCK
#include <linux/wakelock.h>
#elif defined(CONFIG_ANDROID_POWER)
#include <linux/android_power.h>
#endif

// limitation of path length
#define PATH_LENGTH_MAX PATH_MAX

//Atomic integer operations
#define ATOMIC_T atomic_t

#define rtw_netdev_priv(netdev) ( netdev_priv(netdev)->priv )

#if 0 // alic
#define NDEV_FMT "%s"
#define NDEV_ARG(ndev) ndev->name
#define ADPT_FMT "%s"
#define ADPT_ARG(adapter) adapter->pnetdev->name
#define FUNC_NDEV_FMT "%s(%s)"
#define FUNC_NDEV_ARG(ndev) __func__, ndev->name
#define FUNC_ADPT_FMT "%s(%s)"
#define FUNC_ADPT_ARG(adapter) __func__, adapter->pnetdev->name
#else
#define NDEV_FMT "%s"
#define NDEV_ARG(ndev) "adpt"
#define ADPT_FMT "%s"
#define ADPT_ARG(adapter) "adpt"
#define FUNC_NDEV_FMT "%s(%s)"
#define FUNC_NDEV_ARG(ndev) __func__, "adpt"
#define FUNC_ADPT_FMT "%s(%s)"
#define FUNC_ADPT_ARG(adapter) __func__, "adpt"
#endif

struct rtw_netdev_priv_indicator {
	void *priv;
	u32 sizeof_priv;
};
struct net_device *rtw_alloc_etherdev_with_old_priv(int sizeof_priv, void *old_priv);
extern struct net_device * rtw_alloc_etherdev(int sizeof_priv);

#define STRUCT_PACKED __attribute__ ((packed))

#if 0 // alic
#else
extern void	_rtw_spinlock_init(_lock *plock);

union ktime {
	s64	tv64;
#if BITS_PER_LONG != 64 && !defined(CONFIG_KTIME_SCALAR)
	struct {
#ifdef __BIG_ENDIAN
	s32	sec, nsec;
#else
	s32	nsec, sec;
#endif
	} tv;
#endif
};
#define kmemcheck_bitfield_begin(name)
#define kmemcheck_bitfield_end(name)
#define CHECKSUM_NONE 0
typedef unsigned char *sk_buff_data_t;
typedef union ktime ktime_t;		/* Kill this */
struct sk_buff {
	/* These two members must be first. */
	struct sk_buff		*next;
	struct sk_buff		*prev;

	ktime_t			tstamp;

	struct sock		*sk;
	struct net_device	*dev;

	/*
	 * This is the control buffer. It is free to use for every
	 * layer. Please put your private variables there. If you
	 * want to keep them across layers you have to do a skb_clone()
	 * first. This is owned by whoever has the skb queued ATM.
	 */
	char			cb[48] _aligned_(8);

	unsigned long		_skb_refdst;
#ifdef CONFIG_XFRM
	struct	sec_path	*sp;
#endif
	unsigned int		len,
				data_len;
	u16			mac_len,
				hdr_len;
	union {
		u32		csum;
		struct {
			u16	csum_start;
			u16	csum_offset;
		}smbol2;
	}smbol1;
	u32			priority;
	kmemcheck_bitfield_begin(flags1);
	u8			local_df:1,
				cloned:1,
				ip_summed:2,
				nohdr:1,
				nfctinfo:3;
	u8			pkt_type:3,
				fclone:2,
				ipvs_property:1,
				peeked:1,
				nf_trace:1;
	kmemcheck_bitfield_end(flags1);
	u16			protocol;

	void			(*destructor)(struct sk_buff *skb);
#if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
	struct nf_conntrack	*nfct;
	struct sk_buff		*nfct_reasm;
#endif
#ifdef CONFIG_BRIDGE_NETFILTER
	struct nf_bridge_info	*nf_bridge;
#endif

	int			skb_iif;
#ifdef CONFIG_NET_SCHED
	u16			tc_index;	/* traffic control index */
#ifdef CONFIG_NET_CLS_ACT
	u16			tc_verd;	/* traffic control verdict */
#endif
#endif

	u32			rxhash;

	kmemcheck_bitfield_begin(flags2);
	u16			queue_mapping:16;
#ifdef CONFIG_IPV6_NDISC_NODETYPE
	u8			ndisc_nodetype:2,
				deliver_no_wcard:1;
#else
	u8			deliver_no_wcard:1;
#endif
	kmemcheck_bitfield_end(flags2);

	/* 0/14 bit hole */

#ifdef CONFIG_NET_DMA
	dma_cookie_t		dma_cookie;
#endif
#ifdef CONFIG_NETWORK_SECMARK
	u32			secmark;
#endif
	union {
		u32		mark;
		u32		dropcount;
	}symbol3;

	u16			vlan_tci;

	sk_buff_data_t		transport_header;
	sk_buff_data_t		network_header;
	sk_buff_data_t		mac_header;
	/* These elements must be at the end, see alloc_skb() for details.  */
	sk_buff_data_t		tail;
	sk_buff_data_t		end;
	unsigned char		*head,
				*data;
	unsigned int		truesize;
	atomic_t		users;
};
struct sk_buff_head {
	/* These two members must be first. */
	struct sk_buff	*next;
	struct sk_buff	*prev;

	u32		qlen;
	_lock	lock;
};
static inline unsigned char *skb_tail_pointer(const struct sk_buff *skb)
{
	return skb->tail;
}

static inline void skb_reset_tail_pointer(struct sk_buff *skb)
{
	skb->tail = skb->data;
}

static inline void skb_set_tail_pointer(struct sk_buff *skb, const int offset)
{
	skb->tail = skb->data + offset;
}

static inline unsigned char *skb_end_pointer(const struct sk_buff *skb)
{
	return skb->end;
}

static inline unsigned char *skb_put(struct sk_buff *skb, unsigned int len)
{
	unsigned char *tmp = skb_tail_pointer(skb);
	//SKB_LINEAR_ASSERT(skb);
	skb->tail += len;
	skb->len  += len;
	return tmp;
}

static inline unsigned char *__skb_pull(struct sk_buff *skb, unsigned int len)
{
	skb->len -= len;
	if(skb->len < skb->data_len) {
		// alic printf("%s(),%d,error!\n",__FUNCTION__,__LINE__);
    }
	return skb->data += len;
}
static inline unsigned char *skb_pull(struct sk_buff *skb, unsigned int len)
{
	#ifdef PLATFORM_FREEBSD
	return __skb_pull(skb, len);
	#else
	return unlikely(len > skb->len) ? NULL : __skb_pull(skb, len);
	#endif //PLATFORM_FREEBSD
}
static inline u32 skb_queue_len(const struct sk_buff_head *list_)
{
	return list_->qlen;
}
static inline void __skb_insert(struct sk_buff *newsk,
				struct sk_buff *prev, struct sk_buff *next,
				struct sk_buff_head *list)
{
	newsk->next = next;
	newsk->prev = prev;
	next->prev  = prev->next = newsk;
	list->qlen++;
}
static inline void __skb_queue_before(struct sk_buff_head *list,
				      struct sk_buff *next,
				      struct sk_buff *newsk)
{
	__skb_insert(newsk, next->prev, next, list);
}
static inline void __skb_queue_after(struct sk_buff_head *list,
				     struct sk_buff *prev,
				     struct sk_buff *newsk)
{
	__skb_insert(newsk, prev, prev->next, list);
}
static inline void __skb_queue_head(struct sk_buff_head *list,
				    struct sk_buff *newsk)
{
	__skb_queue_after(list, (struct sk_buff *)list, newsk);
}
static inline void skb_queue_head(struct sk_buff_head *list, struct sk_buff *newsk)
{
	unsigned long flags;

	spin_lock_irqsave(&list->lock, flags);
	__skb_queue_head(list, newsk);
	spin_unlock_irqrestore(&list->lock, flags);
}
static inline void skb_queue_tail(struct sk_buff_head *list,
				   struct sk_buff *newsk)
{
	mtx_lock(&list->lock);
	__skb_queue_before(list, (struct sk_buff *)list, newsk);
	mtx_unlock(&list->lock);
}
static inline struct sk_buff *skb_peek(struct sk_buff_head *list_)
{
	struct sk_buff *list = ((struct sk_buff *)list_)->next;
	if (list == (struct sk_buff *)list_)
		list = NULL;
	return list;
}
static inline void __skb_unlink(struct sk_buff *skb, struct sk_buff_head *list)
{
	struct sk_buff *next, *prev;

	list->qlen--;
	next	   = skb->next;
	prev	   = skb->prev;
	skb->next  = skb->prev = NULL;
	next->prev = prev;
	prev->next = next;
}

static inline struct sk_buff *skb_dequeue(struct sk_buff_head *list)
{
	mtx_lock(&list->lock);

	struct sk_buff *skb = skb_peek(list);
	if (skb)
		__skb_unlink(skb, list);

	mtx_unlock(&list->lock);

	return skb;
}
static inline void skb_reserve(struct sk_buff *skb, int len)
{
	skb->data += len;
	skb->tail += len;
}
static inline void __skb_queue_head_init(struct sk_buff_head *list)
{
	list->prev = list->next = (struct sk_buff *)list;
	list->qlen = 0;
}
/*
 * This function creates a split out lock class for each invocation;
 * this is needed for now since a whole lot of users of the skb-queue
 * infrastructure in drivers have different locking usage (in hardirq)
 * than the networking core (in softirq only). In the long run either the
 * network layer or drivers should need annotation to consolidate the
 * main types of usage into 3 classes.
 */
static inline void skb_queue_head_init(struct sk_buff_head *list)
{
	_rtw_spinlock_init(&list->lock);
	__skb_queue_head_init(list);
}

static inline unsigned char *skb_push(struct sk_buff *skb, unsigned int len)
{
	skb->data -= len;
	skb->len  += len;
// alic	if (unlikely(skb->data<skb->head))
// alic		skb_under_panic(skb, len, __builtin_return_address(0));
	return skb->data;
}

static inline u16 skb_get_queue_mapping(const struct sk_buff *skb)
{
	return skb->queue_mapping;
}

static inline void skb_set_queue_mapping(struct sk_buff *skb, u16 queue_mapping)
{
	skb->queue_mapping = queue_mapping;
}

static inline void __skb_trim(struct sk_buff *skb, unsigned int len)
{
	if (unlikely(skb->data_len)) {
		WARN_ON(1);
		return;
	}
	skb->len = len;
	skb_set_tail_pointer(skb, len);
}

static inline void skb_trim(struct sk_buff *skb, unsigned int len)
{
	if (skb->len > len)
		__skb_trim(skb, len);
}

static inline int skb_is_nonlinear(const struct sk_buff *skb)
{
	return skb->data_len;
}

static inline int skb_tailroom(const struct sk_buff *skb)
{
	return skb_is_nonlinear(skb) ? 0 : skb->end - skb->tail;
}

static inline unsigned int skb_headroom(const struct sk_buff *skb)
{
	return skb->data - skb->head;
}

static inline void skb_reset_mac_header(struct sk_buff *skb)
{
	skb->mac_header = (sk_buff_data_t)(skb->data - skb->head);
}

static inline int skb_linearize(struct sk_buff *skb)
{
	return skb_is_nonlinear(skb) ? 0 /* alic __skb_linearize(skb) */ : 0;
}

static inline unsigned char *skb_pull_inline(struct sk_buff *skb, unsigned int len)
{
	return unlikely(len > skb->len) ? NULL : __skb_pull(skb, len);
}

static inline unsigned char *skb_network_header(const struct sk_buff *skb)
{
	return skb->network_header;
}
static inline unsigned char *skb_mac_header(const struct sk_buff *skb)
{
	return skb->mac_header;
}

static inline struct ethhdr *eth_hdr(const struct sk_buff *skb)
{
	return (struct ethhdr *)skb_mac_header(skb);
}

static inline struct iphdr *ip_hdr(const struct sk_buff *skb)
{
	return (struct iphdr *)skb_network_header(skb);
}

#if WIRELESS_EXT < 13
struct iw_request_info {
	__u16 cmd;
	__u16 flags;
};

typedef int (*iw_handler) (struct net_device *dev,
			   struct iw_request_info *info,
			   union iwreq_data *wrqu, char *extra);
#endif

#define KERN_INFO
#define WIRELESS_EXT -1

struct ieee80211_radiotap_header {
	u8 it_version;		/* Version 0. Only increases
				 * for drastic changes,
				 * introduction of compatible
				 * new fields does not count.
				 */
	u8 it_pad;
	__le16 it_len;		/* length of the whole
				 * header in bytes, including
				 * it_version, it_pad,
				 * it_len, and data fields.
				 */
	__le32 it_present;	/* A bitmap telling which
				 * fields are present. Set bit 31
				 * (0x80000000) to extend the
				 * bitmap by another 32 bits.
				 * Additional extensions are made
				 * by setting bit 31.
				 */
} __packed;

enum ieee80211_radiotap_type {
	IEEE80211_RADIOTAP_TSFT = 0,
	IEEE80211_RADIOTAP_FLAGS = 1,
	IEEE80211_RADIOTAP_RATE = 2,
	IEEE80211_RADIOTAP_CHANNEL = 3,
	IEEE80211_RADIOTAP_FHSS = 4,
	IEEE80211_RADIOTAP_DBM_ANTSIGNAL = 5,
	IEEE80211_RADIOTAP_DBM_ANTNOISE = 6,
	IEEE80211_RADIOTAP_LOCK_QUALITY = 7,
	IEEE80211_RADIOTAP_TX_ATTENUATION = 8,
	IEEE80211_RADIOTAP_DB_TX_ATTENUATION = 9,
	IEEE80211_RADIOTAP_DBM_TX_POWER = 10,
	IEEE80211_RADIOTAP_ANTENNA = 11,
	IEEE80211_RADIOTAP_DB_ANTSIGNAL = 12,
	IEEE80211_RADIOTAP_DB_ANTNOISE = 13,
	IEEE80211_RADIOTAP_RX_FLAGS = 14,
	IEEE80211_RADIOTAP_TX_FLAGS = 15,
	IEEE80211_RADIOTAP_RTS_RETRIES = 16,
	IEEE80211_RADIOTAP_DATA_RETRIES = 17,

	IEEE80211_RADIOTAP_MCS = 19,

	/* valid in every it_present bitmap, even vendor namespaces */
	IEEE80211_RADIOTAP_RADIOTAP_NAMESPACE = 29,
	IEEE80211_RADIOTAP_VENDOR_NAMESPACE = 30,
	IEEE80211_RADIOTAP_EXT = 31
};

/* Channel flags. */
#define	IEEE80211_CHAN_TURBO	0x0010	/* Turbo channel */
#define	IEEE80211_CHAN_CCK	0x0020	/* CCK channel */
#define	IEEE80211_CHAN_OFDM	0x0040	/* OFDM channel */
#define	IEEE80211_CHAN_2GHZ	0x0080	/* 2 GHz spectrum channel. */
#define	IEEE80211_CHAN_5GHZ	0x0100	/* 5 GHz spectrum channel */
#define	IEEE80211_CHAN_PASSIVE	0x0200	/* Only passive scan allowed */
#define	IEEE80211_CHAN_DYN	0x0400	/* Dynamic CCK-OFDM channel */
#define	IEEE80211_CHAN_GFSK	0x0800	/* GFSK channel (FHSS PHY) */

/* For IEEE80211_RADIOTAP_FLAGS */
#define	IEEE80211_RADIOTAP_F_CFP	0x01	/* sent/received
						 * during CFP
						 */
#define	IEEE80211_RADIOTAP_F_SHORTPRE	0x02	/* sent/received
						 * with short
						 * preamble
						 */
#define	IEEE80211_RADIOTAP_F_WEP	0x04	/* sent/received
						 * with WEP encryption
						 */
#define	IEEE80211_RADIOTAP_F_FRAG	0x08	/* sent/received
						 * with fragmentation
						 */
#define	IEEE80211_RADIOTAP_F_FCS	0x10	/* frame includes FCS */
#define	IEEE80211_RADIOTAP_F_DATAPAD	0x20	/* frame has padding between
						 * 802.11 header and payload
						 * (to 32-bit boundary)
						 */
#define IEEE80211_RADIOTAP_F_BADFCS	0x40	/* bad FCS */

/* For IEEE80211_RADIOTAP_RX_FLAGS */
#define IEEE80211_RADIOTAP_F_RX_BADPLCP	0x0002	/* frame has bad PLCP */

/* For IEEE80211_RADIOTAP_TX_FLAGS */
#define IEEE80211_RADIOTAP_F_TX_FAIL	0x0001	/* failed due to excessive
						 * retries */
#define IEEE80211_RADIOTAP_F_TX_CTS	0x0002	/* used cts 'protection' */
#define IEEE80211_RADIOTAP_F_TX_RTS	0x0004	/* used rts/cts handshake */


/* For IEEE80211_RADIOTAP_MCS */
#define IEEE80211_RADIOTAP_MCS_HAVE_BW		0x01
#define IEEE80211_RADIOTAP_MCS_HAVE_MCS		0x02
#define IEEE80211_RADIOTAP_MCS_HAVE_GI		0x04
#define IEEE80211_RADIOTAP_MCS_HAVE_FMT		0x08
#define IEEE80211_RADIOTAP_MCS_HAVE_FEC		0x10

#define IEEE80211_RADIOTAP_MCS_BW_MASK		0x03
#define		IEEE80211_RADIOTAP_MCS_BW_20	0
#define		IEEE80211_RADIOTAP_MCS_BW_40	1
#define		IEEE80211_RADIOTAP_MCS_BW_20L	2
#define		IEEE80211_RADIOTAP_MCS_BW_20U	3
#define IEEE80211_RADIOTAP_MCS_SGI		0x04
#define IEEE80211_RADIOTAP_MCS_FMT_GF		0x08
#define IEEE80211_RADIOTAP_MCS_FEC_LDPC		0x10

/* Base version of the radiotap packet header data */
#define PKTHDR_RADIOTAP_VERSION		0

#define LOWER 	_TRUE
#define RAISE	_FALSE

#define __be16 u16
#define __be32 u32
#define __sum16 u16

struct udphdr {
	__be16	source;
	__be16	dest;
	__be16	len;
	__sum16	check;
};

#define u_int8_t u8
#define u_int16_t u16
#define u_int32_t u32

/* Standard well-defined IP protocols.  */
enum {
  IPPROTO_IP = 0,		/* Dummy protocol for TCP		*/
  IPPROTO_ICMP = 1,		/* Internet Control Message Protocol	*/
  IPPROTO_IGMP = 2,		/* Internet Group Management Protocol	*/
  IPPROTO_IPIP = 4,		/* IPIP tunnels (older KA9Q tunnels use 94) */
  IPPROTO_TCP = 6,		/* Transmission Control Protocol	*/
  IPPROTO_EGP = 8,		/* Exterior Gateway Protocol		*/
  IPPROTO_PUP = 12,		/* PUP protocol				*/
  IPPROTO_UDP = 17,		/* User Datagram Protocol		*/
  IPPROTO_IDP = 22,		/* XNS IDP protocol			*/
  IPPROTO_DCCP = 33,		/* Datagram Congestion Control Protocol */
  IPPROTO_RSVP = 46,		/* RSVP protocol			*/
  IPPROTO_GRE = 47,		/* Cisco GRE tunnels (rfc 1701,1702)	*/

  IPPROTO_IPV6	 = 41,		/* IPv6-in-IPv4 tunnelling		*/

  IPPROTO_ESP = 50,            /* Encapsulation Security Payload protocol */
  IPPROTO_AH = 51,             /* Authentication Header protocol       */
  IPPROTO_BEETPH = 94,	       /* IP option pseudo header for BEET */
  IPPROTO_PIM    = 103,		/* Protocol Independent Multicast	*/

  IPPROTO_COMP   = 108,                /* Compression Header protocol */
  IPPROTO_SCTP   = 132,		/* Stream Control Transport Protocol	*/
  IPPROTO_UDPLITE = 136,	/* UDP-Lite (RFC 3828)			*/

  IPPROTO_RAW	 = 255,		/* Raw IP packets			*/
  IPPROTO_MAX
};

struct in6_addr {
	union {
		__u8		u6_addr8[16];
		__be16		u6_addr16[8];
		__be32		u6_addr32[4];
	} in6_u;
#define s6_addr			in6_u.u6_addr8
#define s6_addr16		in6_u.u6_addr16
#define s6_addr32		in6_u.u6_addr32
};

struct ipv6hdr {
#if defined(CONFIG_BIG_ENDIAN)
	__u8			priority:4,
				version:4;
#elif defined(CONFIG_LITTLE_ENDIAN)
	__u8			version:4,
				priority:4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	__u8			flow_lbl[3];

	__be16			payload_len;
	__u8			nexthdr;
	__u8			hop_limit;

	struct	in6_addr	saddr;
	struct	in6_addr	daddr;
};

struct icmp6hdr {

	__u8		icmp6_type;
	__u8		icmp6_code;
	__sum16		icmp6_cksum;


	union {
		__be32			un_data32[1];
		__be16			un_data16[2];
		__u8			un_data8[4];

		struct icmpv6_echo {
			__be16		identifier;
			__be16		sequence;
		} u_echo;

                struct icmpv6_nd_advt {
#if defined(CONFIG_LITTLE_ENDIAN)
                        __u32		reserved:5,
                        		override:1,
                        		solicited:1,
                        		router:1,
					reserved2:24;
#elif defined(CONFIG_BIG_ENDIAN)
                        __u32		router:1,
					solicited:1,
                        		override:1,
                        		reserved:29;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
                } u_nd_advt;

                struct icmpv6_nd_ra {
			__u8		hop_limit;
#if defined(CONFIG_LITTLE_ENDIAN)
			__u8		reserved:3,
					router_pref:2,
					home_agent:1,
					other:1,
					managed:1;

#elif defined(CONFIG_BIG_ENDIAN)
			__u8		managed:1,
					other:1,
					home_agent:1,
					router_pref:2,
					reserved:3;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
			__be16		rt_lifetime;
                } u_nd_ra;

	} icmp6_dataun;

#define icmp6_identifier	icmp6_dataun.u_echo.identifier
#define icmp6_sequence		icmp6_dataun.u_echo.sequence
#define icmp6_pointer		icmp6_dataun.un_data32[0]
#define icmp6_mtu		icmp6_dataun.un_data32[0]
#define icmp6_unused		icmp6_dataun.un_data32[0]
#define icmp6_maxdelay		icmp6_dataun.un_data16[0]
#define icmp6_router		icmp6_dataun.u_nd_advt.router
#define icmp6_solicited		icmp6_dataun.u_nd_advt.solicited
#define icmp6_override		icmp6_dataun.u_nd_advt.override
#define icmp6_ndiscreserved	icmp6_dataun.u_nd_advt.reserved
#define icmp6_hop_limit		icmp6_dataun.u_nd_ra.hop_limit
#define icmp6_addrconf_managed	icmp6_dataun.u_nd_ra.managed
#define icmp6_addrconf_other	icmp6_dataun.u_nd_ra.other
#define icmp6_rt_lifetime	icmp6_dataun.u_nd_ra.rt_lifetime
#define icmp6_router_pref	icmp6_dataun.u_nd_ra.router_pref
};

/*
 *	IPV6 extension headers
 */
#define IPPROTO_HOPOPTS		0	/* IPv6 hop-by-hop options	*/
#define IPPROTO_ROUTING		43	/* IPv6 routing header		*/
#define IPPROTO_FRAGMENT	44	/* IPv6 fragmentation header	*/
#define IPPROTO_ICMPV6		58	/* ICMPv6			*/
#define IPPROTO_NONE		59	/* IPv6 no next header		*/
#define IPPROTO_DSTOPTS		60	/* IPv6 destination options	*/
#define IPPROTO_MH		135	/* IPv6 mobility header		*/

/* Codes to identify message types */
#define PADI_CODE	0x09
#define PADO_CODE	0x07
#define PADR_CODE	0x19
#define PADS_CODE	0x65
#define PADT_CODE	0xa7
struct pppoe_tag {
	__be16 tag_type;
	__be16 tag_len;
	char tag_data[0];
} __attribute__ ((packed));

/* Tag identifiers */
#define PTT_EOL		__cpu_to_be16(0x0000)
#define PTT_SRV_NAME	__cpu_to_be16(0x0101)
#define PTT_AC_NAME	__cpu_to_be16(0x0102)
#define PTT_HOST_UNIQ	__cpu_to_be16(0x0103)
#define PTT_AC_COOKIE	__cpu_to_be16(0x0104)
#define PTT_VENDOR 	__cpu_to_be16(0x0105)
#define PTT_RELAY_SID	__cpu_to_be16(0x0110)
#define PTT_SRV_ERR     __cpu_to_be16(0x0201)
#define PTT_SYS_ERR  	__cpu_to_be16(0x0202)
#define PTT_GEN_ERR  	__cpu_to_be16(0x0203)

struct pppoe_hdr {
#if defined(CONFIG_LITTLE_ENDIAN)
	__u8 ver : 4;
	__u8 type : 4;
#elif defined(CONFIG_BIG_ENDIAN)
	__u8 type : 4;
	__u8 ver : 4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	__u8 code;
	__be16 sid;
	__be16 length;
	struct pppoe_tag tag[0];
} __attribute__((packed));

struct ddpehdr {
	__be16	deh_len_hops;	/* lower 10 bits are length, next 4 - hops */
	__be16	deh_sum;
	__be16	deh_dnet;
	__be16	deh_snet;
	__u8	deh_dnode;
	__u8	deh_snode;
	__u8	deh_dport;
	__u8	deh_sport;
	/* And netatalk apps expect to stick the type in themselves */
};

#define ETH_ALEN	6

/* AppleTalk AARP headers */
struct elapaarp {
	__be16	hw_type;
#define AARP_HW_TYPE_ETHERNET		1
#define AARP_HW_TYPE_TOKENRING		2
	__be16	pa_type;
	__u8	hw_len;
	__u8	pa_len;
#define AARP_PA_ALEN			4
	__be16	function;
#define AARP_REQUEST			1
#define AARP_REPLY			2
#define AARP_PROBE			3
	__u8	hw_src[ETH_ALEN];
	__u8	pa_src_zero;
	__be16	pa_src_net;
	__u8	pa_src_node;
	__u8	hw_dst[ETH_ALEN];
	__u8	pa_dst_zero;
	__be16	pa_dst_net;
	__u8	pa_dst_node;
} __attribute__ ((packed));

#define IPX_NODE_LEN	6
#define IPX_MTU		576

struct ipx_address {
	__be32  net;
	__u8    node[IPX_NODE_LEN];
	__be16  sock;
};

struct ipxhdr {
	__be16			ipx_checksum;
#define IPX_NO_CHECKSUM	cpu_to_be16(0xFFFF)
	__be16			ipx_pktsize;
	__u8			ipx_tctrl;
	__u8			ipx_type;
#define IPX_TYPE_UNKNOWN	0x00
#define IPX_TYPE_RIP		0x01	/* may also be 0 */
#define IPX_TYPE_SAP		0x04	/* may also be 0 */
#define IPX_TYPE_SPX		0x05	/* SPX protocol */
#define IPX_TYPE_NCP		0x11	/* $lots for docs on this (SPIT) */
#define IPX_TYPE_PPROP		0x14	/* complicated flood fill brdcast */
	struct ipx_address	ipx_dest;
	struct ipx_address	ipx_source;
};

#define NDISC_ROUTER_SOLICITATION	133
#define NDISC_ROUTER_ADVERTISEMENT	134
#define NDISC_NEIGHBOUR_SOLICITATION	135
#define NDISC_NEIGHBOUR_ADVERTISEMENT	136
#define NDISC_REDIRECT			137

struct arphdr {
	__be16		ar_hrd;		/* format of hardware address	*/
	__be16		ar_pro;		/* format of protocol address	*/
	unsigned char	ar_hln;		/* length of hardware address	*/
	unsigned char	ar_pln;		/* length of protocol address	*/
	__be16		ar_op;		/* ARP opcode (command)		*/

#if 0
	 /*
	  *	 Ethernet looks like this : This bit is variable sized however...
	  */
	unsigned char		ar_sha[ETH_ALEN];	/* sender hardware address	*/
	unsigned char		ar_sip[4];		/* sender IP address		*/
	unsigned char		ar_tha[ETH_ALEN];	/* target hardware address	*/
	unsigned char		ar_tip[4];		/* target IP address		*/
#endif

};

/*
 *	The DEVICE structure.
 *	Actually, this whole structure is a big mistake.  It mixes I/O
 *	data with strictly "high-level" data, and it has to know about
 *	almost every data structure used in the INET module.
 *
 *	FIXME: cleanup struct net_device such that network protocol info
 *	moves out.
 */
struct net_device {
    /*
	 * This is the first field of the "visible" part of this structure
	 * (i.e. as seen by users in the "Space.c" file).  It is the name
	 * of the interface.
	 */
	char			name[IFNAMSIZ];

    unsigned char dev_addr[ETH_ALEN];
    void *br_port;
    u32 type;
    void *priv; // _adapter *
};

#endif

#endif

