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

task_t *kernel_task;

extern void *__fork64_return;

void task_init()
{
	task_log("task init complete\n");
}

kern_return_t kernel_task_create (task_t *kernel_task, task_entry_t *entry, 
								vm_address_t map, vm_address_t stack)
{
	task_log ("attempting to create kernel task: 0x%lx, 0x%lx, 0x%lx, 0x%lx\n",
		kernel_task, entry, map, stack);

	kernel_task->pid = 0;
	kernel_task->active = TASK_INACTIVE;

	kernel_task->map = (vm_map_t *) map;
	kernel_task->prev = kernel_task;
	kernel_task->next = kernel_task;

	//kernel_task->stack = stack;

	kernel_task->context.x19 = entry;
	kernel_task->context.pc = (uint64_t) __fork64_return;
	kernel_task->context.sp = stack;

	return KERN_RETURN_SUCCESS;
}