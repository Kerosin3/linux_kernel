#include "kernel_stack.h"

//---------- push --------------
ssize_t push_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
	return sprintf(buf,"%d\n",push_val);
}

ssize_t push_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
	int ret = kstrtoint(buf, 10, &push_val);
    if (ret < 0) {
        return ret;
    }
	stack_push(push_val); // push value to stack
	return count;
}

static struct kobj_attribute push = __ATTR_RW(push);

//---------- peek --------------
ssize_t peek_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
	int ret = stack_peek(&peek_val);
	if (ret){
		return ret;
	}
	return sprintf(buf,"%d\n",peek_val);
}

static struct kobj_attribute peek = __ATTR_RO(peek);

//---------- pop --------------

ssize_t pop_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
	int ret = stack_pop(&pop_val);
	if (ret)
		return ret;
	return sprintf(buf,"%d\n",pop_val);
}

static struct kobj_attribute pop = __ATTR_RO(pop);

//---------- is_empty --------------

ssize_t is_empty_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
	is_empty_val = stack_is_empty();
	return sprintf(buf,"%d\n", is_empty_val);
}

static struct kobj_attribute is_empty = __ATTR_RO(is_empty);

//---------- clear --------------

ssize_t clear_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
	int ret = kstrtoint(buf, 10, &clear_val);
    if (ret < 0) {
        return ret;
    }
	if (clear_val != 1)
		return -EINVAL;
	stack_clear();
	return count;
}

static struct kobj_attribute clear = __ATTR_WO(clear);

//---------- size --------------

ssize_t size_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
	size_val = stack_size();
	return sprintf(buf,"%d\n", size_val);
}

static struct kobj_attribute size = __ATTR_RO(size);

//------------------------------

static struct attribute *attrs[] = {
    &push.attr,
    &peek.attr,
    &pop.attr,
    &is_empty.attr,
    &clear.attr,
    &size.attr,
    NULL,
};

struct attribute_group attr_group = {
    .attrs = attrs,
};

