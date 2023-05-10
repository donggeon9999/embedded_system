#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SIZE 1048576 // 2^20

unsigned long long int get_system_memory()
{
	long pages = sysconf(_SC_AVPHYS_PAGES);
	long page_size = sysconf(_SC_PAGESIZE);
	return pages * page_size;
}

int main(int argc, char **argv)
{
	unsigned long long int memory_size;
	long request_size;
	int fd;
	int st_id = 1664;
	char input1[SIZE-1664] = {0};
	char input2[SIZE+1664] = {0};
	char output1[SIZE -1664];
	char output2[SIZE +1664];
	
	fd = open("/dev/mem_ctrl", O_RDWR);
	if(fd < 0){
		printf("Open Failured!\n");
		return -1;
	}

	request_size = SIZE;
	memory_size = get_system_memory();
	
	
	
	write(fd, &input1, request_size - 1664);
	
	
	printf("Request Size : %lu - %d, Diff System Memory: %llu \n", SIZE,st_id, memory_size - get_system_memory());
	memory_size = get_system_memory();
	write(fd, &input2, request_size + 1664);
	printf("Request Size : %lu + %d, Diff System Memory: %llu \n", SIZE,st_id, memory_size - get_system_memory());
	
	off_t loc = lseek(fd, 0, SEEK_SET);
	read(fd, &output1, request_size- st_id);
	read(fd, &output2, request_size+ st_id);
	//printf("Request Size : %lu -%lu, Diff System Memory: %llu \n", SIZE,st_id, memory_size - get_system_memory());

	close(fd);
	return 0;
}

