#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#define DEVICE_PATH "/dev/xchardev"

#define XCDEV_IOC_MAGIC 'x'

#define XCDEV_IOC_GETFREEBLOCKS      _IOR(XCDEV_IOC_MAGIC, 1, unsigned)
#define XCDEV_IOC_ALLOCBLOCK         _IO(XCDEV_IOC_MAGIC, 2)
#define XCDEV_IOC_FREEBLOCK          _IO(XCDEV_IOC_MAGIC, 3)
#define XCDEV_IOC_GETALLOCATEDBLOCKS _IOR(XCDEV_IOC_MAGIC, 4, unsigned)
#define XCDEV_IOC_FREE_A_BLOCK       _IOW(XCDEV_IOC_MAGIC, 5, unsigned)
#define XCDEV_IOC_ALLOC_A_BLOCK      _IOW(XCDEV_IOC_MAGIC, 6, unsigned)
#define XCDEV_IOC_STAT               _IO(XCDEV_IOC_MAGIC, 7)

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"

static int total_tests = 0;
static int passed_tests = 0;
static int failed_tests = 0;

void record_test_result(int passed)
{
    total_tests++;
    if (passed) {
        passed_tests++;
    } else {
        failed_tests++;
    }
}

void print_test_summary(void)
{
    printf("\n");
    printf(COLOR_CYAN "========================================\n");
    printf(" Test Summary\n");
    printf("========================================\n" COLOR_RESET);
    printf("Total tests:  %d\n", total_tests);
    printf(COLOR_GREEN "Passed:       %d\n" COLOR_RESET, passed_tests);
    
    if (failed_tests > 0) {
        printf(COLOR_RED "Failed:       %d\n" COLOR_RESET, failed_tests);
    } else {
        printf(COLOR_GREEN "Failed:       %d\n" COLOR_RESET, failed_tests);
    }
    
    if (failed_tests == 0) {
        printf(COLOR_GREEN "\n✓ All tests PASSED!\n" COLOR_RESET);
    } else {
        printf(COLOR_YELLOW "\n⚠ Some tests FAILED\n" COLOR_RESET);
    }
}

void print_separator(const char *title)
{
    printf("\n");
    printf(COLOR_CYAN "========================================\n");
    printf(" %s\n", title);
    printf("========================================\n" COLOR_RESET);
}

void print_status(int fd)
{
    unsigned free_blocks = 0;
    unsigned allocated_blocks = 0;
    int ret;
    
    ret = ioctl(fd, XCDEV_IOC_GETFREEBLOCKS, &free_blocks);
    if (ret < 0) {
        printf(COLOR_RED "Failed to get free blocks\n" COLOR_RESET);
        return;
    }
    
    ret = ioctl(fd, XCDEV_IOC_GETALLOCATEDBLOCKS, &allocated_blocks);
    if (ret < 0) {
        printf(COLOR_RED "Failed to get allocated blocks\n" COLOR_RESET);
        return;
    }
    
    printf(COLOR_YELLOW "Status: " COLOR_RESET);
    printf("Free: %u, Allocated: %u\n", free_blocks, allocated_blocks);
}

int test_get_free_blocks(int fd)
{
    unsigned free_blocks;
    int ret;
    
    print_separator("Test 1: Get Free Blocks");
    
    ret = ioctl(fd, XCDEV_IOC_GETFREEBLOCKS, &free_blocks);
    if (ret < 0) {
        perror(COLOR_RED "ioctl GETFREEBLOCKS failed" COLOR_RESET);
        record_test_result(0);
        return -1;
    }
    
    printf(COLOR_GREEN "✓ Free blocks: %u\n" COLOR_RESET, free_blocks);
    record_test_result(1);
    return 0;
}

