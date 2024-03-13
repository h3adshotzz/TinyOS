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

#ifndef __STRING_H__
#define __STRING_H__

#include <tinylibc/_types.h>
#include <tinylibc/stdint.h>

/* string */
extern size_t   strlcpy     (char *dest, const char *src, size_t size);
extern size_t   strlen      (const char *str);
extern int      strcmp      (const char *cs, const char *ct);
extern int      strncmp     (const char *s1, const char *s2, size_t n);
extern size_t   strnlen     (const char *s, size_t count);
extern char *   strrchr     (const char *s, int c);
extern char *   strchr      (const char *s, int c);
extern unsigned long 
                strtoul     (const char *nptr, char **endptr, register int base);

/* memory */
extern void *   memcpy      (void *dest, const void *src, size_t count);
extern void *   memset      (void *s, int c, size_t count);
extern void *   memchr      (const void *s, int c, size_t n);
extern void *   memmove     (void *dest, const void *src, size_t count);
extern int      memcmp      (const void *cs, const void *ct, size_t count);


#endif /* __string_h__ */
