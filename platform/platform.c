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

#include <platform/platform.h>
#include <platform/devicetree.h>

#include <libkern/assert.h>

kern_return_t platform_get_memory (phys_addr_t *membase, phys_size_t *mmesize)
{
	phys_addr_t addr;
	phys_size_t size;
	DTNode mem_node;
	int res;

	res = DeviceTreeLookupNode ("/memory", &mem_node);
	assert (res == kDeviceTreeSuccess);

	DeviceTreeLookupRegValue (&mem_node, &addr, &size);

	*mmesize = size;
	*membase = addr;

	return KERN_RETURN_SUCCESS;
}