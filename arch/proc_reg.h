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
 * NOTE: 	This file contains the bit field definitions of the various System
 * 			Register in the AArch64 architecture utilised by TinyOS. Each System
 * 			Register has a short description, any relevant notes, and each bit
 * 			field.
 * 
 * 			Bit fields are listed with their name, a short description and a ref
 * 			to the ARM Architecture Reference Manual (ARM). 
 * 
 * 			This file is partially based on xnu/osfmk/arm64/proc_reg.h
 * 
 * 			Headers for sections with register bit definitions should use the
 * 			following format to document where in the Arm ARM more information
 * 			can be obtained:
 * 
 * 				ARM:	DXX-YYYY
 * 				Field:	<name>, Bit(s) [X]
 * 				Desc:	This bit field controls X and Y.
 * 
*/

#ifndef __KERN_PROC_REG_H__
#define __KERN_PROC_REG_H__

#include <libkern/compiler.h>

/** TODO: HCR_EL2, SPSR_EL2 */

/*******************************************************************************
 * Name:	DAIF, Interrupt Mask Bits
 * Desc:	Mask various interrupts on the system.
*******************************************************************************/

/**
 * ARM:		...
 * Field:	DEBUG, Bit [9]
 * Desc:	Process state D mask. When the target exception level of the debug
 * 			exception is higher than the current, the exception is not masked by
 * 			this bit.
 * 
 * 			0b0		Watchpoint, Breakpoint and Software Step exceptions at the
 * 					current exception level are not masked.
 * 			0b1		Watchpoint, Breakpoint and Software Step exceptions at the
 * 					current exception level are masked.
*/
#define DAIF_MASK_DEBUG_SHFIT			(9)
#define DAIF_MASK_DEBUG					(1ULL << DAIF_MASK_DEBUG_SHFIT)

/**
 * ARM:
 * Field:	ASYNC, Bit [8]
 * Desc:	SError exception mask bit. On a warm reset, this field resets to 1.
 * 
 * 			0b0		Exception not masked
 * 			0b1		Exception masked.
*/
#define DAIF_MASK_ASYNC_SHIFT			(8)
#define DAIF_MASK_ASYNC					(1ULL << DAIF_MASK_ASYNC_SHIFT)

/**
 * ARM:
 * Field:	IRQ, Bit [7]
 * Desc:	IRQ mask bit. On a warm reset, this field resets to 1.
 * 
 * 			0b0		Exception not mask
 * 			0b1		Exception masked
*/
#define DAIF_MASK_IRQ_SHIFT				(7)
#define DAIF_MASK_IRQ					(1ULL << DAIF_MASK_IRQ_SHIFT)

/**
 * ARM:
 * Field:	FIQ, Bit [6]
 * Desc:	IRQ mask bit. On a warm reset, this field resets to 1.
 * 
 * 			0b0		Exception not mask
 * 			0b1		Exception masked
*/
#define DAIF_MASK_FIQ_SHIFT				(6)
#define DAIF_MASK_FIQ					(1ULL << DAIF_MASK_FIQ_SHIFT)

/* Mask to set/clear all bits */
#define DAIF_MASK_ALL					UL(0xf)

/*******************************************************************************
 * Name:	SCTLR_EL1, System Control Register (EL1)
 * Desc:	Provides top-level control of the system for EL1 and EL0.
 * 
 * Note:	Only fields that are supported by TinyOS are implemented. Fields
 *			specific to certain architecture features, I.e FEAT_TIDCP1 are not
 *			defined here.
*******************************************************************************/

/* SCTLR_EL1 RES1 Bits */
#define SCTLR_RES1_MASK					(0x30d00800)

/**
 * ARM:		D17-6179
 * Field:	UCI, Bit [26]
 * Desc:	Traps EL0 execution of cache maintenance instructions to EL1, or to
 * 			EL2 when it is implemented and enabled for the current security state
 * 			and HCR_EL2.TGE is 1.
 * 
 * 			0b0		Execution of specified instructions at EL0 are trapped.
 * 			0b1		No effect.
*/
#define SCTLR_UCI_SHIFT					(26)
#define SCTLR_UCI_TRAP_DISABLE			(1ULL << SCTLR_UCI_SHIFT)

/**
 * ARM:		D17-6180
 * Field:	EE, Bit [25]
 * Desc:	Endianness of data accesses at EL1, and stage 1 translation table
 * 			walks in the EL1 and EL0 translation regime.
 * 
 * 			0b0		Little Endian
 * 			0b1		Big Endian
*/
#define SCTLR_EE_SHFIT					(25)
#define SCTLR_EE_BIG_ENDIAN				(1 << SCTLR_EE_SHFIT)

