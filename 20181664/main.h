#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdio.h>
#include <sys/sem.h>
#include <errno.h>  
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>
// process pid num define
#define MAIN_PID 0
#define IO_PID 1
#define MERGE_PID 2












pid_t io_pid;
pid_t merge_pid;
//unsigned char quit = 0;



void main_process(void);