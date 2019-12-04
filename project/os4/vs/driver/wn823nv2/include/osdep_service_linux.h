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

#define UNDEFINED_FUNCTION \
    do { \
        flog("undefined fucntion %s\n", __FUNCTION__); \
    } while (0)

/* for debug error start */
//#define snprintf(buf, n, fmt, arg...) snprint(buf, n, fmt, ##arg)
#define sprintf(buf, fmt, arg...) snprintf(buf, UINT32_MAX, fmt, ##arg)
/* for debug error end */

/* for link error star */
#define printk(fmt, arg...) flog(fmt, ##arg)
#define seq_printf(fmt, arg...) flog(fmt, ##arg),OS_TRUE

#if 1
#define mtx_lock(o) do { /* flog("lock %s %d\n", __FUNCTION__, __LINE__);wtflog(); */spin_lock(o); } while (0)
#define mtx_unlock(o) do { /* flog("unlock %s %d\n", __FUNCTION__, __LINE__);wtflog(); */spin_unlock(o); } while (0)
#define spin_lock_bh(o) do { /* flog("lock %s %d\n", __FUNCTION__, __LINE__);wtflog(); */spin_lock(o); } while (0)
#define spin_unlock_bh(o) do { /* flog("unlock %s %d\n", __FUNCTION__, __LINE__);wtflog(); */spin_unlock(o); } while (0)
#define spin_lock_irqsave(o,t) do { /* flog("lock %s %d\n", __FUNCTION__, __LINE__);wtflog(); */spin_lock(o); } while (0)
#define spin_unlock_irqsave(o,t) do { /* flog("unlock %s %d\n", __FUNCTION__, __LINE__);wtflog(); */spin_unlock(o); } while (0)
#define spin_lock_irqrestore(o,t) do { /* flog("lock %s %d\n", __FUNCTION__, __LINE__);wtflog(); */spin_lock(o); } while (0)
#define spin_unlock_irqrestore(o,t) do { /* flog("unlock %s %d\n", __FUNCTION__, __LINE__);wtflog(); */spin_unlock(o); } while (0)
#else
#define mtx_lock(o) do { spin_lock(o); } while (0)
#define mtx_unlock(o) do { spin_unlock(o); } while (0)
#define spin_lock_bh(o) do { spin_lock(o); } while (0)
#define spin_unlock_bh(o) do { spin_unlock(o); } while (0)
#define spin_lock_irqsave(o,t) do { spin_lock(o); } while (0)
#define spin_unlock_irqsave(o,t) do { spin_unlock(o); } while (0)
#define spin_lock_irqrestore(o,t) do { spin_lock(o); } while (0)
#define spin_unlock_irqrestore(o,t) do { spin_unlock(o); } while (0)
#endif

#define netdev_priv(a) (a)
#define SET_NETDEV_DEV(net, pdev) (net)->priv = (pdev)

/* return value is meanless */
#define del_timer_sync(ptimer) do { kill_timer(ptimer->handle); ptimer->handle = OS_NULL; } while (0)

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
//#define udelay(n) delay_us(n)

#define yield schedule

#define up(sema) notify_mevent(*(sema), __LINE__)
#define down_interruptible(sema) wait_mevent(*(sema), 0)

#define free_netdev(d)

# define do_div(n,base) ({					\
	u32 __base = (base);				\
	u32 __rem;						\
	__rem = ((u64)(n)) % __base;			\
	(n) = ((u64)(n)) / __base;				\
	__rem;							\
 })

#define interface_to_usbdev(a) a

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

#define thread_enter(name)
#define flush_signals_thread()
#define complete_and_exit(a,b)
/* for link error end */

#define   KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))

#ifndef IW_CUSTOM_MAX
#define IW_CUSTOM_MAX 256
#endif

#define GFP_KERNEL 0

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

#ifdef RTK_DMP_PLATFORM
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12))
	#include <linux/pageremap.h>
#endif
	#include <asm/io.h>
#endif
#endif

#ifdef CONFIG_NET_RADIO
	#define CONFIG_WIRELESS_EXT
#endif

	/* Monitor mode */
//	#include <net/ieee80211_radiotap.h>

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
#if 0 // alic
#include <linux/usb.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21))
	#include <linux/usb_ch9.h>
#else
	#include <linux/usb/ch9.h>
#endif
#endif
#endif

