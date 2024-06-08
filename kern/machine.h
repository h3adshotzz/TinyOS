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
 *	Name:	machine.h
 *	Desc:	Kernel machine interface. Handles tracking the system topology.
 *
 */

#ifndef __KERN_MACHINE_H__
#define __KERN_MACHINE_H__

#include <tinylibc/stdint.h>

#include <libkern/types.h>
#include <kern/cpu.h>

/* Interface logger */
#define machine_log(fmt, ...)		interface_log("machine", fmt, ##__VA_ARGS__)

/******************************************************************************
 * Machine Topology Layout
 *
 * Defines the structure of the machine topology.
 *****************************************************************************/

typedef struct machine_topology_cpu
{
	unsigned int				cpu_phys_id;
	unsigned int				cpu_id;
	unsigned int				cluster_id;
	cpu_type_t					cpu_type_t;
} machine_topology_cpu_t;

typedef struct machine_topology_cluster
{
	unsigned int				cluster_id;
	unsigned int				num_cpus;
	unsigned int				first_cpu_id;
	uint64_t					cpu_mask;
} machine_topology_cluster_t;

typedef struct machine_topology_info
{
	unsigned int				version;
	unsigned int				num_cpus;
	unsigned int				num_clusters;
	unsigned int				max_cpu_id;
	unsigned int				max_cluster_id;
	machine_topology_cpu_t		*cpus;
	machine_topology_cpu_t		*boot_cpu;
	machine_topology_cluster_t	*clusters;
	machine_topology_cluster_t	*boot_cluster;
} machine_topology_info_t;

/* Getters for machine topology */
unsigned int	machine_get_boot_cpu_num ();
unsigned int	machine_get_max_cpu_num ();
unsigned int	machine_get_num_cpus ();
unsigned int	machine_get_num_clusters ();

/**
 *	machine_parse_cpu_topology
 *
 * 	Read the CPU topology from the device tree and construct the machine_topology_info
 * 	structure to track it.
 */
kern_return_t machine_parse_cpu_topology (void);

/**
 *	machine_get_cpu_num
 *
 * 	Fetch the cpu_number_t of the current CPUs machine topology entry.
 */
cpu_number_t machine_get_cpu_num (void);


#endif /* __kern_machine_h__ */

