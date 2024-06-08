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
 *	Name:	data.S
 *	Desc:	Static memory carvouts for stack, pagetables, etc.
 */

#include <kern/defaults.h>

/******************************************************************************
 * Stacks
 *****************************************************************************/

	.section	".rodata"
	.align		16

	/* Interrupt Stack */
	.globl		intstack_base
intstack_base:
	.space		DEFAULTS_KERNEL_VM_STACK_SIZE
	.globl		intstack_top
intstack_top:

	/* Exception Stack */
	.globl		excepstack_base
excepstack_base:
	.space		DEFAULTS_KERNEL_VM_STACK_SIZE
	.globl		excepstack_top
excepstack_top:

	/* Stack region guard */
	.globl		__stack_high_guard
__stack_high_guard:
	.space		DEFAULTS_KERNEL_VM_PAGE_SIZE

/******************************************************************************
 * Pagetables
 *****************************************************************************/

	.section	".rodata"
	.align		16
	.globl		pagetables_region_base
pagetables_region_base:
	.space		DEFAULTS_KERNEL_VM_PAGE_SIZE*16
	.globl		pagetables_region_end
pagetables_region_end:


/* vim: set ts=4 */
