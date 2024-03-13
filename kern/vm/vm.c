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
#include <kern/kdebug.h>
#include <kern/vm/vm.h>

#include <arch/proc_reg.h>

/**
 * Initially the kernel operates on "bootstrap pagetables". These are an identiy
 * mapping, where physical addresses are mapped 1:1 into virtual memory, and a
 * Kernel Virtual Address mapping, where the base of physical memory is mapped
 * to a virtual address specified by tBoot in the boot arguments.
 * 
 * The bootstrap tables must be within the .data section, and aligned to the
 * page size.
*/
KERNEL_GLOBAL_DEFINE(uint64_t) bootstrap_pagetables[BOOTSTRAP_TABLE_SIZE] 
	__attribute__((section(".data"))) __attribute__((aligned(TT_PAGE_SIZE)));


/**
 * The kernel only runs on these bootstrap tables for a short amount of time,
 * and they map everything including peripherals. Ideally, we want seperate
 * address ranges for the various peripherals and the kernel binary.
 * 
 * All the Kernel mappings will be made in the 'pagetables_region'. Unlike the
 * bootstrap tables, these are defined in data.S at a fixed size.
*/
KERNEL_EXTERN_DEFINE(vm_address_t) pagetables_region_base;
KERNEL_EXTERN_DEFINE(vm_address_t) pagetables_region_end;

/* The cursor is used to track as we allocate tables */
KERNEL_GLOBAL_DEFINE(vm_address_t) pagetables_region_cursor;

/* Kernel Translation Table Base */
PRIVATE_STATIC_DEFINE(tt_table_t *) gKernelTranslationTable;

/* Physical memory properties */
PRIVATE_STATIC_DEFINE(vm_address_t) gVirtBase;
PRIVATE_STATIC_DEFINE(vm_address_t) gPhysBase;
PRIVATE_STATIC_DEFINE(vm_address_t) gMemSize;

/* Static function definitions. Static functions are prepended with '_' if they are 'vm_' */
PRIVATE_STATIC_DEFINE_FUNC(vm_address_t) _vm_mmu_emulate_translation (vm_address_t virt);



/**
 * Name:	vm_kvtop
 * Desc:	Translate a given Virtual Address using the AT instruction, or, if
 * 			that fails, a manual page table walk of the current TTBR1.
*/
vm_address_t vm_kvtop (vm_address_t addr)
{
	vm_address_t phys;

	/**
	 * Try to do the translation lookup using the AT instruction first. If this
	 * fails, we'll do a manual pagetable walk of TTBR1.
	*/
	phys = mmu_translate_kvtop (addr);
	if (phys)
		return phys;

	return _vm_mmu_emulate_translation (addr);
}

/**
 * Name:	vm_page_zero
 * Desc:	Clean a given memory page.
*/
void vm_page_zero (vm_address_t page, vm_size_t size)
{
	/**
	 * There may be a case when two or more consecutive pages need to be cleaned
	 * at the same time, which is why a size can be specified.
	*/
	memset ((void *) page, '\0', size);
	return;
}

/**
 * Name:	_vm_page_alloc
 * Desc:	Allocate a new page within the kernel pagetables region. This is a
 * 			private function, as the process for this will be different if new
 * 			allocations are needed for other pagetables.
*/
PRIVATE_STATIC_DEFINE_FUNC(vm_address_t)
_vm_page_alloc ()
{
	vm_address_t vaddr;

	vaddr = pagetables_region_cursor;
	pagetables_region_cursor += DEFAULTS_KENREL_VM_PAGE_SIZE;

	// Ensure that the virtual address is within the pagetable region bounds
	assert (pagetables_region_end < vaddr);
	return vaddr;
}

