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
	str		x1, [x0, #244]
	mrs		x1, ESR_EL1
	str		x1, [x0, #248]
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

	str		x22, [x0, #236]		// frame->lr = ELR
	str		x23, [x0, #240]		// frame->sp = SPSR_EL1

	mov		x28, x0
	blr		x1
	b		L__exception_exit


	.align 2
L__exception_exit:
	mov		x0, x28

	ldr		x22, [x0, #236]
	ldr		x23, [x0, #240]

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

	ldr		x1, [x0, #236]
	mov		sp, x1

	ldp		x0, x1, [x0, #0]
	eret

/*******************************************************************************
 * CPU Context Switching
 ******************************************************************************/

/**
 * __fork64_switch
 *
 * Switch from one task to another, saving the current cpu state and stack to
 * the current task (x0) and restoring that of the next (x1).
 *
 */
	.align		12
	.globl		__fork64_switch
__fork64_switch:

	/**
	 *	The task's context is placed 8 bytes from the base of the task struct,
	 *	we use x9 here as the pointer to the task context. The first step is to
	 *	write the current tasks x19-x31 registers to the context.
	 */
	mov		x10, #8
	add		x9, x0, x10			/* x9 = cpu_context */

	/**
	 *	We cannot use the stack pointer with load/store instructions, so move it
	 *	into a register we don't care about first, and then store it. After that
	 *	save the calee registers into the context.
	 */
	mov		x8, sp

	stp		x19, x20, [x9, #16 * 0]
	stp		x21, x22, [x9, #16 * 1]
	stp		x23, x24, [x9, #16 * 2]
	stp		x25, x26, [x9, #16 * 3]
	stp		x27, x28, [x9, #16 * 4]
	stp		x29, x8, [x9, #16 * 5]
	stp		lr, xzr, [x9, #16 * 6]

	/* Now, restore the cpu context from the target task structure */
	add		x9, x1, x10			/* x9 = cpu_context */

	ldp		x19, x20, [x9, #16 * 0]
	ldp		x21, x22, [x9, #16 * 1]
	ldp		x23, x24, [x9, #16 * 2]
	ldp		x25, x26, [x9, #16 * 3]
	ldp		x27, x28, [x9, #16 * 4]
	ldp		x29, x8, [x9, #16 * 5]
	ldr		lr, [x9, #16 * 6]

	/* Set the stack pointer */
	mov		sp, x8

	/**
	 *	The link register should now contain the address of the next instruction
	 *	for the target task. If this is the first time a new task has executed
	 *	after being spawned, the return address should be to __fork64_return.
	 */
	mov		x0, x20
	ret


	.align		2
	.globl		__fork64_return
__fork64_return:
	brk #1
	ret