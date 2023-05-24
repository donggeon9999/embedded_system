#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/version.h>

#include "./fpga_dot_font.h"


#define IOM_LED_MAJOR 260		
#define IOM_LED_NAME "fpga_led"		

#define IOM_FPGA_TEXT_LCD_MAJOR 263	
#define IOM_FPGA_TEXT_LCD_NAME "fpga_text_lcd"

#define IOM_FPGA_DOT_MAJOR 262		
#define IOM_FPGA_DOT_NAME "fpga_dot"		

#define IOM_FND_MAJOR 261		
#define IOM_FND_NAME "fpga_fnd"	




#define IOM_FPGA_DIP_SWITCH_ADDRESS 0x08000000		
#define IOM_LED_ADDRESS 0x08000016
#define IOM_FPGA_TEXT_LCD_ADDRESS 0x08000090 
#define IOM_FPGA_DOT_ADDRESS 0x08000210 
#define IOM_FND_ADDRESS 0x08000004 

#define st_id_size 8
#define name_size 13
#define BUF_SIZE 16
#define DIGIT_END 8

static unsigned char *iom_fpga_led_addr;
static unsigned char *iom_fpga_text_lcd_addr;
static unsigned char *iom_fpga_dot_addr;
static unsigned char *iom_fpga_fnd_addr;


char *st_id = "20181664";
char *name = "lee dong geon";


// 출력 저장할 state
typedef struct state{
    struct timer_list timer; //timer
    char text[2][17];        //text lcd 저장할 변수
    bool mv_right[2];   // 오른쪽으로 갈 수 있는지 check
    short text_index[2]; //text lcd 출력 시작할 index
    int count;           //시간 count
    int end_cnt;       //end 하는 count
    int interval;       //timer interval
    short digit;         //현재 숫자 값
    short index;   // fnd 에 출력 하는 index
} state;