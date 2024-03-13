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
 * 	Name:	cpu.h
 * 	Desc:	Kernel CPU interface. Handles interacting with physical CPUs, enabling
 * 			disabling, waking, and other operations.
 */

#ifndef __KERN_CPU_H__
#define __KERN_CPU_H__

#include <kern/defaults.h>
#include <kern/vm/vm.h>

#include <libkern/types.h>
#include <tinylibc/stdint.h>


/* CPU typedefs */
typedef int			cpu_number_t;
typedef int			cpu_state_t;
typedef int			cpu_type_t;

/* Invalid CPU number*/
#define CPU_NUMBER_INVALID		(-0x1)

/* List of CPU states */
#define CPU_STATE_SLEEP			(0x0)		/* CPU sleeping, wfi loop */
#define CPU_STATE_ACTIVE		(0x1)		/* CPU actively executing a thread */
#define CPU_STATE_IDLE			(0x2)		/* CPU is on idle_thread */

/* List of CPU types */
#define CPU_TYPE_UNKNOWN			(0x0)
#define CPU_TYPE_ARM_CORTEX_A53		(0x1)
#define CPU_TYPE_ARM_CORTEX_A76		(0x2)

/** TODO: Move to interrupts header */
typedef void (*irq_handler)(unsigned int source);

/**
 * CPU structure. This can loosely be referred to as either the CPU structure,
 * or the CPU data structure. It defines the current state of a CPU.
*/
typedef struct cpu_data
{
	cpu_number_t		cpu_num;
	cpu_type_t			cpu_type;
	uint32_t			cpu_flags;

	/* Interrupt Handling */
	vm_address_t		excepstack_top;
	vm_address_t		intstack_top;

	unsigned int		interrupt_source;
	unsigned int		interrupt_state;
	irq_handler			interrupt_handler;


	/* Reset */
	vm_offset_t			cpu_reset_handler;

	/* Thread */
	//thread_t			cpu_active_thread;
	vm_address_t		cpu_active_stack;

	uint64_t			cpu_tpidr_el0;
} cpu_t;


/**
 * CPU entry points - these are found in arch/start.S.
*/
KERNEL_EXTERN_DEFINE(vm_address_t)		LowResetVector;
KERNEL_EXTERN_DEFINE(vm_address_t)		LowExceptionVectorBase;

/**
 * This is the list of CPUs active in the system. The array is allocated to the
 * maximum number of CPUs which tinyOS can support.
*/
KERNEL_EXTERN_DEFINE(cpu_t)				CpuDataEntries[DEFAULTS_KERNEL_MAX_CPUS];

/* Interface Logging */
#define cpu_log(fmt, ...)				interface_log ("cpu", fmt)

/* CPU API */
kern_return_t cpu_data_init (cpu_t *cpu_data_ptr);
kern_return_t cpu_data_register (cpu_t *cpu_data_ptr);

kern_return_t cpu_set_boot_cpu (cpu_t *cpu_data_ptr);

kern_return_t cpu_init (void);
kern_return_t cpu_halt (void);


/* Fetch the cpu_t for the current CPU */
#define CPU_GET_CURRENT()						\
	CpuDataEntries[machine_get_cpu_num()]

/* Fetch the cpu_t for a given cpu_number_t */
#define CPU_GET(_x)								\
	CpuDataEntries[_x]


#endif /* __kern_cpu_h__ */