/**
 * ARM:		D17-6180
 * Field:	E0E, Bit [24]
 * Desc:	Endianness of data accesses at EL0.
 * 
 * 			0b0			Little Endian
 * 			0b1			Big Endian
*/
#define SCTLR_E0E_SHIFT					(24)
#define SCTLR_E0E_BIG_ENDIAN			(1 << SCTLR_E0E_SHIFT)

/**
 * ARM:		D17-6182
 * Field:	WXN, Bit [19]
 * Desc:	Write Permission implies XN (Execute-Never). For EL1 and EL1 memory
 * 			translations, this bit can force all regions that are writeable to
 * 			be treated as XN.
 * 
 * 			0b0			No effect
 * 			0b1			Any region that is writeable in the EL1 and EL0 translation
 * 						tables is forced XN (Execute Never).
*/
#define SCTLR_WXN_SHIFT					(19)
#define SCTLR_WXN_ENABLE				(1 << SCTLR_WXN_SHIFT)

/**
 * ARM:		D17-6182
 * Field:	nTWE, Bit [18]
 * Desc:	Traps EL0 execution of WFE instructions to EL1, or EL2 when it is
 * 			implemented for the current security state and HCR_EL2.TGE is 1, from
 * 			both execution states, resported using an ESR_ELx.EC value of 0x1.
 * 
 * 			0b0			Any attempt to create an WFE instruction is trapped
 * 			0b1			No effect
*/
#define SCTLR_nTWE_SHIFT				(18)
#define SCTLR_nTWE_TRAP_DISABLE			(1 << SCTLR_nTWE_SHIFT)

/**
 * ARM:		D17-6182
 * Field:	nTWI, Bit [16]
 * Desc:	Traps EL0 execution of WFI instructions to EL1, or EL2 when it is
 * 			implemented for the current security state and HCR_EL2.TGE is 1, from
 * 			both execution states, resported using an ESR_ELx.EC value of 0x1.
 * 
 * 			0b0			Any attempt to create an WFI instruction is trapped
 * 			0b1			No effect
*/
#define SCTLR_nTWI_SHIFT				(16)
#define SCTLR_nTWI_TRAP_DISABLE			(1 << SCTLR_nTWI_SHIFT)

/**
 * ARM:		D17-6183
 * Field:	UCT, Bit [15]
 * Desc:	Traps EL0 accesses to the CTR_EL0 register to EL1, or EL2 when it is
 * 			implemented and enabled for the current Security state and HCR_EL2.TGE
 * 			is 1, reported using an ESR_ELx.EC value of 0x18.
 * 
 * 			0b0			Accesses to CTR_EL0 from EL0 are trapped.
 * 			0b1			No effect
*/
#define SCTLR_UCT_SHIFT					(15)
#define SCTLR_UCT_TRAP_DISABLE			(1 << SCTLR_UCT_SHIFT)

/**
 * ARM:		D17-6183
 * Field:	DZE, Bit [14]
 * Desc:	Traps EL0 execution of 'DZ ZVA' instructions to EL1, or EL2 when it
 * 			is enabled for the current security state and HCR_EL2.TGE is 1, reported
 * 			using an ESR_ELx.EC value of 0x18.
 * 
 * 			0b0			Executions of 'DZ ZVA' are trapped.
 * 			0b1			No effect
*/
#define SCTLR_DZE_SHIFT					(14)
#define SCTLR_DZE_TRAP_DISABLE			(1 << SCTLR_DZE_SHIFT)

/**
 * ARM:		D17-6184
 * Field:	I, Bit [12]
 * Desc:	Stage 1 instruction access Cacheability control, for accesses at EL0
 * 			or EL1.
 * 
 * 			0b0			All instruction access to Stage 1 Normal memory from EL0
 * 						and EL1 are Stage 1 Non-cacheable.
 * 			0b1			No effect
*/
#define SCTLR_I_SHIFT					(12)
#define SCTLR_I_DISABLE					(1 << SCTLR_I_SHIFT)

