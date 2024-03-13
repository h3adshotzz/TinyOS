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
 * 	Name:	machine.h
 * 	Desc:	Kernel machine interface.
 */

#include <kern/defaults.h>
#include <kern/machine.h>

#include <libkern/assert.h>

#include <platform/devicetree.h>

#include <tinylibc/byteswap.h>
#include <tinylibc/limits.h>

/* Declare the cpu topology structure */
PRIVATE_STATIC_DEFINE(machine_topology_info_t)	topology_info;

/**
 * Name:	machine_read_prop
 * Desc:	Private function to read a particular property from the device tree.
 * 			Ideally we shouldn't do this outside of the device tree interface,
 * 			but it's necessary in some cases.
*/
PRIVATE_STATIC_DEFINE_FUNC(uint64_t)
machine_read_prop (const DTNode node, const char *prop_name)
{
	void const *prop;
	unsigned int prop_size;

	if (DeviceTreeLookupPropertyValue (node, prop_name, &prop, &prop_size) ==
		kDeviceTreeSuccess) {
		return __bswap_32 (*((uint32_t const *) prop));
	}
	return 0;
}

/* Topology getters */
unsigned int machine_get_boot_cpu_num () {return topology_info.boot_cpu->cpu_id;}
unsigned int machine_get_num_clusters () {return topology_info.num_clusters;}
unsigned int machine_get_max_cpu_num () {return topology_info.max_cpu_id;}
unsigned int machine_get_num_cpus () {return topology_info.num_cpus;}

/******************************************************************************/

cpu_number_t
machine_get_cpu_num ()
{
	uint64_t mpidr_val;
	cpu_number_t cpu_num;

	mpidr_val = arm64_read_cpuid ();
	cpu_num = (mpidr_val & (MPIDR_AFF1_MASK | MPIDR_AFF0_MASK));

	/* Until SMP is enabled, don't verify against hte topology */
//	for (cpu_number_t i = 0; i < topology_info.num_cpus; i++) {
//		if (topology_info.cpus[i].cpu_phys_id == cpu_num) {
//			assert (cpu_num <= (unsigned int) machine_get_max_cpu_num ());
//			return cpu_num;
//		}
//	}

	return cpu_num;
}

char *
machine_get_name ()
{
	DTNode *node;
	char *machine;
	int len;

	node = BootDeviceTreeGetRootNode ();

	DeviceTreeLookupPropertyValue (*node, "compatible", &machine, &len);
	return machine;
}

kern_return_t
machine_parse_cpu_topology (void)
{
	DTNode parent, node, subnode;
	const char *cpu_map_path;
	DeviceTreeIterator iter;
	uint16_t boot_cpu;
	int res;

	boot_cpu = arm64_read_cpuid ();

	/**
	 * the default device tree in qemu defines a 'socket0'. Although the kernel can still
	 * read these device trees, in the case of a machine with multiple sockets, only the
	 * first will be read.
	*/
	if (kDeviceTreeSuccess == DeviceTreeNodeExists ("/cpus/cpu-map/socket0")) {
		cpu_map_path = "/cpus/cpu-map/socket0";
	} else {
		cpu_map_path = "/cpus/cpu-map";
	}

#if DEFAULTS_MACHINE_LIBFDT_WORKAROUND
	/**
	 * TEMP:	The following is a workaround for an issue with libfdt. Once the
	 * 			kernel is running with KVAs, meaning in high memory, when libfdt
	 * 			tries to do a search for a node, e.g. /cpus/cpu-map, it returns
	 * 			with an error.
	 * 
	 * 			The only way to get around this is to first manually search for
	 * 			/cpus, and then search for cpu-map within it. We can't directly
	 * 			search for /cpus/cpu-map.
	 * 
	 * 			What this also means is that currently we cannot support any
	 * 			"socket" entries in the cpu-map.
	*/
	res = DeviceTreeLookupNode ("/cpus", &parent);
	assert (res == kDeviceTreeSuccess);

	res = DeviceTreeIteratorInit (&parent, &iter);
	while (kDeviceTreeSuccess == DeviceTreeIterateNodes (&iter, &node)) {
		if (!strcmp (node.name, "cpu-map"))
			parent = node;
	}
#else
	res = DeviceTreeLookupNode (cpu_map_path, &parent);
	assert (res == kDeviceTreeSuccess);
#endif

	res = DeviceTreeIteratorInit (&parent, &iter);
	assert (res == kDeviceTreeSuccess);

	/**
	 * TODO:	Debug why these can't be declared globally and added to 
	 * 			topology_info when it's created.
	*/
	machine_topology_cluster_t clusters[DEFAULTS_KERNEL_MAX_CPU_CLUSTERS];
	machine_topology_cpu_t cpus[DEFAULTS_KERNEL_MAX_CPUS];

	/**
	 * Loop through the various clusters in the device tree to identify available
	 * cpus. Clusters and cpus are numbered from zero. Cpus have a physical and
	 * logial id, where the physical id is directly read from the device tree,
	 * and the logical id assigned by tinyKern.
	*/
	while (kDeviceTreeSuccess == DeviceTreeIterateNodes (&iter, &node)) {
		machine_topology_cluster_t cluster;
		DeviceTreeIterator subiter;

		cluster.cluster_id = topology_info.num_clusters;
		cluster.num_cpus = 0;

		res = DeviceTreeIteratorInit (&node, &subiter);
		assert (res == kDeviceTreeSuccess);

		while (kDeviceTreeSuccess == DeviceTreeIterateNodes (&subiter, &subnode)) {
			machine_topology_cpu_t cpu;
			DTNode cpu_node;
			void *entry;
			int len;

			cpu.cpu_id = topology_info.num_cpus;
			cpu.cluster_id = cluster.cluster_id;
			topology_info.max_cpu_id = MAX(topology_info.max_cpu_id, cpu.cpu_id);

			DeviceTreeLookupPropertyValue (subnode, "cpu", &entry, &len);
			DeviceTreeLookupNodeByPhandle (__bswap_32 (*(__uint32_t *) entry), &cpu_node);
			cpu.cpu_phys_id = (uint32_t) machine_read_prop (cpu_node, "reg");

			if (cpu.cpu_id == boot_cpu) {
				topology_info.boot_cpu = &cpus[topology_info.num_cpus];
				topology_info.boot_cluster = &clusters[topology_info.num_clusters];
			}

			cpus[topology_info.num_cpus] = cpu;
			topology_info.num_cpus += 1;
			cluster.num_cpus += 1;

		}

		clusters[topology_info.num_clusters] = cluster;
		topology_info.num_clusters += 1;
		topology_info.max_cluster_id = MAX(topology_info.max_cluster_id, cluster.cluster_id);
	}

	topology_info.clusters = clusters;
	topology_info.cpus = cpus;

	assert (topology_info.boot_cpu != NULL);

	return KERN_RETURN_SUCCESS;
}
