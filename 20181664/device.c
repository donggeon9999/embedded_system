#include "device.h"



//device open
void open_devices(){
    
    fpga_addr = 0;
	unsigned char data; 
    int i ;
    devices[DOT] = open(DOT_DEVICE, O_WRONLY);
    devices[FND] = open(FND_DEVICE, O_RDWR);
    devices[LED] = open(LED_DEVICE, O_RDWR | O_SYNC);
    devices[TEXT_LCD] = open(TEXT_LCD_DEVICE, O_WRONLY);
    devices[BUZZER] = open(BUZZER_DEVICE, O_RDWR );
    devices[PUSH_SWITCH] = open(PUSH_SWITCH_DEVICE, O_RDWR );
    devices[DIP_SWITCH] = open(DIP_SWITCH_DEVICE, O_RDWR);
    devices[STEP_MOTOR] = open(STEP_MOTOR_DEVICE, O_WRONLY);
    devices[KEY] = open(KEY_DEVICE, O_RDONLY | O_NONBLOCK);
    for( i =0;i<DEVICES_CNT;i++)
    {
        if(devices[i]<0)
        {
            printf("%d device open error\n",i);
            close(devices[i]);
        }
        else
            printf("%d success open\n", i);
    }

    fpga_addr = (unsigned long *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, devices[LED], FPGA_BASE_ADDRESS);
    if (fpga_addr == MAP_FAILED)
	{
		printf("mmap error!\n");
		close(devices[LED]);
		return;
	}

}

void close_devices(){
    
    int i;
    /*
    munmap(led_addr_map, 4096);
    for(i =0;i<DEVICES_CNT;i++){
        if(close(devices[i])<0)
            {
                //deviceLog(i, WARNING, "Error while closing device\n");
                return;
            } 
    }
    */
return;
}
