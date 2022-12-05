#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__

int transport_mod_init(void);
void transport_mod_down(void);

int	transport_device_open(struct inode *, struct file *);
long	transport_device_ioctl(struct file *, unsigned int, unsigned long);
ssize_t	transport_device_read(struct file *, char __user *, size_t, loff_t *);
ssize_t	transport_device_write(struct file *, const char __user *, size_t, loff_t *);
int	transport_device_release(struct inode *, struct file *);

#endif
