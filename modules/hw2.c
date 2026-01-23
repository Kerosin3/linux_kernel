#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/limits.h>

// max array size
#define MAX_SIZE 512
// writable size
#define WRITE_LIMIT (MAX_SIZE - 1)

// array for storing chars
unsigned char my_char_array[MAX_SIZE];

// array index setup
static uint idx = UINT_MAX;
MODULE_PARM_DESC(idx, "index in the char aray to write");

static int set_index(const char *val, const struct kernel_param *kp){
	int ret;
	ret = kstrtouint(val, 10, &idx);
	if (ret)
        return ret;
	if (idx == UINT_MAX) {
        pr_err("Error: 'idx' must be set explicitly before writing to 'ch_val'.\n");
        return -EINVAL;
    }
	if (idx >= WRITE_LIMIT){
        pr_err("Error: Specified index should be less than %u.\n", WRITE_LIMIT - 1);
		return -EINVAL;
	}
	pr_info("set index to write : %u\n",idx);
	return 0;
}

static int get_index(char *buf, const struct kernel_param *kp){
	return sprintf(buf, "%u", idx);
}

static const struct kernel_param_ops set_index_params = {
	.set = set_index,
	.get = get_index,
};

module_param_cb(idx, &set_index_params, &idx, 0660);

// ASCII code to write in dec
static unsigned char ch_val;
MODULE_PARM_DESC(ch_val, "ASCII code to write");

static int set_char(const char *val, const struct kernel_param *kp){
	int ret;
	ret = kstrtou8(val, 10, &ch_val);
	if (ret)
        return ret;
	if (idx == UINT_MAX) {
        pr_err("Error: 'idx' must be set explicitly before writing to 'ch_val'.\n");
        return -EINVAL;
    }
	if (idx >= WRITE_LIMIT){
        pr_err("Error: addressable index should be less than %u.\n", WRITE_LIMIT - 1);
		return -EINVAL;
	}

	my_char_array[idx] = ch_val;
	pr_info("written %c to %u \n", ch_val, idx);
	return 0;
}

static int get_char(char *buf, const struct kernel_param *kp){
	return sprintf(buf, "%u", ch_val);
}

static const struct kernel_param_ops set_char_params = {
	.set = set_char,
	.get = get_char,
};

module_param_cb(ch_val, &set_char_params, &ch_val, 0660);

//array to store
static int get_my_str(char *buf, const struct kernel_param *kp) {
	const unsigned char *start = my_char_array;
	while (*start == '\0' && start < my_char_array + MAX_SIZE)
        start++;
	size_t remaining_len = my_char_array + MAX_SIZE - start;
    return scnprintf(buf, PAGE_SIZE, "%.*s\n", (int)remaining_len, start);
}

static const struct kernel_param_ops my_str_ops = {
    .get = get_my_str,
};

module_param_cb(my_str, &my_str_ops, NULL, 0444);
MODULE_PARM_DESC(my_str, "char array (read-only)");

static int __init hello_init(void) {
	pr_info("%s: init: Module loaded!\n", KBUILD_MODNAME);
	my_char_array[WRITE_LIMIT] = '\0'; // explicitly null terminated for clarity
    return 0;
}

static void __exit hello_exit(void) {
	pr_info("%s: exit: Module unloaded!\n", KBUILD_MODNAME);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alex V");
MODULE_DESCRIPTION("Homework 2");
MODULE_VERSION("1.0");
