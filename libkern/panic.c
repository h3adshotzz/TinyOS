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

#include <libkern/panic.h>

#include <kern/defaults.h>
#include <kern/version.h>
#include <kern/kprintf.h>
#include <kern/cpu.h>

#include <tinylibc/stddef.h>

void panic (const char *fmt, ...)
{
	va_list args;
	cpu_t cpu;
	int pid;

	/* get the current cpu */
	cpu = cpu_get_id (0);	// cpu_get_current();
	pid = 0;

	kprintf ("\n---- Kernel Panic ----\n");
	kprintf ("CPU: %d, PID: %d: ", cpu.cpu_num, pid);

	va_start (args, fmt);
	vprintf (fmt, args);
	va_end (args);

	kprintf ("\n\ntinyOS Version:\n%s\n\n", KERNEL_BUILD_VERSION);
	kprintf ("Kernel Version:\ntinyOS Kernel Version %s; %s; %s:%s/%s_%s\n\n",
		KERNEL_BUILD_VERSION, __TIMESTAMP__,
		DEFAULTS_KERNEL_BUILD_MACHINE,
		KERNEL_SOURCE_VERSION, KERNEL_BUILD_STYLE, KERNEL_BUILD_TARGET);
	kprintf ("---- End Panic ----\n");

	/* TODO: change this to task_kill, rather than cpu_halt() */
	cpu_halt ();
}
