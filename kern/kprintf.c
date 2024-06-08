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

#include <tinylibc/stdint.h>
#include <tinylibc/string.h>

#include <kern/kprintf.h>
#include <kern/defaults.h>
#include <drivers/pl011/pl011.h>

/* printf formatting */
#define POPT_LONG		(1 << 1)
#define POPT_LONGLONG	(1 << 2)
#define POPT_PADRIGHT	(1 << 3)

static int console_initialised = 0;
static int internal_kprintf (const char *fmt, va_list ap);

/**
 * kprintf is an early logging interface for the kernel. Eventually we'll have
 * some kind of console driver, stdout, etc. For now, the kernel just calls
 * kprintf_init() and can then start logging.
 */
void kprintf_init ()
{
	pl011_init (DEFAULTS_KERNEL_VM_PERIPH_BASE,
				DEFAULTS_KERNEL_DEBUG_UART_BAUD,
				DEFAULTS_KERNEL_DEBUG_UART_CLK);
	pl011_puts ("\n");

	console_initialised = 1;
}

/******************************************************************************
 * Interface logger
 *
 * Each Kernel component/interface has it's own logger macro, so logs can be
 * properly prefixed, e.g. vm.c logs are prefixed "vm: ". In the header, create
 * the macro calling interface_log, passing the desired prefix.
 *
 *****************************************************************************/
int interface_log (const char *interface, const char *fmt, ...)
{
	va_list	args;
	int	res;

	kprintf ("%s: ", interface);
	va_start (args, fmt);
	res = internal_kprintf (fmt, args);
	va_end (args);

	return res;
}

/*******************************************************************************
 * Hexdump
 *
 * Print a hexdump of 'size' bytes from a desired memory address.
 *
 ******************************************************************************/

void kprintf_hexdump (char *mem, uint64_t base, uint64_t size)
{
		uint32_t offset;
	int ws, lines, count, pos;

	offset = base;
	lines = size / 16;

	if (size % 16)
		lines += 1;

	pos = 0;
	for (int i = 0; i < lines; i++) {
		uint8_t ln[16];

		count = 16;
		if ((size - (i * 16)) / 16 == 0)
			count = size % 16;

		kprintf ("%08x  ", offset);
		for (int j = 0; j < count; j++) {
			uint8_t byte = (uint8_t) mem[pos];
			kprintf ("%02x ", byte);

			pos++;
			ln[j] = byte;
		}

		for (int k = 0, ws = 16 - count; k < ws; k++)
			kprintf ("    ");

		kprintf (" |");
		for (int l = 0; l < 16; l++) {
			if (ln[l] < 0x20 || ln[l] > 0x7e)
				kprintf (".");
			else
				kprintf ("%c", (char) ln[l]);
		}

		kprintf ("|\n");
		offset += 0x10;
	}
	kprintf ("\n");
}

/*******************************************************************************
 * printf API
 *
 ******************************************************************************/

int kprintf (const char *fmt, ...)
{
	va_list args;
	int res;

	va_start (args, fmt);
	vprintf (fmt, args);
	va_end (args);

	return 0;
}

int sprintf (char *buffer, const char *fmt, ...)
{

}

int snprintf (char *buffer, size_t size, const char *fmt, ...)
{

}

int vprintf (const char *fmt, va_list ap)
{
	return internal_kprintf (fmt, ap);
}

/*******************************************************************************
 * printf backend
 *
 ******************************************************************************/

static void output_stdout (char c)
{
	// TODO: this will be updated when the console driver is implemented.
	if (c != '\0')
		pl011_putc (c);
}

static char *llstr (char *buf, unsigned long long n, int len)
{
	int digit, pos = len, neg = 0;

	buf[--pos] = 0;
	while (n >= 10) {
		digit = n % 10;
		n /= 10;
		buf[--pos] = digit + '0';
	}
	buf[--pos] = n + '0';
	return &buf[pos];
}

static char *llhex (char *buf, unsigned long long u, int len)
{
	static const char hextable[] = { '0', '1', '2', '3', '4', '5', '6', '7',
									 '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	unsigned int d;
	int pos = len;

	buf[--pos] = 0;
	do {
		d = u % 16;
		u /= 16;
		buf[--pos] = hextable[d];
	} while (u != 0);
	return &buf[pos];
}

static int internal_kprintf (const char *fmt, va_list ap)
{
	unsigned long long n;
	const char *str;
	uint32_t opts;
	int width = 0, len = 0;
	char buf[64];
	char c;

	if (!console_initialised)
		return -1;

	for (;;) {
		/**
		 *  Output normal characters over serial, but break if the formatter
		 *  '%' is found.
		 */
		while ((c = *fmt++) != 0) {
			if (c == '%')
			break;
			output_stdout (c);
		}

		/* check that the character is not NULL */
		if (c == 0) 
			break;

next:
		/* next character */
		c = *fmt++;
		if (c == 0)
			break;

		switch (c) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				width = (width * 10) + c - '0';
				goto next;

			case '%':
				output_stdout ('%');
				break;

			case '-':
				opts |= POPT_PADRIGHT;
				goto next;

			case 'c':
				c = va_arg (ap, unsigned int);
				output_stdout (c);
				break;

			case 'l':
				if (opts & POPT_LONG)
					opts |= POPT_LONGLONG;
				opts |= POPT_LONG;
				goto next;

			case 'i':
			case 'd':
				n = va_arg (ap, unsigned int);
				str = llstr (buf, n, sizeof (buf));
				goto output;

			case 'p':
			case 'x':
				n = va_arg (ap, long long);

				str = llhex (buf, n, sizeof (buf));
				goto output;

			case 's':
				str = va_arg (ap, const char *);
				goto output;
		}

		continue;

output:
		width -= strlen (str);
		if (!(opts & POPT_PADRIGHT))
			while (width-- > 0)
				output_stdout ('0');

		while (*str != 0)
			output_stdout (*str++);

		if (opts & POPT_PADRIGHT)
			while (width-- > 0)
				output_stdout (' ');

		width = 0;
		continue;
	}
	return width;
}
