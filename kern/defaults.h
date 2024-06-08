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
 *	Name:	defaults.h
 *	Desc:	Kernel default/tunnable values.
 */

#ifndef __KERN_DEFAULTS_H__
#define __KERN_DEFAULTS_H__

#include <arch/proc_reg.h>
#include <libkern/compiler.h>


/* Enable/Disable values */
#define DEFAULTS_ENABLE			UL(1)
#define DEFAULTS_DISABLE		UL(0)

#define DEFAULTS_SET(__X)		(__X == DEFAULTS_ENABLE)

/* Kernel - core */
#define DEFAULTS_KERNEL_BUILD_MACHINE		"tempest"
#define DEFAULTS_KERNEL_IMAGEINFO_HEADER	DEFAULTS_ENABLE

/* Kernel - memory */
#define DEFAULTS_KERNEL_VM_STACK_SIZE		UL(16386)
#define DEFAULTS_KERNEL_VM_PAGE_SIZE		TT_PAGE_SIZE
#define DEFAULTS_KERNEL_VM_VIRT_BASE		UL(0xfffffff000000000)
#define DEFAULTS_KERNEL_VM_PERIPH_BASE		UL(0xffffffff10000000)

#define DEFAULTS_KERNEL_VM_USE_L3_TABLE		DEFAULTS_DISABLE

/* Kernel - debug */
#define DEFAULTS_KERNEL_DEBUG_UART_BAUD		115200
#define DEFAULTS_KERNEL_DEBUG_UART_CLK		0x16e3600

/* Machine */
#define DEFAULTS_MACHINE_MAX_CPUS			UL(16)
#define DEFAULTS_MACHINE_MAX_CPU_CLUSTERS	UL(4)

#define DEFAULTS_MACHINE_LIBFDT_WORKAROUND	DEFAULTS_ENABLE

/* Platform */
#define DEFAULTS_PLAT_DEVICETREE_CELL_SIZE	2

/* Boot Arguments */
#define DEFAULTS_BA_OFFSET_VIRTBASE			UL(8)
#define DEFAULTS_BA_OFFSET_PHYSBASE			UL(16)
#define DEFAULTS_BA_OFFSET_MEMSIZE			UL(24)

#define DEFAULTS_BA_OFFSET_KERN_PHYBASE		UL(30)




#endif /* __kern_defaults_h__ */
