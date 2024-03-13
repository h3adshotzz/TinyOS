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
 * 	Name:	panic.h
 * 	Desc:	Kernel panic handler.
*/

#include <libkern/panic.h>

#include <kern/defaults.h>
#include <kern/version.h>
#include <kern/kprintf.h>
#include <kern/cpu.h>

#include <tinylibc/stddef.h>

/**
 * Name:	panic
 * Desc:	Panic the Kernel on the current thread.
*/
void
panic (const char *fmt, ...)
{
	cpu_t		cpu = CPU_GET_CURRENT ();
	va_list		args;

	kprintf ("\n---- Kernel Panic ----\n");
	kprintf ("CPU: %d, PID: %d: ", cpu.cpu_num, 0);

	va_start (args, fmt);
	vprintf (fmt, args);
	va_end (args);

	kprintf ("\n\nTinyOS Version:\n%s\n\n", KERNEL_BUILD_VERSION);
	kprintf ("Kernel Version:\ntinyOS Kernel Version %s; %s; %s:%s/%s_%s\n\n",
		KERNEL_BUILD_VERSION, __TIMESTAMP__,
		DEFAULTS_KERNEL_BUILD_MACHINE,
		KERNEL_SOURCE_VERSION, KERNEL_BUILD_STYLE, KERNEL_BUILD_TARGET);
	kprintf ("---- End Panic ----\n");

	/** TODO: Once tasks are implemented, change this to task_kill rather than
	 * an entire system abort
	 */
	cpu_halt ();
}


void panic_print (const char *func, const char *fmt, ...)
{
	va_list ap;

    kprintf ("--- Kernel Panic ---\n");

	kprintf ("KERNEL PANIC - %s:", func);

	va_start (ap, fmt);
	vprintf (fmt, ap);
	va_end (ap);
}