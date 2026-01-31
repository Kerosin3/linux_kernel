#ifndef KFIFO
#define KFIFO

#include <linux/gfp.h>
#include <linux/mm.h>
#include <linux/kfifo.h>

#define FIFO_OK          0      /* Операция успешна */
#define FIFO_EMPTY      -1      /* Очередь пуста */
#define FIFO_FULL       -2      /* Очередь полна */
#define FIFO_NOMEM      -3      /* Нет памяти */
#define FIFO_INVALID    -4      /* Неверный параметр */

#define BUFFER_ORDER 0
#define BUFFER_SIZE (PAGE_SIZE << BUFFER_ORDER)

/* kfifo */
static struct kfifo my_fifo;

/* page of memory for store */
static void* kbuffer;

/* initialize the fifo */
int fifo_init(int);

/* clean the memory */
void fifo_cleanup(void);

/* enqueue an element */

int fifo_enqueue(int);

#endif
