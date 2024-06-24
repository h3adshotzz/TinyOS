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
 * Name:	list.h
 * Desc:	Ciruclar doubly-linked list implementation for tinyOS. 
*/

#ifndef __LIBKERN_LIST_H__
#define __LIBKERN_LIST_H__

#include <tinylibc/stdint.h>

#undef offsetof
#define offsetof(type, member)	((size_t)&((type *)0)->member)

/**
 * container_of
 * 
 * Cast a member of a struct out to the containing structure
 * 
 * @param ptr		the pointer to the member
 * @param type		the type of the container struct this is embedded in
 * @param member	the name of the member within the struct
*/
#define container_of(ptr, type, member) ({						\
	const typeof( ((type *)0)->member ) * __mptr = (ptr);		\
	(type *)( (char *)__mptr - offsetof(type, member) );		\
})


struct list_head {
	struct list_head	*next, *prev;
};

#define LIST_HEAD_INIT(name)	{ &(name), &(name) }

#define LIST_HEAD(name)	\
	struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

/**
 * Insert a new entry between two known consecutive entries.
 * 
 * This is only for internal list manipulation where we know the prev/next
 * entries already!
*/
static inline void __list_add (struct list_head *new, struct list_head *prev,
							struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/**
 * Delete a list entry by making the prev/next entries point to each other.
 * 
 * This is only for internal list manipulation where we know the prev/next
 * entries already!
*/
static inline void __list_del (struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * list_add
 * 
 * Add a new entry. Insert a new entry after the specified head. This is good
 * for implementing stacks.
 * 
 * @param new		new entry to be added
 * @param head		list head to add it after
 * 
 * @returns void
*/
static inline void list_add (struct list_head *new, struct list_head *head)
{
	__list_add (new, head, head->next);
}

/**
 * list_add_tail
 * 
 * Add a new entry. Insert a new entry before the specified head. This is useful
 * for implementing queues.
 * 
 * @param new		new entry to be added
 * @param head		list head to add it before
 * 
 * @returns void
*/
static inline void list_add_tail (struct list_head *new, struct list_head *head)
{
	__list_add (new, head->prev, head);
}

/**
 * list_del
 * 
 * Deletes an entry from the list. Note: list_empty() on entry does not return
 * true after this, the entry is an undefined state.
 * 
 * @param entry		the element to delete from the list
 * 
 * @returns void
*/
static inline void list_del (struct list_head *entry)
{
	__list_del (entry->prev, entry->next);
	entry->next = 0x0;
	entry->prev = 0x0;
}

/**
 * list_replace
 * 
 * Replace old entry by a new one. If old is empty, it will be overwritten.
 * 
 * @param old		the element to be replaced
 * @param new		the new element to be inserted
 * 
 * @returns void
*/
static inline void list_replace (struct list_head *old, struct list_head *new)
{
	new->next = old->next;
	new->next->prev = new;
	new->prev = old->prev;
	new->prev->next = new;
}

static inline void list_replace_init (struct list_head *old,
								struct list_head *new)
{
	list_replace (old, new);
	INIT_LIST_HEAD(old);
}

/**
 * list_del_init
 * 
 * Deletes entry from list and reinitialize it.
 * 
 * @param entry 	the element to delete from the list.
 * 
 * @returns void
 */
static inline void list_del_init (struct list_head *entry)
{
	__list_del (entry->prev, entry->next);
	INIT_LIST_HEAD(entry);
}

/**
 * list_move
 * 
 * Delete from one list and add as another's head
 * 
 * @param list		the entry to move
 * @param head		the head that will precede our entry
 * 
 * @returns void
 */
static inline void list_move (struct list_head *list, struct list_head *head)
{
	__list_del (list->prev, list->next);
	list_add (list, head);
}

/**
 * list_move_tail
 * 
 * Delete from one list and add as another's tail
 * 
 * @param list		the entry to move
 * @param head		the head that will follow our entry
 * 
 * @returns void
 */
static inline void list_move_tail (struct list_head *list,
				  struct list_head *head)
{
	__list_del (list->prev, list->next);
	list_add_tail (list, head);
}

/**
 * list_is_last
 * 
 * Tests whether 'list' is the last entry in list 'head'
 * 
 * @param list		the entry to test
 * @param head		the head of the list
 * 
 * @returns void
 */
static inline int list_is_last (const struct list_head *list,
				const struct list_head *head)
{
	return list->next == head;
}

/**
 * list_empty
 * 
 * Tests whether a list is empty
 * 
 * @param head		the list to test.
 * 
 * @returns void
 */
static inline int list_empty (const struct list_head *head)
{
	return head->next == head;
}

/**
 * list_empty_careful
 * 
 * Tests whether a list is empty and not being modified. Tests whether a list
 * is empty _and_ checks that no other CPU might be in the process of modifying
 * either member (next or prev)
 *
 * NOTE: using list_empty_careful() without synchronization can only be safe if
 * the only activity that can happen to the list entry is list_del_init(). 
 * Eg. it cannot be used if another CPU could re-list_add() it.
 * 
 * @param head		the list to test
 */
static inline int list_empty_careful (const struct list_head *head)
{
	struct list_head *next = head->next;
	return (next == head) && (next == head->prev);
}

/**
 * list_is_singular
 * 
 * Tests whether a list has just one entry.
 * 
 * @param head		the list to test.
 */
static inline int list_is_singular (const struct list_head *head)
{
	return !list_empty (head) && (head->next == head->prev);
}

static inline void __list_cut_position (struct list_head *list,
		struct list_head *head, struct list_head *entry)
{
	struct list_head *new_first = entry->next;
	list->next = head->next;
	list->next->prev = list;
	list->prev = entry;
	entry->next = list;
	head->next = new_first;
	new_first->prev = head;
}

/**
 * list_cut_position
 * 
 * Cut a list into two. This helper moves the initial part of 'head', up to and
 * including 'entry', from 'head' to 'list'. You should pass on 'entry' an
 * element you know is on 'head'. 'list' should be an empty list or a list you
 * do not care about losing its data.
 * 
 * @param list		a new list to add all removed entries
 * @param head		a list with entries
 * @param entry		an entry within head, could be the head itself and if so we
 * 					won't cut the list
 */
static inline void list_cut_position (struct list_head *list,
		struct list_head *head, struct list_head *entry)
{
	if (list_empty (head))
		return;
	if (list_is_singular (head) &&
		(head->next != entry && head != entry))
		return;
	if (entry == head)
		INIT_LIST_HEAD(list);
	else
		__list_cut_position (list, head, entry);
}

static inline void __list_splice(const struct list_head *list,
				 struct list_head *prev,
				 struct list_head *next)
{
	struct list_head *first = list->next;
	struct list_head *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

/**
 * list_splice
 * 
 * Join two lists, this is designed for stacks
 * 
 * @param list		the new list to add.
 * @param head		the place to add it in the first list.
 */
static inline void list_splice (const struct list_head *list,
				struct list_head *head)
{
	if (!list_empty (list))
		__list_splice (list, head, head->next);
}

/**
 * list_splice_tail
 * 
 * Join two lists, each list being a queue
 * 
 * @param list		the new list to add.
 * @param head		the place to add it in the first list.
 */
static inline void list_splice_tail (struct list_head *list,
				struct list_head *head)
{
	if (!list_empty (list))
		__list_splice (list, head->prev, head);
}

/**
 * list_splice_init
 * 
 * Join two lists and reinitialise the emptied list. The list at 'list' is
 * reinitialised.
 * 
 * @param list		the new list to add.
 * @param head		the place to add it in the first list.
 */
static inline void list_splice_init (struct list_head *list,
				    struct list_head *head)
{
	if (!list_empty (list)) {
		__list_splice (list, head, head->next);
		INIT_LIST_HEAD(list);
	}
}

/**
 * list_splice_tail_init
 * 
 * Join two lists and reinitialise the emptied list. Each of the lists is a
 * queue, the 'list' is reinitialised.
 * 
 * @param list		the new list to add.
 * @param head		the place to add it in the first list.
 */
static inline void list_splice_tail_init (struct list_head *list,
					 struct list_head *head)
{
	if (!list_empty (list)) {
		__list_splice (list, head->prev, head);
		INIT_LIST_HEAD(list);
	}
}

/******************************************************************************/

/* TODO: fixup comment format */

/**
 * list_entry
 * 
 * Get the struct for this entry.
 * 
 * @param ptr		the &struct list_head pointer
 * @param type		the type of the struct this is embedded in
 * @param member	the name of the list_struct within the struct.
*/
#define list_entry(ptr, type, member)	\
	container_of(ptr, type, member)

/**
 * list_first_entry - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

/**
 * list_last_entry - get the last element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

/**
 * list_first_entry_or_null - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 *
 * Note that if the list is empty, it returns NULL.
 */
#define list_first_entry_or_null(ptr, type, member) ({ \
	struct list_head *head__ = (ptr); \
	struct list_head *pos__ = READ_ONCE(head__->next); \
	pos__ != head__ ? list_entry(pos__, type, member) : NULL; \
})

/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; prefetch(pos->next), pos != (head); \
		pos = pos->next)

