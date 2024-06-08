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

#include <kern/machine.h>
#include <arch/arch.h>

#include <platform/devicetree.h>
#include <libkern/assert.h>

#include <tinylibc/byteswap.h>
#include <tinylibc/string.h>

/* translate mpidr to cpu number */
#define MPIDR_TO_CPU_NUM(__mpidr)	(__mpidr & (MPIDR_AFF1_MASK | MPIDR_AFF0_MASK))

/* declare the cpu topology structure */
static machine_topology_info_t		topology_info;

/* cpu and cluster arrays */
static machine_topology_cluster_t	clusters[DEFAULTS_MACHINE_MAX_CPU_CLUSTERS];
static machine_topology_cpu_t		cpus[DEFAULTS_MACHINE_MAX_CPUS];

/**
 *	machine_read_prop
 *
 *	Read a particular property from the device tree. Ideally, this shouldn't be
 *	done outside of the devicetree wrapper, but in some cases doing it this way
 *	is required.
 */
static uint64_t machine_read_prop (const DTNode node, const char *prop_name)
{
	unsigned int prop_size;
	void const *prop;

	return 0;
}

/* machine topology getters */
unsigned int machine_get_boot_cpu_num () {return topology_info.boot_cpu->cpu_id;}
unsigned int machine_get_num_clusters () {return topology_info.num_clusters;}
unsigned int machine_get_max_cpu_num () {return topology_info.max_cpu_id;}
unsigned int machine_get_num_cpus () {return topology_info.num_cpus;}

/*****************************************************************************/

cpu_number_t machine_get_cpu_num ()
{
	cpu_number_t cpu_num;

	cpu_num = MPIDR_TO_CPU_NUM (arm64_read_cpuid ());

	/* until smp is enabled, don't verify this against the topology */
	for (cpu_number_t i = 0; i < topology_info.num_cpus; i++) {
		if (topology_info.cpus[i].cpu_phys_id == cpu_num) {
			assert (cpu_num <= (unsigned int) topology_info.max_cpu_id);
			return cpu_num;
		}
	}

	return cpu_num;
}

char *machine_get_name ()
{
	const DTNode *node;
	char *machine;
	int len;

	node = BootDeviceTreeGetRootNode ();
	DeviceTreeLookupPropertyValue (*node, "compatible", &machine, &len);

	return machine;
}

kern_return_t machine_parse_cpu_topology (void)
{
	DTNode parent, node, subnode;
	const char *cpu_map_path;
	DeviceTreeIterator iter;
	cpu_number_t boot_cpu;
	int res;


	/* the cpu topology should only ever be called on the boot cpu */
	boot_cpu = MPIDR_TO_CPU_NUM (arm64_read_cpuid ());

	/**
	 * the following is a workaround for an issue with libfdt. once the kernel
	 * is running with KVAs, meaning in high memory, when libfdt tries to do
	 * a search for a node, e.g. /cpus/cpu-map, it returns with an error.
	 *
	 * the only way to get around this is to first manually search for /cpus,
	 * and then search for the cpu-map within it. we can't directly search for
	 * /cpus/cpu-map.
	 *
	 * what this essentially means is that we cannot support any "socket"
	 * entries within the device tree cpu-map.
	 *
	 */
	cpu_map_path = "/cpus/cpu-map";

#if DEFAULTS_SET(DEFAULTS_MACHINE_LIBFDT_WORKAROUND)
	
	/* look for "/cpus" first */
	res = DeviceTreeLookupNode ("/cpus", &parent);
	assert (res == kDeviceTreeSuccess);

	/* now look for the cpu-map */
	res = DeviceTreeIteratorInit (&parent, &iter);
	while (DeviceTreeIterateNodes (&iter, &node) == kDeviceTreeSuccess) {
		if (!strcmp (node.name, "cpu-map"))
			parent = node;
	}

#else

	/* if the workaround is not needed, we can directly search for cpu_map_path */
	res = DeviceTreeLookupNode (cpu_map_path, &parent);
	assert (res == kDeviceTreeSuccess);

#endif
	
	/* initialise a new iterator for the cpu-map */
	res = DeviceTreeIteratorInit (&parent, &iter);
	assert (res == kDeviceTreeSuccess);

	topology_info.num_clusters = 0;
	topology_info.num_cpus = 0;

	/**
	 * loop through the clusters in the device tree to identifiy available
	 * cpus. clusters and cpus are numbered from zero, and cpus have both a
	 * physical and logical identifier. the physical id is read directly
	 * from the device tree, and the logical id is assigned by the kernel.
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
			char *entry;
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
