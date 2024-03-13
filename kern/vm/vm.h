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
 * 	Name:	vm.h
 * 	Desc:	Primary Kernel Virtual Memory interface header.
 */

#ifndef __KERN_VM_H__
#define __KERN_VM_H__

#include <tinylibc/stdint.h>

#include <libkern/assert.h>
#include <libkern/boot.h>

#include <arch/proc_reg.h>

/*******************************************************************************
 * Virtual Memory (VM) Interface
*******************************************************************************/

/* vm types */
typedef uint64_t		vm_address_t;	// virt
typedef uint64_t		address_t;		// phys

typedef uintptr_t		vm_offset_t;
typedef uint64_t		vm_addr_t;
typedef uint64_t		vm_size_t;

typedef int				vm_return_t;
typedef int				vm_prot_t;

/* vm protection (move to vm_prot.h) */
#define VM_PROT_NONE		((vm_prot_t) 0x0)

#define VM_PROT_READ		((vm_prot_t) 0x1)
#define VM_PROT_WRITE		((vm_prot_t) 0x2)
#define VM_PROT_EXECUTE		((vm_prot_t) 0x3)

/* vm operation return types */
enum {
	VM_SUCCESS,
	VM_FAILURE,
};

/* vm map types */
typedef enum {
	VM_MAP_TYPE_DEVICE = 1,
	VM_MAP_TYPE_KERNEL,
	VM_MAP_TYPE_NORMAL,
} vm_map_type_t;

/* vm map standard names */
#define VM_MAP_NAME_CPU_PERIPHERALS		"vm_map_cpu_peripherals"
#define VM_MAP_NAME_UART				"vm_map_uart"
#define VM_MAP_NAME_MMIO				"vm_map_mmio"
#define VM_MAP_NAME_KERNEL				"vm_map_kernel"


/**
 * 
*/
typedef struct {
	const char		*name;
	vm_addr_t		virt_base;
	vm_addr_t		phys_base;
	vm_size_t		size;
	vm_map_type_t	type;
} vm_map_t;


/*******************************************************************************
 * Translation Table Entry (TTE) Interface
*******************************************************************************/

typedef uint64_t		tt_page_t;
typedef uint64_t		tt_entry_t;
typedef uint64_t		tt_table_t;

typedef struct
{


} translation_table_t;


#endif /* __kern_vm_h__ */