/**
 * __list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
 *
 * This variant differs from list_for_each() in that it's the
 * simplest possible list iteration code, no prefetching is done.
 * Use this for code that knows the list to be very short (empty
 * or 1 entry) most of the time.
 */
#define __list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_prev	-	iterate over a list backwards
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; prefetch(pos->prev), pos != (head); \
		pos = pos->prev)

/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:	the &struct list_head to use as a loop cursor.
 * @n:		another &struct list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * list_for_each_prev_safe - iterate over a list backwards safe against removal of list entry
 * @pos:	the &struct list_head to use as a loop cursor.
 * @n:		another &struct list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define list_for_each_prev_safe(pos, n, head) \
	for (pos = (head)->prev, n = pos->prev; \
	     prefetch(pos->prev), pos != (head); \
	     pos = n, n = pos->prev)

/**
 * list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry(pos, head, member)				\
	for (pos = list_entry((head)->next, typeof(*pos), member);	\
	     prefetch(pos->member.next), &pos->member != (head);	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry_reverse(pos, head, member)			\
	for (pos = list_entry((head)->prev, typeof(*pos), member);	\
	     prefetch(pos->member.prev), &pos->member != (head);	\
	     pos = list_entry(pos->member.prev, typeof(*pos), member))

/**
 * list_prepare_entry - prepare a pos entry for use in list_for_each_entry_continue()
 * @pos:	the type * to use as a start point
 * @head:	the head of the list
 * @member:	the name of the list_struct within the struct.
 *
 * Prepares a pos entry for use as a start point in list_for_each_entry_continue().
 */
