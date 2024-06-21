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
 * Name:	main.c
 * Desc:	Kernel startup code. Initial entry point from assembly, execution
 * 			continues here until virtual memory and tasking is setup, and then
 * 			we jump to the kernel_task.
 */

/* tinylibc */
#include <tinylibc/stdint.h>
#include <tinylibc/byteswap.h>

/* libkern */
#include <libkern/boot.h>
#include <libkern/assert.h>
#include <libkern/version.h>
#include <libkern/panic.h>

/* kernel */
#include <kern/machine.h>
#include <kern/machine/machine-irq.h>
#include <kern/vm/vm.h>
#include <kern/vm/pmap.h>
#include <kern/task.h>

/* platform */
#include <platform/devicetree.h>
#include <platform/platform.h>


/**
 * Interrupt and Exception stack pointers
 */
extern vm_address_t intstack_top;
extern vm_address_t excepstack_top;

extern vm_address_t kernel_task_stack;

static task_t *test_task_ptr;

/* statics */
void print_boot_banner ();

void kernel_task_test();

/**
 * The kernel will enter here from start.S and will complete the necessary setup
 * until the kernel_task can be launched, at which point the .startup section
 * will be erased from memory and unampped.
*/
void kernel_init (struct boot_args *boot_args, uint64_t x1, uint64_t x2)
{
	uint64_t x0;
	const DTNode *dt_root;
	phys_addr_t membase;
	phys_size_t memsize;
	cpu_t boot_cpu;

	/* initialise the cpu_data for the boot cpu */
	cpu_data_init (&boot_cpu);
	boot_cpu.intstack_top = (vm_address_t) &intstack_top;
	boot_cpu.excepstack_top = (vm_address_t) &excepstack_top;

	/* verify the boot parameters */
	if (boot_args->version != BOOT_ARGS_VERSION_1_1)
		panic ("boot_args version mismatch\n");

	/* convert the fdt base to a virtual address */
	if (boot_args->fdtbase < boot_args->virtbase)
		boot_args->fdtbase = (uint64_t) (boot_args->virtbase + 
			(boot_args->fdtbase - boot_args->physbase));

	/* initialise the device tree */
	DeviceTreeInit ((void *) boot_args->fdtbase, boot_args->fdtsize);
	dt_root = BootDeviceTreeGetRootNode ();

	/* update the address of the boot args struct */
	x0 = (uint64_t) boot_args->virtbase + (((uint64_t) boot_args) - ((uint64_t) boot_args->physbase)); //(boot_args->virtbase + (x0 - boot_args->physbase));
	boot_args = (struct boot_args *) x0;

	/* fetch platform memory layout and setup virtual memory */
	platform_get_memory (&membase, &memsize);
	arm_vm_init (boot_args, membase, memsize);

	/* initialise the console and enable kprintf */
	kprintf_init ();

	/* now we have logs, verify the device tree */
	DeviceTreeVerify ();

	/* parse the machine cpu topology */
	machine_parse_cpu_topology ();
	boot_cpu.cpu_num = machine_get_boot_cpu_num ();
	assert (boot_cpu.cpu_num <= machine_get_max_cpu_num ());

	cpu_data_register (&boot_cpu);
	cpu_set_boot_cpu (&boot_cpu);

	/* cpu initialisation */
	cpu_init ();

	/* boot banner */
	print_boot_banner (dt_root, boot_cpu.cpu_num, boot_args->tboot_vers);

	// debugging
	kprintf ("DEBUG x0: 0x%lx, x1: 0x%lx, x2: 0x%lx\n", x0, x1, x2);

	/* configure remaining virtual memory subsystems */
	vm_configure ();

	/* configure the interrupt controller */
	machine_init_interrupts ();

	/* create the kernel task */
	task_init();
	kernel_task_create (kernel_task, kernel_task_test, vm_get_kernel_map (), 
		((vm_address_t) kernel_task + sizeof(task_t)));
	kprintf ("kernel_task: 0x%lx\n", kernel_task);

	kprintf("minimal kernel startup complete\n");

	task_t test_task;
	uint64_t res = __fork64_switch(kernel_task, kernel_task);

	arm64_cpu_context_t context = kernel_task->context;

	kprintf_hexdump((&context), 0x0, sizeof(arm64_cpu_context_t));
	kprintf("x19: 0x%016lx x20: 0x%016lx x21: 0x%016lx x22: 0x%016lx\n"
			"x23: 0x%016lx x24: 0x%016lx x25: 0x%016lx x26: 0x%016lx\n",
			context.x19, context.x20, context.x21,
			context.x22, context.x23, context.x24,
			context.x25, context.x26);
	kprintf ("x27: 0x%016lx x28: 0x%016lx fp:  0x%016lx sp:  0x%016lx\n"
			"pc:  0x%016lx\n",
			context.x27, context.x28, context.fp, context.sp,
			context.pc);

	kprintf("__fork64_switch: 0x%016lx\n", res);

	kprintf ("kernel_task_test: 0x%lx\n", kernel_task_test);

	test_task_ptr = ptokva(&test_task);

	test_task.context = kernel_task->context;
	test_task.context.pc = kernel_task_test;
	__fork64_switch(kernel_task, ptokva(&test_task));

	//machine_register_interrupt (4, 0);
	//machine_send_interrupt (4, 1);
	//machine_send_interrupt (4, 1);
	//machine_send_interrupt (4, 1);

	kprintf("kernel_init complete\n");

	//__asm__ volatile ("brk #1");
	__asm__ volatile ("b .");
}

void print_boot_banner (const DTNode *dt_root, cpu_number_t cpu_num, 
	const char *tboot_vers)
{
	char *machine;
	int len;

	kprintf ("Booting TinyOS on Physical CPU: 0x%08llx [0x%llx]\n",
		cpu_num, kernel_init);
	kprintf ("tinyOS Kernel Version %s; %s; %s:%s/%s_%s\n",
		KERNEL_BUILD_VERSION, __TIMESTAMP__,
		DEFAULTS_KERNEL_BUILD_MACHINE,
		KERNEL_SOURCE_VERSION, KERNEL_BUILD_STYLE, KERNEL_BUILD_TARGET);
	kprintf ("tBoot version: %s\n", tboot_vers);

	DeviceTreeLookupPropertyValue (*dt_root, "compatible", &machine, &len);
	kprintf ("machine: %s\n", machine);
	kprintf ("machine: detected '%d' cpus across '%d' clusters\n",
		machine_get_num_cpus (), machine_get_num_clusters ());
}

void kernel_task_test ()
{
	kprintf ("\t\t==== hello from kernel task ====\n");
	__fork64_switch(test_task_ptr, kernel_task);
}