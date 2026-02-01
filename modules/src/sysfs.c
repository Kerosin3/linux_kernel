#include "sysfs.h"

static ssize_t sorted_files_show(struct kobject *kobj,
				 struct kobj_attribute *attr, char *buf)
{
	return 1;
}

static ssize_t scan_path_store(struct kobject *kobj,
			       struct kobj_attribute *attr, const char *buf,
			       size_t count)
{
	char path[PATH_MAX_LENGTH];
	int ret;
	strscpy(path, buf, sizeof(path));
	strim(path);

	pr_info("Scanning directory: %s\n", path);

	ret = scan_directory(path);
	if (ret < 0) {
		pr_err("Failed to scan directory: %d\n", ret);
		return ret;
	}
	return count;
}

static struct kobj_attribute scan_path_attr = __ATTR_WO(scan_path);
static struct kobj_attribute sorted_files_attr = __ATTR_RO(sorted_files);

int sysfs_init(void)
{
	int ret;

	kobj = kobject_create_and_add("path_scanner", kernel_kobj);
	if (!kobj) {
		return -ENOMEM;
	}

	ret = sysfs_create_file(kobj, &scan_path_attr.attr);
	if (ret) {
		goto cleanup_kobj;
	}

	ret = sysfs_create_file(kobj, &sorted_files_attr.attr);
	if (ret) {
		sysfs_remove_file(kobj, &scan_path_attr.attr);
		goto cleanup_kobj;
	}

	pr_info("Sysfs: /sys/kernel/path_scanner/ created with attributes\n");
	return 0;

cleanup_kobj:
	kobject_put(kobj);
	kobj = NULL;
	return ret;
}

void sysfs_exit(void)
{
	sysfs_remove_file(kobj, &scan_path_attr.attr);
	sysfs_remove_file(kobj, &sorted_files_attr.attr);
	kobject_put(kobj);
}
