#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/kernel.h>

int new_device_open(struct inode *inode, struct file *filp){
  printk("[new_device] open");
  return 0;
}

static const struct file_operations ops = {
  .open = new_device_open
};

static struct miscdevice miscdevice = {
  .minor = MISC_DYNAMIC_MINOR,
  .fops = &ops
};



static int  __init myinit(void) {
  int status = 0;
  printk("%s\n","[new_device] init");
  status = misc_register(&miscdevice);
  printk("[new_device] misc_register returned %d\n", status);
  return status;
}

static void __exit myexit(void) {
  printk("%s\n", "[new_device] exit");
  misc_deregister(&miscdevice);
}

module_init(myinit);
module_exit(myexit);

MODULE_LICENSE("GPL");
