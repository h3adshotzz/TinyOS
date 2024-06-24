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
 * 	Desc:	Task creation structure and definitions.
*/

#ifndef __KERN_TASK_H__
#define __KERN_TASK_H__

#include <kern/vm/vm_types.h>
#include <kern/vm/vm_map.h>
#include <arch/arch.h>

#include <libkern/types.h>
#include <libkern/list.h>

/* Interface logger */
#define task_log(fmt, ...)		interface_log("task", fmt, ##__VA_ARGS__)

/* Tasks status */
#define TASK_STATE_INACTIVE		(0)
#define TASK_STATE_ACTIVE		(1)

/* Maximum number of tasks */
#define TASK_COUNT_MAX			(12)

/* Maximum task name length */
#define TASK_NAME_MAX_LEN		(32)

/* Special task types */
typedef int						pid_t;
typedef int						task_state_t;
typedef vm_address_t			task_entry_t;

/**
 * Task structure
 * 
 * Represents a task. A task can contain multiple threads that all share the
 * same resources, as described by the task structure. Tasks are stored linked
 * together in a list_t, and a global task list is kept within task.c.
 * 
 * A number of things about a task is recorded, for example the total time
 * running, current execution time, priority, reference counting, etc.
 * 
 * A task has a single vm_map_t, which in-turn points to a pmap_t. All tasks run
 * in low-memory, whereas the kernel_task will run in high-memory.
*/
typedef struct task {

	pid_t				pid;		/* task id */
	task_state_t		state;		/* current state */

	/**
	 * When switching between tasks, the schedular will save the callee regsiter
	 * to the task context, and load it when switching back. This is instead of
	 * saving the context on the stack.
	*/
	arm64_cpu_context_t	context;

	/* task name, has a maximum length */
	char				name[TASK_NAME_MAX_LEN];

	/**
	 * Tasks are linked together in a doubly-linked list. The list head is kept
	 * within task.c.
	*/
	list_node_t			tasks;

	/**
	 * Task's virtual memory map. This also points to the pmap_t structure,
	 * which contains the translation table information for this task.
	*/
	vm_map_t			*map;

	/**
	 * Timing statistic. The time the task has been executing since last being
	 * scheduled, and the total amount of time the task has been executing for.
	*/
	uint64_t			current_time;
	uint64_t			total_time;

	integer_t			priority;
	integer_t			preempt;

	/* Number of references */
	integer_t			ref_count;

} task_t;

/* Initialise the task interface */
extern void				task_init(void);

extern kern_return_t	task_create_internal(
							task_entry_t *entry,
							vm_map_t *map,
							const char *name,
							task_t *task);

extern kern_return_t	task_kill_internal(
							task_t *task);

extern void				task_reference(
							task_t *task);

/* Fetch the current task structure */
extern task_t			*get_current_task();

/* Fetch attributes from a task */
extern pid_t			get_task_pid(task_t *task);
extern task_state_t		get_task_state(task_t *task);
extern vm_map_t			get_task_map(task_t *task);
extern pmap_t			get_task_pmap(task_t *task);

extern boolean_t		task_is_kerneltask(task_t *task);

/* Task context */
extern void				task_context_set_entry(
							task_t *task,
							task_entry_t *entry);

/* Debugging */
extern void				task_dump(task_t *task);
extern void				task_dump_all();

#endif /* __kern_task_h__ */