#define list_prepare_entry(pos, head, member) \
	((pos) ? : list_entry(head, typeof(*pos), member))

/**
 * list_for_each_entry_continue - continue iteration over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define list_for_each_entry_continue(pos, head, member)			\
	for (pos = list_entry(pos->member.next, typeof(*pos), member);	\
	     prefetch(pos->member.next), &pos->member != (head);	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define list_for_each_entry_continue_reverse(pos, head, member)		\
	for (pos = list_entry(pos->member.prev, typeof(*pos), member);	\
	     prefetch(pos->member.prev), &pos->member != (head);	\
	     pos = list_entry(pos->member.prev, typeof(*pos), member))

/**
 * list_for_each_entry_from - iterate over list of given type from the current point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define list_for_each_entry_from(pos, head, member)			\
	for (; prefetch(pos->member.next), &pos->member != (head);	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_entry((head)->next, typeof(*pos), member),	\
		n = list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_continue
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define list_for_each_entry_safe_continue(pos, n, head, member)			\
	for (pos = list_entry(pos->member.next, typeof(*pos), member),		\
		n = list_entry(pos->member.next, typeof(*pos), member);		\
	     &pos->member != (head);						\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_from
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define list_for_each_entry_safe_from(pos, n, head, member)			\
	for (n = list_entry(pos->member.next, typeof(*pos), member);		\
	     &pos->member != (head);						\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_reverse
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define list_for_each_entry_safe_reverse(pos, n, head, member)		\
	for (pos = list_entry((head)->prev, typeof(*pos), member),	\
		n = list_entry(pos->member.prev, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = n, n = list_entry(n->member.prev, typeof(*n), member))

#endif /* __libkern_list_h__ */