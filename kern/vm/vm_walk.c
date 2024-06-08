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

#include <kern/vm/pmap.h>
#include <kern/vm/vm.h>

#include <arch/proc_reg.h>

#define PRINT_PADDING(__n)		for (int c=0;c<__n;c++){kprintf("\t");}

static void _vm_pagetable_walk(tt_table_t *table_base, int level, int padding)
{
	/**
	 * 1.	loop through all 512 entries in the table
	 * 2.	decode each entry and determine if it's a block/page, or table
	 * 3.	block/page entries are printed, table entries recursively call
	 * 		this function again, until we reach the last table.
	*/

	for (int idx = 0; idx < (TT_PAGE_SIZE / 8); idx++) {
		tt_entry_t entry = table_base[idx];
		uint8_t type = (entry & TTE_TYPE_MASK);

		/* table entry */
		if (type == TTE_TYPE_TABLE && level < 3) {
			vm_address_t table_address = ptokva(entry & TT_TABLE_MASK);
			PRINT_PADDING(padding);
			kprintf("Level %d [%d]: Table descriptor @ 0x%lx:\n",
				level, idx, (entry & TT_TABLE_MASK));

			_vm_pagetable_walk((tt_table_t *) table_address, level+1, padding+1);
			continue;
		}

		/* Block */
		if (type == TTE_TYPE_BLOCK) {
			vm_address_t block_address = ptokva(entry & TT_BLOCK_MASK);
			PRINT_PADDING(padding);
			kprintf("Level %d [%d]: Block descriptor: 0x%lx (mapped to 0x%lx)\n",
				level, idx, (entry & TT_BLOCK_MASK), block_address);

			continue;
		}

		/* Page */
		if (type == TTE_TYPE_PAGE) {
			vm_address_t page_address = (entry & TT_PAGE_MASK);
			kprintf ("Level %d [%d]: Page Descriptor: 0x%lx\n",
				level, idx, page_address);

			continue;
		}
	}
}

void vm_pagetable_walk_ttbr1 ()
{
	vm_address_t base;

	base = (ptokva (mmu_get_tt_base_alt () & TTBR_BADDR_MASK));
	vm_log("walking pagetable at base: 0x%lx\n", base);
	_vm_pagetable_walk((tt_table_t *) base, 1, 0);
}

void vm_pagetable_walk_ttbr0 ()
{
	vm_address_t base;

	base = (mmu_get_tt_base () & TTBR_BADDR_MASK);
	_vm_pagetable_walk ((tt_table_t *) base, 1, 0);
}

void vm_pagetable_walk (tt_table_t *table, int level)
{
	_vm_pagetable_walk (table, level, 0);
}