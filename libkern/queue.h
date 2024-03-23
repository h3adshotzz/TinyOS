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
 *	Desc:	Double-linked list implementation. This is a bit of a mix between
 *			the Linux and XNU list/queue structures.
 *
 */

#ifndef __LIBKERN_QUEUE_H__
#define __LIBKERN_QUEUE_H__

#include <tinylibc/stddef.h>


/* Doubly-linked list structure */
struct queue {
	struct queue	*next;
	struct queue	*prev;
};

/* Various alias's for queue */
typedef struct queue	queue_t;
typedef struct queue	queue_head_t;
typedef struct queue	queue_entry_t;


static inline void __queue_insert (queue_entry_t *new, 
	queue_entry_t *next, queue_entry_t *prev)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
}

static inline void __queue_remove (queue_entry_t *del,
	queue_entry_t *next, queue_entry_t *prev)
{
	next->prev = prev;
}


/**
 * Name:	enqueue_head
 * Desc:	Enqueue a new node onto the head of the existing queue.
*/
static inline void enqueue_head (queue_t *queue, queue_entry_t *new)
{
	__queue_insert (new, queue, queue->next);
}

/**
 * Name:	enqueue_tail
 * Desc:	Enqueue a new node onto the tail of the existing queue.
*/
static inline void enqueue_tail (queue_t *queue, queue_entry_t *new)
{
	__queue_insert (new, queue->prev, queue);
}

/**
 * Name:	dequeue_head
 * Desc:	Dequeue a node from the head of an existing queue.
*/
static inline void dequeue_head (queue_t *queue, queue_entry_t *del)
{
	__queue_remove (del, queue->next, queue->prev);
}


#define queue_element(ptr, type, field)	\
	((type *)((void *)((char *)(ptr) - __offsetof(type, field))))

#endif /* __libkern_queue_h__ */