/**
 * ARM:		D17-6185
 * Field:	UMA, Bit [9]
 * Desc:	User Mask Access. Traps EL0 execution of MSR and MRS instructions
 * 			that access the PSTATE.{D,A,I,F} masks to EL1, or to EL2 if implemented
 * 			and enabled for the current security state and HCR_EL2.TGE is 1, reported
 * 			using an ESR_ELx.EC of 0x18.
 * 
 * 			0b0			Attempts to execute MSR/MRS at EL0 are trapped.
 * 			0b1			No effect
*/
#define SCTLR_UMA_SHIFT					(9)
#define SCTLR_UMA_TRAP_DISABLE			(1 << SCTLR_UMA_SHIFT)

/**
 * ARM:		D17-6187
 * Field:	SA0, Bit [4]
 * Desc:	Stack Pointer Alignment Check EL0. When set to 1, if a load or store
 * 			instruction executed at EL0 uses SP as the base address and the SP is
 * 			not aligned to a 16-byte boundary, then an SP alignment fault is
 * 			generated.
*/
#define SCTLR_SA0_SHIFT					(4)
#define SCTLR_SA0_ENABLE				(1 << SCTLR_SA0_SHIFT)

/**
 * ARM:		D17-6188
 * Field:	SA, Bit [3]
 * Desc:	Stack Pointer Alignment Check EL1. When set to 1, if a load or store
 * 			instruction executed at EL1 uses SP as the base address and the SP is
 * 			not aligned to a 16-byte boundary, then an SP alignment fault is
 * 			generated.
*/
#define SCTLR_SA_SHIFT					(4)
#define SCTLR_SA_ENABLE					(1 << SCTLR_SA_SHIFT)

/**
 * ARM:		D17-6188
 * Field:	C, Bit [2]
 * Desc:	Stage 1 Cacheability Control for data accesses.
 * 
 * 			0b0			All data access to Stage 1 Normal memory from EL0 and EL1
 * 						and all Normla memory access from unified cache to the
 * 						EL0/EL1 translation tables are treated as Stage 1 
 * 						Non-Cacheable.
 * 			0b1			No effect
*/
#define SCTLR_C_SHIFT					(2)
#define SCTLR_C_ENABLE					(1 << SCTLR_C_SHIFT)

/**
 * ARM:		D17-6188
 * Field:	A, Bit [1]
 * Desc:	Alignment check enable. This is the enable bit for Alignment fault
 * 			checking at EL1 and EL0.
 * 
 * 			0b0			Alignment fault checking disabled for EL1 and EL0.
 * 			0b1			Alignment fault checking enabled for EL1 and EL0. If a
 * 						load or store of one or more registers is not aligned to
 * 						the size of the data elements(s) being accessed, an
 * 						Alignment Fault is raised.
*/
#define SCTLR_A_SHIFT					(1)
#define SCTLR_A_ENABLE					(1 << SCTLR_A_SHIFT)

/**
 * ARM:		D17-6188
 * Field:	M, Bit [0]
 * Desc:	MMU Enable for EL1 and EL0 stage 1 address translation.
 * 
 * 			0b0			MMU Disabled
 * 			0b1			MMU Enabled
*/
#define SCTLR_M_SHIFT					(0)
#define SCTLR_M_ENABLE					(1 << SCTLR_M_SHIFT)


/*******************************************************************************
 * Name:	TCR_EL1, Translation Control Register (EL1)
 * Desc:	The control register for stage 1 of the EL0/EL1 Translation Regime.
 * 
 * Note:	Only fields that are supported by TinyOS are implemented. Fields
 *			specific to certain architecture features, I.e FEAT_LPA2 are not
 *			defined here.
 *
 *			Bit fields that are duplicated for TTRB0 and TTBR1 are grouped
 *			together where possible.
*******************************************************************************/

/**
 * ARM:		D17-6265
 * Field:	TBI1, Bit [38]
 * 			TBI0, Bit [37]
 * Desc:	Top Byte ignored. Indicates whether the top byte of an address is
 * 			used for address match in the TTBR1_ELn region, or ignored and used
 * 			for tagged addresses.
 * 
 * 			0b0			Top Byte used in address calculation
 * 			0b1			Top Byte ignored in address calculation
*/
#define TCR_TBI1_SHIFT					(38)
#define TCR_TBI0_SHIFT					(37)

#define TCR_TBI1_TB_IGNORED				(1 << TCR_TBI1_SHIFT)
#define TCR_TBI0_TB_IGNORED				(1 << TCR_TBI0_SHIFT)

