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
#include <sys/mman.h>




#define DOT_DEVICE "/dev/fpga_dot"
#define FND_DEVICE "/dev/fpga_fnd"
#define LED_DEVICE "/dev/mem"
#define TEXT_LCD_DEVICE "/dev/fpga_text_lcd"
#define BUZZER_DEVICE "/dev/fpga_buzzer"
#define PUSH_SWITCH_DEVICE "/dev/fpga_push_switch"
#define DIP_SWITCH_DEVICE "/dev/fpga_dip_switch"
#define STEP_MOTOR_DEVICE "/dev/fpga_step_motor"
#define KEY_DEVICE "/dev/input/event0"

#define FPGA_BASE_ADDRESS 0x08000000 //fpga_base address
#define LED_ADDR 0x16

#define DEVICES_CNT 9
#define BUFF_SIZE 64



enum _devices{DOT, FND, LED, TEXT_LCD, BUZZER, PUSH_SWITCH, DIP_SWITCH,STEP_MOTOR,KEY };
enum _logLevel { ERROR, WARNING, INFO };


//unsigned char *led_addr_map =0;
int devices[DEVICES_CNT];
unsigned long *fpga_addr;
unsigned char *led_addr;

void open_devices();
void close_devices();