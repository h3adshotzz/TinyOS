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
 * 	Name:	devicetree.c
 * 	Desc:	Platform wrapper for the libfdt library.
 */

#include <platform/devicetree.h>
#include <libkern/types.h>

// TEMP: replace with kern/types.h
#include <kern/defaults.h>

#include <tinylibc/byteswap.h>

/**
 * The boot device tree structure is kept private to this code, any components
 * of the kernel which need it have to use the getters.
*/
struct boot_device_tree
{
	uint64_t	base;
	uint64_t	size;

	DTNode		root;

	const char 	*model;
	const char	*compatible;

	DTInteger	initialised;
};

/* Root device tree status */
#define ROOT_DEVICE_TREE_DEAD		0
#define ROOT_DEVICE_TREE_INIT		1

/* Private boot device tree structure */
PRIVATE_STATIC_DEFINE(struct boot_device_tree)	BootDeviceTree;


/**
 * 
*/
PRIVATE_STATIC_DEFINE_FUNC(DTInteger)
node_offset_count_children (DTNodeOffset node_offset)
{
	DTNodeOffset cur;
	DTInteger count;

	count = 0;

	cur = fdt_first_subnode (BootDeviceTree.base, node_offset);
	while (cur >= 0) {
		count += 1;
		cur = fdt_next_subnode (BootDeviceTree.base, cur);
	}
	return count;
}

/**
 * 
*/
PRIVATE_STATIC_DEFINE_FUNC(DTInteger)
node_offset_count_properties (DTNodeOffset node_offset)
{
	DTNodeOffset cur;
	DTInteger count;

	count = 0;

	cur = fdt_first_property_offset (BootDeviceTree.base, node_offset);
	while (cur >= 0) {
		count += 1;
		cur = fdt_next_property_offset (BootDeviceTree.base, cur);
	}
	return count;
}

/******************************************************************************/
#if DEFAULTS_KERNEL_KDEBUG_MODE

KERNEL_DEBUG_DEFINE(uint64_t)
Debug_DeviceTreeGetBootDeviceTreeBase ()
{
	return BootDeviceTree.base;
}

#endif
/******************************************************************************/

const DTNode *
BootDeviceTreeGetRootNode ()
{
	return (DTNode *) &BootDeviceTree.root;
}

DTInteger
DeviceTreeInit (void *base, size_t size)
{
	int res, len;
	const char *name;

	BootDeviceTree.initialised = ROOT_DEVICE_TREE_DEAD;

	BootDeviceTree.base = base;
	BootDeviceTree.size = size;

	res = fdt_check_header ((void *) BootDeviceTree.base);
	if (res) {
		kprintf ("DeviceTreeInit: ERROR: failed to read device tree: 0x%llx\n", res);
		return kDeviceTreeFailure;
	}

	res = DeviceTreeLookupNode ("/", &BootDeviceTree.root);
	if (res == kDeviceTreeFailure) {
		kprintf ("DeviceTreeInit: ERROR: failed to find root node: 0x%llx\n", res);
		return kDeviceTreeFailure;
	}

	BootDeviceTree.initialised = ROOT_DEVICE_TREE_INIT;
	return kDeviceTreeSuccess;
}

DTInteger
DeviceTreeNodeExists (const char *name)
{
	return (fdt_path_offset (BootDeviceTree.base, name) < 0) ? 
		kDeviceTreeFailure : kDeviceTreeSuccess;
}

DTInteger
DeviceTreeLookupNode (const char *lookup, DTNode *node)
{
	int res, len;
	const char *name;

	res = fdt_path_offset (BootDeviceTree.base, lookup);
	if (res < 0) {
		kprintf ("DeviceTreeLookupNode: ERROR: failed to find node '%s': 0x%llx\n",
			lookup, res);
		return kDeviceTreeFailure;
	}

	name = fdt_get_name (BootDeviceTree.base, res, &len);
	if (len < 0) {
		kprintf ("DeviceTreeLookupNode: ERROR: failed to verify name of node: '%s': %d\n",
			lookup, len);
		return kDeviceTreeFailure;
	}
	if (len > kPropNameLength) {
		kprintf ("DeviceTreeLookupNode: WARNING: node name '%d' longer than max: %d, %d\n",
			name, len, kPropNameLength);
	}

	node->offset = (DTNodeOffset) res;
	memcpy (node->name, name, len+1);

	node->nChildren = node_offset_count_children (node->offset);
	node->nProperties = node_offset_count_properties (node->offset);

	return kDeviceTreeSuccess;
}

DTInteger
DeviceTreeLookupNodeByOffset (DTNodeOffset offset, DTNode *node)
{
	int len;
	const char *name;
	node->offset = offset;

	name = fdt_get_name (BootDeviceTree.base, offset, &len);
	if (len < 0) {
		kprintf ("DeviceTreeLookupNodeByOffset: ERROR: failed to find name for node offset: %d: %d\n",
			offset, len);
		return kDeviceTreeFailure;
	}
	memcpy (node->name, name, len+1);

	node->nChildren = node_offset_count_children (node->offset);
	node->nProperties = node_offset_count_properties (node->offset);

	return kDeviceTreeSuccess;
}