/**
 * ARM:		D17-6265
 * Field:	AS, Bit [36]
 * Desc:	ASID Size.
 * 
 * 			If the implementation only has 8-bits, this field is RES0.
 * 
 * 			0b0			The upper 8-bits of TTBR0_EL1 and TTRB1_EL1 are ignored
 * 						by hardware for every purpose except reading back the
 * 						register, and are treated if they are all zeros for whne
 * 						used for allocation and matching entries in the TLB.
 * 			0b1			The upper 16-bits of TTBR0_EL1 and TTBR1_EL1 are used for
 * 						allocation and matching in the TLB
*/
#define TCR_ASID_SHIFT					(36)
#define TCR_ASID_16BIT					(1 << TCR_ASID_SHIFT)

/**
 * ARM:		D17-6266
 * Field:	IPS, Bits [34:32]
 * Desc:	Intermediate Physical Address Size.
 * 
 * 			0b000		32 bits, 4GB
 * 			0b001		36 bits, 64GB
 * 			0b010		40 bits, 1TB
 * 			0b011		42 bits, 4TB
 * 			0b100		44 bits, 16TB
 * 			0b101		48 bits, 256TB
 * 			0b110		52 bits, 4PB
*/
#define TCR_IPS_SHIFT					(32)
#define TCR_IPS_32BITS					(0 << TCR_IPS_SHIFT)
#define TCR_IPS_36BITS					(1 << TCR_IPS_SHIFT)
#define TCR_IPS_40BITS					(2 << TCR_IPS_SHIFT)
#define TCR_IPS_42BITS					(3 << TCR_IPS_SHIFT)
#define TCR_IPS_44BITS					(4 << TCR_IPS_SHIFT)
#define TCR_IPS_48BITS					(5 << TCR_IPS_SHIFT)

/**
 * ARM:		D17-6266, D17-6268
 * Field:	TG1, Bits [31:30]
 * 			TG0, Bits [15:14]
 * Desc:	Translation Granule Size.
 * 
 * 			0b01		4KB
 * 			0b10		16KB
 * 			0b11		64KB
*/
#define TCR_TG1_SHIFT					(30)
#define TCR_TG0_SHIFT					(14)

// Granule size for TTBR1_EL1
#define TCR_TG1_GRANULE_SIZE_4KB		(0 << TCR_TG1_SHIFT)
#define TCR_TG1_GRANULE_SIZE_16KB		(1 << TCR_TG1_SHIFT)
#define TCR_TG1_GRANULE_SIZE_64KB		(2 << TCR_TG1_SHIFT)

// Granule size for TTBR0_EL1
#define TCR_TG0_GRANULE_SIZE_4KB		(0 << TCR_TG0_SHIFT)
#define TCR_TG0_GRANULE_SIZE_16KB		(1 << TCR_TG0_SHIFT)
#define TCR_TG0_GRANULE_SIZE_64KB		(2 << TCR_TG0_SHIFT)

// TinyOS Default Granule Size
#define TCR_TG1_GRANULE_SIZE_MASK		(TCR_TG1_GRANULE_SIZE_4KB)
#define TCR_TG0_GRANULE_SIZE_MASK		(TCR_TG0_GRANULE_SIZE_4KB)

/**
 * ARM:		D17-6266, D17-6268
 * Field:	SH1, Bits [29:28]
 * 			SH0, Bits [13:12]
 * Desc:	Shareability attribute for memory associated with translation table
 * 			walks.
 * 
 * 			0b00		Non-shareable
 * 			0b01		Outer Shareable
 * 			0b11		Inner Shareable
*/
#define TCR_SH1_SHIFT					(28)
#define TCR_SH0_SHIFT					(12)

// Shareability attribute for TTBR1_EL1
#define TCR_SH1_NONE					(0 << TCR_SH1_SHIFT)
#define TCR_SH1_OUTER					(1 << TCR_SH1_SHIFT)
#define TCR_SH1_INNER					(2 << TCR_SH1_SHIFT)

// Shareability attribute for TTBR0_EL1
#define TCR_SH0_NONE					(0 << TCR_SH0_SHIFT)
#define TCR_SH0_OUTER					(1 << TCR_SH0_SHIFT)
#define TCR_SH0_INNER					(2 << TCR_SH0_SHIFT)

/**
 * ARM:		D17-6267, D17-6269
 * Field:	ORGN1, Bits [27:26]
 * 			ORGN0, Bits [11:10]
 * Desc:	Outer Cacheability attribute for memory associate with translation
 * 			table walks.
 * 
 * 			0b00		Normal memory, Outer Non-cacheable
 * 			0b01		Normal memory, Outer Write-Back Read-Allocate Write-Allocate Cacheable
 * 			0b10		Normal memory, Outer Write-Through Read-Allocate No Write-Allocate Cacheable
 * 			0b11		Normal memory, outer Write-Back Read-Allocate No Write-Allocate Cacheable
*/
#define TCR_ORGN1_SHIFT					(26)
#define TCR_ORGN0_SHIFT					(10)

