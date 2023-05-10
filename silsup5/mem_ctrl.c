#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define DEVICE_NAME "mem_ctrl"

static struct class *cls;
int major;

char **ptr;
int idx;

int mem_ctrl_open(struct inode *, struct file *);
int mem_ctrl_release(struct inode *, struct file *);
ssize_t mem_ctrl_write(struct file *, const char *, size_t, loff_t *);
ssize_t mem_ctrl_read(struct file *, char *, size_t, loff_t *);

static struct file_operations mem_ctrl_fops = { 
    .open = mem_ctrl_open,
    .release = mem_ctrl_release,
    .write = mem_ctrl_write,
    .read = mem_ctrl_read,
};

int mem_ctrl_open(struct inode *minode, struct file *mfile) {
	ptr = kmalloc(sizeof(char *) * 2, GFP_KERNEL);
	idx = 0;	
	//printk("open!!!!!%d\n",idx);
	return 0;
}

int mem_ctrl_release(struct inode *minode, struct file *mfile) {
	
	
	
	kfree(ptr);

	return 0;
}

ssize_t mem_ctrl_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {

	if(idx == 0){
		ptr[0] = kmalloc(sizeof(char) * length, GFP_KERNEL);
		if (copy_from_user(ptr[0], gdata, length)) {
			return -EFAULT;
		}	
	}
	else if(idx ==1){
		ptr[1] = kmalloc(sizeof(char) * length, GFP_KERNEL);

		if (copy_from_user(ptr[1], gdata, length)) {
			return -EFAULT;
		}
	}
	idx = (idx+1)%2;
	//printk("mem_ctrl_write!!!!!%d\n",idx);
	
	return 1;
}


ssize_t mem_ctrl_read(struct file *inode, char *data, size_t length, loff_t *off_what) {
	
	if(idx == 0){
		
		if(copy_to_user(data, ptr[0], length)){
		return -EFAULT;
		}
		kfree(ptr[0]);
	}
	
	else if(idx ==1){
		
		if(copy_to_user(data, ptr[1], length)){
		return -EFAULT;
		}
		kfree(ptr[1]);
	}
	idx = (idx+1)%2;
	return 1;
}


int __init mem_ctrl_init(void) {
	major = register_chrdev(0, DEVICE_NAME, &mem_ctrl_fops);
	if(major < 0) {
		printk( "error %d\n",major);
		return major;
	}
	printk("dev_file: /dev/%s , major: %d\n", DEVICE_NAME, major);	
	printk("init module\n");

	cls = class_create(THIS_MODULE, DEVICE_NAME);
	device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);

	return 0;
}
void __exit mem_ctrl_exit(void) {
   	printk("mem_ctrl_exit\n");
	device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, DEVICE_NAME);
}


module_init(mem_ctrl_init); 
module_exit(mem_ctrl_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("author");
