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
 *	Name:	start.S
 *	Desc:	Kernel entry point, reset vector, low exception vectors and image
 *			header. Prepares bootstrap pagetables and jump to C code init.
 */

#include <arch/proc_reg.h>


/******************************************************************************
 * Early System Reset Vector
 *
 * This is the first code to execute in the Kernel. tBoot will jump here after
 * loading the kernel, either from a disk or a memory-mapped load region. The
 * kernel will always load at EL2. tBoot expects an image_info header at the
 * base of the kernel, and will jump to the offset specified in that header.
 *
 * The first steps are to mask all interrupts and prepare the boot arguments
 * pointer, which is sent across in register x0. Additionally, this is only
 * for the Boot CPU, secondary CPUs are handled differently.
 *****************************************************************************/

	.align		12
	.section	".boot"
#if defined(DEFAULTS_KERNEL_IMAGEINFO_HEADER)
	.globl		_header
_header:
	
#endif
	.globl		_LowResetVector
_LowResetVector:

	/* disable interrupts */
	msr		DAIFSet, #(0xf)

	/* preserve boot args */
	mov		x27, x0

	/* jump */
	b		_reset_vector

/******************************************************************************
 * Early Exception Vectors
 *
 * During the early boot process, we use an Early Exception Vector so as to
 * preserve the register state when an exception occurs, until we can configure
 * proper exception handling which saves the register state.
 *****************************************************************************/

	.align		12
	.globl		_LowExceptionVectorBase
_LowExceptionVectorBase:
	/* EL1 SP0 */
	b		.
	.align	7
	b		.
	.align	7
	b		.
	.align 	7
	b		.

	/* EL1 SP1 */
	.align	7
	b		.
	.align	7
	b		.
	.align	7
	b		.
	.align 	7
	b		.
	
	/* EL0 64 */
	.align	7
	b		.
	.align	7
	b		.
	.align	7
	b		.
	.align 	7
	b		.
	
	/* EL0 32 */
	.align	7
	b		.
	.align	7
	b		.
	.align	7
	b		.
	.align 	7
	b		.

/******************************************************************************
 * Kernel Reset Vector
 *
 * This configures the exception vector base register and switches the kernel
 * from EL2 to Non-secure EL1. At this stage, the boot arguments are still in
 * x27 and haven't been touched yet.
 *****************************************************************************/

	.align		2
	.globl		_ResetVector
_ResetVector:

	/* setup low reset vector before attempting anything */
	adr		x0, _LowResetVectorBase
	msr		VBAR_EL1, x0

	/* initialise SCTLR_EL1 */
	mov		x0, xzr
	ldr		x1, =SCTLR_RES1_MASK
	orr		x0, x0, x1
	msr		SCTLR_EL1, x0

	/* initialise HCR_EL2 */
	mrs		x0, HCR_EL2
	orr		x0, x0, #(1 << 31)		// RW=1 EL1 exec state is AArch64
	msr		HCR_EL2, x0

	/* initialise SPSR_EL2 */
	mov		x0, xzr
	mov		x0, #0b00101			// EL1
	orr		x0, x0, #(1 << 8)		// Enable SError and External Abort
	msr		SPSR_EL2, x0

	/* initialise EL2 gicv3 system register */
	mov		x0, xzr
	orr		x0, x0, #(1 << 3)		// Enable bit
	orr		x0, x0, #(1 << 0)		// SRE bit
	msr		ICC_SRE_EL2, x0

	/* initialise ELR_EL2 */
	adr		x0, _start
	msr		ELR_EL2, x0

	eret

/******************************************************************************
 * Kernel Secondary CPU Entry Vector
 *
 * This is the entry point for secondary CPUs. This is still TODO.
 *****************************************************************************/

	.align		2
	.globl		_SecondaryResetVector
_SecondaryResetVector:

	/* Prep PSCI */ 
	ldr		w0, =0xc4000003
	mov		x1, #1
	ldr		x2, =_cpu_secondary_entry
	mov		x3, #0

	hvc		0

_cpu_secondary_entry:
	brk		#1

/******************************************************************************
 * Kernel Entry Point (EL1)
 *
 * This is where the Kernel will actually start executing. By now, it's in the
 * correct execution state and we know where the boot arguments are. This will
 * configure some initial virtual memory mappings using the virtual and
 * physical addresses passed by the bootloader, and then jump to the C entry
 * point.
 *****************************************************************************/

	.section	".text"
	.align		2
	.globl		_start
_start:

	/* setup the stack pointer */
	msr		SPSel, #0
	adr		x0, intstack_top
	mov		sp, x0

	/* callback to tinyboot */
	smc		#0

	/* read memory info from boot args */
	ldr		x25, [x27, DEFAULTS_BA_OFFSET_VIRTBASE]
	ldr		x26, [x27, DEFAULTS_BA_OFFSET_PHYSBASE]
	ldr		x28, [x27, DEFAULTS_BA_OFFSET_MEMSIZE]

	adr		x1, bootstrap_pagetables	// Level 1
	mov		x2, x1						// Level 0

	mov		x3, x26						// V=P Virt cursor
	mov		x4, x26						// V=P Phys cursor

	/* V=P mapping */
	lsr		x5, x28, #(TT_L2_SHIFT)
	create_bootstrap_region		x3, x4, x5, x1, x2, x6, x7, x8, x9, x10, x11

	/* move the L1 table after the last V=P L2 table */
	add		x1, x2, TT_PAGE_SIZE
	mov		x2, x1

	mov		x3, x25						// KVA Virt cursor
	mov		x4, x26						// KVA Phys cursor

	/* KVA mapping */
	lsr		x5, x28, #(TT_L2_SHIFT)
	create_bootstrap_region		x3, x4, x5, x1, x2, x6, x7, x8, x9, x10, x11

	/* set Translation Table Base Registers for EL1 */
	adr		x0, bootstrap_pagetables
	and		x0, x0, #(TTBR_BADDR_MASK)
	and		x1, x1, #(TTBR_BAADR_MASK)
	msr		TTBR0_EL1, x0
	msr		TTBR1_EL1, x1
	tlbi	vmalle1
	dsb		ish
	isb

	/* configure TCR_EL1 */
	mov     x0, xzr
	mov     x1, #(TCR_TG0_GRANULE_SIZE_MASK)
	orr     x0, x0, x1
	mov     x1, #(TCR_TG1_GRANULE_SIZE_MASK)
	orr     x0, x0, x1
	mov     x1, #(TCR_IPS_40BITS)
	orr     x0, x0, x1
	mov     x1, #(TCR_T0SZ_MASK)
	orr     x0, x0, x1
	mov     x1, #(TCR_T1SZ_MASK)
	orr     x0, x0, x1
	msr		TCR_EL1, x0

	/* enable the MMU */
	mrs		x0, SCTLR_EL1
	orr		x0, x0, #1
	msr		SCTLR_EL1, x0
	isb

	/* set the proper exception vector */
	adr		x0, _ExceptionVectorBase
	sub		x0, x0, x26					// Convert to a KVA
	add		x0, x0, x25
	msr		VBAR_EL1, x0

	/* unmask interrupts */
	msr		DAIFClr, #(DEFAULTS_KERNEL_DAIF)

	/* calculate the KVA for kernel_init */
	mov		x0, x27
	adr		x1, kernel_init
	sub		x1, x1, x26			// entry_offset = kernel_init - phys base
	add		x1, x1, x25			// kva = entry_offset + virt base
	mov		lr, x1				// lr = kva

	/* return to C entry point */
	ret

/* vim: set ts=4 */