// Outer Cacheability attribute for TTRB1_EL1
#define TCR_ORGN1_NONE					(0 << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_WRITEBACK				(1 << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_WRITETHRU				(2 << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_WRITEBACK_NO			(3 << TCR_ORGN1_SHIFT)

// Outer Cacheability attribute for TTRB0_EL1
#define TCR_ORGN0_NONE					(0 << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_WRITEBACK				(1 << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_WRITETHRU				(2 << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_WRITEBACK_NO			(3 << TCR_ORGN0_SHIFT)

/**
 * ARM:		D17-6267, D17-6269
 * Field:	IRGN1, Bits [25:24]
 * 			IRGN0, Bits [9:8]
 * Desc:	Inner Cacheability attribute for memory associate with translation
 * 			table walks.
 * 
 * 			0b00		Normal memory, Inner Non-cacheable
 * 			0b01		Normal memory, Inner Write-Back Read-Allocate Write-Allocate Cacheable
 * 			0b10		Normal memory, Inner Write-Through Read-Allocate No Write-Allocate Cacheable
 * 			0b11		Normal memory, Inner Write-Back Read-Allocate No Write-Allocate Cacheable
*/
#define TCR_IRGN1_SHIFT					(24)
#define TCR_IRGN0_SHIFT					(8)

// Inner Cacheability attribute for TTRB1_EL1
#define TCR_IRGN1_NONE					(0 << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_WRITEBACK				(1 << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_WRITETHRU				(2 << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_WRITEBACK_NO			(3 << TCR_IRGN1_SHIFT)

// Inner Cacheability attribute for TTRB0_EL1
#define TCR_IRGN0_NONE					(0 << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_WRITEBACK				(1 << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_WRITETHRU				(2 << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_WRITEBACK_NO			(3 << TCR_IRGN0_SHIFT)

/**
 * ARM:		D17-6267, D17-6269
 * Field:	EPD1, Bit [23]
 * 			EPD0, Bit [7]
 * Desc:	Translation table walk disable for translation using TTBRn_EL1. This
 * 			bit controls whether a translation table walk is performed on a TLB
 * 			miss, for an address that is translated using TTBRn_EL1.
 * 
 * 			0b0			Perform translation table walks using TTBRn_EL1
 * 			0b1			A TLB miss on an address that is translated using TTBRn_EL1
 * 						generates a Translation Fault. No table walk is performed.
*/
#define TCR_EPD1_SHIFT					(23)
#define TCR_EPD0_SHIFT					(7)

#define TCR_EPD1_DISABLE				(1 << TCR_EPD1_SHIFT)
#define TCR_EPD0_DISABLE				(1 << TCR_EPD0_SHIFT)

/**
 * ARM:		D17-6267
 * Field:	A1, Bit [22]
 * Desc:	Selects whether TTBR0_EL1 or TTBR1_EL1 defines the ASID.
 * 
 * 			0b0			TTBR0_EL1 defines the ASID
 * 			0b1			TTBR1_EL1 defines the ASID
*/
#define TCR_A1_SHIFT					(22)
#define TCR_A1_ASID0					(0 << TCR_A1_SHIFT)
#define TCR_A1_ASID1					(1 << TCR_A1_SHIFT)

/**
 * ARM:		D17-6267, D17-6269
 * Field:	T1SZ, Bits [21:16]
 * 			T0SZ, Bits [5:0]
 * Desc:	The size offset of the memory region addressed by TTBRn_EL1. The
 * 			region size is 2^(64-TnSZ) bytes.
*/
#define TCR_T1SZ_SHIFT					(16)
#define TCR_T0SZ_SHIFT					(0)

#define TINYOS_TSZ						39
#define TCR_T1SZ_MASK					((64-TINYOS_TSZ) << TCR_T1SZ_SHIFT)
#define TCR_T0SZ_MASK					((64-TINYOS_TSZ) << TCR_T0SZ_SHIFT)


/*******************************************************************************
 * Name:	Virtual Memory System Architecture (VMSAv8-A) definitions.
 * Desc:	Various values and structure definitions for the Arm VMSAv8-A soec.
 * 
 * Note:	TinyOS only supports 4KB page sizes, therefore only values for that
 * 			page size are included here.
*******************************************************************************/

