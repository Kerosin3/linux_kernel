#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define DEVICE    "/dev/rust_mmap"
#define PAGES     4
#define PAGE_SIZE 4096
#define BUF_SIZE  (PAGES * PAGE_SIZE)

int main(void)
{
    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    uint8_t *mem = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (mem == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    printf("mapped %d pages at %p\n\n", PAGES, (void *)mem);

    // распечатаем страницы
    for (int p = 0; p < PAGES; p++) {
        uint8_t *page = mem + p * PAGE_SIZE;
        uint32_t magic = *(uint32_t *)page;

        printf("page[%d]  magic=0x%08x  data: ", p, magic);
        for (int i = 4; i < 20; i++)
            printf("%02x ", page[i]);
        printf("\n");
    }

    // запись и чтение 
    const char *msg = "hello from userspace";
    printf("\nwrite [%s] to page[1]... ", msg);
    memcpy(mem + PAGE_SIZE, msg, strlen(msg) + 1);
    printf("read back: \"%s\"\n", (char *)(mem + PAGE_SIZE));
    sleep(0.1);
    munmap(mem, BUF_SIZE);
    return 0;
}
