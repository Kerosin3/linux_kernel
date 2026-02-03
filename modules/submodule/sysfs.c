#include "sysfs.h"

static ssize_t sorted_files_show(struct kobject *kobj,
				 struct kobj_attribute *attr, char *buf)
{
	ssize_t len = 0;
	len = print_sorted_filenames(buf, len);
	return len;
}

static struct kobj_attribute sorted_files_attr = __ATTR_RO(sorted_files);

int sysfs_init(void)
{
	int ret;

	if (!kobj) {
		return -EINVAL;
	}

	ret = sysfs_create_file(kobj, &sorted_files_attr.attr);
	if (ret) {
		goto cleanup_kobj;
	}

	pr_info("Sysfs: /sys/kernel/path_scanner/sorted_files was created \n");
	return 0;

cleanup_kobj:
	kobject_put(kobj);
	kobj = NULL;
	return ret;
}

void sysfs_exit(void)
{
	sysfs_remove_file(kobj, &sorted_files_attr.attr);
	kobject_put(kobj);
}