#ifdef CONFIG_BT_COEXIST_SOCKET_TRX
	#include <net/sock.h>
	#include <net/tcp.h>
	#include <linux/udp.h>
	#include <linux/in.h>
	#include <linux/netlink.h>
#endif //CONFIG_BT_COEXIST_SOCKET_TRX
#if 0 // alic
#ifdef CONFIG_USB_HCI
	typedef struct urb *  PURB;
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,22))
#ifdef CONFIG_USB_SUSPEND
#define CONFIG_AUTOSUSPEND	1
#endif
#endif
#endif
#endif

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif
#define __user

#define __u32 u32
#define __u16 u16
#define __u8 u8
#define __s32 s32
#define __s16 s16
#define __s8 s8

#if 0 // alic mod for scan
#define container_of(p,t,n) (t*)((p)-&(((t*)0)->n))
#else
#define container_of(p,t,n) (t*)((u32)(p) - (u32) &(((t*)0)->n))
#endif

	typedef HEVENT _sema;
	typedef	spinlock_t	_lock;
    typedef HEVENT _mutex;
    typedef struct {
        HTIMER handle;
        TIMER_FUNC_PTR function;
        void *data;
    } _timer;

	struct	__queue	{
		struct	list_node	queue;
		_lock	lock;
	};

	typedef	struct sk_buff	_pkt;
	typedef unsigned char	_buffer;

	typedef struct	__queue	_queue;
	typedef struct	list_node	_list;
	typedef	int	_OS_STATUS;
	//typedef u32	_irqL;
	typedef unsigned long _irqL;

struct net_device {
    unsigned short		type;
    void *priv; // _adapter *
};
	typedef	struct	net_device * _nic_hdl;

	typedef HTASK		_thread_hdl_;
	typedef int		thread_return;
	typedef void*	thread_context;

	#define thread_exit() complete_and_exit(NULL, 0)

	typedef void timer_hdl_return;
	typedef void* timer_hdl_context;
#if 0 // alic
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,5,41))
	typedef struct work_struct _workitem;
#else
	typedef struct tq_struct _workitem;
#endif
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
	#define DMA_BIT_MASK(n) (((n) == 64) ? ~0ULL : ((1ULL<<(n))-1))
#endif

    typedef unsigned char *sk_buff_data_t;

struct sk_buff {
	/* These two members must be first. */
	struct sk_buff		*next;
	struct sk_buff		*prev;

	/*
	 * This is the control buffer. It is free to use for every
	 * layer. Please put your private variables there. If you
	 * want to keep them across layers you have to do a skb_clone()
	 * first. This is owned by whoever has the skb queued ATM.
	 */
	char			cb[48] _aligned_(8);

    u8			pkt_type;
    u8 ip_summed;

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

	_nic_hdl dev;
};

struct sk_buff_head {
	/* These two members must be first. */
	struct sk_buff	*next;
	struct sk_buff	*prev;

	u32		qlen;
	_lock	lock;
};
#define skb_tail_pointer(skb)	skb->tail
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
	if(skb->len < skb->data_len)
		printf("%s(),%d,error!\n",__FUNCTION__,__LINE__);
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
static inline void skb_queue_tail(struct sk_buff_head *list,
				   struct sk_buff *newsk)
{
	mtx_lock(&list->lock);
	__skb_queue_before(list, (struct sk_buff *)list, newsk);
	mtx_unlock(&list->lock);
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
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22))
// Porting from linux kernel, for compatible with old kernel.
#if 0 // alic
static inline unsigned char *skb_tail_pointer(const struct sk_buff *skb)
{
	return skb->tail;
}
#endif
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
static inline unsigned char *skb_push(struct sk_buff *skb, unsigned int len)
{
	skb->data -= len;
	skb->len  += len;
	return skb->data;
}

static inline unsigned int skb_headroom(const struct sk_buff *skb)
{
	return skb->data - skb->head;
}
static inline void skb_reset_mac_header(struct sk_buff *skb)
{
	skb->mac_header = skb->data;
}

void	_rtw_spinlock_init(_lock *plock);
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
    enter_critical_section(*pmutex);
    return 0;
#if 0
	int ret = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	//mutex_lock(pmutex);
	ret = mutex_lock_interruptible(pmutex);
#else
	ret = down_interruptible(pmutex);
