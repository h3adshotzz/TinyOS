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
 * Name:	pmap.c
 * Desc:	Implementation of the physical memory mapping (pmap) header.
*/

#include <tinylibc/stdint.h>

#include <kern/defaults.h>

#include <kern/vm/pmap.h>
#include <kern/vm/vm.h>

/**
 * The physical mapping list is contained within pmap.c so it can only be
 * accessed via the pmap api. Realistically there will only be two physical maps
 * however to future-proof things, it's implemented as an array.
*/
static pmap_t pmap_list[PMAP_LIST_MAX] __attribute__((section(".data")));

/**
 * The pagetable region can only be initialised once.
*/
static int ptregion_initialised = 0;
static pmap_addr_t ptregion_phys_base;

/**
 * The pagetables_region is declared in data.S, and is a 16-page region carveout
 * which is used for storing the kernels pagetables. The size of the region is
 * hard-coded in defaults.h
 * 
 * The pagetables_region_cursor is used to track through the region.
*/
KERNEL_EXTERN_DEFINE(vm_address_t)	pagetables_region_base;
KERNEL_EXTERN_DEFINE(vm_address_t)	pagetables_region_end;

KERNEL_GLOBAL_DEFINE(vm_address_t)	pagetables_region_cursor;

/* Pointer to the root kernel translation table entry */
KERNEL_GLOBAL_DEFINE(tt_table_t)	*kernel_tte __attribute__((section(".data")));
KERNEL_GLOBAL_DEFINE(pmap_addr_t)	kernel_ttep __attribute__((section(".data")));


/**
 * Allocates a new 4KB page within the pagetable region in the kernel, from the
 * address of pagetables_region_cursor.
*/
static vm_address_t pmap_ptregion_alloc ()
{
	vm_address_t vaddr;
	
	vaddr = pagetables_region_cursor;
	pagetables_region_cursor += DEFAULTS_KENREL_VM_PAGE_SIZE;

	/* ensure that the address is within the pagetable region bounds */
	assert (pagetables_region_end < vaddr);
	return vaddr;
}

/**
 * Name:	pmap_ptregion_create
 * Desc:	Create a new translation table within the pagetables region. The
 * 			pagetables region is 16-pages in size, and is used for storing kernel
 * 			tables, and the invalid tables.
*/
pmap_return_t pmap_ptregion_create ()
{
	/* check whether we have already initialised the pagetable region */
	if (ptregion_initialised)
		return PMAP_RETURN_ILLEGAL;

	/* ensure that the address is valid */
	if (!&pagetables_region_base)
		return PMAP_RETURN_INVALID;

	/* set the initial pagetable region cursor */
	pagetables_region_cursor = &pagetables_region_base;
	ptregion_phys_base = mmu_translate_kvtop (&pagetables_region_base);

	pmap_log ("initialised pagetables region: 0x%llx - 0x%llx\n", 
		ptregion_phys_base, ptregion_phys_base + DEFAULTS_KENREL_VM_PAGE_SIZE*16);

	ptregion_initialised = 1;
	return PMAP_RETURN_SUCCESS;
}

/**
 * Name:	pmap_create_tte
 * Desc:	
*/
pmap_return_t pmap_create_tte (tt_table_t *table, pmap_addr_t pbase,
							vm_address_t vbase, vm_size_t size)
{
	vm_address_t map_address, map_address_l2, vend;
	vm_offset_t index;
	tt_table_t *l2_table;
	tt_entry_t entry;

	/* calculate the virtual end of the region */
	vend = vbase + size;

	/* fill the L1 table */
	map_address = vbase;
	while (map_address < vend) {

		/* calculate the index for the L2 table */
		index = ((map_address & TT_L1_INDEX_MASK) >> TT_L1_SHIFT);

		/* if the index is not already a table descriptor, create the L2 table */
		if ((table[index] & TTE_TYPE_MASK) != TTE_TYPE_TABLE) {
			l2_table = (tt_table_t *) mmu_translate_kvtop (pmap_ptregion_alloc ());
			entry = (mmu_translate_kvtop (l2_table) & TT_TABLE_MASK) | 0x3;
			table[index] = entry;
		} else {
			l2_table = (tt_table_t *) (table[index] & TT_TABLE_MASK);
		}

		/* fill the L2 table */
		map_address_l2 = map_address;
		while (map_address_l2 < (map_address + TT_L1_SIZE) && map_address_l2 < vend) {
			
			index = ((map_address_l2 & TT_L2_INDEX_MASK) >> TT_L2_SHIFT);
			entry = TTE_BLOCK_TEMPLATE | (pbase + (map_address_l2 - vbase) & TT_TABLE_MASK);
			l2_table[index] = entry;

			map_address_l2 += TT_L2_SIZE;
		}
		map_address += TT_L1_SIZE;
	}

	pmap_log ("mapped 0x%llx -> 0x%llx to phys 0x%llx\n", vbase, vend, pbase);
	return PMAP_RETURN_SUCCESS;
}
