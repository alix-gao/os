/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : atomic.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2013-04-22
 ***************************************************************/

#ifndef __ATOMIC_H__
#define __ATOMIC_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/
/***************************************************************
 * description : ÈÎÎñ¾ä±ú
 ***************************************************************/
typedef struct { volatile int counter; } atomic_t;

/***************************************************************
 extern function
 ***************************************************************/

#define atomic_read(v) ((v)->counter)

#define atomic_set(v, i) (((v)->counter) = (i))

static __inline__ void atomic_init(atomic_t *v)
{
    v->counter = 0;
}

static __inline__ void atomic_add(atomic_t *v, int i)
{
    __asm__ __volatile__("lock; addl %1,%0"
                         :"=m"(v->counter)
                         :"ir"(i), "m"(v->counter));
}

static __inline__ void atomic_sub(atomic_t *v, int i)
{
    __asm__ __volatile__("lock; subl %1,%0"
                         :"=m"(v->counter)
                         :"ir"(i), "m"(v->counter));
}

static __inline__ void atomic_inc(atomic_t *v)
{
    __asm__ __volatile__("lock; incl %1"
                         :"=m"(v->counter)
                         :"m"(v->counter));
}

static __inline__ void atomic_dec(atomic_t *v)
{
    __asm__ __volatile__("lock; decl %1"
                         :"=m"(v->counter)
                         :"m"(v->counter));
}

#pragma pack()

#endif /* end of header */