int test_get_allocated_blocks(int fd)
{
    unsigned allocated_blocks;
    int ret;
    
    print_separator("Test 2: Get Allocated Blocks");
    
    ret = ioctl(fd, XCDEV_IOC_GETALLOCATEDBLOCKS, &allocated_blocks);
    if (ret < 0) {
        perror(COLOR_RED "ioctl GETALLOCATEDBLOCKS failed" COLOR_RESET);
        record_test_result(0);
        return -1;
    }
    
    printf(COLOR_GREEN "✓ Allocated blocks: %u\n" COLOR_RESET, allocated_blocks);
    record_test_result(1);
    return 0;
}

int test_allocate_next_block(int fd)
{
    int ret;
    
    print_separator("Test 3: Allocate Next Available Block");
    
    printf("Before allocation: ");
    print_status(fd);
    
    ret = ioctl(fd, XCDEV_IOC_ALLOCBLOCK);
    if (ret < 0) {
        perror(COLOR_RED "ioctl ALLOCBLOCK failed" COLOR_RESET);
        record_test_result(0);
        return -1;
    }
    
    printf(COLOR_GREEN "✓ Successfully allocated next block\n" COLOR_RESET);
    
    printf("After allocation: ");
    print_status(fd);
    
    record_test_result(1);
    return 0;
}

int test_free_some_block(int fd)
{
    int ret;
    
    print_separator("Test 4: Free Some Block");
    
    printf("Before freeing: ");
    print_status(fd);
    
    ret = ioctl(fd, XCDEV_IOC_FREEBLOCK);
    if (ret < 0) {
        perror(COLOR_RED "ioctl FREEBLOCK failed" COLOR_RESET);
        record_test_result(0);
        return -1;
    }
    
    printf(COLOR_GREEN "✓ Successfully freed a block\n" COLOR_RESET);
    
    printf("After freeing: ");
    print_status(fd);
    
    record_test_result(1);
    return 0;
}

int test_allocate_specific_block(int fd, unsigned block_num)
{
    int ret;
    
    printf("\n" COLOR_MAGENTA "Test: Allocate Specific Block #%u\n" COLOR_RESET, block_num);
    
    printf("Before allocation: ");
    print_status(fd);
    
    ret = ioctl(fd, XCDEV_IOC_ALLOC_A_BLOCK, &block_num);
    if (ret < 0) {
        if (errno == EINVAL) {
            printf(COLOR_RED "✗ Invalid block number %u\n" COLOR_RESET, block_num);
        } else if (errno == EADDRINUSE) {
            printf(COLOR_RED "✗ Block %u already allocated\n" COLOR_RESET, block_num);
        } else if (errno == ENOMEM) {
            printf(COLOR_RED "✗ Out of memory\n" COLOR_RESET);
        } else {
            perror(COLOR_RED "ioctl ALLOC_A_BLOCK failed" COLOR_RESET);
        }
        record_test_result(0);
        return -1;
    }
    
    printf(COLOR_GREEN "✓ Successfully allocated block #%u\n" COLOR_RESET, block_num);
    
    printf("After allocation: ");
    print_status(fd);
    
    record_test_result(1);
    return 0;
}

int test_free_specific_block(int fd, unsigned block_num)
{
    int ret;
    
    printf("\n" COLOR_MAGENTA "Test: Free Specific Block #%u\n" COLOR_RESET, block_num);
    
    printf("Before freeing: ");
    print_status(fd);
    
    ret = ioctl(fd, XCDEV_IOC_FREE_A_BLOCK, &block_num);
    if (ret < 0) {
        if (errno == EINVAL) {
            printf(COLOR_RED "✗ Invalid block number %u\n" COLOR_RESET, block_num);
        } else {
            perror(COLOR_RED "ioctl FREE_A_BLOCK failed" COLOR_RESET);
        }
        return -1;
    }
    
    printf(COLOR_GREEN "✓ Successfully freed block #%u\n" COLOR_RESET, block_num);
    
    printf("After freeing: ");
    print_status(fd);
    
    return 0;
}

