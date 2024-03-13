#===----------------------------------------------------------------------===//
#
#                                  tinyOS
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#  Copyright (C) 2023, Harry Moulton <me@h3adsh0tzz.com>
#
#===----------------------------------------------------------------------===//

cmake_minimum_required(VERSION 3.10)

############################## CONFIGURATION ###################################

# CMake System Configuration
set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)
set(CMAKE_BUILD_TYPE                Debug)
set(CMAKE_TRY_COMPILE_TARGET_TYPE   STATIC_LIBRARY)

########################## Compiler configuration ##############################

# Project Setup
project("tinyKern" VERSION 0.0.1 LANGUAGES C ASM)

# Compiler Setup
set(TOOLCHAIN "/Users/h3/Work/toolchains/arm-gnu-toolchain-12.2.rel1-darwin-arm64-aarch64-none-elf/bin")
set(CMAKE_C_COMPILER    ${TOOLCHAIN}/aarch64-none-elf-gcc)
set(CMAKE_ASM_COMPILER  ${TOOLCHAIN}/aarch64-none-elf-gcc)
set(CMAKE_LINKER        ${TOOLCHAIN}/aarch64-none-elf-ld)
set(CMAKE_OBJCOPY       ${TOOLCHAIN}/aarch64-none-elf-objcopy)

set(CMAKE_SKIP_SIMPLE_TESTS     TRUE)
set(CMAKE_C_COMPILER_WORKS      1)
set(CMAKE_LINKER_WORKS          1)

set(KERN_LINKER_SCRIPT     ${CMAKE_SOURCE_DIR}/kern/linker.lds)


########################### Target configuration ###############################

add_executable(tinyKern)

# Version generator
include (config/version.cmake)
add_dependencies(tinyKern generate_version)

target_compile_options(tinyKern PRIVATE
    -mgeneral-regs-only
    -march=armv8-a
    -pie
    -O1
    -ffreestanding
    -nostdlib
    -nostartfiles
)
target_link_options(tinyKern PRIVATE
    -nodefaultlibs
    -nostartfiles
    -nostdlib
    -ffreestanding
    -Iinclude
    -T ${KERN_LINKER_SCRIPT}
)

target_include_directories(tinyKern PRIVATE .)

target_sources(tinyKern PRIVATE
    # ARM Architecture code
    arch/start.S
    arch/data.S
    arch/helpers.S
    arch/exception.S
    arch/exception.c
    arch/timer.S

    # Kernel Core
    kern/cpu.c
    kern/main.c
    kern/kdebug.c
    kern/kprintf.c

    # Kernel Core - Machine Interface
    kern/machine.c
    kern/machine/machine_irq.c
    kern/machine/machine_timer.c

    # Kernel Core - Virtual Memory Interface
    kern/vm/vm.c
    kern/vm/vm_walk.c

    # Platform API
    platform/devicetree.c
    platform/platform.c

    # Kernel Library
    libkern/panic.c

    # Drivers
    drivers/pl011/pl011.c
    drivers/irqs/irq-gicv3.c

    # Tinylibc
    tinylibc/string/memchr.c
    tinylibc/string/memcmp.c
    tinylibc/string/memcpy.c
    tinylibc/string/memmove.c
    tinylibc/string/memset.c
    tinylibc/string/strchr.c
    tinylibc/string/strcmp.c
    tinylibc/string/strlcpy.c
    tinylibc/string/strlen.c
    tinylibc/string/strncmp.c
    tinylibc/string/strnlen.c
    tinylibc/string/strrchr.c
    tinylibc/string/strtoul.c

    # Libfdt
    libfdt/fdt_addresses.c
    libfdt/fdt_empty_tree.c
    libfdt/fdt_overlay.c
    libfdt/fdt_ro.c
    libfdt/fdt_rw.c
    libfdt/fdt_strerror.c
    libfdt/fdt_sw.c
    libfdt/fdt_wip.c
    libfdt/fdt.c
)

add_custom_command(TARGET tinyKern POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_BINARY_DIR}/tinyKern
                                        ${CMAKE_BINARY_DIR}/kernel.arm64)

