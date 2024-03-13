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

/** NOTE: These are test values until reading them from the device tree works */
#define QEMU_GIC_PERIPH_BASE		0x8000000 //0xfffffff000000000

#define DEBUG_TEST_GICD_BASE		(QEMU_GIC_PERIPH_BASE + 0x000000)
#define DEBUG_TEST_GICR_BASE		(QEMU_GIC_PERIPH_BASE + 0x0A0000)


kern_return_t
machine_init_interrupts ()
{
	gic_interface_init (DEBUG_TEST_GICD_BASE, DEBUG_TEST_GICR_BASE);
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