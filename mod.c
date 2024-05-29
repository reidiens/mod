#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>

static int    __init 	start(void);
static void   __exit 	stop(void);
static int 		open(struct inode *inode, struct file *file);
static int 		close(struct inode *inode, struct file *file);
static ssize_t 		read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t 		write(struct file *filp, const char *buf, size_t len, loff_t *off);
static int 		get_free_bytes(void);

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = read,
	.write = write, 
	.open = open,
	.release = close,
};

dev_t ruben_dev = 0;
static struct class *dev;
static struct cdev cdev;
static char cbuf[1000];
static char *bufp = cbuf;

static int __init start(void) {
	if (alloc_chrdev_region(&ruben_dev, 0, 1, "ruben") < 0) {
		pr_err("could not alloc device number!\n");
		return -1;
	}
	pr_info("major: %d, minor: %d", MAJOR(ruben_dev), MINOR(ruben_dev));

	cdev_init(&cdev, &fops);

	if (cdev_add(&cdev, ruben_dev, 1) < 0) {
		pr_err("could not create cdev!\n");
		unregister_chrdev_region(ruben_dev, 1);
		return -1;
	}

	dev = class_create("ruben");
	if (IS_ERR(dev)) {
		pr_err("could not create device class!\n");
		unregister_chrdev_region(ruben_dev, 1);
		return -1;
	}

	if (IS_ERR(device_create(dev, NULL, ruben_dev, NULL, "ruben"))) {
		pr_err("could not create device!\n");
		class_destroy(dev);
		unregister_chrdev_region(ruben_dev, 1);
		return 1;
	}

	pr_notice("device created successfully\n");
	return 0;
}

static void __exit stop(void) {
	device_destroy(dev, ruben_dev);
	class_destroy(dev);
	cdev_del(&cdev);
	unregister_chrdev_region(ruben_dev, 1);
	pr_notice("device destroyed successfully\n");
}

static int open(struct inode *indoe, struct file *file) {
	pr_info("opened\n");
	return 0;
}

static int close(struct inode *inode, struct file *file) {
	pr_info("closed\n");
	return 0;
}

static ssize_t read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
	int count = 0;
	int index = bufp - cbuf;
		
	bufp = cbuf;

	for (int i = 0; i < index; i++) {
		buf[i] = cbuf[i];
		cbuf[i] = '\0';

		count++;
	}
	
	pr_info("read %d bytes\n", count);
	pr_info("buffer restored to full %d bytes", get_free_bytes());
	return count;
}

static ssize_t write(struct file *filp, const char *buf, size_t len, loff_t *off) {
	int count = 0;
	for (int i = 0; i < len; i++) {
		*bufp = buf[i];
		bufp++;
		count++;
	}

	pr_info("wrote: %s", cbuf);
	pr_info("%d bytes. %d bytes remaining in buffer", count, get_free_bytes());

	return count;
}

static int get_free_bytes(void) {
	int count = 0;
	for (int i = 0; i < sizeof(cbuf) / sizeof(char); i++) {
		if (cbuf[i] == '\0')
			count++;
	}
	return count;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ruben Toledo <tibamita@tutamail.com");

module_init(start);
module_exit(stop); 
