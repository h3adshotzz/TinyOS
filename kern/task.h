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
 * 	Name:	task.h
 * 	Desc:	Tasking
*/

#ifndef __KERN_TASK_H__
#define __KERN_TASK_H__

#include <kern/vm/vm_types.h>
#include <kern/vm/vm_map.h>
#include <arch/arch.h>

#include <libkern/types.h>

/* Interface logger */
#define task_log(fmt, ...)			interface_log("task", fmt, ##__VA_ARGS__)

/* Task macros */
#define TASK_INACTIVE		(0)
#define TASK_ACTIVE			(1)

#define TASK_NR_MAX			12

/* Process Identifier */
typedef int			pid_t;

/* Task entry pointer */
typedef uint64_t	task_entry_t;

/**
 * Task structure
*/
typedef struct task {

	pid_t			pid;			/* task id */
	boolean_t		active;			/* task state */

	arm64_cpu_context_t		context;

	vm_map_t		*map;			/* virtual memory map */
	struct task		*next;			/* next task in the task list */
	struct task		*prev;			/* previous task in the task list */

	uint64_t		total_time;		/* total time the process has been running */
	uint64_t		current_time;	/* how long the process has been running this cycle */

	integer_t		priority;

} task_t;

/* */
extern task_t		*kernel_task;
extern task_t		*current_task;

extern task_t		*task_list[TASK_NR_MAX];

/* Initialise the task interface */
extern void task_init();

/**
 * Create the kernel task
*/
extern kern_return_t kernel_task_create (task_t *kernel_task, task_entry_t *entry, 
								vm_address_t map, vm_address_t stack);

/* General Task API */
extern kern_return_t task_create();
extern kern_return_t task_sleep();
extern kern_return_t task_destroy();

#endif /* __kern_task_h__ */