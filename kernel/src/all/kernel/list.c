#include <kernel/list.h>

bool list_empty(struct list_head *head) {
	return head->next == head->previous;
}

void list_add(struct list_head *head, struct list_head *new) {
	struct list_head *after_head = head->next;
	head->next = new;
	new->next = after_head;
	after_head->previous = new;
	new->previous = head;
}