/* 4KB Page Size */
#define TT_PAGE_SIZE		4096

#define TTE_SHIFT			3
#define TTE_PAGE_ENTRIES	(TT_PAGE_SIZE >> TTE_SHIFT)

/**
 * Template bitmasks for 4KB Page and Block Translation Table Entries (TTE).
 * 
 * TTE's are identified by bits [1:0]. A TTE is only valid if Bit [0] is set to
 * 1, otherwise the TTE is invalid (as denoted by TTE_ENTRY_INVALID). If Bit [0]
 * is set, identification then depends on the translation level:
 * 
 * For levels 0, 1 and 2:
 * 	- If Bit[1] is `0`, the entry is a "Block Descriptor".
 * 	- If Bit[1] is `1`, the entry is a "Table Descriptor".
 * For level 3:
 * 	- If Bit[1] is `0`, the entry is "Reserved".
 * 	- If Bit[1] is `1`, the entry is a "Page Descriptor".
 * 
 * Additionally, Block and Page Decsriptor TTEs have "Lower Attributes", thses
 * control whether the area of memory is execute-never, privileged execute-never,
 * access flag, etc. 
 * 
 * For now, TinyOS is only concerned with the Access Flag. By default, when a
 * block/page is accessed with the AF Bit set to `0`, an Access Flag Fault is
 * generated in order for it to be set. In the case of TinyOS, we don't need to
 * track memory accesses at this stage, so part of the template is to set the
 * AF bit to `1`.
*/
#define TTE_PAGE_TEMPLATE		0x0000000000000403ULL		/* page entry template */
#define TTE_BLOCK_TEMPLATE		0x0000000000000401ULL		/* block entry template */


/**
 * Level 0,1 and 2 Table Decsriptor format (4KB)
 * 
 * 63     59 58   51 50    48 47             12 11  2 1 0
 * +--------+-------+--------+-----------------+-----+-+-+
 * | Attrib |  IGN  |  RES0  | TableOutputAddr | IGN |1|1|
 * +--------+-------+--------+-----------------+-----+-+-+
 *      |
 *      |         63         62       61         60
 *      |     +---------+---------+---------+----------+
 *      +---> | NSTable | APTable | XNTable | PXNTable |
 *            +---------+---------+---------+----------+
 * 
 * Where:
 * 	- RES0:		Default, bits should be zero.
 * 	- NSTable:	TTE points to a Table in Non-secure memory.
 *  - APTable:	TTE points to a Table with Access Protection.
 *  - XNTable:	TTE points to a Table marked Execute-Never.
 *  - PXNTable:	TTE points to a Table marked Privileged Execute-Never.
 * 
 * -----------------------------------------------------------------------------
 * 
 * Level 1 and 2 Block Descriptor format (4KB)
 * 
 * 63          50 49  48 47              n n-1     16 15   12 11          2  1   0
 * +-------------+------+-----------------+------+-----------+-------------+---+---+
 * | UpperAttrib | RES0 | BlockOutputAddr | RES0 | nT | RES0 | LowerAttrib | 0 | 1 |
 * +-------------+------+-----------------+------+-----------+-------------+---+---+
 *   |                                                                   |
 *   |            11   10  9  8 7  6  5   4        2                     |
 *   |          +----+----+----+----+----+----------+                    |
 *   |          | nG | AF | SH | AP | NS | AttrIndx | <------------------+
 *   |          +----+----+----+----+----+----------+
 *   |
 *   |     63        55  54   53     52        51
 *   |     +-----------+----+-----+--------+--------+
 * 	 +---->|  IGNORED  | XN | PXN | CONTIG |  RES0  |
 *         +-----------+----+-----+--------+--------+
 * 
 * The value `n` is:
 * 	- For Level 1, 30.
 * 	- For Level 2, 21.
 * 
 * Where:
 * 	- RES0:		Default, bits should be zero.
 *  - IGN:		Ignored.
 * 	- XN:		TTE points to a Block marked Execute-Never.
 *  - PXN:		TTE points to a Block marked Privileged Execute-Never.
 * 	- CONTIG:	TTE points to a Block which is Contiguous.
 * 	- nG:		Not Global bit.
 * 	- AF:		Access Flag bit.
 * 	- SH:		Shareability bit.
 * 	- NS:		Non-secure bit.
 * 	- AttrIdx:	Memory Attribute Index.
 * 
 * -----------------------------------------------------------------------------
 * 
 * Level 3 Page Descriptor format (4KB)
 * 
 * 63          50 49  48 47            12 11          2  1   0
 * +-------------+------+----------------+-------------+---+---+
 * | UpperAttrib | RES0 | PageOutputAddr | LowerAttrib | 1 | 1 |
 * +-------------+------+----------------+-------------+---+---+
 * 
 * UpperAttrib and LowerAttrib are the same as the Level 1/2 Block Descriptors.
*/
#define TTE_TYPE_MASK			0x0000000000000003ULL		/* mask to extract entry type */
#define TTE_TYPE_TABLE			0x0000000000000003ULL		/* table entry type */
#define TTE_TYPE_PAGE			0x0000000000000003ULL		/* page entry type */
#define TTE_TYPE_BLOCK			0x0000000000000001ULL		/* block entry type */

