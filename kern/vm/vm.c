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
#include <kern/vm/pmap.h>
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

/* Physical memory properties */
PRIVATE_STATIC_DEFINE(vm_address_t) gVirtBase;
PRIVATE_STATIC_DEFINE(vm_address_t) gPhysBase;
PRIVATE_STATIC_DEFINE(vm_address_t) gMemSize;


/**
 * Name:	arm_vm_init
 * Desc:	Initialise the kernel virtual memory subsystem, creating the kernel
 * 			pmap structure and allocate the pagetables_region. Create mappings
 * 			for the kernel binary and memory-mapped peripherals.
 * 
 * 			TODO: When we load the kernel as an ELF, this is where the sections
 * 			will be mapped.
*/
void arm_vm_init (struct boot_args *args, address_t membase, vm_size_t memsize)
{
	vm_address_t gic_virt_base, uart_virt_base, kern_virt_base;

	/* initialise the global memory information */
	gVirtBase = args->virtbase;
	gPhysBase = args->physbase;
	gMemSize = memsize;

	assert (args->physbase > 0);
	assert (args->virtbase > 0);
	assert (memsize > 0);

	/* announce the memory information */
	vm_log ("arm_vm_init: [0x%llx - 0x%llx : 0x%llx - 0x%llx]\n",
		gVirtBase, gVirtBase + gMemSize, gPhysBase, gPhysBase + gMemSize);

	/* create the pagetables region, so we can start mapping peripherals */
	pmap_ptregion_create ();

	/**
	 * tBoot should prepare the base of DRAM to first contain the kernel binary,
	 * then the device tree, and then the boot arguments structure. Here, we
	 * will create three mappings - GICv3 and UART memory-mapped register files,
	 * and the kernel binary. All other memory is unampped.
	*/
	gic_virt_base = (args->virtbase + (args->uartbase - args->physbase - 0x1000000));
	uart_virt_base = (args->virtbase + (args->uartbase - args->physbase));
	kern_virt_base = (args->virtbase + (args->kernbase - args->physbase));

	/** TODO: tBoot needs to send the GIC region, as we're just guessing atm */
	pmap_create_tte (kernel_tte, args->uartbase - 0x1000000, gic_virt_base, 0x1000000);
	pmap_create_tte (kernel_tte, args->uartbase, uart_virt_base, args->uartsize);
	pmap_create_tte (kernel_tte, args->kernbase, kern_virt_base, args->kernsize);

	/* Walk the pagetables to ensure things are in-order */
	vm_pagetable_walk (kernel_tte, 1);

	/* Notify the MMU of the new translation table base */
	mmu_set_tt_base_alt (kernel_ttep);

	vm_log ("initial virtual memory subsystem setup complete\n");
}
