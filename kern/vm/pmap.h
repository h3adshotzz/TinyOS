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
 * 	Name:	pmap.h
 * 	Desc:	Physical Memory Mapping interface header. This is used for managing
 * 			pagetables, translation tables, physical memory and interactions
 * 			with the MMU and it's system registers.
 * 
 * 			The virtual memory interface is built on-top of pmap. When a vm_map_t
 * 			is created, it's created on a pmap_t. There are, for now, two pmap_t
 * 			that are created - one for the kernel and another for userspace.
 * 
 * 			...
*/

#ifndef __KERN_VM_PMAP_H__
#define __KERN_VM_PMAP_H__

#include <tinylibc/stdint.h>
#include <kern/vm/vm.h>

/* interface logger */
#define pmap_log(fmt, ...)		interface_log ("pmap", fmt, ##__VA_ARGS__)

/* pmap operation result */
typedef int				pmap_return_t;

/* pmap operation result codes */
#define PMAP_RETURN_SUCCESS		0			/* success */
#define PMAP_RETURN_FAILED		1			/* generic failed */
#define PMAP_RETURN_ILLEGAL		2			/* illegal operation */
#define PMAP_RETURN_INVALID		3			/* invalid address */

/* Translation Table Entry types */
typedef uint64_t		tt_table_t;			/* Translation table */
typedef uint64_t		tt_page_t;			/* Translation table page */
typedef uint64_t		tt_entry_t;			/* Translation table page entry */

typedef uint64_t		pmap_addr_t;		/* Physical memory address */

/**
 * MMU helpers. These are external declarations of assembly functions. As there
 * are two Translation Table Base Registers (TTBRn_EL1) for the Kernel to use,
 * we'll refer to them as 'tt_base' and 'tt_base_alt', where 'tt_base' is TTBR0,
 * and 'tt_base_alt' is TTBR1,
*/
extern void mmu_set_tcr (uint64_t);					/* not-implemented */
extern void mmu_set_tt_base (pmap_addr_t);			/* not-implemented */
extern void mmu_set_tt_base_alt (pmap_addr_t);

extern uint64_t mmu_get_tcr ();						/* not-implemented */
extern pmap_addr_t mmu_get_tt_base ();				/* not-implemented */
extern pmap_addr_t mmu_get_tt_base_alt ();			/* not-implemented */

/* Use the MMU to translate virtual addresses */
extern pmap_addr_t mmu_translate_kvtop ();

/**
 * Physical Memory Mapping structure.
 * 
 * The pmap_t structure represents the state of a physical memory region that
 * can be configured to one of the Translation Table Base Register (TTBRn_EL1).
 *
 * Virtual memory allocations (vm_map_t) are each linked to a particular pmap_t.
 *
*/
typedef struct pmap {
	tt_page_t		*tte;		/* Root translation table entry */
	pmap_addr_t		ttep;		/* Translation table physical address */
	pmap_addr_t		base;		/* Physical memory region base address */
	vm_size_t		size;		/* physical memory region size */

} pmap_t;

/* Maximum number of physical maps */
#define PMAP_LIST_MAX		2

/* Globals for kernel pagetables shared between the pmap and vm interfaces */
extern pmap_addr_t 	pagetables_region_base;		/* Bootstrap pagetables */
extern pmap_addr_t 	pagetables_region_end;
extern pmap_addr_t	pagetables_region_cursor;

extern tt_table_t	*kernel_tte;				/* Core Kernel pagetables */
extern pmap_addr_t	kernel_ttep;				/* Physical address of kernel_tte */

/* pagetable region management */
extern pmap_return_t pmap_ptregion_create ();
extern vm_address_t pmap_ptregion_alloc ();

/* translation table management */
extern pmap_return_t pmap_create_tte (tt_table_t *, pmap_addr_t, vm_address_t, vm_size_t);

#endif /* __kern_vm_pmap_h__ */