#endif
	return ret;
#endif
}


__inline static void _exit_critical_mutex(_mutex *pmutex, _irqL *pirqL)
{
    leave_critical_section(*pmutex);
#if 0
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
		mutex_unlock(pmutex);
#else
		up(pmutex);
#endif
#endif
}

__inline static void rtw_list_delete(_list *plist)
{
	list_del_init(plist);
}

#define RTW_TIMER_HDL_ARGS os_u32 FunctionContext

extern _timer *trace_timer;

__inline static void _init_timer(_timer *ptimer,_nic_hdl nic_hdl,TIMER_FUNC_PTR pfunc,void* cntx)
{
    ptimer->function = pfunc;
    ptimer->data = cntx;
    ptimer->handle = OS_NULL;
#if 0
	//setup_timer(ptimer, pfunc,(u32)cntx);
	ptimer->function = pfunc;
	ptimer->data = (unsigned long)cntx;
	init_timer(ptimer);
#endif
}

#if 0
extern int debug_timer_cnt;
#define _set_timer(tt, delay_time) \
    do { \
        _timer *ptimer = (tt); \
        if (ptimer->handle) { \
            kill_timer(ptimer->handle); \
        } \
        debug_timer_cnt++; \
        if (0 >= debug_timer_cnt) { \
            print("set timer %s\n", __FUNCTION__); \
        } \
        ptimer->handle = set_timer_callback((u32) ptimer->data, divl_cell((delay_time), 1000/OS_HZ), ptimer->function, TIMER_MODE_NOT_LOOP); \
        if (OS_NULL == ptimer->handle) { \
            flog("set timer fail\n"); \
        } \
    } while (0)
#else // dead
__inline static void _set_timer(_timer *ptimer,u32 delay_time)
{
    if (ptimer->handle) {
        kill_timer(ptimer->handle);
    }
    ptimer->handle = set_timer_callback((u32) ptimer->data, divl_cell(delay_time, 1000/OS_HZ), ptimer->function, TIMER_MODE_NOT_LOOP);
    if (OS_NULL == ptimer->handle) {
        flog("set timer fail\n");
    }
//	mod_timer(ptimer , (jiffies+(delay_time*HZ/1000)));
}
#endif
__inline static void _cancel_timer(_timer *ptimer,u8 *bcancelled)
{
	del_timer_sync(ptimer);
	*bcancelled=  _TRUE;//TRUE ==1; FALSE==0
}

#if 0 // alic
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
#endif

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
#if 0 // alic
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
#endif

static inline void rtw_merge_string(char *dst, int dst_len, char *src1, char *src2)
{
	int	len = 0;
	len += snprintf(dst+len, dst_len - len, "%s", src1);
	len += snprintf(dst+len, dst_len - len, "%s", src2);
}
#if 0 // alic
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
#endif
// limitation of path length
#define PATH_LENGTH_MAX PATH_MAX

//Atomic integer operations
#define ATOMIC_T atomic_t

#define rtw_netdev_priv(netdev) ( netdev_priv(netdev)->priv )

#define NDEV_FMT "%s"
#define NDEV_ARG(ndev) ndev->name
#define ADPT_FMT "%s"
#define ADPT_ARG(adapter) "adpt"
#define FUNC_NDEV_FMT "%s(%s)"
#define FUNC_NDEV_ARG(ndev) __func__, "adpt"
#define FUNC_ADPT_FMT "%s(%s)"
#define FUNC_ADPT_ARG(adapter) __func__, "adpt"

#define KERN_WARNING

struct net_device *rtw_alloc_etherdev_with_old_priv(int sizeof_priv, void *old_priv);
extern struct net_device * rtw_alloc_etherdev(int sizeof_priv);

#define STRUCT_PACKED __attribute__ ((packed))

typedef unsigned short	sa_family_t;
struct sockaddr {
	sa_family_t	sa_family;	/* address family, AF_xxx	*/
	char		sa_data[14];	/* 14 bytes of protocol address	*/
};

struct	if_settings {};
struct  ifmap {};

struct ifreq {
#define IFHWADDRLEN	6
	union
	{
		char	ifrn_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	} ifr_ifrn;

