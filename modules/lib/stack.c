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
		pr_err("%s:%s stack is empty! id: %lu\n", KBUILD_MODNAME, __func__, jiffies);
        return STACK_EMPTY;
    }
    struct stack_entry *top = list_first_entry(&stack_head, struct stack_entry, list);
	*val = top->data;
    return 0;
}

int stack_pop(int *val) {
	if (!val) {
        return -EINVAL;
    }
    if (list_empty(&stack_head)) {
		pr_err("%s:%s stack is empty! id: %lu\n", KBUILD_MODNAME, __func__, jiffies);
        return STACK_EMPTY;
    }
    struct stack_entry *top = list_first_entry(&stack_head, struct stack_entry, list);
	*val = top->data;
	list_del(&top->list);
    kfree(top);
	top = NULL;
    return 0;
}

int stack_is_empty(void){
	return list_empty(&stack_head);
}

void stack_clear(void){
	struct stack_entry *node;
    struct list_head *pos, *tmp;
    list_for_each_safe(pos, tmp, &stack_head) {
        node = list_entry(pos, struct stack_entry, list);
        list_del(pos);
        kfree(node);
    }
}

size_t stack_size(void){
	size_t size = 0;
    struct list_head *pos, *tmp;
    if (list_empty(&stack_head)) {
        return 0;
    }
    list_for_each_safe(pos, tmp, &stack_head)
		size++;
	return size;
}