#define TTE_ENTRY_INVALID		0x0000000000000000ULL		/* invalid entry */
#define TTE_ENTRY_VALID			0x0000000000000001ULL		/* valid entry */

/* Level 0 General Values */
#define TT_L0_INDEX_MASK		0x0000ff8000000000ULL		/* mask to extract L0 table index from VA */
#define TT_L0_SIZE				0x0000008000000000ULL		/* size of memory covered by a TTE */
#define TT_L0_SHIFT				39							/* page descriptor shift */

/* Level 1 Generic Values */
#define TT_L1_INDEX_MASK		0x0000007fc0000000ULL		/* mask to extract L1 table index from VA */
#define TT_L1_SIZE				0x0000000040000000ULL		/* size of memory covered by a TTE */
#define TT_L1_SHIFT				30							/* page descriptor shift */

/* Level 2 General Values */
#define TT_L2_INDEX_MASK		0x000000003fe00000ULL		/* mask to extract L2 table index from VA */
#define TT_L2_SIZE				0x0000000000200000ULL		/* size of memory covered by a TTE */
#define TT_L2_SHIFT				21							/* page descriptor shift */

/* Level 3 General Values */
#define TT_L3_INDEX_MASK		0x00000000001ff000ULL		/* mask to extract L3 table index from VA */
#define TT_L3_SIZE				0x0000000000001000ULL		/* size of memory covered by a TTE */
#define TT_L3_SHIFT				12							/* page descriptor shift */


/**
 * Various bitmasks for extracting the Output Address from a particular TTE.
*/
#define TT_TABLE_MASK			0x0000fffffffff000			/* get the address of the next table */
#define TT_BLOCK_MASK			0x0000ffffffe00000
#define TT_PAGE_MASK			0x0000fffffffff000

/* Level 3-specific */
#define TT_L3_BLOCK_MASK		0x0000FFFFFFFF0000

/* Translation Table Base Address Mask*/
#define TTBR_BADDR_MASK			0x0000ffffffffffff


/*******************************************************************************
 * Name:	Exception Syndrom Register
 * Desc:	Macros for handling the ESR_ELx register upon an exception.
*******************************************************************************/

/**
 *  Exception Syndrome Register (ESR_EL1, ESR_EL0)
 * 
 *  31  26 25 24                 0
 *  +------+--+------------------+
 *  |  EC  |IL|       ISS        |
 *  +------+--+------------------+
 * 
 * Where:
 *      EC:     Exception Class
 *      IL:     Instruction Length
 *     ISS:     Instruction Specific Syndrome
 */

#define ESR_EC_SHIFT			26
#define ESR_EC_MASK				(0x3f << ESR_EC_SHIFT)
#define ESR_EC(_x)				((_x & ESR_EC_MASK) >> ESR_EC_SHIFT)

#define ESR_IL_SHIFT			25
#define ESR_IL					(1 << ESR_IL_SHIFT)

#define ESR_INSTR_IS_2BYTES(_x)	(!(_x & ESR_IL))

#define ESR_ISS_MASK			0x01ffffff
#define ESR_ISS(_x)				(_x & ESR_ISS_MASK)


/*******************************************************************************
 * Name:	Data Abort ISS (EL1)
 * Desc:	Data Abort bitfields.
*******************************************************************************/

/**
 * 	Data Abort ISS (EL1)
 * 
 * 	24              9  8  7  6  5    0
 * 	+---------------+--+--+-+---+----+
 * 	|000000000000000|EA|CM|S1PTW|WnR|DFSC|
 * 	+---------------+--+--+-+---+----+
 * 
 * Where:
 * 		EA:		External Abort type
 * 		CM:		Cache Maintenance operation
 * 		S1PTW:	Stage 2 exception on Stage 1 page table walk
 * 		WnR:	Write to Read
 * 		DFSC:	Data Fault Status Code
*/

