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
 * 	Name:	devicetree.h
 * 	Desc:	Platform wrapper for the libfdt library.
 */

#ifndef __PLATFORM_DEVICETREE_H__
#define __PLATFORM_DEVICETREE_H__

#include <tinylibc/stdint.h>
#include <libfdt/libfdt.h>

/* Device tree types */
typedef int			DTNodeOffset;
typedef int 		DTInteger;
typedef void		DTVoid;

/* Device tree consts */
enum {
	kDeviceTreeSuccess = 0,
	kDeviceTreeFailure,

	kPropNameLength = 32,
};

/**
 * Structures for Flattened Device Tree
*/

typedef struct DeviceTreeNode
{
	char			name[kPropNameLength];
	DTNodeOffset	offset;
	DTInteger		nChildren;
	DTInteger		nProperties;
} DeviceTreeNode, DTNode;

typedef struct DeviceTreeIterator
{
	DeviceTreeNode	baseNode;
	DeviceTreeNode	currentNode;
	DTInteger		index;
	DTInteger		count;
} DeviceTreeIterator;


/*******************************************************************************
 *  Device Tree Initialisation
 ******************************************************************************/

/**
 * DeviceTreeInit
 * 
 * Initialise the private boot device tree structure with an fdt located at the
 * specified offset, and of the specified size. This will be used as the fdt
 * pointer for the rest of the lifetime of the kernel, until reboot.
 * 
 * @param	base		Base address of fdt. MUST be in non-secure memory.
 * @param	size		Size of the fdt in memory.
 * 
 * @returns		kDeviceTreeSuccess or kDeviceTreeFailure.
*/
extern DTInteger
DeviceTreeInit (void *base, size_t size);


/*******************************************************************************
 *  Device Tree Node Lookup
 ******************************************************************************/

/**
 * DeviceTreeLookupNode
 * 
 * Lookup a node within the device tree that matches a given name, and place it
 * in `node`.
 * 
 * @param	lookup		Name of the node to search for.
 * @param	node		Pointer to store the node in, if found.
 * 
 * @returns		kDeviceTreeSuccess or kDeviceTreeFailure
*/
extern DTInteger
DeviceTreeLookupNode (const char *lookup, DTNode *node);

/**
 * DeviceTreeLookupNodeByOffset
 * 
 * Lookup a node within the device tree by it's offset, and place it in `node`.
 * 
 * @param	offset		Offset of the node.
 * @param	node		Pointer to store the node in, if found.
 * 
 * @returns		kDeviceTreeSuccess or kDeviceTreeFailure
*/
extern DTInteger
DeviceTreeLookupNodeByOffset (DTNodeOffset offset, DTNode *node);

/**
 * DeviceTreeNodeFirstSubnode
 * 
 * Find the first subnode within a given parent node, and store the subnode in 
 * `first`.
 * 
 * @param	node		Parent node to search in.
 * @param	first		Pointer to store the subnode in.
 * 
 * @returns		kDeviceTreeSuccess or kDeviceTreeFailure
*/
extern DTInteger
DeviceTreeNodeFirstSubnode (DTNode node, DTNode *first);

/**
 * DeviceTreeNodeFirstSubnode
 * 
 * Find the next subnode within a given parent node, and store the subnode in 
 * `next`.
 * 
 * @param	node		Parent node to search in.
 * @param	next		Pointer to store the subnode in.
 * 
 * @returns		kDeviceTreeSuccess or kDeviceTreeFailure
*/
extern DTInteger
DeviceTreeNodeNextSubnode (DTNode node, DTNode *next);


/*******************************************************************************
 *  Device Tree Node Property Lookup
 ******************************************************************************/

/**
 * DeviceTreeLookupPropertyValue
 * 
 * Lookup a property within a given node which patches the given `propName`, and
 * store it in `propValue` along with it's size in `propSize`.
 * 
 * @param	node		Parent node to search in.
 * @param	propName	Name of the property to find.
 * @param	propValue	Buffer pointer to store the result in.
 * @param	propSize	Pointer to store the size in.
 * 
 * @returns		kDeviceTreeSuccess or kDeviceTreeFailure
*/
extern DTInteger
DeviceTreeLookupPropertyValue (DTNode node, const char *propName, 
							   char **propValue, DTInteger *propSize);


/**
 * DeviceTreeLookupRegValue
 * 
 * Lookup the Address and Size value in a nodes 'reg' field, and store the
 * results in the `addr` and `size` parameters.
 * 
 * @param	node		Node containg a `reg` field.
 * @param	addr		Pointer to store the Address value in.
 * @param	size		Poitner to store the Size value in.
 * 
 * @returns		kDeviceTreeSuccess or kDeviceTreeFailure
*/
extern DTInteger
DeviceTreeLookupRegValue (DTNode *node, uint64_t *addr, uint64_t *size);

/*******************************************************************************
 *  Device Tree Node Iterator
 ******************************************************************************/

/**
 * DeviceTreeIteratorInit
 * 
 * Initialise a DeviceTreeIterator structure with a given start node.
 * 
 * @param	start		Pointer to a Node, containing subnodes, to intialise the 
 *						iterator with.
 * @param	iter		Pointer to the iterator to initialise.
 * 
 * @returns		kDeviceTreeSuccess or kDeviceTreeFailure
*/
extern DTInteger
DeviceTreeIteratorInit (DTNode *start, DeviceTreeIterator *iter);

/**
 * DeviceTreeIterateNodes
 * 
 * Iterate through the parent node to the next subnode of the given iterator.
 * 
 * @param	iter		Pointer to the iterator structure.
 * @param	next		Pointer to the location to store the next subnode.
 * 
 * @returns		kDeviceTreeSuccess or kDeviceTreeFailure
*/
extern DTInteger
DeviceTreeIterateNodes (DeviceTreeIterator *iter, DTNode *next);


/*******************************************************************************
 *  Boot Device Tree-specific Helpers
 ******************************************************************************/

/**
 * BootDeviceTreeGetRootNode
 * 
 * Return the root DTNode for the Boot device tree. This may occasionally be
 * needed.
 * 
 * TODO: 	Consider replacing with a func to directly get properties from the
 * 			root node, so only the devicetree.c has access to the root node.
 * 
 * @returns		Pointer to the Root node of the Boot Device Tree.
*/
extern const DTNode *
BootDeviceTreeGetRootNode ();

#endif