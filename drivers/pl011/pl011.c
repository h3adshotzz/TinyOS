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
 * 	Name:	pl011.h
 * 	Desc:	Arm PL011 Serial driver.
 */

#include <drivers/pl011/pl011.h>

static uint64_t pl011_base;
static uint64_t pl011_baud;
static uint64_t pl011_clock;

int pl011_putc (const char c)
{
	struct pl011_regs *regs = (struct pl011_regs *) ((uintptr_t) pl011_base);

	/* wait for the data buffer to be full */
	while (mmio_read (&regs->fr) & 0x20) {}
	mmio_write (&regs->dr, c);
	return 0;
}

int pl011_puts (const char *s)
{
	struct pl011_regs *regs = (struct pl011_regs *) ((uintptr_t) pl011_base);

	while (*s) {
		pl011_putc (*s);
		s++;
	}
	return 0;
}

int pl011_getc ()
{
	struct pl011_regs *regs = (struct pl011_regs *) ((uintptr_t) pl011_base);

	while (mmio_read (&regs->fr) & 0x10) {}
	return mmio_read (&regs->dr);
}

int pl011_init (uint64_t base, uint64_t baud, uint64_t clock)
{
	struct pl011_regs *regs;
	unsigned int divider;

	// base values
	pl011_base = base;
	pl011_baud = baud;
	pl011_clock = clock;

	regs = (struct pl011_regs *) ((uintptr_t) pl011_base);
	mmio_write (&regs->cr, 0);

	divider = (pl011_clock * 4) / pl011_baud;
	mmio_write (&regs->ibrd, divider >> 6);
	mmio_write (&regs->fbrd, divider & 0x3f);

	mmio_write (&regs->lcr_h, SERIAL_PL011_LCRH_WLEN_8);
	mmio_write (&regs->imsc, (SERIAL_PL011_IMSC_RXIM | SERIAL_PL011_IMSC_RTIM));
	mmio_write (&regs->cr, (SERIAL_PL011_CR_UARTEN | SERIAL_PL011_CR_TXE | 
						    SERIAL_PL011_CR_RXE));

	return 0;
}
