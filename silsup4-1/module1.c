#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

struct my_data{
	int a,b,c;
};

long newcall(struct my_data *data){
	int a, b;

	a=data->a;
	b=data->b;

	data->a=a+b;
	data->b=a-b;
	data->c=a%b;

	return 313;
}
void mynew_function(int Id){
	int a, b, c, d;
	a = Id / 1000;
	b = Id %1000 / 100;
	c = Id % 1000 %100 / 10;
	d = Id %10;
	printk("Thousands = %d / hundreds = %d / tens = %d / units = %d\n",a,b,c,d);
}



int funcdev_init( void ) {
	return 0;
}
void funcdev_exit( void ) { }

EXPORT_SYMBOL( newcall );
EXPORT_SYMBOL( mynew_function);

module_init ( funcdev_init );
module_exit ( funcdev_exit );
MODULE_LICENSE("Dual BSD/GPL");
