#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define XCDEV_IOC_MAGIC 'x'
#define XCDEV_IOC_GETFREEBLOCKS _IOR(XCDEV_IOC_MAGIC, 1, unsigned)
#define XCDEV_IOC_ALLOCBLOCK    _IO(XCDEV_IOC_MAGIC, 2)
#define XCDEV_IOC_RESET         _IO(XCDEV_IOC_MAGIC, 3)

int main() {
	int fd;
	unsigned free_blocks;
	int ret;
	
	fd = open("/dev/xchardev", O_RDWR);
	if (fd < 0) {
		perror("open");
		return 1;
	}
	
	ret = ioctl(fd, XCDEV_IOC_GETFREEBLOCKS, &free_blocks);
	if (ret < 0) {
		perror("ioctl GETFREEBLOCKS");
		close(fd);
		return 1;
	}
	printf("Free blocks: %u\n", free_blocks);
	
	ret = ioctl(fd, XCDEV_IOC_ALLOCBLOCK);
	if (ret < 0) {
		perror("ioctl ALLOCBLOCK");
		close(fd);
		return 1;
	}
	printf("Block allocated successfully\n");
	
	ret = ioctl(fd, XCDEV_IOC_GETFREEBLOCKS, &free_blocks);
	printf("Free blocks after allocation: %u\n", free_blocks);
	
	close(fd);
	return 0;
}