	union {
		struct	sockaddr ifru_addr;
		struct	sockaddr ifru_dstaddr;
		struct	sockaddr ifru_broadaddr;
		struct	sockaddr ifru_netmask;
		struct  sockaddr ifru_hwaddr;
		short	ifru_flags;
		int	ifru_ivalue;
		int	ifru_mtu;
		struct  ifmap ifru_map;
		char	ifru_slave[IFNAMSIZ];	/* Just fits the size */
		char	ifru_newname[IFNAMSIZ];
		void __user *	ifru_data;
		struct	if_settings ifru_settings;
	} ifr_ifru;
};

#define IW_PRIV_TYPE_MASK	0x7000	/* Type of arguments */
#define IW_PRIV_TYPE_NONE	0x0000
#define IW_PRIV_TYPE_BYTE	0x1000	/* Char as number */
#define IW_PRIV_TYPE_CHAR	0x2000	/* Char as character */
#define IW_PRIV_TYPE_INT	0x4000	/* 32 bits int */
#define IW_PRIV_TYPE_FLOAT	0x5000	/* struct iw_freq */
#define IW_PRIV_TYPE_ADDR	0x6000	/* struct sockaddr */

#define IW_PRIV_SIZE_FIXED	0x0800	/* Variable or fixed number of args */

#define IW_PRIV_SIZE_MASK	0x07FF	/* Max number of those args */

struct	iw_priv_args
{
	u32		cmd;		/* Number of the ioctl to issue */
	u16		set_args;	/* Type and number of args */
	u16		get_args;	/* Type and number of args */
	char		name[IFNAMSIZ];	/* Name of the extension */
};

#define strsep(s, ct) split_string((u8 **) s, ct)

struct	iw_freq
{
	s32		m;		/* Mantissa */
	s16		e;		/* Exponent */
	u8		i;		/* List index (when in range struct) */
	u8		flags;		/* Flags (fixed/auto) */
};

struct iw_request_info {
	u16 cmd;
	u16 flags;
};

/* Flags for encoding (along with the token) */
#define IW_ENCODE_INDEX		0x00FF	/* Token index (if needed) */
#define IW_ENCODE_FLAGS		0xFF00	/* Flags defined below */
#define IW_ENCODE_MODE		0xF000	/* Modes defined below */
#define IW_ENCODE_DISABLED	0x8000	/* Encoding disabled */
#define IW_ENCODE_ENABLED	0x0000	/* Encoding enabled */
#define IW_ENCODE_RESTRICTED	0x4000	/* Refuse non-encoded packets */
#define IW_ENCODE_OPEN		0x2000	/* Accept non-encoded packets */
#define IW_ENCODE_NOKEY		0x0800  /* Key is write only, so not present */
#define IW_ENCODE_TEMP		0x0400  /* Temporary key */

/* SIOCSIWENCODEEXT definitions */
#define IW_ENCODE_SEQ_MAX_SIZE	8
/* struct iw_encode_ext ->alg */
#define IW_ENCODE_ALG_NONE	0
#define IW_ENCODE_ALG_WEP	1
#define IW_ENCODE_ALG_TKIP	2
#define IW_ENCODE_ALG_CCMP	3
#define IW_ENCODE_ALG_PMK	4
#define IW_ENCODE_ALG_AES_CMAC	5
/* struct iw_encode_ext ->ext_flags */
#define IW_ENCODE_EXT_TX_SEQ_VALID	0x00000001
#define IW_ENCODE_EXT_RX_SEQ_VALID	0x00000002
#define IW_ENCODE_EXT_GROUP_KEY		0x00000004
#define IW_ENCODE_EXT_SET_TX_KEY	0x00000008
struct	iw_encode_ext
{
	u32		ext_flags; /* IW_ENCODE_EXT_* */
	u8		tx_seq[IW_ENCODE_SEQ_MAX_SIZE]; /* LSB first */
	u8		rx_seq[IW_ENCODE_SEQ_MAX_SIZE]; /* LSB first */
	struct sockaddr	addr; /* ff:ff:ff:ff:ff:ff for broadcast/multicast
			       * (group) keys or unicast address for
			       * individual keys */
	u16		alg; /* IW_ENCODE_ALG_* */
	u16		key_len;
	u8		key[0];
};