int test_allocate_multiple_specific(int fd)
{
    print_separator("Test 5: Allocate Multiple Specific Blocks");
    
    unsigned blocks[] = {0, 5, 10, 15, 20};
    int i;
    int freed_count = 0;
    
    for (i = 0; i < 5; i++) {
        int ret = ioctl(fd, XCDEV_IOC_FREE_A_BLOCK, &blocks[i]);
        if (ret == 0) {
            freed_count++;
        }
    }
    
    if (freed_count > 0) {
        printf(COLOR_YELLOW "Pre-test cleanup: Freed %d blocks that were already allocated\n" COLOR_RESET, freed_count);
    } else {
        printf(COLOR_GREEN "All target blocks are free - ready for clean test\n" COLOR_RESET);
    }
    printf("\n");
    
    for (i = 0; i < 5; i++) {
        test_allocate_specific_block(fd, blocks[i]);
    }
    
    return 0;
}

int test_free_multiple_specific(int fd)
{
    print_separator("Test 6: Free Multiple Specific Blocks");
    
    unsigned blocks[] = {5, 15};
    int i;
    
    for (i = 0; i < 2; i++) {
        test_free_specific_block(fd, blocks[i]);
    }
    
    return 0;
}

int test_double_allocate(int fd)
{
    print_separator("Test 7: Try to Allocate Already Allocated Block");
    
    unsigned block_num = 10;
    int ret;
    int test_passed = 0;
    
    printf("First allocation of block #%u:\n", block_num);
    ret = ioctl(fd, XCDEV_IOC_ALLOC_A_BLOCK, &block_num);
    if (ret < 0) {
        printf(COLOR_YELLOW "Block already allocated (will use for test)\n" COLOR_RESET);
    } else {
        printf(COLOR_GREEN "✓ Allocated block #%u\n" COLOR_RESET, block_num);
    }
    
    printf("\nSecond allocation of same block #%u:\n", block_num);
    ret = ioctl(fd, XCDEV_IOC_ALLOC_A_BLOCK, &block_num);
    if (ret < 0 && errno == EADDRINUSE) {
        printf(COLOR_GREEN "✓ Correctly rejected: block already in use\n" COLOR_RESET);
        test_passed = 1;
    } else if (ret < 0) {
        printf(COLOR_YELLOW "Got error: %s (expected EADDRINUSE)\n" COLOR_RESET, strerror(errno));
    } else {
        printf(COLOR_RED "✗ Error: allowed double allocation!\n" COLOR_RESET);
    }
    
    record_test_result(test_passed);
    return test_passed ? 0 : -1;
}

int test_invalid_block_number(int fd)
{
    print_separator("Test 8: Try Invalid Block Numbers");
    
    unsigned invalid_blocks[] = {3025, 9999, 65535};
    int i;
    int all_passed = 1;
    
    printf("Note: Valid block range is 0-1023 (1024 blocks total)\n\n");
    
    for (i = 0; i < 3; i++) {
        printf("Trying to allocate block #%u:\n", invalid_blocks[i]);
        int ret = ioctl(fd, XCDEV_IOC_ALLOC_A_BLOCK, &invalid_blocks[i]);
        if (ret < 0 && errno == EINVAL) {
            printf(COLOR_GREEN "✓ Correctly rejected: invalid block number\n" COLOR_RESET);
        } else if (ret < 0) {
            printf(COLOR_YELLOW "Got error: %s (expected EINVAL)\n" COLOR_RESET, strerror(errno));
            all_passed = 0;
        } else {
            printf(COLOR_RED "✗ Error: allowed invalid block!\n" COLOR_RESET);
            all_passed = 0;
        }
        printf("\n");
    }
    
    record_test_result(all_passed);
    return all_passed ? 0 : -1;
}

