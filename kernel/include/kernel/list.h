#ifndef _KERNEL_LIST_H
#define _KERNEL_LIST_H
#include <stdbool.h>

struct list_head {
	struct list_head* next;
	struct list_head* previous;
};

#define LIST_HEAD_INIT(head) (struct list_head){ .next=&(head), .previous=&(head)}

/* Returns a pointer to the structure "type" that contains a list_head* "headp" in "member". */
#define LIST_ENTRY(headp, type, member) \
	((type*)((void*)(headp)-(uintptr_t)(&((type*)0)->member)))

#define LIST_FOR_EACH(/*struct list_head* */cursor, /*struct list_head* */list) \
	for(cursor = (list)->next; cursor != (list); cursor = cursor->next)

/* Returns true if the list is empty. */
bool list_empty(struct list_head *head);

/* Adds the new entry immediately after the list head — normally at the beginning of the list. */
void list_add(struct list_head *head, struct list_head *new);


#endif