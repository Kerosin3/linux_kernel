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

//---------- size --------------
ssize_t size_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
	return sprintf(buf,"%u\n",size_val);
}

static struct kobj_attribute size = __ATTR_RO(size);

//---------- peek --------------
ssize_t peek_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
	int peeked_value;
	int ret = stack_peek(&peeked_value);
	if (ret)
		return ret;
	return sprintf(buf,"%d\n",peeked_value);
}

static struct kobj_attribute peek = __ATTR_RO(peek);

//------------------------------

static struct attribute *attrs[] = {
    &push.attr,
    &size.attr,
    &peek.attr,
    NULL,
};

struct attribute_group attr_group = {
    .attrs = attrs,
};

