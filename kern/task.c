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

#include <kern/task.h>
#include <kern/kprintf.h>
#include <kern/defaults.h>
#include <kern/vm/vm_page.h>

#include <tinylibc/string.h>

#include <libkern/list.h>
#include <libkern/panic.h>

/* Toggle stack guard check */
#define TASK_DO_STACK_GUARD_CHECK		DEFAULTS_ENABLE

/**
 * A reference to the kernel task is stored here, along with the task list head
 * and PID count.
*/
task_t		kernel_task;
integer_t	task_pid = 0;
list_t		tasks;

/**
 * For now, we'll manually track where the allocations are, but we should switch
 * to some kind of zone allocator or something, so tasks can be allocated in
 * the same place.
*/
vm_address_t	task_page;	/* tmp: replace with a zone? */
vm_offset_t		task_page_cursor = 0;

/**
 * NOTE:	For the moment, we'll record the current task here, so the panic
 * 			handler doesn't fuck up. But this needs to change in two ways:
 * 				- first, the current thing executing should be in the cpu data
 * 				- second, the cpu data should have the current thread, which in
 * 				  turn points to the current task.
 * 
 * 			So, when calling task_get_current(), this will actually fetch the 
 * 			current cpu_t, extract the thread, and access thread->task. Instead
 * 			of trying to track the current task here too. But the kernel task
 * 			will stay here, as that never changes.
*/
task_t		current_task;

/******************************************************************************/
//
// THIS AREA IS TEMPORARY, AND IS REQUIRED TO STOP THINGS CRASHING WITHOUT THE
// THREADS IMPLEMENTATION.
//

// tmp
void		__fork64_return();
void		kernel_task_entry()
{

}
// this is annoying, and really should be in lists.h
static inline void prefetch(const void *x) {;}

task_t *get_current_task()
{
	return &current_task;
}

/**
 * _dump_tasks
*/
void _dump_tasks()
{
	task_t *entry;

	task_log("dumping global task list information:\n");

	list_for_each_entry(entry, &tasks, tasks) {
		vm_address_t stack_guard_addr;
		const char *stack_guard = "__STACK_GUARD__";

		kprintf("task[%d] %s: entry: 0x%lx, stack: 0x%lx\n", 
			entry->pid, entry->name, entry->context.x19, entry->context.sp);

		/**
		 * This isn't intended to stay here, it's just to test that the stack
		 * for each task is valid. If it's not, the kernel will crash.
		*/
#if TASK_DO_STACK_GUARD_CHECK
		stack_guard_addr = entry->context.sp;
		memcpy(stack_guard_addr, stack_guard, strlen(stack_guard));

		kprintf_hexdump(stack_guard_addr, stack_guard_addr, 64);
#endif
	}
}

/******************************************************************************/

/**
 * task_init
 * 
 * Setup the global task list, then create and register the first task for the
 * kernel.
*/
void task_init()
{
	/**
	 * Allocate a page within the kernel task's map to store the task structure,
	 * this could be replaced with a zone allocation, but this needs more
	 * investigation.
	*/
	task_page = vm_map_alloc(vm_get_kernel_map(), sizeof(task_t) * TASK_COUNT_MAX);

	/* Initialise the tasks list */
	INIT_LIST_HEAD(&tasks);

	/**
	 * Create the kernel_task as the first task in the list. For now, the task
	 * context is created in this, so we need to pass the entry point. Once
	 * threads are implemented, the configuration for the context will be
	 * different.
	 * 
	 * The kernel's vm_map is kept within vm.c, and can be fetched with
	 * vm_get_kernel_amp.
	*/
	if (task_create_internal (kernel_task_entry, vm_get_kernel_map(), 
		"kernel_task", &kernel_task) != KERN_RETURN_SUCCESS)
	{
		panic("failed to create task: \"kernel_task\"\n");
	}

	kprintf ("%s: %d\n", kernel_task.name, kernel_task.pid);
}


/**
 * task_create_internal
 * 
 * Creates a new task_t with a given entry point and vm_map. The task is placed
 * within the task page, and a new stack is allocated on the given map. The task
 * is added to the global tasks list, and a pid is assigned.
*/
kern_return_t
task_create_internal(task_entry_t *entry,
					vm_map_t *map,
					const char *name,
					task_t *task)
{
	vm_address_t 	stack;
	size_t			name_len;
	task_t			*new;

	new = (task_t *) (task_page + task_page_cursor);
	task_page_cursor += sizeof (task_t);

	/* two references: caller, and this function */
	new->ref_count = 2;

	new->state = TASK_STATE_INACTIVE;
	new->pid = task_pid;
	task_pid += 1;

	if ((name_len = strlen(name)) > TASK_NAME_MAX_LEN)
		name_len = TASK_NAME_MAX_LEN;
	memcpy(new->name, name, name_len);

	/* TODO: fix the issue in vm.c that is causing this */
	vm_map_alloc(map, 1);

	/* allocate a stack */
	stack = vm_map_alloc(map, VM_PAGE_SIZE);
	if (vm_is_address_valid(stack) != KERN_RETURN_SUCCESS)
		return KERN_RETURN_FAIL;

	/**
	 * Setup the task context. Once threads, are implemented, this will be moved
	 * so each thread has it's own context, but shares the same vm_map/pmap as
	 * the parent task.
	*/
	new->context.x19 = entry;
	new->context.lr = (uint64_t) &__fork64_return;
	new->context.sp = stack;

	list_add_tail(&new->tasks, &tasks);
	*task = *new;

	return KERN_RETURN_SUCCESS;
}