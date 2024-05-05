//===----------------------------------------------------------------------===//
//
//                                  tinyOS
//
// 	This program is free software: you can redistribute it and/or modify
// 	it under the terms of the GNU General Public License as published by
// 	the Free Software Foundation, either version 3 of the License, or
// 	(at your option) any later version.
//
// 	This program is distributed in the hope that it will be useful,
// 	but WITHOUT ANY WARRANTY; without even the implied warranty of
// 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// 	GNU General Public License for more details.
//
// 	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//	Copyright (C) 2023-2024, Harry Moulton <me@h3adsh0tzz.com>
//
//===----------------------------------------------------------------------===//

SECTIONS
{
	. = 0x0;

	.text : {
		__text_start = .;
		*(.text, .text.*)
		__text_end = .;
	}

	.rodata : {
		__rodata_start = .;
		*(.rodata, .rodata.*)
		__rodata_end = .;
	}

	PROVIDE(_data = .);
	.data : {
		__data_start = .;
		*(.data, .data.*)
		__data_end = .;
	}

	.bss(NOLOAD) : {
		. = ALIGN(16);
		__bss_start = .;
		*(.bss, .bss.*)
		*(COMMON)
		__bss_end = .;
	}

	.vm(NOLOAD) : {
		. = ALIGN(16);
		__vm_start = .;
		*(.vm, .vm.*)
		*(COMMON)
		__vm_end = .;
	}

	__end = .;

	/DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) *(.igot*) *(.iplt*) *(.rela*) }
}
