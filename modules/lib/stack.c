#include "./inc/stack.h"
#include <linux/slab.h>

struct list_head stack_head;

void stack_init(void){
		INIT_LIST_HEAD(&stack_head);
}

int stack_push(int value)
{
	struct stack_entry *new_stack_node = kmalloc(sizeof(struct stack_entry), GFP_KERNEL);
	if (!new_stack_node)
		return STACK_NOMEM;
	new_stack_node->data = value;
	INIT_LIST_HEAD(&new_stack_node->list);
	list_add(&new_stack_node->list, &stack_head);
	return 0;
}

int stack_peek(int *val) {
	if (!val) {
        return -EINVAL;
    }
    if (list_empty(&stack_head)) {
        return STACK_EMPTY;
    }
    struct stack_entry *top = list_first_entry(&stack_head, struct stack_entry, list);
	*val = top->data;
    return 0;
}
