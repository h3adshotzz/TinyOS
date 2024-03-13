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
 * 	Name:	kdebug.h
 * 	Desc:	Kernel debug interface.
 */

#include <kern/defaults.h>
#include <kern/kdebug.h>

/******************************************************************************
 * Kernel Early Debug Logging.
 ******************************************************************************/

#include <drivers/pl011/pl011.h>

/**
 * Setup early debug logging.
 * 
 * Directly call the uart driver initialisation function with hard-coded values
 * so when building DEBUG kernels, we can have basic logging until serial is
 * properly initialised using the values in the device tree.
 * 
 * NOTE:    Ensure this makes the right driver call, depending on the platform.
*/
void kernel_debug_early_log_init ()
{
	pl011_init (DEFAULTS_KERNEL_KDEBUG_UART_BASE,
				DEFAULTS_KERNEL_KDEBUG_UART_BAUD,
				DEFAULTS_KERNEL_KDEBUG_UART_CLCK);
}

void kernel_debug_early_log_init_addr (uint64_t base)
{
//	pl011_init (base,
//				DEFAULTS_KERNEL_KDEBUG_UART_BAUD,
//				DEFAULTS_KERNEL_KDEBUG_UART_CLCK);
//	pl011_base_reset (base);
}

/**
 * Write a single character string to the console.
*/
void kernel_debug_early_log (const char *message)
{
	pl011_puts (message);
}