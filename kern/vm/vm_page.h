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
 * Name:	vm_page.h
 * Desc:	Kernel Virtual Memory physical page manager.
*/

#ifndef __KERN_VM_PAGE_H__
#define __KERN_VM_PAGE_H__

#include <tinylibc/stdint.h>

#include <kern/vm/vm_types.h>
#include <kern/vm/pmap.h>
#include <kern/vm/vm.h>

#include <kern/defaults.h>

/* interface logger */
#define vm_page_log(fmt, ...)		interface_log("vm_page", fmt, ##__VA_ARGS__)

/* Debugging */
#define VM_PAGE_DEBUG_LOGGING		DEFAULTS_ENABLE

/* Page size */
#define VM_PAGE_SIZE				DEFAULTS_KERNEL_VM_PAGE_SIZE
#define VM_PAGE_STRUCT_SIZE			sizeof(vm_page_t)

/**
 * Virtual Memory Physical Page
*/
typedef struct vm_page				vm_page_t;
struct vm_page {

	/* Physical memory address of the page */
	phys_addr_t		paddr;

	/* Next and previous pages */
	vm_page_t		*next;
	vm_page_t		*prev;

	/* Page index */
	uint64_t		idx;

	/* Page flags */
	unsigned int
		/* current page alloc state */
					state:1,
#define VM_PAGE_STATE_ALLOC		UL(0x1)
#define VM_PAGE_STATE_FREE		UL(0x0)

		/* current page mapping state */
					mapped:1,
#define VM_PAGE_IS_MAPPED		UL(0x1)
#define VM_PAGE_IS_NOT_MAPPED	UL(0x0)

		/* unused bits */
					__unused_bits:30;

};

/* initialise pages */
extern void vm_page_bootstrap (phys_addr_t membase, 
			phys_size_t memsize, phys_size_t kernsize);

extern phys_addr_t vm_page_alloc ();


#endif /* __kern_vm_page_h__ */