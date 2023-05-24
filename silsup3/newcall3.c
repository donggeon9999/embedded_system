#include <linux/kernel.h>
#include <linux/uaccess.h>

struct mystruct2 {
        int num_front;
        int num_rear;
};

asmlinkage int sys_newcall3(struct mystruct2 *dd) {
        struct mystruct2 my_st;
        copy_from_user(&my_st, dd, sizeof(my_st));

        int front = my_st.num_front;
        int rear = my_st.num_rear;
        printk("sys_newcall3 : student ID is %d %d, sum is %d", front, rear, front+rear);

        return 23;
}

