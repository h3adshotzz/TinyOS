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
 * 	Name:	defaults.h
 * 	Desc:	Kernel default values, toggles and tunable values.
 */

#ifndef __KERN_DEFAULTS_H__
#define __KERN_DEFAULTS_H__

#include <arch/proc_reg.h>

#define KERNEL_STATIC_DEFINE(_t)    
#define PRIVATE_STATIC_DEFINE(_t)		static _t
#define PRIVATE_STATIC_DEFINE_FUNC(_t)	static _t

#define KERNEL_GLOBAL_DEFINE(_t)		_t
#define KERNEL_EXTERN_DEFINE(_t)		extern _t

#define KERNEL_DEBUG_DEFINE(_t)			_t

/**
 * Kernel Defaults will be stored in the .rodata.defaults section, 
 * when using an ELF as the resulting binary, and __DATA_CONST.__defaults
 * when using a Tiny-O file.
 * 
 * Use this when declaring global default variables.
*/
#define KERNEL_DEFAULTS_DEFINE(_t)		_t __attribute__((section(".rodata.defaults")))


/**
 * This is the list of Default definitions. All declarations must be written in
 * the style of "DEFAULTS_<NAME>".
*/

/* Enabled / Disabled macros */
#define DEFAULTS_ENABLE			(1)
#define DEFAULTS_DISABLE		(0)

/* tBoot */
#define DEFAULTS_TBOOT_TRANSLATE_PA			DEFAULTS_ENABLE

/* Kernel */
#define DEFAULTS_KERNEL_DAIF				0xf
#define DEFAULTS_KERNEL_STACK_SIZE			16386
#define DEFAULTS_KERNEL_USE_KVA				DEFAULTS_ENABLE
#define DEFAULTS_KERNEL_BUILD_MACHINE		"tempest"

#define DEFAULTS_KERNEL_MAX_CPUS			16
#define DEFAULTS_KERNEL_MAX_CPU_CLUSTERS	4

#define DEFAULTS_KERNEL_KDEBUG_MODE			DEFAULTS_ENABLE

#define DEFAULTS_KERNEL_KDEBUG_UART_BASE	0xfffffff001000000
#define DEFAULTS_KERNEL_KDEBUG_UART_BAUD	115200
#define DEFAULTS_KERNEL_KDEBUG_UART_CLCK	0x16e3600

#define DEFAULTS_KENREL_VM_PAGE_SIZE		TT_PAGE_SIZE

/* Boot Arguments (v1.1) */
#define DEFAULTS_BA_OFFSET_VIRTBASE			8
#define DEFAULTS_BA_OFFSET_PHYSBASE			16
#define DEFAULTS_BA_OFFSET_MEMSIZE			24

/* Machine */
#define DEFAULTS_MACHINE_LIBFDT_WORKAROUND	DEFAULTS_ENABLE

/* Device Tree */
#define DEFAULTS_DEVICETREE_CELL_SIZE		2

/* Debug controls */
#define DEFAULTS_KERNEL_NO_BOOTLOADER		DEFAULTS_DISABLE

#define DEFAULTS_KERNEL_VIRTBASE    0xfffffff000000000
#define DEFAULTS_KENREL_PHYSBASE    0x08000000
#define DEFAULTS_KERNEL_MEMSIZE     0x50000000

#define DEFAULTS_KERNEL_FDT_BASE    0x41500000


#endif /* __kern_defaults_h__ */
