#===-----------------------------------------------------------------------===//
#
#                                  tinyOS
#
# 	This program is free software: you can redistribute it and/or modify
# 	it under the terms of the GNU General Public License as published by
# 	the Free Software Foundation, either version 3 of the License, or
# 	(at your option) any later version.
#
# 	This program is distributed in the hope that it will be useful,
# 	but WITHOUT ANY WARRANTY; without even the implied warranty of
# 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# 	GNU General Public License for more details.
#
# 	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#	Copyright (C) 2023-2024, Harry Moulton <me@h3adsh0tzz.com>
#
#===-----------------------------------------------------------------------===//

KERNEL_SOURCES	+=	tinylibc/string/memchr.o	\
					tinylibc/string/memcmp.o	\
					tinylibc/string/memcpy.o	\
					tinylibc/string/memmove.o	\
					tinylibc/string/memset.o	\
					tinylibc/string/strchr.o	\
					tinylibc/string/strcmp.o	\
					tinylibc/string/strlcpy.o	\
					tinylibc/string/strlen.o	\
					tinylibc/string/strncmp.o	\
					tinylibc/string/strnlen.o	\
					tinylibc/string/strrchr.o	\
					tinylibc/string/strtoul.o