/* SIOCSIWAUTH/SIOCGIWAUTH struct iw_param flags */
#define IW_AUTH_INDEX		0x0FFF
#define IW_AUTH_FLAGS		0xF000
/* SIOCSIWAUTH/SIOCGIWAUTH parameters (0 .. 4095)
 * (IW_AUTH_INDEX mask in struct iw_param flags; this is the index of the
 * parameter that is being set/get to; value will be read/written to
 * struct iw_param value field) */
#define IW_AUTH_WPA_VERSION		0
#define IW_AUTH_CIPHER_PAIRWISE		1
#define IW_AUTH_CIPHER_GROUP		2
#define IW_AUTH_KEY_MGMT		3
#define IW_AUTH_TKIP_COUNTERMEASURES	4
#define IW_AUTH_DROP_UNENCRYPTED	5
#define IW_AUTH_80211_AUTH_ALG		6
#define IW_AUTH_WPA_ENABLED		7
#define IW_AUTH_RX_UNENCRYPTED_EAPOL	8
#define IW_AUTH_ROAMING_CONTROL		9
#define IW_AUTH_PRIVACY_INVOKED		10
#define IW_AUTH_CIPHER_GROUP_MGMT	11
#define IW_AUTH_MFP			12

struct	iw_param
{
  s32		value;		/* The value of the parameter itself */
  u8		fixed;		/* Hardware should not use auto select */
  u8		disabled;	/* Disable the feature */
  u16		flags;		/* Various specifc flags (if any) */
};

/* MLME requests (SIOCSIWMLME / struct iw_mlme) */
#define IW_MLME_DEAUTH		0
#define IW_MLME_DISASSOC	1
#define IW_MLME_AUTH		2
#define IW_MLME_ASSOC		3

struct	iw_mlme
{
	u16		cmd; /* IW_MLME_* */
	u16		reason_code;
	struct sockaddr	addr;
};

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

/* IWEVMICHAELMICFAILURE : struct iw_michaelmicfailure ->flags */
#define IW_MICFAILURE_KEY_ID	0x00000003 /* Key ID 0..3 */
#define IW_MICFAILURE_GROUP	0x00000004
#define IW_MICFAILURE_PAIRWISE	0x00000008
#define IW_MICFAILURE_STAKEY	0x00000010
#define IW_MICFAILURE_COUNT	0x00000060 /* 1 or 2 (0 = count not supported) */

#define CHECKSUM_NONE 0

/* Maximum bit rates in the range struct */
#define IW_MAX_BITRATES		32
/* Maximum number of size of encoding token available
 * they are listed in the range structure */
#define IW_MAX_ENCODING_SIZES	8
/* Maximum tx powers in the range struct */
#define IW_MAX_TXPOWER		8
/* Maximum frequencies in the range struct */
#define IW_MAX_FREQUENCIES	32

struct	iw_quality
{
	u8		qual;		/* link quality (%retries, SNR,
					   %missed beacons or better...) */
	u8		level;		/* signal level (dBm) */
	u8		noise;		/* noise level (dBm) */
	u8		updated;	/* Flags to know if updated */
};

struct	iw_range
{
	/* Informative stuff (to choose between different interface) */
	__u32		throughput;	/* To give an idea... */
	/* In theory this value should be the maximum benchmarked
	 * TCP/IP throughput, because with most of these devices the
	 * bit rate is meaningless (overhead an co) to estimate how
	 * fast the connection will go and pick the fastest one.
	 * I suggest people to play with Netperf or any benchmark...
	 */

	/* NWID (or domain id) */
	__u32		min_nwid;	/* Minimal NWID we are able to set */
	__u32		max_nwid;	/* Maximal NWID we are able to set */

	/* Old Frequency (backward compat - moved lower ) */
	__u16		old_num_channels;
	__u8		old_num_frequency;

	/* Scan capabilities */
	__u8		scan_capa; 	/* IW_SCAN_CAPA_* bit field */

	/* Wireless event capability bitmasks */
	__u32		event_capa[6];

	/* signal level threshold range */
	__s32		sensitivity;

	/* Quality of link & SNR stuff */
	/* Quality range (link, level, noise)
	 * If the quality is absolute, it will be in the range [0 ; max_qual],
	 * if the quality is dBm, it will be in the range [max_qual ; 0].
	 * Don't forget that we use 8 bit arithmetics... */
	struct iw_quality	max_qual;	/* Quality of the link */
	/* This should contain the average/typical values of the quality
	 * indicator. This should be the threshold between a "good" and
	 * a "bad" link (example : monitor going from green to orange).
	 * Currently, user space apps like quality monitors don't have any
	 * way to calibrate the measurement. With this, they can split
	 * the range between 0 and max_qual in different quality level
	 * (using a geometric subdivision centered on the average).
	 * I expect that people doing the user space apps will feedback
	 * us on which value we need to put in each driver... */
	struct iw_quality	avg_qual;	/* Quality of the link */

