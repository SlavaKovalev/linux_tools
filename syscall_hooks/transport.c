#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/string.h>
#include "transport.h"
#include "message.h"
#include "device.h"
#include "syscall_hook.h"

typedef struct
{
	struct message_t* msg;
	size_t open_count;
	struct mutex open_close_mutex;
} transport_t;

static void transport_free(transport_t *transport)
{
	kfree(transport);
}

static transport_t *transport_new(void)
{
	transport_t *transport = kmalloc(sizeof(transport_t), GFP_KERNEL);
	if (transport) {
		mutex_init(&transport->open_close_mutex);
		transport->open_count = 0;

		//spin_lock_init(&transport->msg_spinlock);

		//spin_lock_init(&transport->sent_spinlock);
	}
	return transport;
}

static transport_t* transport_default;

int transport_mod_init(void)
{
	int ret = 0;
	transport_default = transport_new();
	if (!transport_default) {
		printk(KERN_ERR "Can't allocate memory for transport");
		return -ENOMEM;
	}

	ret = device_mod_init();
	if (ret) {
		transport_free(transport_default);
		transport_default = NULL;
	}
	return ret;
}

void transport_mod_down(void)
{
	device_mod_down();
	transport_free(transport_default);
	transport_default = NULL;
}

int transport_device_open(struct inode *inode, struct file *filp)
{
	int err = 0;
	if (filp->f_flags & O_NONBLOCK) {
		printk(KERN_ERR "O_NONBLOCK style of file operations not supported");
		return -EINVAL;
	}

	if (!transport_default) {
		printk(KERN_ERR "fatal error! Transport is not initialized!");
		return ENODEV;
	}

	mutex_lock(&transport_default->open_close_mutex);
	{
		if (!transport_default->open_count) { 
			if ((err = syscall_hook_activate())) {
				printk(KERN_ERR "Can't hook syscalls");
				goto exit;
			}
			filp->private_data = transport_default;
		}
		++transport_default->open_count;
	}
exit:
	mutex_unlock(&transport_default->open_close_mutex);
	return err;
}

ssize_t transport_device_read(struct file *filp, char __user *user_data, size_t size, loff_t *offset)
{
	return 0;
}

ssize_t transport_device_write(struct file *filp, const char __user *user_data, size_t size, loff_t *offset)
{
	return 0;
}

int transport_device_release(struct inode *inode, struct file *filp)
{
	transport_t *transport = filp->private_data;
	if (!transport) {
		printk(KERN_ERR "Closing device error. Transport is not initialized!");
		return ENODEV;
	}
	
	mutex_lock(&transport->open_close_mutex);
	if (!--transport->open_count) {
		syscall_hook_deactivate();
		filp->private_data = NULL;
	}
	mutex_unlock(&transport->open_close_mutex);
	return 0;
}

