/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : instruction.h
 * version     : 1.0
 * description : cpu÷∏¡Ó∫Í∂®“Â
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

/***************************************************************
 include header file
 ***************************************************************/
#include <io.h>
#include <int.h>

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/

/* memory fence */
#define mb() __asm__ __volatile__("mfence":::"memory")
/* load fence */
#define rmb() __asm__ __volatile__("lfence":::"memory")
/* store fence */
#define wmb() __asm__ __volatile__("sfence" ::: "memory")

#define MSR_IA32_APICBASE 0x1b
#define MSR_IA32_APICBASE_BSP (1<<8)
#define MSR_IA32_APICBASE_ENABLE (1<<11)
#define MSR_IA32_APICBASE_BASE (0xfffff<<12)

#define rdmsr(msr,val1,val2) \
    __asm__ __volatile__("rdmsr" \
                       : "=a" (val1), "=d" (val2) \
                       : "c" (msr))

#define wrmsr(msr,val1,val2) \
    __asm__ __volatile__("wrmsr" \
                       : /* no outputs */ \
                       : "c" (msr), "a" (val1), "d" (val2))

/* Intel-defined CPU features, CPUID level 0x00000001, word 0 */
#define X86_FEATURE_FPU		(0*32+ 0) /* Onboard FPU */
#define X86_FEATURE_VME		(0*32+ 1) /* Virtual Mode Extensions */
#define X86_FEATURE_DE		(0*32+ 2) /* Debugging Extensions */
#define X86_FEATURE_PSE 	(0*32+ 3) /* Page Size Extensions */
#define X86_FEATURE_TSC		(0*32+ 4) /* Time Stamp Counter */
#define X86_FEATURE_MSR		(0*32+ 5) /* Model-Specific Registers, RDMSR, WRMSR */
#define X86_FEATURE_PAE		(0*32+ 6) /* Physical Address Extensions */
#define X86_FEATURE_MCE		(0*32+ 7) /* Machine Check Architecture */
#define X86_FEATURE_CX8		(0*32+ 8) /* CMPXCHG8 instruction */
#define X86_FEATURE_APIC	(0*32+ 9) /* Onboard APIC */
#define X86_FEATURE_SEP		(0*32+11) /* SYSENTER/SYSEXIT */
#define X86_FEATURE_MTRR	(0*32+12) /* Memory Type Range Registers */
#define X86_FEATURE_PGE		(0*32+13) /* Page Global Enable */
#define X86_FEATURE_MCA		(0*32+14) /* Machine Check Architecture */
#define X86_FEATURE_CMOV	(0*32+15) /* CMOV instruction (FCMOVCC and FCOMI too if FPU present) */
#define X86_FEATURE_PAT		(0*32+16) /* Page Attribute Table */
#define X86_FEATURE_PSE36	(0*32+17) /* 36-bit PSEs */
#define X86_FEATURE_PN		(0*32+18) /* Processor serial number */
#define X86_FEATURE_CLFLSH	(0*32+19) /* Supports the CLFLUSH instruction */
#define X86_FEATURE_DTES	(0*32+21) /* Debug Trace Store */
#define X86_FEATURE_ACPI	(0*32+22) /* ACPI via MSR */
#define X86_FEATURE_MMX		(0*32+23) /* Multimedia Extensions */
#define X86_FEATURE_FXSR	(0*32+24) /* FXSAVE and FXRSTOR instructions (fast save and restore */
				          /* of FPU context), and CR4.OSFXSR available */
#define X86_FEATURE_XMM		(0*32+25) /* Streaming SIMD Extensions */
#define X86_FEATURE_XMM2	(0*32+26) /* Streaming SIMD Extensions-2 */
#define X86_FEATURE_SELFSNOOP	(0*32+27) /* CPU self snoop */
#define X86_FEATURE_HT		(0*32+28) /* Hyper-Threading */
#define X86_FEATURE_ACC		(0*32+29) /* Automatic clock control */
#define X86_FEATURE_IA64	(0*32+30) /* IA-64 processor */

/* AMD-defined CPU features, CPUID level 0x80000001, word 1 */
/* Don't duplicate feature flags which are redundant with Intel! */
#define X86_FEATURE_SYSCALL	(1*32+11) /* SYSCALL/SYSRET */
#define X86_FEATURE_MMXEXT	(1*32+22) /* AMD MMX extensions */
#define X86_FEATURE_LM		(1*32+29) /* Long Mode (x86-64) */
#define X86_FEATURE_3DNOWEXT	(1*32+30) /* AMD 3DNow! extensions */
#define X86_FEATURE_3DNOW	(1*32+31) /* 3DNow! */

/* Transmeta-defined CPU features, CPUID level 0x80860001, word 2 */
#define X86_FEATURE_RECOVERY	(2*32+ 0) /* CPU in recovery mode */
#define X86_FEATURE_LONGRUN	(2*32+ 1) /* Longrun power control */
#define X86_FEATURE_LRTI	(2*32+ 3) /* LongRun table interface */

/* Other features, Linux-defined mapping, word 3 */
/* This range is used for feature bits which conflict or are synthesized */
#define X86_FEATURE_CXMMX	(3*32+ 0) /* Cyrix MMX extensions */
#define X86_FEATURE_K6_MTRR	(3*32+ 1) /* AMD K6 nonstandard MTRRs */
#define X86_FEATURE_CYRIX_ARR	(3*32+ 2) /* Cyrix ARRs (= MTRRs) */
#define X86_FEATURE_CENTAUR_MCR	(3*32+ 3) /* Centaur MCRs (= MTRRs) */

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/

/***************************************************************
 extern function
 ***************************************************************/

/* Generic CPUID function */
static inline void cpuid(int op, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx)
{
    __asm__("cpuid"
          : "=a" (*eax),
            "=b" (*ebx),
            "=c" (*ecx),
            "=d" (*edx)
          : "0" (op));
}

/* CPUID functions returning a single datum */
static inline unsigned int cpuid_eax(unsigned int op)
{
    unsigned int eax;

    __asm__("cpuid"
          : "=a" (eax)
          : "0" (op)
          : "bx", "cx", "dx");
    return eax;
}
static inline unsigned int cpuid_ebx(unsigned int op)
{
    unsigned int eax, ebx;

    __asm__("cpuid"
          : "=a" (eax), "=b" (ebx)
          : "0" (op)
          : "cx", "dx");
    return ebx;
}
static inline unsigned int cpuid_ecx(unsigned int op)
{
    unsigned int eax, ecx;

    __asm__("cpuid"
          : "=a" (eax), "=c" (ecx)
          : "0" (op)
          : "bx", "dx");
    return ecx;
}
static inline unsigned int cpuid_edx(unsigned int op)
{
    unsigned int eax, edx;

    __asm__("cpuid"
          : "=a" (eax), "=d" (edx)
          : "0" (op)
          : "bx", "cx");
    return edx;
}

#pragma pack()

#endif /* end of header */

