/* Benötigter Header */
#include <linux/kobject.h>

static ssize_t mydev_show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
	return sprintf(buffer, "Hello world!\n");
}

static ssize_t mydev_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
	printk("I got %s\n", buffer);
	return count;
}

static struct kobj_attribute mydev_attr = __ATTR(my_attr, 0660, mydev_show, mydev_store);

struct kobject * my_kobj */
/* in init oder probe Funktion */
int status;

my_kobj = kobject_create_and_add("my_kobj", my_kobj);
if (!my_kobj) {
	printk("Error creating kernel object\n");
	return -ENOMEM;
}

status = sysfs_create_file(my_kobj, &mydev_attr.attr);
if (status) {
	printk("Error creating /sys/my_kobj/my_attr\n");
	return status;
}

/* in exit oder remove Funktion */
sysfs_remove_file(my_kobj, &mydev_attr.attr);
kobject_put(my_kobj);
