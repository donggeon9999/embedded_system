#include <unistd.h>
#include <syscall.h>
#include <stdio.h>
struct mystruct{

	int myvar;
};
struct mystruct2 {
        int num_front;
        int num_rear;
};

int main(void){
	int i=syscall(376, 11);
	printf("syscall %d\n" ,i);

	struct mystruct my_st;
	my_st.myvar =999;
	
	int i2=syscall(377, &my_st);
	printf("syscall %d\n" ,i2);
	
	struct mystruct2 my_st2;
	my_st2.num_front = 2018;
	my_st2.num_rear = 1664;
	
	int i3=syscall(378, &my_st2);
	printf("syscall %d\n", i3);
	
	return 0;
}


