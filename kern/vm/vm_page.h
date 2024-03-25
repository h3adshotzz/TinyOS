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
 * Desc:	Virtual Memory Page Allocator/Manager.
*/

#ifndef __KERN_VM_PAGE_H__
#define __KERN_VM_PAGE_H__

#include <tinylibc/stdint.h>

#include <libkern/queue.h>

#include <kern/vm/vm_types.h>
#include <kern/vm/pmap.h>
#include <kern/vm/vm.h>

#include <kern/defaults.h>

/* interface logger */
#define vm_page_log(fmt, ...)		interface_log("vm_page", fmt, ##__VA_ARGS__)

/* Page size */
#define VM_PAGE_SIZE				DEFAULTS_KENREL_VM_PAGE_SIZE

/**
 * Page flags.
*/



/**
 * The vm_page structure describes each physical page in the system, using the
 * memory attributes read from the device tree, covering all memory that is
 * addressable to the kernel and available to vm_map.
*/
typedef struct vm_page {
	queue_entry_t		pageq;

	phys_addr_t			paddr;		/* Physical address of the page */

	uint64_t			idx;		/* Page index within the pageq */
	uint64_t			flags;		/* Page flags */

} vm_page_t;


/* vm page configuration */
void vm_page_bootstrap (phys_addr_t start, phys_addr_t end);


#endif /* __kern_vm_page_h__ */