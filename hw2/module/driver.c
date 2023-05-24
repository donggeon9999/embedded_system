#include "driver.h"
#include "chardev.h"

static int driver_port_usage = 0;

static int __init device_init(void);
static void __exit device_exit(void);
static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static long device_ioctl(struct file*, unsigned int,unsigned long);

void init_timer_state(const char* init, const int interval,const int count);
void start_timer(void);
static void timer_func(unsigned long timeout);
void fpga_write(void);
                                      


state cur_state;

static struct file_operations device_driver_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = device_ioctl,
    .open = device_open,
    .release = device_release,
};

//insmod 시 실행
static int __init device_init(void) {
    const int registration = register_chrdev(MAJOR_NUM, DEVICE_FILE_NAME, &device_driver_fops);


    printk("device_init\n");

    if (registration != 0) 
        return registration;
    

    printk( "device file: /dev/%s ", DEVICE_FILE_NAME);
    printk( "device major number: %d\n",MAJOR_NUM);


    //fpga device driver mapping
    iom_fpga_led_addr = ioremap(IOM_LED_ADDRESS, 0x1);
    if(iom_fpga_led_addr == NULL)
        printk("Failed to IO-map device %s\n" ,iom_fpga_led_addr);
    iom_fpga_text_lcd_addr = ioremap(IOM_FPGA_TEXT_LCD_ADDRESS, 0x32);
    if(iom_fpga_text_lcd_addr == NULL)
        printk( "Failed to IO-map device %s\n" ,iom_fpga_text_lcd_addr);
    iom_fpga_dot_addr = ioremap(IOM_FPGA_DOT_ADDRESS, 0x10);
    if(iom_fpga_dot_addr == NULL)
        printk( "Failed to IO-map device %s\n" ,iom_fpga_dot_addr);
    iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);
    if(iom_fpga_fnd_addr == NULL)
        printk( "Failed to IO-map device %s\n" ,iom_fpga_dot_addr);

    //timer init
    init_timer(&(cur_state.timer));

    return 1;
}

//rmmod 시 실행
static void __exit device_exit(void) {

    //unregister
    unregister_chrdev(MAJOR_NUM, DEVICE_FILE_NAME);
    //timer delete
    del_timer_sync(&(cur_state.timer));
    //fpga unmapping
    iounmap(iom_fpga_led_addr);
    iounmap(iom_fpga_text_lcd_addr);
    iounmap(iom_fpga_dot_addr);
    iounmap(iom_fpga_fnd_addr);

}

//open 함수시 실행
static int device_open(struct inode* inode, struct file* file) {
    printk("device open\n");

    if (driver_port_usage != 0) {
        return -EBUSY;
    }
    //이미 open 되어 있는지 확인
    driver_port_usage = 1;

    return 0;
}

//close 시 실행 
static int device_release(struct inode* inode, struct file* file) {

    driver_port_usage = 0;
    return 0;
}

//ioctl시 실행
static long device_ioctl(struct file* file, unsigned int ioctl_num, unsigned long ioctl_param) {
    char buffer[11], temp[5] = {'\0'};
    char* param;
    long timerInterval=0, timerCount=0;
    int i, mul= 1;
    switch (ioctl_num) {
    case IOCTL_SET_OPTION: // ioctl option IOCTL_SET_OPTION의 경우
        printk("IOCTL_SET_OPTION\n");

        //param 가져오기 
        param = (char *)ioctl_param;
        if (strncpy_from_user(buffer, param, strlen_user(param)) < 0)
        {
            return -1;
        }

        // timerInterval parsing
        strncpy(temp, buffer, 3);
        //printk("%s\n", temp);
        for (i = 2; i >= 0; i--)
        {
            timerInterval += ((temp[i] - '0') * mul);
            mul *= 10;
        }
        //timeInterval 값이 0이면 Error
        if (timerInterval == 0) 
            return -1;

            
        // timerCount parsing
        strncpy(temp, buffer + 3, 3);

        //printk("%s\n", temp);
        mul = 1;
        for (i = 2; i >= 0; i--)
        {
            timerCount += ((temp[i] - '0') * mul);
            mul *= 10;
        }
        if (timerCount == 0)
            return -1;
        
        
        strncpy(temp, buffer + 6, 4);
        printk("%s\n", temp);

        init_timer_state(temp, timerInterval, timerCount);
        //state initialize
        break;

    case IOCTL_COMMAND: // ioctl option IOCTL_COMMAND 경우

        printk("IOCTL_COMMAND\n");
        start_timer();
        break;

    }

    return 0;
}

//상태 초기화
void init_timer_state(const char* init, const int interval,const int count)
{
    printk("initialize state !\n");
    int i;
    for (i = 0; i < 4; i++) {
        if (init[i] != '0') {
            cur_state.digit = init[i] - '0'; 
            cur_state.index = i; 
            //digit이 뭔지, 해당 index가 어디서 시작하는지 저장
            break;
        }
    }
    
    cur_state.mv_right[0] = cur_state.mv_right[1] = true;
    // text lcd 가 오른쪽으로 갈지 왼쪽으로 갈지 저장

    cur_state.text_index[0] = 0;
    cur_state.text_index[1] = 0;
    //text lcd 시작 index 저장
    cur_state.count = 0; 
    //time count 저장
    cur_state.end_cnt = count;
    //time end count 저장
    cur_state.interval = interval;
    //time interval 저장

   
    for(i=0;i<16;i++)
    {
        cur_state.text[0][i] = ' ';
        //text에 ' ' 문자로 모두 저장.
    }

    cur_state.text[0][i] = '\0';
    
    for(i = 0;i<st_id_size;i++)
    {
        cur_state.text[0][i] = st_id[i];
        //학번 저장
    }
    
    
    for(i=0;i<16;i++)
    {
        cur_state.text[1][i] = ' ';
        //text에 ' '문자로 모두 저장
    }
    cur_state.text[1][i] = '\0';
    for(i = 0;i<name_size;i++)
    {
        cur_state.text[1][i] = name[i];
        //이름 저장
    }
    
  
    // printk("initialize state !%d %d %d %d !%s!%s!\n",cur_state.digit,cur_state.count,cur_state.end_cnt,
    // cur_state.interval, cur_state.text[0],cur_state.text[1]);
}

