/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : initial.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __INITIAL_H__
#define __INITIAL_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

typedef void (*INIT_FUNC)(void);

#define __ABSTRACT __attribute__((unused,section(".ainit")))

#define __BUS __attribute__((unused,section(".binit")))

#define __DEVICE __attribute__((unused,section(".dinit")))

#define __MODULE __attribute__((unused,section(".minit")))

#define init_abstract_call(x) static INIT_FUNC __initcall_##x##__ __ABSTRACT = (x)

struct __init_bus_info {
    unsigned long priority;
    INIT_FUNC func;
};
#define init_bus_info(p, x) static struct __init_bus_info __initbus_##x##__ = { (p), (x) }; static unsigned int __initcall_##x##__ __BUS = (unsigned int) &__initbus_##x##__

#define init_device_call(x) static INIT_FUNC __initcall_##x##__ __DEVICE = (x)

#define init_module_call(x) static INIT_FUNC __initcall_##x##__ __MODULE = (x)

#define DUMMY_CODE __attribute__((unused,section(".dummy")))

#define BIOS_CODE __attribute__((unused,section(".bios")))

#define BDATA __attribute__((unused,section(".boot.data")))
#define STARTUP __attribute__((unused,section(".boot.startup")))

#define OS_INIT

#pragma pack()

#endif /* end of header */

