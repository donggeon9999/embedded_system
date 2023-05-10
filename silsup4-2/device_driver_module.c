#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>

#define DEVICE_DRIVER_NAME "driver_test"

long number = 0;
int major_number;


int test_device_driver_open(struct inode *, struct file *);
int test_device_driver_release(struct inode *, struct file *);
ssize_t test_device_driver_write(struct file *, const char *, size_t, loff_t *);
ssize_t test_device_driver_read(struct file *, const char *, size_t, loff_t *);


static struct file_operations device_driver_fops =
{ .open = test_device_driver_open, .write = test_device_driver_write,.read = test_device_driver_read, .release = test_device_driver_release };

int test_device_driver_open(struct inode *minode, struct file *mfile) {
	printk("test_device_driver_open\n");
	return 0;
}

int test_device_driver_release(struct inode *minode, struct file *mfile) {
	printk("test_device_driver_release\n");
	return 0;
}


ssize_t test_device_driver_read(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {

	const char * tmp = gdata, *tmp2;
	char kernel_buff[3];
	printk("read\n");
	int i;
	int num = number;
	int sum = 0;
	int size;
	for(i = 0;i<length;i++)
	{
		sum += num%10;
		num = num/10;
		if(num == 0)
			break;
	}
	if(sum>=10)
	{
		kernel_buff[0] = sum /10 + '0';
		kernel_buff[1] = sum %10 + '0';
		kernel_buff[2] = '\0';
		size = 3;
	}
	else
	{
		kernel_buff[0] = sum + '0';
		kernel_buff[1] = '\0';
		size = 2;
	}
	tmp2 = kernel_buff;
	copy_to_user(tmp, tmp2,size);
	printk("Rst: %s\n",tmp2);
	return 1;

	


}
ssize_t test_device_driver_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {
	const char *tmp = gdata;
	char kernel_buff[4];


	printk("write\n");
	if (copy_from_user(&kernel_buff, tmp, 4)) {
		return -EFAULT;
	}
	number = simple_strtol(kernel_buff, NULL, 10);
	printk("Number: %ld \n",number);

	return 1;
}

int __init device_driver_init(void)
{

	printk("device_driver_init\n");
	
	major_number = register_chrdev(0, DEVICE_DRIVER_NAME, &device_driver_fops);
	if(major_number <0) {
		printk( "error %d\n",major_number);
		return major_number;
	}

    printk( "dev_file: /dev/%s , major: %d\n", DEVICE_DRIVER_NAME, major_number);
	
	printk("init module\n");
	return 0;
}

void __exit device_driver_exit(void)
{
	printk("device_driver_exit\n");

	unregister_chrdev(major_number, DEVICE_DRIVER_NAME);
}

module_init(device_driver_init);
module_exit(device_driver_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("author");
