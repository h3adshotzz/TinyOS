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
 * Name:	vm_page.c
 * Desc:	Virtual Memory Page Allocator/Manager.
*/

#include <tinylibc/string.h>
#include <kern/vm/vm_page.h>

/**
 * Pages are stored within the vm_page_region, within the ".vm" section of the
 * kernel ELF binary. This section is at the end of the kernel binary so can be
 * sized as required for the number of pages.
 * 
 * We calculate the upper bounds of the region, and use the cursor to track where
 * the region has been written up to.
*/
static vm_address_t		vm_page_region_lower_bound __attribute__((section(".vm")));
static vm_address_t		vm_page_region_upper_bound;
static vm_address_t		vm_page_region_cursor;

/* Pointer to page region */
static vm_page_t		*vm_page_region;

/* Page region size */
static vm_size_t		vm_page_region_size;

/* Highest page index */
static uint64_t			vm_page_idx;


/**
 * Macro:	__vm_page_region_get_idx
 * Desc:	Fetch the vm_page_t for the given page index.
*/
#define __vm_page_region_get_idx(__x)										\
	((vm_page_t *) &vm_page_region[__x])

/**
 * Macro:	__vm_page_region_first
 * Desc: 	Fetch the first page in the page region.
*/
#define __vm_page_region_first()											\
	__vm_page_region_get_idx(0)

/**
 * Macro:	__vm_page_region_last
 * Desc:	Fetch the last page in the page region.
*/
#define __vm_page_region_last()												\
	__vm_page_region_get_idx(vm_page_idx)


static inline void
__vm_page_region_inc ()
{
	vm_page_region_cursor += VM_PAGE_STRUCT_SIZE;
	vm_page_idx += 1;
}

/**
 * Internal page initialiser. Creates a vm_page_t at a given physical address and
 * adds it to the page free queue.
*/
static int
__vm_page_create (phys_addr_t paddr)
{
	vm_page_t	*page, *prev;

	/* check that the page region hasn't been exceeded */
	if (vm_page_region_cursor >= vm_page_region_upper_bound) {
		vm_page_log ("error: page region exceeded upper bounds\n");
		return 1;
	}

	/* get the location within the page region to create the new structure */
	page = (vm_page_t *) vm_page_region_cursor;

	/* create the new page */
	page->paddr = paddr;
	page->flags = (VM_PAGE_FLAG_FREE | VM_PAGE_FLAG_ALLOC_USER);
	page->idx = vm_page_idx;

	/* set next and previous elements */
	page->prev = __vm_page_region_get_idx (page->idx - 1);
	page->prev->next = page;

	page->next = __vm_page_region_first ();
	page->next->prev = page;

	/* set the next pointer of the previous page */
	page->prev->next = page;

	/**
	 * an issue exists where without this print, the compiler does not generate
	 * the instructions at the top and bottom of this function for the stack. It
	 * also doesn't generate the 'ret' for the bottom either.
	*/
	kprintf ("");

	/* increment the max index and region cursor */
	__vm_page_region_inc ();

	return 0;
}

/**
 * Name:	vm_page_bootstrap
 * Desc:	Bootstrap the kernel page allocator, and configure the page region
 * 			for already in-use memory. 
*/
void
vm_page_bootstrap (phys_addr_t mem_base, phys_size_t mem_size, phys_size_t kern_size)
{
	phys_addr_t	pcursor;
	vm_size_t	psize;
	uint64_t	page_count, kern_page_count, i;
	vm_page_t	*first_page, *kern_page;

	vm_page_log ("starting vm_page_bootstrap\n");

	/* set the initial page index */
	vm_page_idx = 0;

	/* calculate the actual physical memory size, and total number of pages */
	psize = (vm_size_t) mem_size;
	page_count = psize / VM_PAGE_SIZE;
	
	vm_page_region_size = page_count * sizeof (vm_page_t);

	vm_page_log ("page count: %d\n", page_count);
	vm_page_log ("size required: %d bytes, %d kilobytes\n",
		vm_page_region_size, vm_page_region_size / 1024);

	/* initialise the page region */
	vm_page_region_upper_bound = &vm_page_region_lower_bound + vm_page_region_size;
	vm_page_region_cursor = &vm_page_region_lower_bound;

	vm_page_region = (vm_page_t *) &vm_page_region_lower_bound;

	vm_page_log ("initialised page region: 0x%llx-0x%llx\n",
		&vm_page_region_lower_bound, vm_page_region_upper_bound);

	/**
	 * we must manually create the first page, so as to properly setup the doubly
	 * linked list.
	*/
	first_page = (vm_page_t *) vm_page_region_cursor;
	first_page->paddr = mem_base;
	first_page->flags = (VM_PAGE_FLAG_FREE | VM_PAGE_FLAG_ALLOC_USER);
	first_page->idx = 0;

	/* configure the doubly-linked list */
	first_page->prev = (vm_page_t *) vm_page_region_cursor;
	first_page->next = first_page;

	/* move the cursor and increment the index */
	__vm_page_region_inc ();

	/* create a page struct for every physical page, covering (start -> end) */
	pcursor = mem_base + VM_PAGE_SIZE;
	for (i = vm_page_idx; i < page_count; i++) {
		if (__vm_page_create (pcursor) != 0)
			break;
		pcursor += VM_PAGE_SIZE;
	}
	vm_page_log ("created %d pages (0x%llx-0x%llx\n", i, mem_base, mem_size);

	/* verify that all pages were created */
	if (i != page_count) {
		vm_page_log ("error: not all pages were created: %d missing\n", page_count - i);
		/* panic */
		return;
	}

	/**
	 * we need to mark the kernel pages as VM_PAGE_FLAG_ALLOC_KERNEL. Cycle again
	 * through each page from kernel start -> kernel end
	*/
	pcursor = mem_base;
	kern_page_count = (kern_size + vm_page_region_size) / VM_PAGE_SIZE;
	for (i = 0; i < kern_page_count; i++) {
		kern_page = __vm_page_region_get_idx (i);
		kern_page->flags = (VM_PAGE_FLAG_ALLOC | VM_PAGE_FLAG_ALLOC_KERNEL);

		pcursor += VM_PAGE_SIZE;

		/* again, this werid issues. if this isn't here, this code fails */
		kprintf ("");
	}
	vm_page_log ("marked %d pages as VM_PAGE_FLAG_ALLOC_KERNEL\n", i);


	/* testing: check that first->prev = last, and last->next = first */
	for (int a = 0; a < page_count; a++) {
		vm_page_t *tmp = __vm_page_region_get_idx (a);
		if (a == 0 || a == page_count-1)
			vm_page_log ("tmp[%d]: tmp: 0x%llx next: 0x%llx, prev: 0x%llx\n",
				tmp->idx, tmp, tmp->next, tmp->prev);
	}


//	vm_page_t *page, *first;
//	page = first = __vm_page_region_first();
//	
//	do {
//		vm_page_log ("page[%d]: 0x%llx: %d\n", page->idx, page->paddr, page->flags);
//		page = page->next;
//	} while (page->next != first);
//	vm_page_log ("page: 0x%llx, next: 0x%llx, first: 0x%llx\n", page, page->next, first);

	return;
}