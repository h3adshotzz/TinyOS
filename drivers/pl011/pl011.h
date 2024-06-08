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

#ifndef __DRIVER_PL011_H__
#define __DRIVER_PL011_H__

#include <tinylibc/stdint.h>

/* memory-mapped io read/write */
#define mmio_write(register, val)		*(volatile uint32_t *) register = val
#define mmio_read(register)				*(volatile uint32_t *) register

/* pl011 mmio frame */
struct pl011_regs
{
	uint32_t	dr;			/* Data Register (UARTDR)*/
	uint32_t	ecr;		/* Error Clear Register (UARTECR/UARTRSR)*/

	/* 0x008-0x0 reserved (pl010) */
	uint32_t	rs1, rs2, rs3, rs4;

	/* remaining */
	uint32_t	fr;			/* Flag Register (UARTTFR) */
	uint32_t	rs5;		/* Reserved (pl010) */
	uint32_t	ilpr;		/* IrDA Low-Power Counter Register (UARTILPR) */
	uint32_t	ibrd;		/* Integer Baud Rate Register (UARTIBRD) */
	uint32_t	fbrd;		/* Fractioanl Baud Rate Register (UARTFBRD) */
	uint32_t	lcr_h;		/* Line Control Register (UARTLCR_H) */
	uint32_t	cr;			/* Control Register (UARTCR) */
	uint32_t	ifls;		/* Interrupt FIFO Level Select (UARTIFLS) */
	uint32_t	imsc;		/* Interrupt Mask Set/Clear (UARTIMSC) */
};

/* APIs */
int pl011_init (uint64_t base, uint64_t baud, uint64_t clock);
int pl011_putc (const char c);
int pl011_puts (const char *s);
int pl011_getc ();

/* Control Register (UARTCR) bits */
#define SERIAL_PL011_CR_CTSEN			(1 << 15)
#define SERIAL_PL011_CR_RTSEN			(1 << 14)
#define SERIAL_PL011_CR_OUT2			(1 << 13)
#define SERIAL_PL011_CR_OUT1			(1 << 12)
#define SERIAL_PL011_CR_RTS				(1 << 11)
#define SERIAL_PL011_CR_DTR				(1 << 10)
#define SERIAL_PL011_CR_RXE				(1 << 9)
#define SERIAL_PL011_CR_TXE				(1 << 8)
#define SERIAL_PL011_CR_LPE				(1 << 7)
#define SERIAL_PL011_CR_IIRLP			(1 << 2)
#define SERIAL_PL011_CR_SIREN			(1 << 1)
#define SERIAL_PL011_CR_UARTEN			(1 << 0)

/* Line Control Register (LCR_H) bits */
#define SERIAL_PL011_LCRH_SPS			(1 << 7)
#define SERIAL_PL011_LCRH_WLEN_8		(3 << 5)
#define SERIAL_PL011_LCRH_WLEN_7		(2 << 5)
#define SERIAL_PL011_LCRH_WLEN_6		(1 << 5)
#define SERIAL_PL011_LCRH_WLEN_5		(0 << 5)
#define SERIAL_PL011_LCRH_FEN			(1 << 4)
#define SERIAL_PL011_LCRH_STP2			(1 << 3)
#define SERIAL_PL011_LCRH_EPS			(1 << 2)
#define SERIAL_PL011_LCRH_PEN			(1 << 1)
#define SERIAL_PL011_LCRH_BRK			(1 << 0)

/* Interrupt Mask Set/Clear Register (UARTIMSC) */
#define SERIAL_PL011_IMSC_OEIM			(1 << 10)
#define SERIAL_PL011_IMSC_BEIM			(1 << 9)
#define SERIAL_PL011_IMSC_PEIM			(1 << 8)
#define SERIAL_PL011_IMSC_FEIM			(1 << 7)
#define SERIAL_PL011_IMSC_RTIM			(1 << 6)
#define SERIAL_PL011_IMSC_TXIM			(1 << 5)
#define SERIAL_PL011_IMSC_RXIM			(1 << 4)
#define SERIAL_PL011_IMSC_DSRMIM		(1 << 3)
#define SERIAL_PL011_IMSC_DCDMIM		(1 << 2)
#define SERIAL_PL011_IMSC_CTSMIM		(1 << 1)
#define SERIAL_PL011_IMSC_RIMIM			(1 << 0)

#endif /* __driver_pl011_h__ */