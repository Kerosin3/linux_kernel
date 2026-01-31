#ifndef KFIFO
#define KFIFO

#include <linux/gfp.h>
#include <linux/mm.h>
#include <linux/kfifo.h>
#include <linux/mutex.h>

#define FIFO_OK 0 /* Операция успешна */
#define FIFO_EMPTY -1 /* Очередь пуста */
#define FIFO_FULL -2 /* Очередь полна */
#define FIFO_NOMEM -3 /* Нет памяти */
#define FIFO_INVALID -4 /* Неверный параметр */

static DEFINE_MUTEX(fifo_mutex);

/* kfifo of int elements */
typedef STRUCT_KFIFO_PTR(int) my_fifo_t;

extern my_fifo_t my_fifo;

/* ptr beginning of memory page for store */
static void *kbuffer;

/* initialize the fifo with required number of ELEMENTS (ints) */
int fifo_init(size_t);

void fifo_cleanup(void);

int fifo_enqueue(int);

int fifo_dequeue(int *);

int fifo_is_empty(void);

int fifo_is_full(void);

unsigned fifo_clen(void);

unsigned fifo_available(void);

int fifo_peek(int *);

void fifo_clear(void);

#endif