DTInteger
DeviceTreeNodeFirstSubnode (DTNode node, DTNode *first)
{
	int res;

	res = fdt_first_subnode (BootDeviceTree.base, node.offset);
	if (res == -FDT_ERR_NOTFOUND) {
		kprintf ("DeviceTreeNodeFirstSubnode: ERROR: failed to find subnode for node '%s'\n",
			node.name);
		return kDeviceTreeFailure;
	}

	return DeviceTreeLookupNodeByOffset ((DTNodeOffset) res, first);
}

DTInteger
DeviceTreeNodeNextSubnode (DTNode node, DTNode *next)
{
	int res;

	res = fdt_next_subnode (BootDeviceTree.base, node.offset);
	if (res == -FDT_ERR_NOTFOUND) {
		kprintf ("DeviceTreeNodeFirstSubnode: ERROR: failed to find subnode for node '%s'\n",
			node.name);
		return kDeviceTreeFailure;
	}

	return DeviceTreeLookupNodeByOffset ((DTNodeOffset) res, next);
}

DTInteger
DeviceTreeIteratorInit (DTNode *start, DeviceTreeIterator *iter)
{
	DTNode node;

	if (!BootDeviceTree.initialised)
		return kDeviceTreeFailure;

	if (start != NULL) {
		iter->baseNode = *start;
		iter->currentNode = *start;
	} else {
		iter->baseNode = BootDeviceTree.root;
		iter->currentNode = BootDeviceTree.root;
	}
	iter->count = iter->baseNode.nChildren;
	iter->index = 0;

	return kDeviceTreeSuccess;
}

DTInteger
DeviceTreeIterateNodes (DeviceTreeIterator *iter, DTNode *next)
{
	if (iter->index >= iter->count)
		return kDeviceTreeFailure;

	iter->index += 1;
	if (iter->index == 1) {
		DeviceTreeNodeFirstSubnode (iter->currentNode, &iter->currentNode);
	} else {
		DeviceTreeNodeNextSubnode (iter->currentNode, &iter->currentNode);
	}
	*next = iter->currentNode;

	return kDeviceTreeSuccess;
}

DTInteger
DeviceTreeLookupPropertyValue (DTNode node, const char *propName, char **propValue, DTInteger *propSize)
{
	int len;
	const char *value;

	value = fdt_getprop (BootDeviceTree.base, node.offset, propName, &len);
	if (len < 0) {
		kprintf ("DeviceTreeLookupProperty: ERROR: failed to find prop '%s' in node '%s': %d\n",
			propName, node.name, len);
		return kDeviceTreeFailure;
	}
	*propValue = value;
	propSize = len;

	return kDeviceTreeSuccess;
}

DTInteger
DeviceTreeLookupNodeByPhandle (uint64_t phandle, DTNode *node)
{
	int res, len;
	const char *name;

	res = fdt_node_offset_by_phandle (BootDeviceTree.base, phandle);
	if (res < 0) {
		kprintf ("DeviceTreeLookupNodeByPhandle: ERROR: failed to find node with phandle '0x%x': 0x%llx\n",
			phandle, res);
		return kDeviceTreeFailure;
	}

	name = fdt_get_name (BootDeviceTree.base, res, &len);
	node->offset = (DTNodeOffset) res;
	memcpy (node->name, name, len+1);

	node->nChildren = node_offset_count_children (node->offset);
	node->nProperties = node_offset_count_properties (node->offset);

	return kDeviceTreeSuccess;
}

DTInteger
DeviceTreeLookupReg (DTNode *node, uint32_t *reg)
{
	int res, cell_size;

	reg = fdt_getprop (BootDeviceTree.base, node->offset, "reg", &res);
	if (res < 0) {
		kprintf ("DeviceTreeLookupReg: ERROR: failed to get prop 'reg' from node '%s': 0x%llx\n",
			node->name, res);
		return kDeviceTreeFailure;
	}

	return kDeviceTreeSuccess;
}

DTInteger
DeviceTreeLookupRegValue (DTNode *node, uint64_t *addr, uint64_t *size)
{
	int res, cell_size;
	uint32_t *reg, *cells;

	reg = fdt_getprop (BootDeviceTree.base, node->offset, "reg", &res);
	if (res < 0) {
		kprintf ("DeviceTreeLookupRegValue: ERROR: failed to get prop 'reg' from node '%s': 0x%llx\n",
			node->name, res);
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

	/**
	 * The address and size values are split into two 32-bit integers. Reconstruct
	 * this 64-bit value and write it into `addr` and `size`.
	*/
	*addr = ((uint64_t) cells[0] << 32) + cells[1];
	*size = ((uint64_t) cells[2] << 32) + cells[3];
}
