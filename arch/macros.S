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
 * 	Name:	macros.S
 * 	Desc:	Provide various macros for use within early setup or exception vectors,
 * 			including macros for creating initial bootstrap pagetables.
 */

#include <arch/proc_reg.h>

/*******************************************************************************
 * Name:	create_tt_table_entry
 * Desc:	Create a Table entry at the given translation table which points to
 * 			a next-level translation table. Temporary parameters are overwritten.
 *
 * 			virt_addr		-	Virtual address to create an entry for.
 * 			ln_table		-	Current translation table pointer.
 * 			lx_table		-	Next translation table pointer.
 * 			index_mask		-	Bitmask to get Ln table index for virt_addr.
 * 			shift			-	Shift for this translation table level.
 *
*******************************************************************************/
.macro create_tt_table_entry    virt_addr, ln_table, lx_table, index_mask, shift, tmp1, tmp2
	/* get the index into the Ln table from virt_addr */
	and		\tmp1, \virt_addr, #(\index_mask)
	lsr		\tmp1, \tmp1, #(\shift)

	/* convert index into pointer */
	lsl		\tmp1, \tmp1, #(TTE_SHIFT)
	add		\tmp1, \ln_table, \tmp1

	/* create the entry, encoding the address of the next Lx table */
	and		\tmp2, \lx_table, #(TT_TABLE_MASK)
	orr		\tmp2, \tmp2, #(0x3)
	str		\tmp2, [\tmp1]
.endm


/*******************************************************************************
 * Name:	create_tt_block_entries
 * Desc:	Create a number of Block entries for translating a given virtual
 * 			address into a given physical address. Temporary parameters are
 * 			overwritten.
 *
 * 			virt_addr		-	Virtual address to create block entries for.
 * 			phys_addr		-	Physical address to create block entries for.
 * 			ln_table		-	Current translation table pointer.
 * 			index_mask		-	Bitmask to get Ln table index for virt_addr.
 * 			shift			-	Shift for this translation table level.
 * 			table_mask		-	Bitmask to get address bits from phys_addr.
 * 			nents			-	Number of block entries to create in this table.
 *
*******************************************************************************/
.macro create_tt_block_entries virt_addr, phys_addr, ln_table, index_mask, shift, table_mask, nents, tmp1, tmp2, tmp3, tmp4
	/* get the index into the Ln table from the virt_addr */
	and		\tmp1, \virt_addr, #(\index_mask)
	lsr		\tmp1, \tmp1, #(\shift)

	/* convert index into pointer */
	lsl		\tmp1, \tmp1, #(TTE_SHIFT)
	add		\tmp1, \ln_table, \tmp1

	/* create an initial block entry for the given virt and phys address */
	mov		\tmp2, #(TTE_BLOCK_TEMPLATE)
	and		\tmp3, \phys_addr, #(\table_mask)
	orr		\tmp3, \tmp2, \tmp3

	mov		\tmp2, \nents
	mov		\tmp4, #(TT_L2_SIZE)

1:
	str		\tmp3, [\tmp1], #(1 << TTE_SHIFT)
	add		\tmp3, \tmp3, \tmp4
	subs	\tmp2, \tmp2, #1
	b.ne 	1b
.endm


/*******************************************************************************
 * Name:	create_bootstrap_region
 * Desc:	Create a bootstrap translation table for a given virtual address to
 * 			physical address range. This creates a two-level mapping using Block
 * 			entries. Temporary parameters are overwritten.
 *
 * 			virt_addr		-	Virtual address to create block entries for.
 * 			phys_addr		-	Physical address to create block entries for.
 * 			entries			-	Number of entries to create.
 * 			l1_table		-	Level 1 table pointer.
 * 			l2_table		-	Level 2 table pointer.
 *
*******************************************************************************/
.macro create_bootstrap_region virt_addr, phys_addr, entries, l1_table, l2_table, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6
	/* calculate number of entries remaining */
	and		\tmp1, \virt_addr, #(TT_L2_INDEX_MASK)
	lsr		\tmp1, \tmp1, #(TT_L2_SHIFT)
	mov		\tmp2, #(TTE_PAGE_ENTRIES)
	sub		\tmp1, \tmp2, \tmp1

	/* allocate a Level 2 table */
3:  add		\l2_table, \l2_table, TT_PAGE_SIZE

	/* create the Level 1 table entry */
	create_tt_table_entry	\virt_addr, \l1_table, \l2_table, TT_L1_INDEX_MASK, TT_L1_SHIFT, \tmp3, \tmp4

	cmp		\entries, \tmp1
	csel	\tmp1, \entries, \tmp1, lt

	/* create the Level 2 table entry */
	create_tt_block_entries		\virt_addr, \phys_addr, \l2_table, TT_L2_INDEX_MASK, TT_L2_SHIFT, TT_BLOCK_MASK, \tmp1, \tmp3, \tmp4, \tmp5, \tmp6

	subs	\entries, \entries, \tmp1
	b.eq	2f

	add		\phys_addr, \phys_addr, \tmp1, lsl #(TT_L2_SHIFT)
	add		\virt_addr, \virt_addr, \tmp1, lsl #(TT_L2_SHIFT)

	mov		\tmp1, #(TT_PAGE_SIZE >> 3)
	b		3b
2:
.endm

