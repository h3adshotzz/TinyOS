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
 * 	Name:	handler.S
 * 	Desc:	Kernel Exception Handlers.
 */

#include <kern/defaults.h>

/*******************************************************************************
 * Helpers
 ******************************************************************************/

/* create the exception stack frame for an exception on SP0 */
.macro create_exception_frame_sp0
	msr		SPSel, #0				// Switch to SP0
	sub		sp, sp, #400			// Create the exception frame
	stp		x0, x1, [sp, #0]		// Save x0 and x1 to the exception frame
	add		x0, sp, #400			// Calculate the original SP
	str		x0, [sp, #240]			// Save the SP to the exception frame
	mov		x0, sp					// Copy saved state pointer to x0
.endm

/* create the exception stack frame for an exception on SP1 */
.macro create_exception_frame_sp1
	sub		sp, sp, #400		
	stp		x0, x1, [sp, #0]	
	add		x0, sp, #400		
	str		x0, [sp, #240]		
	mov		x0, sp				
.endm

/* save the exception registers to the exception frame */
.macro save_exception_registers
	mrs		x1, FAR_EL1
	str		x1, [x0, #264]
	mrs		x1, ESR_EL1
	str		x1, [x0, #272]
.endm

/*******************************************************************************
 * Exception Vector Table Page
 ******************************************************************************/

	.align 12
	.globl _ExceptionVectorBase
_ExceptionVectorBase:

	/* EL1 SP0 */
L__el1_sp0_synchronous_handler:
	create_exception_frame_sp0
	save_exception_registers
	adr		x1, arm64_handler_synchronous
	b		L__dispatch64

	.align 7
L__el1_sp0_irq_handler:
	create_exception_frame_sp0
	//switch_to_int_stack_el1_sp0
	adr		x1, arm64_handler_irq
	b		L__dispatch64

	.align 7
L__el1_sp0_fiq_handler:
	create_exception_frame_sp0
	//switch_to_int_stack_el1_sp0
	adr		x1, arm64_handler_fiq
	b		L__dispatch64

	.align 7
L__el1_sp0_serror_handler:
	create_exception_frame_sp0
	adr		x1, arm64_handler_serror
	b		L__dispatch64


	/* EL1 SP1 */
	.align 7
L__el1_sp1_synchronous_handler:
	// todo: check that the SP is still within the exception stack
	create_exception_frame_sp1
	adr		x1, arm64_handler_synchronous
	b		L__dispatch64

	.align 7
L__el1_sp1_irq_handler:
	create_exception_frame_sp1
	adr		x1, arm64_handler_irq
	b		L__dispatch64

	.align 7
L__el1_sp1_fiq_handler:
	create_exception_frame_sp1
	adr		x1, arm64_handler_fiq
	b		L__dispatch64

	.align 7
L__el1_sp1_serror_handler:
	create_exception_frame_sp1
	adr		x1, arm64_handler_serror
	b		L__dispatch64


	/* EL0 64 */
	.align 7
L__el0_64_synchronous_handler:
	b	.

	.align 7
L__el0_64_irq_handler:
	b	.

	.align 7
L__el0_64_fiq_handler:
	b	.

	.align 7
L__el0_64_serror_handler:
	b	.


	/* fill the rest of the page */
	.align 12

/******************************************************************************/

	/**
	 * Name:	__dispatch64
	 * Desc:	Dispatch a 64-bit Exception Handler
	 *
	 * Params:
	 *	x0:		Exception Frame Structure
	 *	x1:		Exception Handler
	 *
	 */
	.align 2
L__dispatch64:

	/* save remaining registers */
	stp		x2, x3, [x0, #16]
	stp		x4, x5, [x0, #32]
	stp		x6, x7, [x0, #48]
	stp		x8, x9, [x0, #64]
	stp		x10, x11, [x0, #80]
	stp		x12, x13, [x0, #96]
	stp		x14, x15, [x0, #112]
	stp		x16, x17, [x0, #128]
	stp		x18, x19, [x0, #144]
	stp		x20, x21, [x0, #160]
	stp		x22, x23, [x0, #176]
	stp		x24, x25, [x0, #192]
	stp		x26, x27, [x0, #208]
	stp		x28, fp, [x0, #224]
	str		lr, [x0, #232]

	mrs		x22, ELR_EL1		// Exception Link Register
	mrs		x23, SPSR_EL1		// Current Stack Pointer

	str		x22, [x0, #248]
	str		x23, [x0, #256]

	mov		x28, x0
	blr		x1
	b		L__exception_exit


	.align 2
L__exception_exit:
	mov		x0, x28

	ldr		x22, [x0, #248]
	ldr		x23, [x0, #256]

	msr		ELR_EL1, x22
	msr		SPSR_EL1, x23

	ldp		x2, x3, [x0, #16]
	ldp		x4, x5, [x0, #32]
	ldp		x6, x7, [x0, #48]
	ldp		x8, x9, [x0, #64]
	ldp		x10, x11, [x0, #80]
	ldp		x12, x13, [x0, #96]
	ldp		x14, x15, [x0, #112]
	ldp		x16, x17, [x0, #128]
	ldp		x18, x19, [x0, #144]
	ldp		x20, x21, [x0, #160]
	ldp		x22, x23, [x0, #176]
	ldp		x24, x25, [x0, #192]
	ldp		x26, x27, [x0, #208]
	ldp		x28, fp, [x0, #224]
	ldr		lr, [x0, #232]

	ldr		x1, [x0, #240]
	mov		sp, x1

	ldp		x0, x1, [x0, #0]
	eret
