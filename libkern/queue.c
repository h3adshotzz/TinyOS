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
 * Name:	queue.c
 * Desc:	Temporary file containing queue tests while the header file is built
 * 			out. One the queue functionality is complete, this won't be necessary
 * 			and examples will be written in documentation.
*/

#include <libkern/queue.h>

static queue_t test_queue;

/* test data structure */
struct test_type {
	queue_head_t	queue;
	unsigned int	data;
};

static inline void __queue_iterate ()
{
	struct test_type *tmp;
	queue_foreach_element (tmp, &test_queue, queue) {
		kprintf ("tmp: data: %d, addr: 0x%llx, next: 0x%llx\n",
			tmp->data, tmp, tmp->queue.next);
	}
}

void __queue_test_1 ()
{
	/**
	 * Test:	This is a generic test to demonstrate adding a number of elements
	 * 			into a queue, and then iterating through them again. This shows
	 * 			this for enqueue_head and enqueue_tail.
	*/

	/**
	 * these macros are used for allocating some memory to the structs, as we
	 * don't have malloc yet.
	*/
#define __test_alloc_base	(0xfffffff038000000 + (0x41000000 - 0x40000000))
#define __test_alloc(__x)	(__test_alloc_base + (sizeof (struct test_type) * __x))

	struct test_type *t1 = (struct test_type *) __test_alloc (1);
	t1->data = 1;

	struct test_type *t2 = (struct test_type *) __test_alloc (2);
	t2->data = 2;

	struct test_type *t3 = (struct test_type *) __test_alloc (3);
	t3->data = 3;

	struct test_type *t4 = (struct test_type *) __test_alloc (4);
	t4->data = 4;

	/* output the base values */
	kprintf ("t1: data: %d, addr: 0x%llx\n", t1->data, &t1);
	kprintf ("t2: data: %d, addr: 0x%llx\n", t2->data, &t2);
	kprintf ("t3: data: %d, addr: 0x%llx\n", t3->data, &t3);
	kprintf ("t4: data: %d, addr: 0x%llx\n", t4->data, &t4);

	/* init the queue */
	queue_init (&test_queue);
	kprintf ("queue addr: 0x%llx\n", &test_queue);

	/* enqueue the elements to the head */
	enqueue_head (&test_queue, (queue_entry_t *) t1);
	enqueue_head (&test_queue, (queue_entry_t *) t2);
	enqueue_head (&test_queue, (queue_entry_t *) t3);
	enqueue_head (&test_queue, (queue_entry_t *) t4);

	/* iterate over the queue */
	__queue_iterate ();

	/* remove the head */
	kprintf ("dequeue_head:\n");
	dequeue_head (&test_queue);
	__queue_iterate ();

	/* remove the tail */
	kprintf ("dequeue_tail:\n");
	dequeue_tail (&test_queue);
	__queue_iterate ();

	if (queue_empty(&test_queue))
		kprintf ("test_queue empty\n");

	queue_t empty_queue;
	queue_init (&empty_queue);
	if (queue_empty (&empty_queue))
		kprintf ("queue empty\n");



	kprintf ("--QUEUE__queue_test_1__COMPLETE\n");
}