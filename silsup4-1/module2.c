#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

struct my_data{	int a,b,c;
};

extern long newcall(struct my_data *block);
extern void mynew_function(int id);
int calldev_init(void){
	int id = 1664;
mynew_function(id);
	
	return 0;
}

void calldev_exit( void ) { }
module_init ( calldev_init );
module_exit ( calldev_exit );
MODULE_LICENSE("Dual BSD/GPL");
