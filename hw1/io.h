#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h> 
#include <sys/ioctl.h>
#include <stdbool.h>
#include <string.h>





#define BUFF_SIZE 64

#define BACK_CODE 158
#define PROC_CODE 116
#define VOL_PLUS_CODE 115
#define VOL_MINUS_CODE 114
#define MODE_CNT 3
#define MODE_EXIT -1

#define NON_INPUT 0
#define KEY_INPUT 1
#define VALUE_INPUT 2

#define KEY_RELEASE 0
#define KEY_PRESS 1
#define MAX_BUTTON 9
#define MAX_DIGIT 4

#define LCD_MAX_BUFF 32

#define NUM_IN 0
#define ALPHA_IN 1

#define MOTOR_STOP 0
#define MOTOR_START 1
#define MOTOR_LEFT 0
#define MOTOR_RIGHT 1

#define MEM_LIMIT 3
#define STORE_LIMIT 100
enum _modes { PUT,GET,MERGE};



struct key_value{
    int idx; 
    unsigned char key[MAX_DIGIT+1];
    unsigned char value[LCD_MAX_BUFF];
    int buf_size;
    int key_size;
};

struct input_buff{
    int mode; // put, get, merge, exit mode variable
    unsigned char key[MAX_DIGIT+1];//key save
    unsigned char key_value_mode; // key input - value input mode set
    unsigned char value[LCD_MAX_BUFF];//value  save
    int buf_size;//value size
    int key_size;//key size
    int key_idx; //key save index
    char is_changed; // mode change flag

    bool get_request; // get request flag
};

struct merge_buff{
    struct input_buff io_merge_buf[3];// key-value data save
    int input_cnt;// in memory save key-value count
    bool need_store; //need store flag
    bool need_merge; // need merge flag
    int storage_cnt; // storage table  count
};

struct input_buff *io_shmaddr;
struct merge_buff *merge_shmaddr; 

struct input_buff * get_result;

int key_idx;
int merge_idx;
char result_m[32];

char file_name[15] ;
int last_file_idx;
int file_cnt; 

int io_shmid, merge_shmid;

int semid;

void merge_func(int semid);
void make_file_name(int num);


struct input_buff* get_func(void);
void initialize(void);
void fnd_out(struct input_buff *io_shmaddr);
void mode_io(int semid,struct input_buff *io_shmaddr,int io_shmid, int merge_shmid);
void put_io(int semid,struct input_buff *);
void led_put(int semid,struct input_buff *io_shmaddr,int data);
void initialize_put(void);
void merge_func(int semid);
void lcd_out(int semid,struct input_buff *io_shmaddr);