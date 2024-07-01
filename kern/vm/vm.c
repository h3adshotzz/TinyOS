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

#include <kern/vm/vm_page.h>
#include <kern/vm/vm_map.h>
#include <kern/defaults.h>
#include <kern/vm/pmap.h>
#include <kern/vm/vm.h>
#include <kern/task.h>

#include <libkern/assert.h>
#include <libkern/boot.h>
#include <libkern/list.h>

#include <arch/proc_reg.h>

unsigned int bootstrap_pagetables[BOOTSTRAP_TABLE_SIZE] 
	__attribute__((section(".data"))) __attribute__((aligned(TT_PAGE_SIZE)));

/* physical memory */
vm_address_t		memory_virt_base;
vm_address_t		memory_phys_base;
vm_address_t		memory_phys_size;

/* physical kernel memory properties */
static phys_addr_t	kernel_phys_base;
static phys_size_t	kernel_phys_size;

static vm_address_t	kernel_virt_base;

/* kernel pmap and vm_map */
static struct pmap 	kernel_pmap_ref __attribute__((section(".data")));
static pmap_t 		*kernel_pmap = &kernel_pmap_ref;
static vm_map_t		*kernel_vm_map = (&kernel_pmap_ref + sizeof(task_t));


/* temporary, for debugging */
void __vm_debug_dump_map (vm_map_t *map)
{
	vm_log("dumping virtual memory map from address: 0x%lx\n", map);
	kprintf("   timestamp: 0x%lx\n", map->timestamp);
	kprintf("        pmap: 0x%lx\n", &map->pmap);
	kprintf("         min: 0x%lx\n", map->min);
	kprintf("         max: 0x%lx\n", map->max);
	kprintf("alloc'd size: 0x%lx\n", map->size);
	kprintf("       flags: lock: %d\n", map->lock);
	kprintf("     entries: %d\n", map->nentries);

	vm_map_entry_t *entry;
	int idx = 0;
	list_for_each_entry(entry, &map->entries, siblings) {
		kprintf("  [%d]: 0x%lx -> 0x%lx (%d bytes)",
			idx, entry->base, entry->base + entry->size, entry->size);
		if (entry->guard_page)
			kprintf("\t- GUARD_PAGE");

		kprintf("\n");
		idx+=1;
	}
	kprintf ("\n");
}

vm_map_t *vm_get_kernel_map()
{
	return (vm_map_t *) kernel_vm_map;
}

kern_return_t vm_is_address_valid(vm_address_t addr)
{
	/**
	 * Check if a given virtual address is valid by converting it to a physical
	 * address via the pmap mmu api. If the result is non-zero, the address is
	 * valid, otherwise it's not.
	*/
	return (mmu_translate_kvtop(addr)) ? 
		KERN_RETURN_SUCCESS : KERN_RETURN_FAIL;
}

/*******************************************************************************
 * Name:	vm_configure
 * Desc:
 ******************************************************************************/

void vm_configure (void)
{
	/**
	 * Create the vm_page's for the entire non-secure memory region. We do not
	 * create pages for device memory. The kernel is placed at the lowest usable
	 * physical memory address.
	*/
	vm_page_bootstrap (kernel_phys_base, memory_phys_size, kernel_phys_size);

	/**
	 * Create the kernel tasks vm_map just after the pmap structure. This is all
	 * (hopefully) within a single 4KB page.
	*/
	vm_map_create(kernel_vm_map, &kernel_pmap, kernel_virt_base, VM_KERNEL_MIN_ADDRESS);
	vm_map_entry_create(kernel_vm_map, kernel_virt_base, kernel_phys_size, VM_NULL);

}

/*******************************************************************************
 * Name:	arm_vm_init
 * Desc:	Initialise the kernel page tables and pmap structure. This will
 * 			create initial translation table entries for the kernel binary and
 * 			boot conosle (uart).
 *
 * 			In the future when the kernel is loaded as an ELF, the sections will
 * 			be mapped here.
 *
 ******************************************************************************/

void arm_vm_init (struct boot_args *args, phys_addr_t membase, phys_size_t memsize)
{
	vm_address_t console_virt_base;

	assert(membase > 0 && memsize > 0);

	/* set global memory information */
	memory_phys_base = args->physbase;		// TODO, properly determine whether memory is discovered
	memory_phys_size = memsize;				//	from device tree, or bootloader.

	/* virtual memory base set by tboot */
	memory_virt_base = args->virtbase;

	/* init the pagetable region */
	pmap_ptregion_create ();

	/* create the kernel and invalid pagetables */
	kernel_tte = (tt_table_t *) pmap_ptregion_alloc ();
	kernel_ttep = mmu_translate_kvtop (kernel_tte);

	invalid_tte = (tt_table_t *) pmap_ptregion_alloc ();
	invalid_ttep = mmu_translate_kvtop (invalid_tte);

	/**
	 * tBoot passes two regions to map before configuring the rest of the virtual
	 * memory system: kernel and console.
	 *
	 * We map the kernel to the same virtual address base as in the reset vector,
	 * and map the console to the default virtual peripherals base address.
	 */
	kernel_virt_base = memory_virt_base;
	kernel_phys_base = args->kernbase;
	kernel_phys_size = args->kernsize;

	console_virt_base = DEFAULTS_KERNEL_VM_PERIPH_BASE;

	/* directly create the translation table entries */
	pmap_tt_create_tte (kernel_tte, kernel_phys_base, kernel_virt_base, kernel_phys_size, PMAP_ACCESS_READWRITE);
	pmap_tt_create_tte (kernel_tte, args->uartbase, console_virt_base, args->uartsize, PMAP_ACCESS_READWRITE);

	/* switch the mmu to use the new translation tables */
	mmu_set_tt_base_alt (kernel_ttep & TTBR_BADDR_MASK);
	mmu_set_tt_base (kernel_ttep & TTBR_BADDR_MASK);

}

void vm_debug_overview ()
{
	kprintf ("initialised kernel from 0x%lx - 0x%lx (0x%lx - 0x%lx)\n",
		kernel_virt_base, kernel_virt_base + kernel_phys_size,
		kernel_phys_base, kernel_phys_base + kernel_phys_size);

	vm_pagetable_walk_ttbr1 ();
}