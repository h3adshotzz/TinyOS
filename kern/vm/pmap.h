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
 *	Name:	pmap.h
 *	Desc:	Physical Memory Mapping header. This is used for managing physical
 *			pagetables, translation tables, physical memory and interactions
 *			between the kernel and the MMU.
 *
 *			The virtual memory system will be built ontop of pmap.
 *
 */

#ifndef __KERN_VM_PMAP_H__
#define __KERN_VM_PMAP_H__

#include <tinylibc/stdint.h>

#include <kern/vm/vm_types.h>
#include <kern/vm/vm.h>

/* interface logger */
#define pmap_log(fmt, ...)		interface_log("pmap", fmt, ##__VA_ARGS__)

/* pmap operation results */
typedef int				pmap_return_t;

#define PMAP_RETURN_SUCCESS		UL(0)	/* success */
#define PMAP_RETURN_FAILED		UL(1)	/* generic failed */
#define PMAP_RETURN_ILLEGAL		UL(2)	/* illegal operation */
#define PMAP_RETURN_INVALID		UL(3)	/* invalid address */

/* Translation table entry types */
typedef uint64_t		tt_table_t;		/* translation table */
typedef uint64_t		tt_page_t;		/* translation table page */
typedef uint64_t		tt_entry_t;		/* translation table page entry */

typedef uint64_t		phys_addr_t;	/* physical address */
typedef uint64_t		phys_size_t;	/* physical size */


/**
 * MMU helpers. These are external declarations of assembly function. There are
 * two Translation Table Base Registers (TTBRn_EL1) for the kernel to use, so
 * we will refer to them as 'tt_base' and 'tt_base_alt', where 'tt_base' uses
 * TTBR0, and 'tt_base_alt' uses TTBR1.
 */
extern void mmu_set_tcr (uint64_t);
extern void mmu_set_tt_base (phys_addr_t);
extern void mmu_set_tt_base_alt (phys_addr_t);

extern uint64_t mmu_get_tcr ();
extern phys_addr_t mmu_get_tt_base ();
extern phys_addr_t mmu_get_tt_base_alt ();

/* Use the MMU to translate virtual addresses */
extern phys_addr_t mmu_translate_kvtop (vm_address_t);

/* Convert translation table entry addresses */
#define ptokva(__p)		((vm_address_t)(__p) - memory_phys_base + memory_virt_base)

/**
 * Kernel pmap structure
 *
 * A pmap is used to represent the state of physical memory for a process/task,
 * holding the pointer to the translation tables, address space identifier and
 * the minimum/maximum virtual address allowed for this pmap.
 *
 * Virtual memory mappings (vm_map) are linked to a pmap. It's possible for a
 * single pmap to have more than one vm_map. When context switching, the pmap
 * is used to restore the TTBRn_EL1 register.
 *
 * The task_t structure will point to the pmap.
 */
typedef struct pmap {
	tt_page_t		*tte;		/* translation table pointer */
	phys_addr_t		ttep;		/* translation table physical address */

	vm_address_t	min;		/* smallest virtual address for this region */
	vm_address_t	max;		/* largest virtual address for this region */

	uint8_t			asid;		/* address space identifier */

	/* more to add */
} pmap_t;

/* Maximum number of pmaps */
#define PMAP_LIST_MAX		UL(2)

/* Memory bases */
extern vm_address_t		memory_virt_base;
extern vm_address_t		memory_phys_base;

/* Globals for kernel pagetables shared between pmap and vm interfaces */
extern phys_addr_t		pagetables_region_base;
extern phys_addr_t		pagetables_region_end;
extern phys_addr_t		pagetables_region_cursor;

extern tt_table_t		*kernel_tte;	/* kernel pagetables */
extern phys_addr_t		kernel_ttep;

extern tt_table_t		*invalid_tte;	/* invalid pagetables */
extern phys_addr_t		invalid_ttep;

/* pagetable region management */
extern pmap_return_t	pmap_ptregion_create ();
extern vm_address_t		pmap_ptregion_alloc ();

/* translation table management */
extern pmap_return_t	pmap_tt_create_tte (tt_table_t *, phys_addr_t, 
											vm_address_t, vm_size_t);
extern pmap_return_t	pmap_map_page (pmap_t *, phys_addr_t);

/* pmap */
extern int				pmap_create_kernel_pmap (pmap_t *kernel_pmap);


#endif /* __kern_vm_pmap_h__ */
