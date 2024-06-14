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
 *	Name:	cpu.h
 *	Desc:	Kernel CPU management interface. Handles interactions with physical
 *			CPUs, enable/disable, wakeup, and other operations.
 *
 */

#ifndef __KERN_CPU_H_
#define __KERN_CPU_H__

#include <kern/defaults.h>
#include <kern/vm/vm_types.h>

#include <libkern/types.h>
#include <tinylibc/stdint.h>


/* CPU typedefs */
typedef int			cpu_number_t;
typedef int			cpu_state_t;
typedef int			cpu_type_t;

/* Invalid CPU number */
#define CPU_NUMBER_INVALID		(-1)

/* List of possible CPU states */
#define CPU_STATE_SLEEP			UL(0x0)		/* CPU sleeping (wfi loop) */
#define CPU_STATE_ACTIVE		UL(0x1)		/* CPU active */
#define CPU_STATE_IDLE			UL(0x2)		/* CPU idle (idle_thread) */

/** TOOD: Move to interrupt handler header */
typedef void (*irq_handler_t) (unsigned int source);

/**
 * CPU Data
 *
 * This can be referred to as either the "CPU structure" or "CPU Data structure",
 * it defines the current state of a CPU in the system.
 */
typedef struct cpu_data
{
	cpu_number_t		cpu_num;
	cpu_type_t			cpu_type;
	uint32_t			cpu_flags;

	/* Interrupt handling */
	vm_address_t		excepstack_top;
	vm_address_t		intstack_top;

	unsigned int		interrupt_source;
	unsigned int		interrupt_state;
	irq_handler_t		interrupt_handler;

	/* Reset */
	vm_address_t		cpu_reset_handler;

	/* Thread */
	//thread_t			cpu_active_thread;
	vm_address_t		cpu_active_stack;

	uint64_t			cpu_tpidr_el0;

} cpu_t;

/**
 * External definitions for start.S functions: Low reset and exception vectors
 */
extern vm_address_t			_LowResetVector;
extern vm_address_t			_LowExceptionVectorBase;

/* Interface logging */
#define cpu_log(fmt, ...)	interface_log("cpu", fmt)

/* CPU API */
kern_return_t 	cpu_data_init (cpu_t *cpu_data_ptr);
kern_return_t 	cpu_data_register (cpu_t *cpu_data_ptr);
kern_return_t 	cpu_set_boot_cpu (cpu_t *cpu_data_ptr);

cpu_t			cpu_get_current ();
cpu_t			cpu_get_id (unsigned int id);

kern_return_t	cpu_init (void);
void			cpu_halt (void);


#endif /* __kern_cpu_h__ */


