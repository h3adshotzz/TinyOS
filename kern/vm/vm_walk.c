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
 * 	Name:	vm.c
 * 	Desc:	Kernel Virtual Memory Interface.
 */

#include <tinylibc/stdint.h>

#include <libkern/assert.h>
#include <libkern/boot.h>

#include <kern/defaults.h>
#include <kern/vm/pmap.h>
#include <kern/vm/vm.h>

#include <arch/proc_reg.h>


#define PRINT_PADDING(_n)		for (int c=0;c<_n;c++) {kprintf("\t");}


PRIVATE_STATIC_DEFINE_FUNC(void)
_vm_pagetable_walk (tt_table_t *table_base, int level, int padding)
{
	// 1. loop through 512 entries in the table
	// 2. decode each entry and determine if it's a block/page, or table entry
	// 3. block/page entries are printed, table entries recursively call this
	//	  again, until we reach an L3 table.

	for (int idx = 0; idx < (TT_PAGE_SIZE / 8); idx++) {
		tt_entry_t entry = table_base[idx];
		uint8_t type = (entry & TTE_TYPE_MASK);

		/* Table entry */
		if (type == TTE_TYPE_TABLE && level < 3) {
			vm_address_t table_address = (entry & TT_TABLE_MASK);
			PRINT_PADDING(padding);
			kprintf ("Level %d [%d]: Table Descriptor: 0x%llx\n",
				level, idx, table_address);

			_vm_pagetable_walk ((tt_table_t *) table_address, level+1, padding+1);
			continue;
		}

		/* Block */
		if (type == TTE_TYPE_BLOCK) {
			vm_address_t block_address = (entry & TT_BLOCK_MASK);
			PRINT_PADDING(padding);
			kprintf ("Level %d [%d]: Block Descriptor: 0x%llx\n",
				level, idx, block_address);
			
			continue;
		}

		/* Page */
		if (type == TTE_TYPE_PAGE) {
			vm_address_t page_address = (entry & TT_PAGE_MASK);
			kprintf ("Level %d [%d]: Page Descriptor: 0x%llx\n",
				level, idx, page_address);

			continue;
		}
	}
}


/**
 * Name:	vm_pagetable_walk_ttbr1
 * Desc:	Walk the current TTBR1 pagetable.
*/
void vm_pagetable_walk_ttbr1 ()
{
	vm_address_t base;

	base = (mmu_get_tt_base_alt () & TTBR_BADDR_MASK);
	_vm_pagetable_walk ((tt_table_t *) base, 1, 0);
}

/**
 * Name:	vm_pagetable_walk_ttrb0
 * Desc:	Walk the current TTBR0 pagetable.
*/
void vm_pagetable_walk_ttrb0 ()
{
	vm_address_t base;

	base = (mmu_get_tt_base () & TTBR_BADDR_MASK);
	_vm_pagetable_walk ((tt_table_t *) base, 1, 0);
}

/**
 * Name:	vm_pagetable_walk
 * Desc:	Walk a specified pagetable.
*/
void vm_pagetable_walk (tt_table_t *table, int level)
{
	_vm_pagetable_walk (table, level, 0);
}