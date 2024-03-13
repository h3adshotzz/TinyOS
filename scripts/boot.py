##===----------------------------------------------------------------------===//
##
##                                  tinyOS
##
##  This program is free software: you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with this program.  If not, see <http://www.gnu.org/licenses/>.
##
##	Copyright (C) 2023, Harry Moulton <me@h3adsh0tzz.com>
##
##===----------------------------------------------------------------------===//

from dataclasses import dataclass
from enum import Enum
import argparse
import os

# Supported TinyPlatforms
class TinyPlatform(Enum):
    TINY_PLATFORM_ID_EX1 = "ex1"

# Supported ARM Cores
class ARMCoreType(Enum):
    ARM_CORE_NEOVERSE_N1 = "neoverse-n1"

# Platform Properties
@dataclass
class Platform:
    platid: TinyPlatform
    dtree: str
    core: ARMCoreType
    clusters: int
    cpus: int
    mem: int

################################################################################
# List of Platform Properties

# Experimental Test Platform
# Tiny EX-1: 4 Neoverse-N1 Cores, 2 Clusters.
TINY_PLATFORM_EX1 = Platform(
    platid = TinyPlatform.TINY_PLATFORM_ID_EX1,
    dtree = "tiny-ex1.dtsi",
    core = ARMCoreType.ARM_CORE_NEOVERSE_N1,
    clusters = 1,
    cpus = 4,
    mem = 512,
)

################################################################################

# Defaults, can be overriden if certain options are set.
DEFAULTS_LOAD_ADDR_TINYROM  = 0x00000000
DEFAULTS_LOAD_ADDR_TBOOT    = 0x48000000
DEFAULTS_LOAD_ADDR_KERNEL   = 0x48500000

################################################################################
# Constructing QEMU run commands

class QemuTraceLevel(Enum):
    QEMU_TRACE_LEVEL_0 = 0
    QEMU_TRACE_LEVEL_1 = 1
    QEMU_TRACE_LEVEL_2 = 2

@dataclass
class QemuFirmware:
    path: str
    load_addr: int

@dataclass
class QemuOptions:
    firmware: list
    bios: str
    tree: str
    trace_level: QemuTraceLevel

# Additional Defaults for Qemu
DEFAULTS_QEMU_OPTIONS = [
    "-machine secure=true,virtualization=on,gic-version=3",
    "-serial stdio",
    "-d int -s"
]

#
#
#
def qemu_construct_run_command(plat: Platform, qemu_opts: QemuOptions):
    qemu_command = ["qemu-system-aarch64", "-M virt"]

    # Set Qemu CPU configuration
    qemu_command.append("-cpu \"{}\"".format(plat.core.value))
    qemu_command.append("-smp {}".format(plat.cpus))
    qemu_command.append("-m {}M".format(plat.mem))
    
    # Set Qemu default options
    for opt in DEFAULTS_QEMU_OPTIONS:
        qemu_command.append(opt)

    # Set Qemu configurable options=
    qemu_command.append("-bios {}".format(qemu_opts.bios))
    for firmware in qemu_opts.firmware:
        qemu_command.append("-device loader,file={},addr={}".format(firmware.path, firmware.load_addr))

    if qemu_opts.tree:
        qemu_command.append("-dtb {}".format(qemu_opts.tree))

    qemu_command = " ".join(map(str, qemu_command))
    return qemu_command

################################################################################

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument("-f", "--firmware", nargs="*", help="List of firmware files and load addresses, seperated by commas")
    parser.add_argument("-b", "--bios", action="store", help="BIOS image")
    parser.add_argument("-t", "--tree", action="store", help="Device tree")
    args = parser.parse_args()

    # Create the Qemu options
    opts = QemuOptions([], None, None, QemuTraceLevel.QEMU_TRACE_LEVEL_1)

    # Create a list of firmware options to pass to Qemu
    if "firmware" in args:
        firmware_list = []
        for firmware in args.firmware:
            firmware = firmware.split(",")
            opts.firmware.append(QemuFirmware(firmware[0], firmware[1]))

    if "bios" in args:
        opts.bios = args.bios

    if "tree" in args:
        opts.tree = args.tree

    # Run TinyOS
    print ("\n---TinyOS Developer Tools---\n")
    print ("ROM:        {}".format(opts.bios))
    print ("Bootloader: {} @ {}".format(opts.firmware[0].path, opts.firmware[0].load_addr))
    print ("Kernel:     {} @ {}".format(opts.firmware[1].path, opts.firmware[1].load_addr))
    print ("DeviceTree: {}".format(opts.tree))
    print ("\n")
    print ("--- Starting Qemu ---\n")

    cmd = qemu_construct_run_command(TINY_PLATFORM_EX1, opts)
    os.system(cmd)
