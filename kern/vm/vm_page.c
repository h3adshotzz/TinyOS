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

#include <tinylibc/string.h>
#include <kern/vm/vm_page.h>
#include <kern/vm/pmap.h>

/**
 * Page structures are stored within the kernel ".vm" segment, which is placed
 * at the end of teh kernel to allow it to grow as required. We calculate the
 * upper bounds of the region and use a cursor to track where we have written
 * up to.
*/
static vm_address_t	vm_page_region_lower_bound __attribute__((section(".vm")));
static vm_address_t vm_page_region_upper_bound;
static vm_address_t vm_page_region_cursor;

static vm_size_t 	vm_page_region_size;

/* Page region pointer */
static vm_page_t 	*vm_page_region;

/* Highest page index */
static uint64_t 	vm_page_idx;

/* fetch the page at given index */
#define __vm_page_get_idx(__idx)		((vm_page_t *) &vm_page_region[__idx])

/* increment the page region curosr */
#define __vm_page_region_cursor_inc							\
	do {													\
		vm_page_region_cursor += VM_PAGE_STRUCT_SIZE;		\
		vm_page_idx += 1;									\
	} while (0);

/*******************************************************************************
 * Name:	__vm_page_alloc_internal
 * Desc:	Internal page allocator. Creates a new 4KB vm_page_t for the given
 * 			base physical address. If the page is already mapped in the MMU, for
 * 			example Kernel code, set 'is_mapped'.
*******************************************************************************/

static int __vm_page_alloc_internal (phys_size_t paddr, int is_mapped)
{
	vm_page_t *page, *prev;

	/* check that the page region hasn't been exceeded */
	if (vm_page_region_cursor >= vm_page_region_upper_bound) {
		vm_page_log ("error: page region exeeded upper bounds\n");
		return 1;
	}

	/* get the location within the page region to create the new structure */
	page = (vm_page_t *) vm_page_region_cursor;

	/* create the new page */
	page->paddr = paddr;
	page->idx = vm_page_idx;

	page->state = VM_PAGE_STATE_FREE;
	page->mapped = (is_mapped) ? VM_PAGE_IS_MAPPED : VM_PAGE_IS_NOT_MAPPED;

	/* set next and prev pointers */
	page->next = __vm_page_get_idx(0);
	page->next->prev = page;

	page->prev = __vm_page_get_idx(page->idx - 1);
	page->prev->prev = page;

	/* set the next pointer of the previous page */
	page->prev->next = page;

	/* increment the max index and region cursor */
	__vm_page_region_cursor_inc;

	return 0;
}

/*******************************************************************************
 * Name:	vm_page_alloc
 * Desc:	Allocate a new physical memory page.
*******************************************************************************/

phys_addr_t vm_page_alloc ()
{
	vm_page_t *last;

	/* find the next free page */
	for (unsigned int i = 0; i < vm_page_idx; i++) {
		last = __vm_page_get_idx(i);
		if (last->state == VM_PAGE_STATE_FREE)
			break;
	}

	/* if the last page isn't free, something went wrong */
	if (last->state != VM_PAGE_STATE_FREE)
		panic("failed to allocate a free physical page\n");

	/* allocate the last page */
	last->state = VM_PAGE_STATE_ALLOC;

	return last->paddr;
}

/*******************************************************************************
 * Name:	vm_page_free
 * Desc:	Free a physical memory page.
*******************************************************************************/

void vm_page_free (phys_addr_t paddr)
{
	vm_page_t *page;
	uint32_t idx;

	idx = (paddr - memory_phys_base) / VM_PAGE_SIZE;
	page = __vm_page_get_idx(idx);

	page->state = VM_PAGE_STATE_FREE;

	vm_page_log("free'd page '%d': 0x%lx\n", idx, page->paddr);
}

/*******************************************************************************
 * Name:	vm_page_bootstrap
 * Desc:	Bootstrap the kernel page allocator. Configure the page table region
 * 			and create pages for provided memory region.
*******************************************************************************/

void vm_page_bootstrap (phys_addr_t membase, phys_size_t memsize, 
						phys_size_t kernsize)
{
	uint64_t page_count, kern_page_count, i;
	vm_page_t *first_page, *kern_page;
	phys_addr_t pcursor;
	phys_size_t psize;

	vm_page_log ("starting vm_page_bootstrap\n");

	/* set the initial page index */
	vm_page_idx = 0;

	/* calculate the number of pages for the physical memory size */
	psize = (phys_size_t) memsize;
	page_count = psize / VM_PAGE_SIZE;

	vm_page_region_size = (vm_size_t) page_count * sizeof (vm_page_t);

	vm_page_log ("page count: %d, size required (%dKB)\n",
		page_count, vm_page_region_size / 1024);

	/* initialise the .vm page region */
	vm_page_region_upper_bound = &vm_page_region_lower_bound + vm_page_region_size;
	vm_page_region_cursor = &vm_page_region_lower_bound;

	vm_page_region = (vm_page_t *) &vm_page_region_lower_bound;

	vm_page_log ("initialised page region: 0x%lx-0x%lx\n",
		&vm_page_region_lower_bound, vm_page_region_upper_bound);

	/**
	 * manually create the first page, so the doubly-linked list of pages can be
	 * properly setup.
	*/
	first_page = (vm_page_t *) vm_page_region_cursor;
	first_page->paddr = membase;
	first_page->idx = vm_page_idx;

	first_page->state = VM_PAGE_STATE_FREE;
	first_page->mapped = VM_PAGE_IS_NOT_MAPPED;		/* not mapped by default */

	/* configure the doubly-linked list */
	first_page->prev = (vm_page_t *) vm_page_region_cursor;
	first_page->next = first_page;

	/* move the page cursor and increment the index */
	__vm_page_region_cursor_inc;

	/* create a page struct for every physical page */
	pcursor = membase + VM_PAGE_SIZE;
	for (i = 0; i < page_count; i++) {
		if (__vm_page_alloc_internal(pcursor, 1))
			break;

		pcursor += VM_PAGE_SIZE;
	}
	vm_page_log("created %d pages (0x%lx-0x%lx)\n", i, membase, membase + memsize);

	/* verify all pages were created */
	if (i != page_count)
		panic("error: not all pages were created: %d missing\n",
			page_count - i);

	/* mark the pages used by the kernel as allocated and mapped */
	pcursor = membase;
	kern_page_count = (kernsize + vm_page_region_size) / VM_PAGE_SIZE;
	for (i = 0; i < kern_page_count; i++) {
		kern_page = __vm_page_get_idx(i);

		kern_page->state = VM_PAGE_STATE_ALLOC;
		kern_page->mapped = VM_PAGE_IS_MAPPED;

		pcursor += VM_PAGE_SIZE;
	}
	vm_page_log("modified %d kernel pages\n");

#if VM_PAGE_DEBUG_LOGGING
	/* testing: check that first->prev = last, and last->next = first */
	for (int j = 0; j < page_count; j++) {
		vm_page_t *tmp = __vm_page_get_idx(j);
		if (j == 0 || j == page_count - 1)
			vm_page_log ("tmp[%d]: tmp: 0x%llx next: 0x%llx, prev: 0x%llx, state: %d\n",
				tmp->idx, tmp, tmp->next, tmp->prev, tmp->state);
	}
#endif
}