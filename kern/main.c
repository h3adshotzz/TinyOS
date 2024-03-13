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
 * 	Name:	main.c
 * 	Desc:	Kernel startup/bootstrap code. Initial entry point from Assembly
 * 			bootstrap.
*/

/* tinylibc */
#include <tinylibc/stdint.h>
#include <tinylibc/byteswap.h>

/* kernel */
#include <kern/defaults.h>
#include <kern/kprintf.h>
#include <kern/version.h>
#include <kern/kdebug.h>
#include <kern/cpu.h>

/* machine */
#include <kern/machine/machine_irq.h>

/* libkern */
#include <libkern/assert.h>
#include <libkern/boot.h>

/* platform */
#include <platform/devicetree.h>

/* arch */
#include <arch/proc_reg.h>

/* stacks */
extern vm_offset_t intstack_top;
extern vm_offset_t excepstack_top;

static void print_boot_banner ();

/**
 * Name:	kernel_init
 * Desc:	Initial Kernel entry point in C code. At this point, we are still on
 * 			bootstrap pagetables, tasking and other kernel functionality is not
 * 			yet enabled, and secondary CPUs are still in an "sleep state".
*/
void
kernel_init (struct boot_args *args,
			 uintptr_t x1)
{
	vm_address_t membase, fdtvirt;
	const DTNode *dt_root;
	const char *machine;
	cpu_t boot_cpu_data;
	vm_size_t memsize;
	int len;

	/* initialise the boot cpu_data structure */
	cpu_data_init (&boot_cpu_data);
	boot_cpu_data.intstack_top = (vm_offset_t) &intstack_top;
	boot_cpu_data.excepstack_top = (vm_offset_t) &excepstack_top;

	/* enable early kernel logging */
	kernel_debug_early_log_init ();
	kernel_debug_early_log ("\n");

#if DEFAULTS_KERNEL_NO_BOOTLOADER

	kprintf ("NOTICE: KERNEL IN NO-BOOTLOADER MODE. VIRTUAL MEMORY, DEVICE TREE AND MACHINE INTERFACE ARE DISABLED\n");
	kprintf ("BOOTING TINYOS ON PHYS_CPU: 0x%08llx [0x%llx]\n",
		boot_cpu_data.cpu_num, kernel_init);
	kprintf ("KERNEL: tinyOS Kernel Version %s; %s; %s:%s/%s_%s\n\n\n",
		KERNEL_BUILD_VERSION, __TIMESTAMP__,
		DEFAULTS_KERNEL_BUILD_MACHINE,
		KERNEL_SOURCE_VERSION, KERNEL_BUILD_STYLE, KERNEL_BUILD_TARGET);
#else
	/* verify boot arguments */
	if (args->version != BOOT_ARGS_VERSION_1_1)
		kprintf ("error: mismtached bootargs struct\n");

	/* convert the device tree base to KVA if it isn't already */
	if (args->fdtbase < args->virtbase)
		fdtvirt = (void *) ((uintptr_t) args->virtbase + (args->fdtbase - args->physbase));

	/* initialise the device tree handler */
	DeviceTreeInit (fdtvirt, args->fdtsize);
	dt_root = BootDeviceTreeGetRootNode ();

	/* parse the cpu topology */
	machine_parse_cpu_topology ();

	boot_cpu_data.cpu_num = machine_get_boot_cpu_num ();
	assert (boot_cpu_data.cpu_num <= machine_get_max_cpu_num ());

	/* register the boot cpu within the cpu_data array */
	cpu_data_register (&boot_cpu_data);
	cpu_set_boot_cpu (&boot_cpu_data);

	// TODO: serial_init() - properly discover and setup the serial interface

	/* boot cpu initialisation */
	cpu_init ();

	/* boot banner */
	print_boot_banner (boot_cpu_data.cpu_num, args->tboot_vers);

	/* output the machine configuration */
	DeviceTreeLookupPropertyValue (*dt_root, "compatible", &machine, &len);
	kprintf ("Machine: %s\n", machine);
	kprintf ("Machine: detected '%d' cpus across '%d' clusters\n",
		machine_get_num_cpus (), machine_get_num_clusters ());

	/* init kernel page tables */
	platform_get_memory (&memsize, &membase);
	arm_vm_init (args, membase, memsize);

	// we are no longer using the bootstrap pagetables from this point onwards

	/**
	 * Initialise the interrupt controller. The machine interface will handle
	 * reading the device tree to discover which interrupt controller is being
	 * used, and then whatever addresses are associated with it.
	 * 
	 * TOOD: 	we also need to map the virtual memory region. At the moment we
	 * 			are directly accessing physical memory. 
	*/
	machine_init_interrupts ();


#endif

////////////////////////////////////////////////////////////////////////////////
// DEBUG AREA - EVERYTHING HERE IS SHIT AND DOESN'T WORK :-(
//

	int test = 8;
	machine_register_interrupt (test, 0);
	machine_send_interrupt (test, 1);

	/* test the timer */
	machine_register_interrupt (30, 0);
	//machine_enable_timers ();


	// TODO: cpu_smp_init() - bring up secondary cpus
	// PSCI??

	kprintf ("--KERNEL_SETUP_COMPLETE--\n");

///////////////////////////////////////////////////////////////////////////////
	/* kernel_init shouldn't reach here, only if switching to kernel_task fails */
	kprintf ("\n\n__halt\n");
//	asm ("brk #1");
	asm ("b .");
}

static void print_boot_banner (cpu_number_t cpu_num, const char *tboot_vers)
{
	kprintf ("Booting TinyOS on Physical CPU: 0x%08llx [0x%llx]\n",
		cpu_num, kernel_init);
	kprintf ("tinyOS Kernel Version %s; %s; %s:%s/%s_%s\n",
		KERNEL_BUILD_VERSION, __TIMESTAMP__,
		DEFAULTS_KERNEL_BUILD_MACHINE,
		KERNEL_SOURCE_VERSION, KERNEL_BUILD_STYLE, KERNEL_BUILD_TARGET);
	kprintf ("tBoot version: %s\n", tboot_vers);
}
