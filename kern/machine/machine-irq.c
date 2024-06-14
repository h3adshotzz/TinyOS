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
 * 	Name:	machine/machine_gic.c
 * 	Desc:	Kernel Machine Interface.
 */

#include <arch/arch.h>

#include <kern/defaults.h>
#include <kern/machine.h>

#include <libkern/assert.h>

#include <kern/vm/pmap.h>


kern_return_t
machine_init_interrupts ()
{
	vm_address_t gic_region_virt_base, gicd_virt_base, gicr_virt_base;
	phys_addr_t gic_region_base, gicd_phys_base, gicr_phys_base;
	uint64_t gicd_size, gicr_size;

	/**
	 * TODO:	Read the base addresses from the device tree
	 * 
	 * Currently we're manually setting these addresses. This needs to move to
	 * reading them from the device tree, and even using the device tree to look
	 * at what interrupt controller is actually present - it might not be gic,
	 * although that's all that is supported atm.
	 * 
	 * The DT node for gic looks like this:
	 * 
	 * intc@8000000 {
	 *	compatible = "arm,gic-v3";
	 *	#interrupt-cells = <0x03>;
	 *	#address-cells = <0x02>;
	 *	#size-cells = <0x02>;
	 *	ranges;
	 *	interrupt-controller;
	 *	reg = 	<0x00 0x8000000 0x00 0x10000>,	// GICD
	 *			<0x00 0x80a0000 0x00 0xf60000>;	// GICR
	 *	...
	 * }
	*/

	gic_region_virt_base = 0xffffffff11000000;
	gic_region_base = (phys_addr_t) 0x8000000;

	gicd_virt_base = (vm_address_t) (gic_region_virt_base + 0x0);
	gicd_phys_base = (phys_addr_t) (gic_region_base + 0x0);
	gicd_size = 0x10000;

	gicr_virt_base = (vm_address_t) (gic_region_virt_base + 0xa0000);
	gicr_phys_base = (phys_addr_t) (gic_region_base + 0xa0000);
	gicr_size = 0xf60000;

	/**
	 * TODO: 	Use proper mapping api
	 * 
	 * By the time machine_init_interrupts is called, we should have both vm_page
	 * and vm_map running. Therefore, we should instead do something like
	 * 	vm_map_peripheral()
	 * 
	 * or whatever the api ends up being, rather than directly calling the pmap
	 * api.
	*/
	pmap_tt_create_tte (kernel_tte, gicd_phys_base, gicd_virt_base, gicd_size);
	pmap_tt_create_tte (kernel_tte, gicr_phys_base, gicr_virt_base, gicr_size);

	gic_interface_init (gicd_virt_base, gicr_virt_base);
	return KERN_RETURN_SUCCESS;
}

void
machine_register_interrupt (uint32_t intid, uint32_t priority)
{
	return gic_irq_register (intid, priority);
}

void
machine_send_interrupt (uint32_t intid, uint32_t target)
{
	gic_send_sgi (intid, target);
}