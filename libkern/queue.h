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
 *	Name:	queue.h
 *	Desc:	Double-linked list implementation based on GNU Mach's queue_t.
 */

#ifndef __LIBKERN_QUEUE_H__
#define __LIBKERN_QUEUE_H__

#include <tinylibc/stddef.h>


/* Doubly-linked list structure */
struct queue {
	struct queue	*next;		/* next element */
	struct queue	*prev;		/* prev element */
};

/* Various alias's for queue */
typedef struct queue	queue_t;
typedef struct queue	queue_head_t;
typedef struct queue	queue_entry_t;


/**
 * Name:	enqueue_head
 * Desc:	Enqueue a new node onto the head of the existing queue.
*/
static inline void enqueue_head (queue_t *queue, queue_entry_t *elm)
{
	queue_entry_t	*old_head;

	old_head = queue->next;
	elm->next = old_head;
	elm->prev = queue;
	old_head->prev = elm;
	queue->next = elm;
}

/**
 * Name:	enqueue_tail
 * Desc:	Enqueue a new node onto the tail of the existing queue.
*/
static inline void enqueue_tail (queue_t *queue, queue_entry_t *elm)
{
	queue_entry_t	*old_tail;

	old_tail = queue->prev;
	elm->next = queue;
	elm->prev = old_tail;
	old_tail->next = elm;
	queue->prev = elm;
}

/**
 * Name:	dequeue_head
 * Desc:	Dequeue a node from the head of an existing queue.
*/
static inline queue_entry_t *dequeue_head (queue_t *queue)
{
	queue_entry_t	*elm = (queue_entry_t *) 0;
	queue_entry_t	*new_head;

	if (queue->next != queue) {
		elm = queue->next;
		new_head = elm->next;
		new_head->prev = queue;
		queue->next = new_head;
	}

	return (elm);
}

/**
 * Name:	dequeue_tail
 * Desc:	Dequeue a node from the tail of an existing queue.
*/
static inline queue_entry_t *dequeue_tail (queue_t *queue)
{
	queue_entry_t	*elm = (queue_entry_t *) 0;
	queue_entry_t	*new_tail;

	if (queue->prev != queue) {
		elm = queue->prev;
		new_tail = elm->prev;
		new_tail->next = queue;
		queue->prev = new_tail;
	}

	return (elm);
}

/**
 * Macro:	queue_init
 * Desc:	Initialise a new queue_t
*/
#define queue_init(q)														\
	do {																	\
		(q)->next = (q);													\
		(q)->prev = (q);													\
	} while (0);

/**
 * Macro:	queue_element
 * Desc:	Convert a queue_entry_t to the original element type pointer
 * 			containing a queue. Where:
 * 				ptr		-	queue entry to convert.
 * 				type	-	original type of the queue entry.
 * 				field	-	field name of the queue in the given type.
*/
#define queue_element(ptr, type, field)										\
	((type *)((void *)((char *)(ptr) - __offsetof(type, field))))			\

/**
 * Macro:	queue_foreach
 * Desc:	Iterate over each queue_entry_t structure, generate a for loop and
 * 			setting 'elm' to each entry.
*/
#define queue_foreach(elm, head)											\
	for (elm = (head)->next; elm != (head); elm = (elm)->next)

/**
 * Macro:	queue_foreach_element
 * Desc:	Iterate over each queue_entry_t structure, generate a for loop and
 * 			setting 'elm' to converted type pointer.
*/
#define queue_foreach_element(elm, head, field)								\
	for (elm = queue_element((head)->next, typeof(*(elm)), field);			\
		&((elm)->field) != (head);											\
		elm = queue_element((elm)->field.next, typeof(*(elm)), field))

/**
 * Macro:	queue_first
 * Desc:	Return the first entry in the queue.
*/
#define queue_first(q)														\
	((q)->next)

/**
 * Macro:	queue_next
 * Desc:	Return the next entry in the queue.
*/
#define queue_next(q)														\
	queue_first(q)

/**
 * Macro:	queue_last
 * Desc:	Return the last entry in the queue.
*/
#define queue_last(q)														\
	((q)->prev)

/**
 * Macro:	queue_prev
 * Desc:	Return the previous entry in the queue.
*/
#define queue_prev(q)														\
	queue_last(q)

/**
 * Macro:	queue_end
 * Desc:	Tests whether a new entry is really the end of the queue.
*/
#define queue_end(q, qe)													\
	((q) == (qe))

/**
 * Macro:	queue_empty
 * Desc:	Tests whether a queue is empty.
*/
#define queue_empty(q)														\
	queue_end((q), queue_first(q))

#endif /* __libkern_queue_h__ */