//start timer 함수
void start_timer(void) {
    del_timer_sync(&(cur_state.timer));

    printk("start timer!\n");

    fpga_write();
    cur_state.timer.expires = get_jiffies_64() + cur_state.interval * (HZ / 10);
    cur_state.timer.data = (unsigned long) &cur_state;
    cur_state.timer.function = timer_func;

    add_timer(&(cur_state.timer));
}

//timer function
static void timer_func(unsigned long timeout) {

    state* next_state = (state*) timeout;

    printk("start update!\n");

    int i;
    next_state->count++;
    if (next_state->count == next_state->end_cnt) {
        
        next_state->digit = 0;
        memset(next_state->text[0],' ',16);
        memset(next_state->text[1],' ',16);
    }
    else {
        ///////////////////
        //digit update
        ///////////////////
        next_state->digit = (next_state->digit == DIGIT_END ? 1 : (next_state->digit + 1));
        //digit 값 증가
        if (next_state->count % DIGIT_END == 0){
            //바뀐 count가 8이 되면 출력 index 증가
            next_state->index = (next_state->index + 1) % 4;
        }
        ///////////////////
        // text lcd update
        ///////////////////
        for(i =0;i<BUF_SIZE;i++){
                next_state->text[0][i] = ' ';
                next_state->text[1][i] = ' ';
        }
        if(next_state->mv_right[0])
        {
            //오른쪽으로 가는 경우
            next_state->text_index[0] += 1;
            for(i = 0;i<st_id_size;i++)
                next_state->text[0][next_state->text_index[0] + i] = st_id[i];
            if(next_state->text_index[0] + st_id_size == BUF_SIZE)// 더 이상 못가면 다음부터는 왼쪽으로
                next_state->mv_right[0] = false;
        }
        else{
            //왼쪽으로 가는 경우
            next_state->text_index[0] -= 1;
            for(i = 0;i<st_id_size;i++)//학번 저장
                next_state->text[0][next_state->text_index[0] + i] = st_id[i];
            if(next_state->text_index[0] == 0)//더 이상 못가면 다음부터는 오른쪽으로 갈 수 있게
                next_state->mv_right[0] = true;
        }
        if(next_state->mv_right[1])
        {
            //오른쪽으로 가는 경우
            next_state->text_index[1] += 1;
            for(i = 0;i<name_size;i++)//이름 저장
                next_state->text[1][next_state->text_index[1] + i] = name[i];
            if(next_state->text_index[1] + name_size == BUF_SIZE)// 더 이상 못가면 왼쪽으로 가도록
                next_state->mv_right[1] = false;
        }
        else{
            //왼쪽으로 가는 경ㅇ 
            next_state->text_index[1] -= 1;

            for(i = 0;i<name_size;i++) // 이름 저장
                next_state->text[1][next_state->text_index[1] + i] = name[i];
            if(next_state->text_index[1] == 0)// 더 이상 못가면 오른쪽으로
                next_state->mv_right[1] = true;
        }

       

        ///////////////
        //timer update
        ///////////////
        next_state->timer.expires = get_jiffies_64() + next_state->interval * (HZ / 10);
        next_state->timer.data = (unsigned long) &cur_state;
        next_state->timer.function = timer_func;
        add_timer(&(next_state->timer));
    }
    printk("finish update!\n");
    fpga_write();
    printk("write update!\n");
    // Print updated state
    //print_state(payload);
}        

//fpga write 하는 함수
void fpga_write(void)
{
//dot_write
    int i;

	unsigned char *value;
	unsigned short int _s_value;
	
    value = fpga_number[cur_state.digit];
	for(i=0;i<10;i++)
    {
		outw(value[i] & 0x7F,(unsigned int)iom_fpga_dot_addr+i*2);
    }

//fnd write
    
	unsigned short int value_short = 0;

    value_short = cur_state.digit << (12 - 4 * cur_state.index);
    outw(value_short,(unsigned int)iom_fpga_fnd_addr);	    

// led write
    const unsigned short led_value = (1 << (8 - cur_state.digit));
    outw(led_value, (unsigned int)iom_fpga_led_addr);

//text_lcd write

	unsigned char text_value[33];
    text_value[32] = 0;


    for(i = 0;i<16;i++)
    {
                text_value[i] = cur_state.text[0][i];
                text_value[16+i] = cur_state.text[1][i];
    }
    printk("!%s!\n",text_value);
	for(i=0;i<32;i++)
    {

		outw((text_value[i] & 0xFF) << 8 | text_value[i + 1] & 0xFF,(unsigned int)iom_fpga_text_lcd_addr+i);
        i++;
    }

}


module_init(device_init);
module_exit(device_exit);

MODULE_LICENSE("Dual MIT/GPL");