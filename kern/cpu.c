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
 *	Name:	cpu.c
 *	Desc:	Kernel CPU management interface. Handles interactions with physical
 *			CPUs, enable/disable, wakeup and other operations.
 */

#include <kern/defaults.h>
#include <kern/machine.h>
#include <kern/vm/pmap.h>

/**
 * List of active CPUs. This array is allocated to the maximum number of allowed
 * CPUs (DEFAULTS_MACHINE_MAX_CPUS), and is updated as each CPU becomes active.
 */
static cpu_t	CpuDataEntries[DEFAULTS_MACHINE_MAX_CPUS];
static cpu_t	BootCpuData;


cpu_t cpu_get_current()
{
	return (cpu_t) CpuDataEntries[machine_get_cpu_num()];
}

cpu_t cpu_get_id (unsigned int id)
{
	return (cpu_t) CpuDataEntries[id];
}

kern_return_t cpu_data_init (cpu_t *cpu_data_ptr)
{
	cpu_data_ptr->cpu_num = 0;
	cpu_data_ptr->cpu_flags = 0;
	cpu_data_ptr->cpu_type = 0;

	cpu_data_ptr->cpu_reset_handler = 0x0;

	return KERN_RETURN_SUCCESS;
}

kern_return_t cpu_data_register (cpu_t *cpu_data_ptr)
{
	int cpu_num;

	cpu_num = cpu_data_ptr->cpu_num;
	CpuDataEntries[cpu_num] = *cpu_data_ptr;

	return KERN_RETURN_SUCCESS;
}

kern_return_t cpu_set_boot_cpu (cpu_t *cpu_data_ptr)
{
	BootCpuData = *(cpu_t *) cpu_data_ptr;
	return KERN_RETURN_SUCCESS;
}

kern_return_t cpu_init (void)
{
	cpu_t cpu_data;

	cpu_data.cpu_reset_handler = (vm_address_t) 
		mmu_translate_kvtop ((vm_address_t) &_LowResetVector);
	return KERN_RETURN_SUCCESS;
}

void cpu_halt (void)
{
	/* put the core to sleep */
	__asm__ volatile ("b .");
}
