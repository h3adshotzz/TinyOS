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
#include <tinylibc/byteswap.h>

#include <kern/vm/vm.h>

#include <libkern/types.h>

#include <platform/devicetree.h>

#include <libfdt/libfdt.h>

#define swap_test(x) \
    ((((x) & 0xff000000) >> 24) | \
    (((x) & 0x00ff0000) >>  8) | \
    (((x) & 0x0000ff00) <<  8) | \
    (((x) & 0x000000ff) << 24))

/* read the memory size from the device tree */
kern_return_t platform_get_memory (vm_size_t *memsize, vm_offset_t *membase)
{
	uint64_t addr, size;
	DTNode mem_node;
	int res;

	/**
	 * NOTE: This is due to an annoying thing with Qemu. It seems to overwrite
	 * the /memory node we set in the device tree, i.e. dts/tiny-ex1.dtsi. So,
	 * instead of using /memory, we'll use /dram and /sram.
	*/
	res = DeviceTreeLookupNode ("/memory", &mem_node);
	assert (res == kDeviceTreeSuccess);

	DeviceTreeLookupRegValue (&mem_node, &addr, &size);
	kprintf ("platform_get_memsize: addr: 0x%llx, size: 0x%llx\n", addr, size);

	/* currently spoofing the memory size at 512MB */
	*memsize = size;
	*membase = addr;
}

#include <kern/defaults.h>

/*  */
kern_return_t platform_get_gicv3 ()
{
	DTNode gic_node;
	uint32_t *cells, *reg;
	int res;

	res = DeviceTreeLookupNode ("/intc", &gic_node);
	assert (res == kDeviceTreeSuccess);

	int cell_size;


	reg = fdt_getprop (Debug_DeviceTreeGetBootDeviceTreeBase (), gic_node.offset, "reg", &res);
	if (res < 0) {
		kprintf ("DeviceTreeLookupRegValue: ERROR: failed to get prop 'reg' from node '%s': 0x%llx\n",
			gic_node.name, res);
		return kDeviceTreeFailure;
	}

	/**
	 * The cell size is fixed and defined in defaults.h. For now, until this is
	 * changed, there is no need to discover the cell size.
	*/
	cell_size = DEFAULTS_DEVICETREE_CELL_SIZE;
	cells = &reg[0];

	for (int i = 0; i < DEFAULTS_DEVICETREE_CELL_SIZE * 2; i++)
		cells[i] = bswap_32 (cells[i]);

//	DeviceTreeLookupReg (&gic_node, reg);
//	cells = &reg[0];
//
//	for (int i = 0; i < 8; i++)
//		kprintf ("%d: 0x%llx\n", i, bswap_32 (&cells[i]));

	return KERN_RETURN_SUCCESS;
}