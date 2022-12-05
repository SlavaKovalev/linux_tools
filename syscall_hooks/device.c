#include <linux/miscdevice.h>
#include <linux/fs.h>
#include "transport.h"
#include "transport_protocol.h"

static const struct file_operations operations = {
	.owner		= THIS_MODULE,
	.open		= transport_device_open,
	.llseek		= no_llseek,
	.read		= transport_device_read,
	.write		= transport_device_write,
	.release	= transport_device_release,
};

static struct miscdevice miscdevice = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = TRANSPORT_DEVICE_NAME,
	.fops = &operations,
};

int device_mod_init(void) {
	int error = misc_register(&miscdevice);
	if (error != 0)
	{
		printk(KERN_ERR "misc_register failed with error %d", error);
	}
	return error;
}

void device_mod_down(void) {
	misc_deregister(&miscdevice);
}
