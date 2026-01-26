#ifndef STACK_H
#define STACK_H

#include <linux/list.h>

#define STACK_OK 0 /* Операция успешна */
#define STACK_EMPTY -1 /* Стек пуст */
#define STACK_NOMEM -2 /* Нет памяти */
#define STACK_INVALID -3 /* Неверный параметр */

//stack entry
struct stack_entry {
	int data;
	struct list_head list;
};

extern struct list_head stack_head;

void stack_init(void);
//
int stack_push(int);
//
int stack_pop(int *val);
//
int stack_peek(int *val);
//
int stack_is_empty(void);
//
size_t stack_size(void);
//
void stack_clear(void);

#endif