#define ISS_DA_EA_SHIFT			9
#define ISS_DA_EA				(1 << ISS_DA_EA_SHIFT)

#define ISS_DA_CM_SHIFT 		8
#define ISS_DA_CM				(1 << ISS_DA_CM_SHIFT)

#define ISS_DA_WNR_SHIFT		6
#define ISS_DA_WNR				(1 << ISS_DA_WNR_SHIFT)

#define ISS_DA_S1PTW_SHIFT		7
#define ISS_DA_S1PTW			(1 << ISS_DA_S1PTW_SHIFT)

#define ISS_DA_FSC_MASK			0x3F
#define ISS_DA_FSC(x)			(x & ISS_DA_FSC_MASK)


/*******************************************************************************
 * Name:	Multi Processor Affinity Register
*******************************************************************************/

/**
 * ARM:		D17-6118
 * Field:	Aff3, Bits [39:32]
 * Desc:	Affinity Level 3. Not supported in AArch32 state.
*/
#define MPIDR_AFF3_SHIFT 		32
#define MPIDR_AFF3_WIDTH 		8
#define MPIDR_AFF3_MASK  		(((1 << MPIDR_AFF3_WIDTH) - 1) << MPIDR_AFF2_SHIFT)

/**
 * ARM:		D17-6119
 * Field:	Aff2, Bits [23:16]
 * Desc:	Affinity Level 2.
*/
#define MPIDR_AFF2_SHIFT 		16
#define MPIDR_AFF2_WIDTH 		8
#define MPIDR_AFF2_MASK  		(((1 << MPIDR_AFF2_WIDTH) - 1) << MPIDR_AFF2_SHIFT)

/**
 * ARM:		D17-6119
 * Field:	Aff1, Bits [15:8]
 * Desc:	Affinity Level 1.
*/
#define MPIDR_AFF1_SHIFT 		8
#define MPIDR_AFF1_WIDTH 		8
#define MPIDR_AFF1_MASK  		(((1 << MPIDR_AFF1_WIDTH) - 1) << MPIDR_AFF1_SHIFT)

/**
 * ARM:		D17-6119
 * Field:	Aff0, Bits [7:0]
 * Desc:	Affinity Level 0.
*/
#define MPIDR_AFF0_SHIFT 		0
#define MPIDR_AFF0_WIDTH 		8
#define MPIDR_AFF0_MASK  		(((1 << MPIDR_AFF0_WIDTH) - 1) << MPIDR_AFF0_SHIFT)

/* Affinity Level Mask */
#define MPIDR_AFFLVL_MASK		(0xff)

/* Affinity Level Helpers */
#define MPIDR_AFFLVL0_VAL(mpidr) \
		(((mpidr) >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK)
#define MPIDR_AFFLVL1_VAL(mpidr) \
		(((mpidr) >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK)
#define MPIDR_AFFLVL2_VAL(mpidr) \
		(((mpidr) >> MPIDR_AFF2_SHIFT) & MPIDR_AFFLVL_MASK)
#define MPIDR_AFFLVL3_VAL(mpidr) \
		(0)

/*******************************************************************************
 * Name:	Virtual Timer Definitions
*******************************************************************************/

/* Virtual Timer Control Register */
#define CNTV_CTL_EL0_ISTATUS	(1 << 2)
#define CNTV_CTL_EL0_IMASKED	(1 << 1)
#define CNTV_CTL_EL0_ENABLE		(1 << 0)


/*******************************************************************************
 * Name:	System Register Read/Write
*******************************************************************************/

#define MSR(reg, src)  __asm__ volatile ("msr " reg ", %0" :: "r" (src))
#define MRS(dest, reg) __asm__ volatile ("mrs %0, " reg : "=r" (dest))


////////////////////////////////////////////////////////////////////////////////

// remaining macros to clean up

/* TODO: These will be moved to tBoot and sent via boot-args */
#define TBOOT_KERNEL_PHYS_BASE  0x08000000
#define TBOOT_KERNEL_VIRT_BASE  0xfffffff000000000
#define TBOOT_KERNEL_MEM_SIZE   0x60000000
#define TBOOT_KERNEL_TOP_LIMIT  0xfffffff060000000
#define BOOTSTRAP_TABLE_SIZE    (TT_PAGE_SIZE * 500)

#endif /* __kern_proc_reg_h__ */
