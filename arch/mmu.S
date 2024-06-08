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
 *  Name:	mmu.S
 *	Desc:	Assembly functions for interacting with the Memory Management Unit.
 */

#include <arch/proc_reg.h>

/*******************************************************************************
 * Name:	mmu_set_tcr
 * Desc:	Set the MMU Translation Control Register (TCR_EL1)
*******************************************************************************/
	.globl mmu_set_tcr
mmu_set_tcr:
	dsb		sy
	msr		TCR_EL1, x0
	isb		sy
	ret

/*******************************************************************************
 * Name:	mmu_set_tt_base
 * Desc:	Set the MMU Translation Table Base Regsiter 0 (TTBR0_EL1)
*******************************************************************************/
	.globl mmu_set_tt_base
mmu_set_tt_base:
	dsb		sy
	msr		TTBR0_EL1, x0
	isb		sy
	ret

/*******************************************************************************
 * Name:	mmu_set_tt_base_alt
 * Desc:	Set the MMU Translation Table Base Regsiter 1 (TTBR1_EL1)
*******************************************************************************/
	.globl mmu_set_tt_base_alt
mmu_set_tt_base_alt:
	dsb		sy
	msr		TTBR1_EL1, x0
	isb		sy
	ret

/*******************************************************************************
 * Name:	mmu_get_tcr
 * Desc:	Get the MMU Translation Control Register (TCR_EL1)
*******************************************************************************/
	.globl mmu_get_tcr
mmu_get_tcr:
	mrs		x0, TCR_EL1
	ret

/*******************************************************************************
 * Name:	mmu_get_tt_base
 * Desc:	Get the MMU Translation Table Base Regsiter 0 (TTBR0_EL1)
*******************************************************************************/
	.globl mmu_get_tt_base
mmu_get_tt_base:
	mrs		x0, TTBR0_EL1
	ret

/*******************************************************************************
 * Name:	mmu_get_tt_base_alt
 * Desc:	Get the MMU Translation Table Base Regsiter 1 (TTBR1_EL1)
*******************************************************************************/
	.globl mmu_get_tt_base_alt
mmu_get_tt_base_alt:
	mrs		x0, TTBR1_EL1
	ret

/*******************************************************************************
 * Name:	mmu_translate_kvtop
 * Desc:	Translate a given Kernel Virtual Address to Physical Address using
 *			the AT instruction.
*******************************************************************************/
	.globl mmu_translate_kvtop
mmu_translate_kvtop:
	at		s1e1r, x0
	isb		sy
	mrs		x1, PAR_EL1
	tbnz	x1, #0, L__mmu_translate_kvtop_invalid
	bfm		x1, x0, #0, #11
	and		x0, x1, #(TTBR_BADDR_MASK)
	ret
L__mmu_translate_kvtop_invalid:
	mov		x0, #0
	ret