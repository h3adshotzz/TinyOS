//===----------------------------------------------------------------------===//
//
//                                  tinyOS
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//	Copyright (C) 2024, Harry Moulton <me@h3adsh0tzz.com>
//
//===----------------------------------------------------------------------===//

/**
 * 	Name:	arch.h
 * 	Desc:	Architecture structures and helper macros for inline assembly.
*/

#ifndef __ARCH_ARCH_H__
#define __ARCH_ARCH_H__

#include <tinylibc/stdint.h>

/*******************************************************************************
 * Macros
*******************************************************************************/

/* Some of these macros are taken from Arm Trusted-firmware-A */

/* Define function for system instruction with type specifier */
#define DEFINE_SYSOP_TYPE_FUNC(_op, _type)				\
static inline void _op ## _type(void)					\
{									\
	__asm__ (#_op " " #_type : : : "memory");			\
}

/* Define function for simple system instruction */
#define DEFINE_SYSOP_FUNC(_op)						\
static inline void _op(void)						\
{									\
	__asm__ (#_op);							\
}

/* Define a system register read function */
#define _DEFINE_SYSREG_READ_FUNC(_name, _sysreg)				\
static inline __uint64_t arm64_read_ ## _name (void)			\
{																\
	__uint64_t v;												\
	__asm__ __volatile__ ("mrs %0, " #_sysreg : "=r" (v));		\
	return v;													\
}

/* Define a system register write function */
#define _DEFINE_SYSREG_WRITE_FUNC(_name, _sysreg)				\
static inline void arm64_write_ ## _name (__uint64_t v)			\
{																\
	__asm__ __volatile__ ("msr " #_sysreg ", %0" : "=r" (v));	\
}

/* System Registers */
#define DEFINE_SYSREG_READ_FUNC(_name)							\
	_DEFINE_SYSREG_READ_FUNC(_name, _name)
#define DEFINE_SYSREG_WRITE_FUNC(_name)							\
	_DEFINE_SYSREG_WRITE_FUNC(_name, _name)

#define DEFINE_SYSREG_READ_WRITE_FUNCS(_name)					\
	_DEFINE_SYSREG_READ_FUNC(_name, _name)						\
	_DEFINE_SYSREG_WRITE_FUNC(_name, _name)


/*******************************************************************************
 * Structures and Enums
*******************************************************************************/

typedef int				exception_level_t;

/**
 * ARM64 Exception Frame.
 * 
 * This contains the saved register state when an exception is taken in the
 * kernel. It is constructed in an area of memory, with the pointer passed to
 * the exception handler.
*/
typedef struct {
	uint64_t	regs[29];
	uint64_t	fp;
	uint64_t	lr;
	uint64_t	sp;
	uint64_t	pc;
	uint64_t	far;
	uint64_t	esr;
	uint64_t	res0;
} arm64_exception_frame_t;

/**
 * ARM64 CPU Context.
 *
 * This is used to save the cpu register state when switching contexts. The
 * difference here is that the cpu context only needs to save registers x19-x31
 * and doesn't need to save 'far' and 'esr'.
*/
typedef struct arm64_cpu_context {
	uint64_t	x19;
	uint64_t	x20;
	uint64_t	x21;
	uint64_t	x22;
	uint64_t	x23;
	uint64_t	x24;
	uint64_t	x25;
	uint64_t	x26;
	uint64_t	x27;
	uint64_t	x28;
	uint64_t	fp;		// x29
	uint64_t	sp;		// x30
	uint64_t	pc;		// x31
	uint64_t	_res;
} arm64_cpu_context_t __attribute__((packed));

/**
 * Exception Types
*/
typedef enum {
	ESR_EC_UNCATEGORIZED       = 0x00,
	ESR_EC_WFI_WFE             = 0x01,
	ESR_EC_MCR_MRC_CP15_TRAP   = 0x03,
	ESR_EC_MCRR_MRRC_CP15_TRAP = 0x04,
	ESR_EC_MCR_MRC_CP14_TRAP   = 0x05,
	ESR_EC_LDC_STC_CP14_TRAP   = 0x06,
	ESR_EC_TRAP_SIMD_FP        = 0x07,
	ESR_EC_PTRAUTH_INSTR_TRAP  = 0x09,
	ESR_EC_MCRR_MRRC_CP14_TRAP = 0x0c,
	ESR_EC_ILLEGAL_INSTR_SET   = 0x0e,
	ESR_EC_SVC_32              = 0x11,
	ESR_EC_HVC_32              = 0x12,
	ESR_EC_SVC_64              = 0x15,
	ESR_EC_HVC_64              = 0x16,
	ESR_EC_MSR_TRAP            = 0x18,
	ESR_EC_IABORT_EL0          = 0x20,
	ESR_EC_IABORT_EL1          = 0x21,
	ESR_EC_PC_ALIGN            = 0x22,
	ESR_EC_DABORT_EL0          = 0x24,
	ESR_EC_DABORT_EL1          = 0x25,
	ESR_EC_SP_ALIGN            = 0x26,
	ESR_EC_FLOATING_POINT_32   = 0x28,
	ESR_EC_FLOATING_POINT_64   = 0x2C,
	ESR_EC_SERROR_INTERRUPT    = 0x2F,
	ESR_EC_BKPT_REG_MATCH_EL0  = 0x30, // Breakpoint Debug event taken to the EL from a lower EL.
	ESR_EC_BKPT_REG_MATCH_EL1  = 0x31, // Breakpoint Debug event taken to the EL from the EL.
	ESR_EC_SW_STEP_DEBUG_EL0   = 0x32, // Software Step Debug event taken to the EL from a lower EL.
	ESR_EC_SW_STEP_DEBUG_EL1   = 0x33, // Software Step Debug event taken to the EL from the EL.
	ESR_EC_WATCHPT_MATCH_EL0   = 0x34, // Watchpoint Debug event taken to the EL from a lower EL.
	ESR_EC_WATCHPT_MATCH_EL1   = 0x35, // Watchpoint Debug event taken to the EL from the EL.
	ESR_EC_BKPT_AARCH32        = 0x38,
	ESR_EC_BRK_AARCH64         = 0x3C,
} esr_exception_class_t;

/**
 * Fault Status
*/
typedef enum {
	FSC_TRANSLATION_FAULT_L0   = 0x04,
	FSC_TRANSLATION_FAULT_L1   = 0x05,
	FSC_TRANSLATION_FAULT_L2   = 0x06,
	FSC_TRANSLATION_FAULT_L3   = 0x07,
	FSC_ACCESS_FLAG_FAULT_L1   = 0x09,
	FSC_ACCESS_FLAG_FAULT_L2   = 0x0A,
	FSC_ACCESS_FLAG_FAULT_L3   = 0x0B,
	FSC_PERMISSION_FAULT_L1    = 0x0D,
	FSC_PERMISSION_FAULT_L2    = 0x0E,
	FSC_PERMISSION_FAULT_L3    = 0x0F,
	FSC_SYNC_EXT_ABORT         = 0x10,
	FSC_SYNC_EXT_ABORT_TT_L1   = 0x15,
	FSC_SYNC_EXT_ABORT_TT_L2   = 0x16,
	FSC_SYNC_EXT_ABORT_TT_L3   = 0x17,
	FSC_SYNC_PARITY            = 0x18,
	FSC_ASYNC_PARITY           = 0x19,
	FSC_SYNC_PARITY_TT_L1      = 0x1D,
	FSC_SYNC_PARITY_TT_L2      = 0x1E,
	FSC_SYNC_PARITY_TT_L3      = 0x1F,
	FSC_ALIGNMENT_FAULT        = 0x21,
	FSC_DEBUG_FAULT            = 0x22,
} fault_status_t;


/*******************************************************************************
 * Functions
*******************************************************************************/

/* Data Synchronization Barrier */
DEFINE_SYSOP_TYPE_FUNC(dsb, sy)
DEFINE_SYSOP_TYPE_FUNC(dsb, st)
DEFINE_SYSOP_TYPE_FUNC(dsb, ish)
DEFINE_SYSOP_TYPE_FUNC(dsb, ishst)
DEFINE_SYSOP_TYPE_FUNC(dsb, nsh)
DEFINE_SYSOP_TYPE_FUNC(dsb, nshst)
DEFINE_SYSOP_TYPE_FUNC(dsb, osh)
DEFINE_SYSOP_TYPE_FUNC(dsb, oshst)

/* Data Memory Barrier */
DEFINE_SYSOP_TYPE_FUNC(dmb, sy)
DEFINE_SYSOP_TYPE_FUNC(dmb, st)
DEFINE_SYSOP_TYPE_FUNC(dmb, ish)
DEFINE_SYSOP_TYPE_FUNC(dmb, ishst)
DEFINE_SYSOP_TYPE_FUNC(dmb, nsh)
DEFINE_SYSOP_TYPE_FUNC(dmb, nshst)
DEFINE_SYSOP_TYPE_FUNC(dmb, osh)
DEFINE_SYSOP_TYPE_FUNC(dmb, oshst)

/* Instruction Synchronization Barrier */
DEFINE_SYSOP_TYPE_FUNC(isb, sy)
DEFINE_SYSOP_FUNC(isb)

// tmp
extern uint32_t arm64_read_cpuid (void);
extern uint32_t arm64_read_icc_iar1_el1 (void);
extern void arm64_write_icc_eoir1_el1 (uint32_t val);

#endif /* __ARCH_ARCH_H__ */