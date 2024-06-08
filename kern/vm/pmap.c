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

#include <tinylibc/stdint.h>

#include <kern/defaults.h>
#include <kern/vm/pmap.h>
#include <kern/vm/vm.h>

#include <libkern/assert.h>


/* pagetable region state */
static int ptregion_initialised = 0;
static phys_addr_t ptregion_phys_base;

/**
 * The pagetables region is declared in data.S, and is a 16-page region carveout
 *
 */
extern vm_address_t	pagetables_region_base;
extern vm_address_t pagetables_region_end;

vm_address_t pagetables_region_cursor;

/* pointers to the kernel and invalid tables */
tt_table_t	*kernel_tte		__attribute__((section(".data")));
tt_table_t	*invalid_tte	__attribute__((section(".data")));

/* physical addresses of the kernel and invalid tables */
phys_addr_t	kernel_ttep 	__attribute__((section(".data")));
phys_addr_t	invalid_ttep	__attribute__((section(".data")));


/******************************************************************************
 * Management of the Kernel pagetable region, only used for kernel pagetables
 *
 *****************************************************************************/

/**
 *	Name:	pmap_ptregion_alloc
 *	Desc:	Allocates space within the pagetables region for a new kernel
 *			pagetable, and increments the pagetable_region_cursor.
 */
vm_address_t pmap_ptregion_alloc ()
{
	vm_address_t vaddr;

	vaddr = pagetables_region_cursor;
	pagetables_region_cursor += DEFAULTS_KERNEL_VM_PAGE_SIZE;

	/* ensure that the address is within the pagetable region bounds */
	assert (pagetables_region_end < vaddr);
	return vaddr;
}

/**
 *
 */
pmap_return_t pmap_ptregion_create ()
{
	/* check whether we have already initialised the pagetable region */
	if (ptregion_initialised)
		return PMAP_RETURN_ILLEGAL;

	/* ensure that the address is valid */
	if (&pagetables_region_base == NULL)
		return PMAP_RETURN_INVALID;

	/* set the initial pagetable region cursor */
	pagetables_region_cursor = &pagetables_region_base;
	ptregion_phys_base = mmu_translate_kvtop (&pagetables_region_base);

	pmap_log ("initialised pagetables region: 0x%lx - 0x%lx\n",
			ptregion_phys_base,
			ptregion_phys_base + DEFAULTS_KERNEL_VM_PAGE_SIZE * 16);

	ptregion_initialised = 1;
	return PMAP_RETURN_SUCCESS;
}

/******************************************************************************
 * General translation table management
 *
 ******************************************************************************/

/**
 *	Name:	pmap_tt_create_tte
 *	Desc:	Create a physical translation table entry in the given table.
 */
pmap_return_t pmap_tt_create_tte (tt_table_t *table, phys_addr_t pbase,
								vm_address_t vbase, vm_size_t size)
{
	vm_address_t map_address, map_address_l2, map_address_l3, vend;
	vm_offset_t index;
	tt_table_t *l2_table, *l3_table;
	tt_entry_t entry;

//	pmap_log ("pmap_tt_create_tte(0x%lx, 0x%lx, 0x%lx, %d)\n",
//		table, pbase, vbase, size);

	/* calculate the virtual end of the region */
	vend = vbase + size;

	/* fill the L1 table */
	map_address = vbase;
	while (map_address < vend) {

		/* calculate the index for the L1 table */
		index = ((map_address & TT_L1_INDEX_MASK) >> TT_L1_SHIFT);

		/* if the index is not already a table descriptor, create the L2 table */
		if ((table[index] & TTE_TYPE_MASK) != TTE_TYPE_TABLE) {
			l2_table = (tt_table_t *) pmap_ptregion_alloc ();
			entry = ((vm_address_t) mmu_translate_kvtop (l2_table) & TT_TABLE_MASK) | 0x3;
			table[index] = entry;
		} else {
			l2_table = (tt_table_t *) (ptokva(table[index] & TT_TABLE_MASK));
		}

		/* fill the L2 table */
		map_address_l2 = map_address;
		while (map_address_l2 < (map_address + TT_L1_SIZE) && map_address_l2 < vend) {

			/* calculate the index into the L2 table */
			index = ((map_address_l2 & TT_L2_INDEX_MASK) >> TT_L2_SHIFT);

#if DEFAULTS_KERNEL_VM_USE_L3_TABLE
			if ((l2_table[index] & TTE_TYPE_MASK) != TTE_TYPE_TABLE) {
				l3_table = (tt_table_t *) mmu_translate_kvtop (pmap_ptregion_alloc ());
				entry = (mmu_translate_kvtop (l3_table) & TT_TABLE_MASK) | 0x3;
				l2_table[index] = entry;
			} else {
				l3_table = (tt_table_t *) (ptokva(l2_table[index] & TT_TABLE_MASK));
			}

			/* fill the L3 table */
			map_address_l3 = map_address_l2;
			while (map_address_l3 < (map_address_l2 + TT_L2_SIZE) && map_address_l3 < vend) {

				index = ((map_address_l3 & TT_L3_INDEX_MASK) >> TT_L3_SHIFT);
				entry = TTE_PAGE_TEMPLATE | (pbase + (map_address_l3 - vbase) & TT_TABLE_MASK);
				l3_table[index] = entry;

				map_address_l3 += TT_L3_SIZE;
			}
#else
			entry = TTE_BLOCK_TEMPLATE | (pbase + (map_address_l2 - vbase) & TT_TABLE_MASK);
			l2_table[index] = entry;
#endif
			map_address_l2 += TT_L2_SIZE;
		}
		map_address += TT_L1_SIZE;
	}

//	pmap_log ("mapped 0x%llx -> 0x%llx to phys 0x%llx\n", vbase, vend, pbase);
	return PMAP_RETURN_SUCCESS;
}

