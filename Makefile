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

Q=@


################################################################################
# Defaults
################################################################################

# Debug build
DEBUG		:=		0
BUILD_TYPE	:=		Debug

# Build Directory
BUILD_DIR	:=		build/

# Includes
INCLUDES	:=		-I.

################################################################################
# Toolchain
################################################################################

CC			:=	${CROSS_COMPILE}gcc
AS			:=	${CROSS_COMPILE}gcc 
LD			:=	${CROSS_COMPILE}ld
OC			:=	${CROSS_COMPILE}objcopy
OD			:=	${CROSS_COMPILE}objdump

ASFLAGS		:=	-nodefaultlibs -nostartfiles -nostdlib -ffreestanding	\
				-D__ASSEMBLY__											\
				${DEFINES} ${INCLUDES}

CFLAGS		:=	-nostdinc -ffreestanding -mgeneral-regs-only -mstrict-align \
				-c -O0 \
				${DEFINES} ${INCLUDES}

LDFLAGS		:=	-O1

################################################################################
# Sources and build configuration
################################################################################

# Source makefiles
include arch/arch.mk
include kern/kern.mk
include libkern/libkern.mk
include tinylibc/libc.mk
include platform/platform.mk
include drivers/drivers.mk
include libfdt/libfdt.mk

# Kernel build config
KERNEL_LINKERSCRIPT		:=	arch/linker.ld
KERNEL_MAPFILE			:=	kernel.map
KERNEL_ENTRYPOINT		:=	kernel_init

LDFLAGS					+=	-Map=${KERNEL_MAPFILE}				\
							--script ${KERNEL_LINKERSCRIPT} 	\
							--entry=${KERNEL_ENTRYPOINT}

################################################################################
# Build target
################################################################################

all:	msg_start kernel

msg_start:
	@echo "Building tinyOS Kernel"

msg_clean:
	@echo "  CLEAN"

clean_obj:
	$(Q)rm -rf *.o
	$(Q)rm -rf arch/*.o
	$(Q)rm -rf arch/*.ld
	$(Q)rm -rf kern/*.o
	$(Q)rm -rf kern/vm/*.o
	$(Q)rm -rf kern/machine/*.o
	$(Q)rm -rf platform/*.o
	$(Q)rm -rf tinylibc/*.o
	$(Q)rm -rf tinylibc/string/*.o
	$(Q)rm -rf libkern/*.o
	$(Q)rm -rf libfdt/*.o
	$(Q)rm -rf drivers/irq/*.o
	$(Q)rm -rf drivers/pl011/*.o

clean_out:
	$(Q)rm -rf *.map
	$(Q)rm -rf *.dump
	$(Q)rm -rf *.elf
	$(Q)rm -rf *.bin

clean: msg_clean clean_obj clean_out

%.o:	%.S
	@echo "  AS      $<"
	$(Q)$(AS) $(ASFLAGS) -c $< -o $@

%.o:	%.c
	@echo "  CC      $<"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

%.ld:	%.ld.S
	@echo "  LDS     $<"
	$(Q)$(AS) $(ASFLAGS) -P -E $< -o $@

kernel:	kernel_conf kernel_elf clean_obj
kernel_conf:
	@echo "  CONF    $<"

kernel_elf:	$(OBJS) $(KERNEL_SOURCES) $(KERNEL_LINKERSCRIPT)
	@echo "  LD      $@"
	$(Q)$(LD) -o $@ $(LDFLAGS) $(OBJS) $(KERNEL_SOURCES)
	@echo "  OBJCOPY $@"
	$(Q)$(OC) -O binary $@ kernel.bin
	$(Q)$(OD) -D $@ >> kernel.dump
	@echo "Built $@ successfully"
	@echo