	/* Rates */
	__u8		num_bitrates;	/* Number of entries in the list */
	__s32		bitrate[IW_MAX_BITRATES];	/* list, in bps */

	/* RTS threshold */
	__s32		min_rts;	/* Minimal RTS threshold */
	__s32		max_rts;	/* Maximal RTS threshold */

	/* Frag threshold */
	__s32		min_frag;	/* Minimal frag threshold */
	__s32		max_frag;	/* Maximal frag threshold */

	/* Power Management duration & timeout */
	__s32		min_pmp;	/* Minimal PM period */
	__s32		max_pmp;	/* Maximal PM period */
	__s32		min_pmt;	/* Minimal PM timeout */
	__s32		max_pmt;	/* Maximal PM timeout */
	__u16		pmp_flags;	/* How to decode max/min PM period */
	__u16		pmt_flags;	/* How to decode max/min PM timeout */
	__u16		pm_capa;	/* What PM options are supported */

	/* Encoder stuff */
	__u16	encoding_size[IW_MAX_ENCODING_SIZES];	/* Different token sizes */
	__u8	num_encoding_sizes;	/* Number of entry in the list */
	__u8	max_encoding_tokens;	/* Max number of tokens */
	/* For drivers that need a "login/passwd" form */
	__u8	encoding_login_index;	/* token index for login token */

	/* Transmit power */
	__u16		txpower_capa;	/* What options are supported */
	__u8		num_txpower;	/* Number of entries in the list */
	__s32		txpower[IW_MAX_TXPOWER];	/* list, in bps */

	/* Wireless Extension version info */
	__u8		we_version_compiled;	/* Must be WIRELESS_EXT */
	__u8		we_version_source;	/* Last update of source */

	/* Retry limits and lifetime */
	__u16		retry_capa;	/* What retry options are supported */
	__u16		retry_flags;	/* How to decode max/min retry limit */
	__u16		r_time_flags;	/* How to decode max/min retry life */
	__s32		min_retry;	/* Minimal number of retries */
	__s32		max_retry;	/* Maximal number of retries */
	__s32		min_r_time;	/* Minimal retry lifetime */
	__s32		max_r_time;	/* Maximal retry lifetime */

	/* Frequency */
	__u16		num_channels;	/* Number of channels [0; num - 1] */
	__u8		num_frequency;	/* Number of entry in the list */
	struct iw_freq	freq[IW_MAX_FREQUENCIES];	/* list */
	/* Note : this frequency list doesn't need to fit channel numbers,
	 * because each entry contain its channel index */

	__u32		enc_capa;	/* IW_ENC_CAPA_* bit field */
};

#define WIRELESS_EXT -1

/* SIOCSIWPMKSA data */
#define IW_PMKSA_ADD		1
#define IW_PMKSA_REMOVE		2
#define IW_PMKSA_FLUSH		3

#define IW_PMKID_LEN	16

struct	iw_pmksa
{
	__u32		cmd; /* IW_PMKSA_* */
	struct sockaddr	bssid;
	__u8		pmkid[IW_PMKID_LEN];
};

/* Modes of operation */
#define IW_MODE_AUTO	0	/* Let the driver decides */
#define IW_MODE_ADHOC	1	/* Single cell network */
#define IW_MODE_INFRA	2	/* Multi cell network, roaming, ... */
#define IW_MODE_MASTER	3	/* Synchronisation master or Access Point */
#define IW_MODE_REPEAT	4	/* Wireless Repeater (forwarder) */
#define IW_MODE_SECOND	5	/* Secondary master/repeater (backup) */
#define IW_MODE_MONITOR	6	/* Passive monitor (listen only) */
#define IW_MODE_MESH	7	/* Mesh (IEEE 802.11s) network */

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

/* Frequency flags */
#define IW_FREQ_AUTO		0x00	/* Let the driver decides */
#define IW_FREQ_FIXED		0x01	/* Force a specific value */


#endif

