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
#include <arch/proc_reg.h>

#include <libkern/assert.h>
#include <libkern/boot.h>

/* interface logger */
#define vm_log(fmt, ...)		interface_log ("vm", fmt, ##__VA_ARGS__)

/* Virtual Memory types */
typedef uint64_t		vm_address_t;		/* Virtual memory address */
typedef uint64_t		vm_offset_t;		/* Virtual memory offset */
typedef uint64_t		vm_size_t;			/* Virtual memory size */

typedef int				vm_map_type_t;		/* Virtual memory region type */
typedef int				vm_prot_t;			/* Protection properties */

/* Kernel virtual memory area bounds */
#define VM_KERNEL_MIN_ADDRESS		((vm_address_t) 0xffffffe000000000ULL)
#define VM_KERNEL_MAX_ADDRESS		((vm_address_t) 0xfffffff3ffffffffULL)

/* Protection types */
#define VM_PROT_NONE				((vm_prot_t) 0x0)
#define VM_PROT_READ				((vm_prot_t) 0x1)
#define VM_PROT_WRITE				((vm_prot_t) 0x2)
#define VM_PROT_EXECUTE				((vm_prot_t) 0x3)

/* Mapping types */
#define VM_MAP_TYPE_INVALID			((vm_map_type_t) 0x0)
#define VM_MAP_TYPE_KERNEL			((vm_map_type_t) 0x1)
#define VM_MAP_TYPE_DEVICE			((vm_map_type_t) 0x2)
#define VM_MAP_TYPE_USER			((vm_map_type_t) 0x3)

/**
 * Virtual Memory Map structure
 * 
 * ...
*/
typedef struct vm_map {

} vm_map_t;


/* Virtual memory system initialisation */
extern void arm_vm_init (struct boot_args *args, vm_address_t membase, vm_size_t memsize);

#endif /* __kern_vm_h__ */
