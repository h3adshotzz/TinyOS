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

#include <kern/defaults.h>

ENTRY(_start)
SECTIONS
{
	. = DEFAULTS_KERNEL_VM_VIRT_BASE;

	.text : {
		*(.text .text.* .gnu.linkonce.t*)
	}

	.rodata : {
		KEEP(*(.rodata.defaults))
		*(.rodata .rodata.* .gnu.linkonce.r*)
	}

	PROVIDE(_data = .);
	.data : {
		*(.data .data.* .gnu.linkonce.d*)
	}

	.bss(NOLOAD) : {
		. = ALIGN(16);
		__bss_start = .;
		*(.bss .bss.*)
		*(COMMON)
		__bss_end = .;
	}

	.vm(NOLOAD) : {
		. = ALIGN(16);
		__vm_start = .;
		*(.vm .vm.*)
		*(COMMON)
		__vm_end = .;
	 }

	_end = .;

	/DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) *(.igot*) *(.iplt*) *(.rela*) }
}
