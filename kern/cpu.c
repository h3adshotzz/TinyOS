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
 * 	Desc:	Kernel CPU interface.
 */

#include <kern/defaults.h>
#include <kern/startup.h>
#include <kern/cpu.h>

#include <kern/vm/pmap.h>

KERNEL_GLOBAL_DEFINE(cpu_t)			CpuDataEntries[DEFAULTS_KERNEL_MAX_CPUS];

/* Boot CPU */
PRIVATE_STATIC_DEFINE(cpu_t)		BootCpuData;


kern_return_t
cpu_data_init (cpu_t *cpu_data_ptr)
{
	cpu_data_ptr->cpu_num = 0;
	cpu_data_ptr->cpu_flags = 0;
	cpu_data_ptr->cpu_type = 0;

	cpu_data_ptr->cpu_reset_handler = 0x0;

	return KERN_RETURN_SUCCESS;
}

kern_return_t
cpu_data_register (cpu_t *cpu_data_ptr)
{
	int cpu_num = cpu_data_ptr->cpu_num;

	CpuDataEntries[cpu_num] = *cpu_data_ptr;
	return KERN_RETURN_SUCCESS;
}

kern_return_t
cpu_set_boot_cpu (cpu_t *cpu_data_ptr)
{
	BootCpuData = *(cpu_t *) cpu_data_ptr;
	return KERN_RETURN_SUCCESS;
}

kern_return_t
cpu_init (void)
{
	cpu_t cpu_data;

	cpu_data = CPU_GET_CURRENT ();

//	if (cpu_data.cpu_num == BootCpuData.cpu_num)
//		kprintf ("boot cpu\n");

	cpu_data.cpu_reset_handler = (vm_address_t) mmu_translate_kvtop (&_LowResetVector);
//	kprintf ("cpu.cpu_reset_handler: 0x%llx\n", cpu_data.cpu_reset_handler);

	// TODO: CPU Feature info
	//
}

kern_return_t
cpu_halt (void)
{
	/* put the core to sleep */
	__asm__ __volatile__ ("b .");
}