/**
 * Name:	init_tt_table
 * Desc:	Initialise the translation table for a given virtual address range. 
*/
PRIVATE_STATIC_DEFINE_FUNC(void)
init_tt_table (tt_table_t *table, vm_address_t pstart, vm_address_t start, vm_address_t end)
{
	vm_addr_t map_address, map_address_l2;
	vm_offset_t index;
	tt_table_t *l2_table;
	tt_entry_t entry;

	/* fill the L1 table */
	map_address = start;
	while (map_address < end) {

		/* calculate the index for the L2 table */
		index = ((map_address & TT_L1_INDEX_MASK) >> TT_L1_SHIFT);

		/* if the index is not already a table descriptor, create the L2 table */
		if ((table[index] & TTE_TYPE_MASK) != TTE_TYPE_TABLE) {
			l2_table = (tt_table_t *) vm_kvtop (_vm_page_alloc ());
			entry = (vm_kvtop (l2_table) & TT_TABLE_MASK) | 0x3;
			table[index] = entry;
		} else {
			l2_table = (tt_table_t *) (table[index] & TT_TABLE_MASK);
		}

		/* fill the L2 table */
		map_address_l2 = map_address;
		while (map_address_l2 < (map_address + TT_L1_SIZE) && map_address_l2 < end) {
			
			index = ((map_address_l2 & TT_L2_INDEX_MASK) >> TT_L2_SHIFT);
			entry = TTE_BLOCK_TEMPLATE | (pstart + (map_address_l2 - start) & TT_TABLE_MASK);
			l2_table[index] = entry;

			map_address_l2 += TT_L2_SIZE;
		}
		map_address += TT_L1_SIZE;
	}
	kprintf ("mapped: 0x%llx -> 0x%llx: 0x%llx\n", start, end, pstart);
}


/**
 * Name:	arm_vm_init
 * Desc:	Initialise the virtual memory system by creating mappings for the
 * 			kernel binary and the UART registers. These mappings are created at
 * 			the same addresses as the bootstrap tables in order for us to easily
 * 			continue executing.
 * 
 * 			Further along in the boot process we will then add mappings for the
 * 			rest of kernel memory for tasks, heaps, etc. For now, there is no
 * 			heap.
*/
void arm_vm_init (struct boot_args *args, address_t membase, vm_size_t memsize)
{
	vm_address_t map_addr, map_addr_l2;
	tt_page_t *l2, *l3;
	vm_offset_t index;
	tt_entry_t entry;

	/* initialise the global memory information */
	assert (args->virtbase > 0);
	gVirtBase = args->virtbase;

	assert (args->physbase > 0);
	gPhysBase = args->physbase;

	assert (memsize > 0);
	gMemSize = memsize;

	kprintf ("arm_vm_init: virt: 0x%llx, phys: 0x%llx, size: 0x%llx\n",
		gVirtBase, gPhysBase, gMemSize);

	/**
	 * tBoot will prepare the base of DRAM to contain first the Kernel, then the
	 * device tree, and finally the boot arguments. Here, we will create two
	 * mappings for the kernel binary and uart:
	 * 
	 * 	.name = vm_map_uart					.name = vm_map_kernel_bootstrap
	 * 	.virt_base = 0xfffffff001000000		.virt_base = 0xfffffff040500000
	 * 	.phys_base = 0x9000000				.phys_base = 0x40000000
	 * 	.size = 0x0001000					.size = 0x01500000
	 * 	.type = VM_TYPE_DEVICE				.type = VM_TYPE_KERNEL
	 * 
	 * So, memory will look like this:
	 * 		0xfffffff001000000 -> 0xfffffff001001000
	 * 		0xfffffff040500000 -> 0xfffffff041e00000
	 * 
	*/

	/* set initial pagetable cursor */
	pagetables_region_cursor = &pagetables_region_base;
	kprintf ("arm_vm_init: pagetable_region_cursor: 0x%llx\n", pagetables_region_cursor);

	/* allocate the first table */
	gKernelTranslationTable = (tt_page_t *) _vm_page_alloc ();
	kprintf ("arm_vm_init: gKernelTranslationTable: 0x%llx\n", gKernelTranslationTable);

	//vm_address_t gic_virt = (args->virtbase + (args->uartbase - args->physbase - 0x1000000));
	//init_tt_table (gKernelTranslationTable, args->uartbase - 0x1000000, gic_virt, gic_virt + 0x1000000);

	/* create a mapping for the uart peripheral */
	vm_address_t uart_virt = (args->virtbase + (args->uartbase - args->physbase));
	init_tt_table (gKernelTranslationTable, args->uartbase, uart_virt, uart_virt + args->uartsize);

	/* create a mapping for the kernel binary */
	vm_address_t kern_virt = (args->virtbase + (args->kernbase - args->physbase));
	init_tt_table (gKernelTranslationTable, args->kernbase, kern_virt, kern_virt + args->kernsize);

	vm_pagetable_walk (gKernelTranslationTable, 1);

	arm64_write_ttbr1_el1 (vm_kvtop (gKernelTranslationTable));
	kprintf ("arm_vm_init: initial kernel page tables initialised\n");
}

/*******************************************************************************
 * MMU Translation Emulation
*******************************************************************************/

PRIVATE_STATIC_DEFINE_FUNC(vm_address_t) 
_vm_mmu_emulate_translation (vm_address_t virt)
{
	return 0;
}