int test_allocate_sequential(int fd, int count)
{
    print_separator("Test 9: Allocate Multiple Sequential Blocks");
    
    int i;
    printf("Allocating %d blocks sequentially...\n", count);
    
    for (i = 0; i < count; i++) {
        int ret = ioctl(fd, XCDEV_IOC_ALLOCBLOCK);
        if (ret < 0) {
            printf(COLOR_RED "✗ Failed at block %d\n" COLOR_RESET, i);
            perror("Error");
            break;
        }
        if ((i + 1) % 5 == 0) {
            printf(COLOR_GREEN "✓ Allocated %d blocks\n" COLOR_RESET, i + 1);
        }
    }
    
    printf("\nFinal status:\n");
    print_status(fd);
    
    return 0;
}

int test_print_statistics(int fd)
{
    int ret;
    
    print_separator("Test 10: Print Allocator Statistics");
    
    printf("printing some allocator statistics...\n\n");
    
    ret = ioctl(fd, XCDEV_IOC_STAT);
    if (ret < 0) {
        perror(COLOR_RED "ioctl STAT failed" COLOR_RESET);
        record_test_result(0);
        return -1;
    }
    
    printf(COLOR_GREEN "✓ Statistics have been printed!\n" COLOR_RESET);
    
    record_test_result(1);
    return 0;
}

void print_usage(const char *prog_name)
{
    printf("  0  - Run all tests\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    int fd;
    int test_num = 0;
    
    printf(COLOR_CYAN);
    printf("========================================\n");
    printf(" Block Allocator Test \n");
    printf("========================================\n");
    printf(COLOR_RESET);
    
    // Parse command line
    if (argc > 1) {
        test_num = atoi(argv[1]);
    } else {
        print_usage(argv[0]);
        printf("Enter test number (0 for all): ");
        scanf("%d", &test_num);
    }
    
    // Open device
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror(COLOR_RED "Failed to open device" COLOR_RESET);
        printf("Make sure the kernel module is loaded!\n");
        return 1;
    }
    
    printf(COLOR_GREEN "✓ Device opened successfully\n" COLOR_RESET);
    
    // Show initial status
    printf("\n" COLOR_YELLOW "Initial Status:\n" COLOR_RESET);
    print_status(fd);
    
    // Run tests
    switch (test_num) {
        case 0:
            test_get_free_blocks(fd);
            test_get_allocated_blocks(fd);
            
            // Test basic allocation/free
            test_allocate_next_block(fd);
            test_allocate_next_block(fd);
            test_allocate_next_block(fd);
            test_free_some_block(fd);
            
            // Test specific allocation
            test_allocate_multiple_specific(fd);
            
            // Test error cases
            test_double_allocate(fd);
            test_invalid_block_number(fd);
            
            // Test specific free
            test_free_multiple_specific(fd);
            
            // Test bulk allocation
            test_allocate_sequential(fd, 10);
            
            // Print statistics at the end
            test_print_statistics(fd);
            break;
            
        case 1:
            test_get_free_blocks(fd);
            break;
            
        case 2:
            test_get_allocated_blocks(fd);
            break;
            
        case 3:
            test_allocate_next_block(fd);
            break;
            
        case 4:
            test_free_some_block(fd);
            break;
            
        case 5:
            test_allocate_multiple_specific(fd);
            break;
            
        case 6:
            test_free_multiple_specific(fd);
            break;
            
        case 7:
            test_double_allocate(fd);
            break;
            
        case 8:
            test_invalid_block_number(fd);
            break;
            
        case 9:
            test_allocate_sequential(fd, 10);
            break;
            
        case 10:
            test_print_statistics(fd);
            break;
            
        default:
            printf(COLOR_RED "Invalid test number: %d\n" COLOR_RESET, test_num);
            print_usage(argv[0]);
            close(fd);
            return 1;
    }
    
    printf("\n" COLOR_YELLOW "Final Status:\n" COLOR_RESET);
    print_status(fd);
    
    close(fd);
    

    print_test_summary();
    
    printf("\n" COLOR_GREEN);
    printf("========================================\n");
    printf(" Testing Complete!\n");
    printf("========================================\n");
    printf(COLOR_RESET);
    
    return (failed_tests > 0) ? 1 : 0;
}
