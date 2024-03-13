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
 * 	Name:	kprintf.h
 * 	Desc:	Kernel printf.
 */

#include <tinylibc/stdint.h>
#include <tinylibc/string.h>

#include <kern/kprintf.h>
#include <drivers/pl011/pl011.h>

/* printf formatting */
#define POPT_LONG		(1 << 1)
#define POPT_LONGLONG	(1 << 2)
#define POPT_PADRIGHT	(1 << 3)

/* Log colours */
#define LOG_COLOUR_YELLOW          "\x1b[38;5;214m"
#define LOG_COLOUR_RED             "\x1b[38;5;88m"
#define LOG_COLOUR_BLUE            "\x1b[38;5;32m"
#define LOG_COLOUR_GREEN           "\x1b[32m"
#define LOG_COLOUR_RESET           "\x1b[0m"

static const char *interfaces[1] =
{
	"machine",
	NULL,
};

static int internal_printf (const char *fmt, va_list ap);

/**
 * This is a special log function so we don't have to write the name of the
 * interface in every message
*/
int interface_log (const char *interface, const char *fmt, ...)
{
	va_list		args;
	int			res;

	kprintf ("%s: ", interface);
	va_start (args, fmt);
	res = internal_printf (fmt, args);
	va_end (args);

	return res;
}


int kprintf (const char *fmt, ...)
{
	va_list args;
	int res;

	va_start (args, fmt);
	vprintf (fmt, args);
	va_end (args);

	return 0;
}

int vprintf (const char *fmt, va_list ap)
{
	return internal_printf (fmt, ap);
}


static void output_stdout (char c)
{
	// update this once the serial interface is implemented.
	if (c != '\0')
		pl011_putc (c);
}

static char *llstr (char *buf, unsigned long long n, int len)
{
	int digit;
	int pos = len;
	int neg = 0;

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
	int pos = len;
	unsigned int d;
	static const char hextable[] = { '0', '1', '2', '3', '4', '5', '6', '7', 
									 '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

	buf[--pos] = 0;
	do {
		d = u % 16;
		u /= 16;
		buf[--pos] = hextable[d];
	} while (u != 0);
	return &buf[pos];
}

static int internal_printf (const char *fmt, va_list ap)
{
	unsigned long long n;
	const char *str;
	uint32_t opts;
	int width = 0, len = 0;
	char buf[64];
	char c;

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
