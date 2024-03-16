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
 * 	Name:	helpers.S
 * 	Desc:	Provide various assembly functions for reading and writing to System
 * 			Registers from C or C++ code. There is no need to use these in other
 * 			assembly files.
 */

#include <arch/proc_reg.h>

/*******************************************************************************
 * Name:	arm64_read_cpuid
 * Desc:	Read MPIDR_EL1 System Register to get the current CPU ID.
*******************************************************************************/
	.globl arm64_read_cpuid
arm64_read_cpuid:
	mrs		x0, MPIDR_EL1
	ret

/*******************************************************************************
 * Name:	arm64_read_ttbr0_el1
 * Desc:	Read TTBR0_EL1 System Register, and unmask it to return the TTBR0
 *          base address.
*******************************************************************************/
	.globl arm64_read_ttbr0_el1
arm64_read_ttbr0_el1:
	mrs		x0, TTBR0_EL1
	and		x0, x0, #(TTBR_BADDR_MASK)
	ret

/*******************************************************************************
 * Name:	arm64_read_ttbr1_el1
 * Desc:	Read TTBR1_EL1 System Register, and unmask it to return the TTBR1
 *          base address.
*******************************************************************************/
	.globl arm64_read_ttbr1_el1
arm64_read_ttbr1_el1:
	mrs		x0, TTBR1_EL1
	and		x0, x0, #(TTBR_BADDR_MASK)
	ret

/*******************************************************************************
 * Name:	arm64_write_ttbr0_el1
 * Desc:	Read TTBR0_EL1 System Register, and unmask it to return the TTBR0
 *          base address.
*******************************************************************************/
	.globl arm64_write_ttbr0_el1
arm64_write_ttbr0_el1:
	and		x0, x0, #(TTBR_BADDR_MASK)
	msr		TTBR0_EL1, x0
	tlbi	vmalle1
	dsb		ish
	isb
	ret

/*******************************************************************************
 * Name:	arm64_write_ttbr1_el1
 * Desc:	Read TTBR1_EL1 System Register, and unmask it to return the TTBR1
 *          base address.
*******************************************************************************/
	.globl arm64_write_ttbr1_el1
arm64_write_ttbr1_el1:
	and		x0, x0, #(TTBR_BADDR_MASK)
	msr		TTBR1_EL1, x0
	tlbi	vmalle1
	dsb		ish
	isb
	ret

/*******************************************************************************
 * Name:	arm64_read_affinity
 * Desc:	Read the affinity value from MPIDR_EL1
*******************************************************************************/
	.globl arm64_read_affinity
arm64_read_affinity:
	mrs		x0, MPIDR_EL1
	ubfx	x1, x0, #32, #8
	bfi		w0, w1, #24, #8
	ret

/*******************************************************************************
 * Name:	arm64_write_icc_sgi1r_el1
 * Desc:	Write a value to ICC_SGI1R_EL1 to trigger an SGI/PPI interrupt.
*******************************************************************************/
	.globl arm64_write_icc_sgi1r_el1
arm64_write_icc_sgi1r_el1:
	msr		ICC_SGI1R_EL1, x0
	ret

/*******************************************************************************
 * Name:	arm64_write_icc_eoir1_el1
 * Desc:	Write an interrupt id to ICC_EOIR1_EL1 to signal an end-of-interrupt
*******************************************************************************/
	.globl arm64_write_icc_eoir1_el1
arm64_write_icc_eoir1_el1:
	msr		ICC_EOIR1_EL1, x0
	ret

/*******************************************************************************
 * Name:	arm64_read_icc_iar1_el1
 * Desc:	Read the Interrupt Acknowledge register.
*******************************************************************************/
	.globl arm64_read_icc_iar1_el1
arm64_read_icc_iar1_el1:
	mrs		x0, ICC_IAR1_EL1
	ret

/*******************************************************************************
 * Name:	arm64_icc_sre_el1_write
 * Desc:	Write to the GIC System Register Enable register.
*******************************************************************************/
	.globl arm64_icc_sre_el1_write
arm64_icc_sre_el1_write:
	msr		ICC_SRE_EL1, x0
	ret

/*******************************************************************************
 * Name:	arm64_icc_sre_el1_read
 * Desc:	Read from the GIC System Register Enable register.
*******************************************************************************/
	.globl arm64_icc_sre_el1_read
arm64_icc_sre_el1_read:
	mrs		x0, ICC_SRE_EL1
	ret

/*******************************************************************************
 * Name:	arm64_icc_pmr_el1_write
 * Desc:	Write to the GIC Priority Mask register.
*******************************************************************************/
	.globl arm64_icc_pmr_el1_write
arm64_icc_pmr_el1_write:
	msr		ICC_PMR_EL1, x0
	ret

/*******************************************************************************
 * Name:	arm64_icc_igrpen1_el1_write
 * Desc:	Write to the GIC Group 1 Enable register.
*******************************************************************************/
	.globl arm64_icc_igrpen1_el1_write
arm64_icc_igrpen1_el1_write:
	msr		ICC_IGRPEN1_EL1, x0
	ret

/*******************************************************************************
 * Name:	arm64_icc_igrpen1_el1_read
 * Desc:	Read from the GIC Group 1 Enable register.
*******************************************************************************/
	.globl arm64_icc_igrpen1_el1_read
arm64_icc_igrpen1_el1_read:
	mrs		x0, ICC_IGRPEN1_EL1
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
	and		x0, x1, 0x0000ffffffffffff
	ret
L__mmu_translate_kvtop_invalid:
	mov		x0, #0
	ret

/*******************************************************************************
 * Name:	mmu_set_tt_base_alt
 * Desc:	Assign TTBR1_EL1
*******************************************************************************/
	.globl mmu_set_tt_base_alt
mmu_set_tt_base_alt:
	dsb		sy
	msr		TTBR1_EL1, x0
	isb		sy
	ret

/*******************************************************************************
 * Name:	mmu_set_tt_base
 * Desc:	Assign TTBR0_EL1
*******************************************************************************/
	.globl mmu_set_tt_base
mmu_set_tt_base:
	dsb		sy
	msr		TTBR0_EL1, x0
	isb		sy
	ret