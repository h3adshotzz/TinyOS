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
 * 	Name:	boot.h
 * 	Desc:	Boot arguments.
 */

#ifndef __KERN_BOOT_H__
#define __KERN_BOOT_H__

#include <tinylibc/stddef.h>
#include <tinylibc/stdint.h>

#define BOOT_ARGS_VERSION_1_1		0x11

/**
 * Kernel Boot Arguments structure
 * 
 * This is constructed by tBoot and placed into an area of shared, non-secure
 * memory. The pointer is then passed to the Kernel in register x0. The boot
 * arguments struct holds the initial memory information about the system for
 * bootstrap, as well as the location of the device tree and the version of
 * tBoot which loaded it.
*/
struct boot_args
{
    uint32_t    version;
    
    /* virtual memory properties */
    uint64_t    virtbase;
    uint64_t    physbase;
    uint64_t    memsize;

    /* kernel region */
    uint64_t    kernbase;
    uint64_t    kernsize;

    /* device tree */
    uint64_t    fdtbase;
    uint64_t    fdtsize;

    /* uart */
    uint64_t    uartbase;
    uint64_t    uartsize;

    /* misc */
    uint32_t    flags;
    char        tboot_vers[32];
};



#endif /* __KERN_BOOT_H__ */