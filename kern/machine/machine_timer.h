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
 * 	Name:	machine/machine_gic.c
 * 	Desc:	Kernel Machine Interface.
 */

#ifndef __MACHINE_TIMER_H__
#define __MACHINE_TIMER_H__

#include <tinylibc/limits.h>
#include <tinylibc/stdint.h>

#include <libkern/types.h>

/**
 * Interrupt IDs for ARM generic timers
*/
#define MACHINE_TIMER_EL1PHYS_IRQ_ID		30
#define MACHINE_TIMER_EL1VIRT_IRQ_ID		30

#define MACHINE_TIMER_RESET_VALUE			0x5000000

/**
 * Machine timer API
*/
extern kern_return_t machine_init_timers ();
extern kern_return_t machine_timer_reset(uint64_t reset);

#endif /* __machine_timer_h__ */