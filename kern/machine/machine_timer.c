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
 * 	Name:	machine/machine_timer.c
 * 	Desc:	Kernel Machine Interface - ARM Generic Virtual Timer.
 */

#include <arch/arch.h>
#include <kern/defaults.h>
#include <kern/machine.h>

#include <libkern/assert.h>
#include <libkern/types.h>

kern_return_t
machine_enable_timers ()
{
	/* Check that interrupts are still enabled */
	//assert (machine_get_interrupts_enabled() == FALSE);

	/* Register the interrupt and enable the timer */
	arm64_timer_test (0x50000);
	machine_log ("virtual timers enabled\n");
}

