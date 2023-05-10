#include "io.h"
#include "sema.h"
#include "device.h"

char alpha[9][3]={
    {'.','Q','Z'},
    {'A','B','C'},
    {'D','E','F'},
    {'G','H','I'},
    {'J','K','L'},
    {'M','N','O'},
    {'P','R','S'},
    {'T','U','V'},
    {'W','X','Y'}
    };
    // alphabet array

//mode change function
void mode_io(int semid,struct input_buff *io_shmaddr,int io_shmid, int merge_shmid)
{
    struct input_event ev[BUFF_SIZE];
	int fd,key_code, rd, last_mode,value, size = sizeof (struct input_event);
	char* device = "/dev/input/event0";
     //open device driver
	if((fd = open (device, O_RDONLY)) == -1) {
		printf ("%s is not a vaild device.n", device);
	}
	    //waiting input ready_key (vol+, vol-, back)
	while (1){
        printf("in mode_io_process\n");
		if ((rd = read (fd, ev, size * BUFF_SIZE)) < size)
		{
			return (0);     
		}
        else{
            value = ev[0].value;
            key_code =ev[0].code;
            if( value == KEY_PRESS ) {// if key press ready key
                semop(semid, &p[mode_sem], 1); //protect shared memory
                printf ("key press\n");
                io_shmaddr->is_changed = 1;

                if(key_code == BACK_CODE){// if back key press -> mode change to exit mode
                    io_shmaddr->mode = MODE_EXIT;
                                        //remove shared memories, semaphore
                    shmctl(io_shmid, IPC_RMID, (struct shmid_ds *)NULL); 
                    shmctl(merge_shmid, IPC_RMID, (struct shmid_ds *)NULL); 
                    erase_sema(semid);
                    semop(semid, &v[mode_sem], 1);
                    return;
                }
                if(key_code == VOL_PLUS_CODE)// if vol+ key press -> mode change to next mode
                    io_shmaddr->mode = (io_shmaddr->mode + 1)%MODE_CNT;
                if(key_code == VOL_MINUS_CODE)//if vol- key press -> mode change to previous mode
                    io_shmaddr->mode = (io_shmaddr->mode +2)%MODE_CNT;
                semop(semid, &v[mode_sem], 1);
                //initialize();
            }
        } 
	}

	return 0;

}

void put_io(int semid,struct input_buff *io_shmaddr)
{//only io process in func
	int switch_buff_size;

	unsigned char switch_buff[MAX_BUTTON];

    
    struct input_event ev[BUFF_SIZE];
	int fd, rd, last_mode,value, size = sizeof (struct input_event);
    int reset_fd;
    int key_value_mode = NON_INPUT;
    int fnd_flag = 0, lcd_flag = 0;
    char before_x = -1, before_y= -1;

    unsigned char dip_sw_buff = 0;  
    int key_code;
    char num_alpha_mode = NUM_IN;

    switch_buff_size=sizeof(switch_buff);

    io_shmaddr->key_idx = 0;
	
	
	while (1){
       
        if(io_shmaddr->mode == MODE_EXIT)   //if mode is exit -> return 
            return;
        /////////////////////////////////////////////////////////////////////////////
        //   reset button: (PUT MODE) mode change ->>  : key-value mode change
       //                  (GET MODE) : get request!!
    //                      (MERGE MODE) : merge request!!
        ////////////////////////////////////////////////////////////////////////////


        semop(semid, &p[mode_sem], 1);// protect shared memory
        read(devices[DIP_SWITCH], &dip_sw_buff, 1);// read reset key value 
        if(dip_sw_buff == 0x00 )//push reset button
        {
            if(io_shmaddr->mode == PUT){
                 // if mode is PUT -> key - value input mode change
                printf("push reset!! %x %d\n",dip_sw_buff,io_shmaddr->key_value_mode);

                if(io_shmaddr->key_value_mode ==  NON_INPUT)
                    io_shmaddr->key_value_mode = KEY_INPUT;
                else if(io_shmaddr->key_value_mode ==  KEY_INPUT)
                    io_shmaddr->key_value_mode = VALUE_INPUT;
                else if(io_shmaddr->key_value_mode ==  VALUE_INPUT)
                    io_shmaddr->key_value_mode = KEY_INPUT;
                usleep(400000);
            }
            else if(io_shmaddr->mode == GET)
            {// if mode is GET -> get request!

                printf("GET REQUEST!!\n");
                get_result = get_func();
                if(get_result == NULL)// if get_result is NULL get request fail
                {
                    int idx = 0;
                    result_m[idx++] = 'E';
                    result_m[idx++] = 'r';
                    result_m[idx++] = 'r';
                    result_m[idx++] = 'o';
                    result_m[idx++] = 'r';
                    result_m[idx] = '\0';
                    lcd_flag = 1;
                    //error message put lcd(error)
                }
                else{// get_request not NULL is find success!!
                    printf("ifind!!!\n");
                    int idx = 0;
                    int i;
                    result_m[idx++] = '(';
                    result_m[idx++] = get_result->key_idx + 0x30;
                    result_m[idx++] = ' ';
                    printf("result is %s %s\n",get_result->key, get_result->value);
                    for(i=0;i<strlen(get_result->key);i++)
                        result_m[idx++] = get_result->key[i];
                    result_m[idx++] = ' ';
                    for(i=0;i<strlen(get_result->value);i++)
                        result_m[idx++] = get_result->value[i];
                    result_m[idx++] = ' )';
                    result_m[idx] = '\0';
                    printf("%s\n",result_m);
                    lcd_flag = 1;
                  // find message put lcd (idx, key, value)

                }

                //request get!!! to storage with key
            }
            else if(io_shmaddr->mode == MERGE)
            {
                printf("it's merge reset\n");
                merge_shmaddr->need_merge = true;
                    //if reset push in merge mode, need_merge flag change
            }

        }
    
    ///////////////////////////////////////////////////////////////////////////
     //  (PUT MODE)->> key & value input ->>   switch button : 1~9 & alphabet
     //  (GET MODE)->>only key input ->>switch button : 1~9 
    /////////////////////////////////////////////////////////////////////////////////////
        read(devices[PUSH_SWITCH], &switch_buff, switch_buff_size);
        usleep(400000);//always switch key read
        int i;
        // #2 #3 button input -> key ,value reset
        if(switch_buff[1]==1 && switch_buff[2]==1)
        {
            printf("initializee haahaha\n");
            for(i=0;i<MAX_BUTTON;i++){
            printf("[%d] ",switch_buff[i]);

            }
         printf("\n");
                initialize();// key, value reset!!
                switch_buff[1]= 0;//switch value reset!!
                switch_buff[2]= 0;
        }
        // #8 #9 button input -> store (key,value)
        if(switch_buff[7]==1 && switch_buff[8]==1)
        {
            printf("store!! %d\n",merge_shmaddr->input_cnt +1);
            for(i=0;i<MAX_BUTTON;i++){
            printf("[%d] ",switch_buff[i]);

            }
            store_to_shm(semid);//store to shared memory 
            
            io_shmaddr->is_changed = 1; //is_changed value set to reset fnd lcd.

            switch_buff[7] = 0;
            switch_buff[8] = 0;
        }
        // if GET Mode or PUT Mode in key_input mode-> key: number input
        if(io_shmaddr->mode == GET || io_shmaddr->key_value_mode == KEY_INPUT)
        {
            int i;
            usleep(400000);
            for(i=0;i<MAX_BUTTON;i++) {
                if(switch_buff[i] == 1){
                    fnd_flag = 1;// fnd flag set to display fnd
                    if(io_shmaddr->key_size == 4)
                    {// if key size over 4, reset key value
                        int j;
                        for(j=0;j<4;j++)
                            io_shmaddr->key[j] = 0;
                    }
			        io_shmaddr->key[io_shmaddr->key_idx] = (unsigned char)(i+1);
                    io_shmaddr->key_size = io_shmaddr->key_idx+1;
                    io_shmaddr->key_idx =  (io_shmaddr->key_idx +1) %MAX_DIGIT;
                    
                   if(io_shmaddr->mode == GET)// if GET MODE, shared memory key_value_mode set
                        io_shmaddr->key_value_mode = KEY_INPUT; 
                }
	    	}
            for(i=0;i<io_shmaddr->key_size;i++)
                printf("%d ",io_shmaddr->key[i]);
            printf("!! key!!\n");
            
            
            
        }
        if(fnd_flag == 1)// if fnd flag set, display key to fnd
        {
            fnd_out(io_shmaddr);
            fnd_flag = 0;
        }
        //if PUT mode and value input mode -> value : num or alphabet input!
        if(io_shmaddr->mode == PUT && io_shmaddr->key_value_mode == VALUE_INPUT)
        {
            printf("value input!");
            int i;
            usleep(400000);
            // #5 #6 button input -> num, alpha mode change
            if(switch_buff[4]==1 && switch_buff[5]==1){
                printf("num-alphabet mode change!\n");
                num_alpha_mode = (num_alpha_mode + 1)%2;
                switch_buff[4] = 0;
                switch_buff[5] = 0;
            }
            else{
                for(i=0;i<MAX_BUTTON;i++) {
                    if(switch_buff[i] == 1 && num_alpha_mode == NUM_IN){//number input
                        lcd_flag = 1;
                        io_shmaddr->value[io_shmaddr->buf_size++] = (i+1) + 0x30;
                    }
                    if(switch_buff[i] == 1 && num_alpha_mode == ALPHA_IN){// alphabet input

                        char ch;
                        lcd_flag = 1;
                        if(before_y != i){ // first switch push
                            ch = alpha[i][0];
                            
                            before_x = 0;
                            before_y = i;
                            io_shmaddr->value[io_shmaddr->buf_size++] = ch;
                            
                        }
                        else{
                            ch = alpha[i][(before_x+1)%3];
                            before_x = (before_x+1)%3;
                            before_y = i;
                            io_shmaddr->value[io_shmaddr->buf_size -1] = ch;
                        }
                        printf("lcd out is %s\n",io_shmaddr->value);
                    }
                }
            }
            
        }
        if(lcd_flag == 1)// if lcd flag set , displat value lcd,
        {
            lcd_out(semid,io_shmaddr);
            lcd_flag = 0;
        }
        
        semop(semid, &v[mode_sem], 1);

        // key input & value input   



	}

	return 0;
}
struct input_buff* get_func(void)
{   //if GET MODE, do this function
    int i,j, k =0;
    int corr_flag = 0;
    struct input_buff * get_tmp= (struct input_buff *)malloc(sizeof(struct input_buff));
    FILE *fp[2];
    FILE *fp_tmp;
    char key_ch[4];
    io_shmaddr->key[io_shmaddr->key_size] = '\0';
    for(i=0;i<io_shmaddr->key_size;i++)
        io_shmaddr->key[i] = io_shmaddr->key[i] + 0x30; //change int key to char key (ex) 1 -> '1'
    memcpy(get_tmp->key,io_shmaddr->key,strlen(io_shmaddr->key));
    printf("key value is %s %s\n",io_shmaddr->key,get_tmp->key);
    for(i=0;i<merge_shmaddr->input_cnt;i++)
    {  //key compare with memory keys.
        printf("in memory finding\n");
        corr_flag =0;
        if(!strcmp(io_shmaddr->key,merge_shmaddr->io_merge_buf[i].key))
        {//if find same key, store key-value to get_tmp.
            memcpy(get_tmp->value, merge_shmaddr->io_merge_buf[i].value, strlen(merge_shmaddr->io_merge_buf[i].value));
            get_tmp->key_idx = i+1;
            return get_tmp;//return key_value pair
        }
    }

   //key find in files
    printf("in file finding\n");
    struct key_value kv[STORE_LIMIT];
    for(i=1;i<100;i++)
    {
        make_file_name(i);
        if( fp_tmp = fopen(file_name, "r"))
        {
            fp[k++] = fp_tmp;
        }
        if(k == 2){
            break;
        }
    }
    int com_idx = 0,cnt = 0;
     // get idx, key, value in first file
    for(i = 0;i<STORE_LIMIT;i++){
        fscanf(fp[0],"%d %s %s",&kv[i].idx, kv[i].key, kv[i].value);
        if(kv[i].idx == com_idx||kv[i].idx == 0)
            break;
        printf("%d %s %s\n",kv[i].idx, kv[i].key, kv[i].value);
        com_idx = kv[i].idx;
        cnt++;
        printf("compare %s %d %s %d\n",io_shmaddr->key,strlen(io_shmaddr->key),kv[i].key,strlen(kv[i].key));
        if(!strcmp(io_shmaddr->key,kv[i].key))
        {
            //if same key value -> store key, value in get_tmp -> return get_tmp
            printf("find in file1\n%s %d\n",kv[i].value, strlen(kv[i].value));
            memcpy(get_tmp->value, kv[i].value, strlen(kv[i].value));
            printf("find in file1\n%s %d\n",kv[i].value, strlen(kv[i].value));
            get_tmp->key_idx = kv[i].idx;
            return get_tmp;
        }
        
    }
    //printf("cnt is %d\n",cnt);
    com_idx = 0;
    for(i=cnt;i< cnt+ STORE_LIMIT;i++){
         // get idx, key, value in second file
        fscanf(fp[1],"%d %s %s",&kv[i].idx, kv[i].key, kv[i].value);
        if(kv[i].idx == com_idx||kv[i].idx == 0)
                break;
        printf("%d %s %s\n",kv[i].idx, kv[i].key, kv[i].value);
        com_idx = kv[i].idx;
        cnt++;
        printf("compare %s %s\n",io_shmaddr->key,kv[i].key);
        if(!strcmp(io_shmaddr->key,kv[i].key))
        {
            printf("find in file2\n");
            // if same key value -> store key, value in get_tmp -> return get_tmp
            memcpy(get_tmp->value, kv[i].value, strlen(kv[i].value));
            get_tmp->key_idx = kv[i].idx;
            return get_tmp;
        }
    }
    
    return NULL; //if not found same key-value, return NULL!!




}
//lcd display function
void lcd_out(int semid,struct input_buff *io_shmaddr){

//if PUT MODE just display value in text lcd
    printf("lcd out gkfkrh tlqkf\n");
    if(io_shmaddr->mode == PUT)
        write(devices[TEXT_LCD], io_shmaddr->value,io_shmaddr->buf_size);	
    else if(io_shmaddr->mode == GET)
    {
        // if GET MODE display key_value or error message in text lcd
        initialize();
        write(devices[TEXT_LCD],result_m,strlen(result_m));	
        //and all led on and off
        led_addr=(unsigned char*)((void*)fpga_addr+LED_ADDR);
        printf("data1\n");
        sleep(1);
        *led_addr=255;
        sleep(1);
        initialize_put();
        io_shmaddr->key_value_mode = NON_INPUT;
        io_shmaddr->is_changed = 1;
    }
}
//key- value store to shared memory function
void store_to_shm(int semid)
{
    int i;
    semop(semid, &p[merge_sem], 1);
  
    printf("here!!\n");
    int idx = merge_shmaddr->input_cnt;
    int size = io_shmaddr->buf_size;
    int key_size = (io_shmaddr->key_idx == 0)? 4 : io_shmaddr->key_idx;
    printf("store information is ");
    //io shared memory data store to merge shared memory
    for(i=0;i<key_size;i++){ // key store
        merge_shmaddr->io_merge_buf[idx].key[i] = io_shmaddr->key[i] +0x30;
        printf("%c",merge_shmaddr->io_merge_buf[idx].key[i]);
    }
    printf("\n");
    merge_shmaddr->io_merge_buf[idx].key_size = key_size;
    for(i=0;i<size;i++)
    {//value store
        merge_shmaddr->io_merge_buf[idx].value[i] = io_shmaddr->value[i];
        printf("%c",merge_shmaddr->io_merge_buf[idx].value[i] );
    }
    merge_shmaddr->io_merge_buf[idx].buf_size = size;
    merge_shmaddr->input_cnt = idx + 1;
     //memory table count plus
    led_addr=(unsigned char*)((void*)fpga_addr+LED_ADDR);
    printf("data1\n");
    sleep(1);
    *led_addr=255;
    sleep(1);
    initialize_put();
    //all led on and off
    
    //led_put(semid,io_shmaddr,1);
    if(merge_shmaddr->input_cnt >=MEM_LIMIT){
        merge_shmaddr->need_store = true;
        merge_shmaddr->input_cnt = 0;  
        merge_shmaddr->storage_cnt += 1;  
    }
    if(merge_shmaddr->storage_cnt >= MEM_LIMIT)
        merge_shmaddr->need_merge = true;
    semop(semid, &v[merge_sem], 1);

    

    return 1;

}
//fnd lcd key value reset function
void initialize(void)
{
    unsigned char empty[MAX_DIGIT] = {0,0,0,0};
    char string[LCD_MAX_BUFF];
    usleep(400000);

    write(devices[FND],&empty,4); //fnd reset
    memset(string,' ',sizeof(string));	

    usleep(400000);
	write(devices[TEXT_LCD],string,LCD_MAX_BUFF);	//lcd reset

    memset(io_shmaddr->key,0,MAX_DIGIT);//key reset
    memset(io_shmaddr->value,' ',LCD_MAX_BUFF);//value reset

    printf("initizliaze!!\n");
    io_shmaddr->key_idx = 0;// fnd idx reset
    
    io_shmaddr->buf_size = 0;
  }
void fnd_out(struct input_buff *io_shmaddr) //display fnd function
{
    unsigned char retval;
    int fnd_dev = devices[FND];
    unsigned char copy[4];
    int size;
    int i;


    
    retval=write(fnd_dev,&io_shmaddr->key,io_shmaddr->key_size);	
    if(retval<0) {
        printf("Write Error!\n");
        return -1;
    }
   
}

//led put function
void led_put(int semid,struct input_buff *io_shmaddr,int data)
{

    semop(semid, &v[mode_sem], 1);
    unsigned char data1, data2;
    int mode = io_shmaddr->mode;
    led_addr=(unsigned char*)((void*)fpga_addr+LED_ADDR);
    if(data==1)
    { // if data is 1 -> all led on and off
        printf("data1\n");
        *led_addr=255;
        sleep(0.5);
        initialize_put();
        return;
    }
    while(1)
    {    //if data is not 1 led on repeatly
        if(io_shmaddr->key_value_mode == KEY_INPUT){ 
            //in put mode , key input -> #3 #4 led on and off
            //or get mode, key input
            data1 = 16;data2= 32;  
        }
        else if(io_shmaddr->key_value_mode == VALUE_INPUT){//in put mode, value input -> #7 #8 led on and pff
            data1 = 1; data2 = 2;
        }
        else
            return;
        sleep(1);
        *led_addr=data1; //write led
        sleep(1);
        *led_addr=data2;
        semop(semid, &p[mode_sem], 1);
        if(io_shmaddr->is_changed){
            io_shmaddr->key_value_mode = NON_INPUT;
            printf("led_put out!!!!\n");
                //semop(semid, &v[mode_sem], 1);
            return;
        }
        semop(semid, &v[mode_sem], 1);
    }
        

}  
    
//always #1 led on 
void initialize_put(void)
{
	unsigned char data=128;



	led_addr=(unsigned char*)((void*)fpga_addr+LED_ADDR);
	*led_addr=data; //write led
	sleep(1);
	
	return 0;    
}



//merge function
void merge_func(int semid){
    
    last_file_idx = 0;
    merge_idx = 0;
    file_cnt = 0;
     //while loop -> detect need_store, need_merge flag
    while(1)
    {   
        //printf("merge_proess\n");
        usleep(4000);
        semop(semid,&p[merge_sem], 1);
        if(merge_shmaddr->need_store == true)
        {  
            //if need_store flag set -> store key-value data to file
            make_file_name(last_file_idx + 1);
            last_file_idx++;
            FILE *fp = fopen(file_name, "w");
            int i,j;
            for(i=0;i<MEM_LIMIT;i++)
            {

                int key_size = merge_shmaddr->io_merge_buf[i].key_size;
                int value_size = merge_shmaddr->io_merge_buf[i].buf_size;
                char *keybuf = merge_shmaddr->io_merge_buf[i].key;
                char *valuebuf = merge_shmaddr->io_merge_buf[i].value;
                char idx = merge_idx++ + 0x31;
                // fputc('K',fp);
                // fputc(' ',fp);
                fputc(idx,fp);
                fputc(' ',fp);
                for(j = 0;j<key_size;j++){
                    fputc(keybuf[j],fp);
                    printf("%c",keybuf[j]);
                }
               // put key 
                fputc(' ',fp);
                for(j = 0;j<value_size;j++){
                    fputc(valuebuf[j],fp);

                    printf("%c",valuebuf[j]);
                }
               // put value
                fputc('\n',fp);
            }
            fclose(fp);
            merge_shmaddr->need_store = false;// flag reset
            
        }
        sleep(2);
        if( merge_shmaddr->need_merge == true)
        {
             // if need_merge flag set -> file merge!!
            printf("motor riding!");
            motor();// motor riding
            data_merge();// file merge
            merge_shmaddr->need_merge = false;// flag reset
        }
        semop(semid,&v[merge_sem], 1);
    }
}
void data_merge(void){//file merge function
    
    FILE *fp[2], *tmp, *new_fp;
    struct key_value kv[STORE_LIMIT], new_kv[STORE_LIMIT];
    int i,j, num1, num2, k = 0,save_cnt = 0;
    int same_flag = 0;
    char rm_file[2][10];
    for(i=1;i<100;i++)
    {
        make_file_name(i);// make file name
        if(tmp = fopen(file_name, "r"))
        {//if file exist, store  2 file pointer
            memcpy(rm_file[k],file_name,strlen(file_name));
            fp[k++] = tmp;
        }
        if(k == 2){
            break;
        }
    }
    int com_idx = 0,cnt = 0;
    for(i = 0;i<STORE_LIMIT;i++){//in first file get key-value data 
        fscanf(fp[0],"%d %s %s",&kv[i].idx, kv[i].key, kv[i].value);
        if(kv[i].idx == com_idx||kv[i].idx == 0)
            break;
        com_idx = kv[i].idx;
        cnt++;
        
    }
    com_idx = 0;//in second file get key-value data 
    for(i=cnt;i< cnt+ STORE_LIMIT;i++){
        fscanf(fp[1],"%d %s %s",&kv[i].idx, kv[i].key, kv[i].value);
        if(kv[i].idx == com_idx||kv[i].idx == 0)
                break;
        com_idx = kv[i].idx;
        cnt++;
    }
    new_kv[save_cnt].idx = save_cnt+1;
    new_kv[save_cnt].key_size = strlen(kv[0].key);
    memcpy (new_kv[save_cnt].key, kv[0].key, strlen(kv[0].key));
    memcpy (new_kv[save_cnt].value, kv[0].value, strlen(kv[0].value));
    save_cnt++;
    for(i=1;i<cnt;i++)// remove same key elemets
    {
        same_flag = 0;
        for(j=0;j<i;j++)
        {
            if(!strcmp(kv[i].key,kv[j].key)) // if same key
            {
                same_flag  = 1;
                break;
            }
        }
        if(same_flag == 0)
        {// if not same -> store to new_kv
            new_kv[save_cnt].idx = save_cnt + 1;
            new_kv[save_cnt].key_size = kv[i].key_size;
            memcpy (new_kv[save_cnt].key, kv[i].key, strlen(kv[i].key));
            memcpy (new_kv[save_cnt].value, kv[i].value, strlen(kv[i].value));
            save_cnt++;
        }
    }
    //make_file_name(18);
    make_file_name(last_file_idx + 1);// make store file name
    last_file_idx++;
    new_fp = fopen(file_name, "w");
    int key_arr[save_cnt];
    for(i=0;i<save_cnt;i++)
    {    // make key vale
        int sum = 0;
        int mul = 1000;
        printf("strlen is %d %s %d\n",i, new_kv[i].key,strlen(new_kv[i].key));
        for(j = 0;j<strlen(new_kv[i].key);j++)
        {
            sum += ((new_kv[i].key[j]-0x30)*mul);
            mul /=10;
        }
        key_arr[i] = sum;

    }
    
    for(i = 0 ; i < save_cnt ; i ++) {
        for(j = 0 ; j < save_cnt -i -1 ; j ++) {
                    if(key_arr[j]>key_arr[j+1]) {
                        int temp;
                        temp = key_arr[j];
                        key_arr[j] = key_arr[j+1];
                        key_arr[j+1] = temp;
            }
        }
    }
    
//store key-value by sorting key 
    for(i=0;i<save_cnt;i++)
    {  
        for(j=0;j<save_cnt;j++){
            if(is_same_key(key_arr[i],new_kv[j].key)){
                fprintf(new_fp,"%d %s %s\n",i+1, new_kv[j].key, new_kv[j].value);
                printf("%d %s %s\n",i+1, new_kv[j].key, new_kv[j].value);
            }
        }

    }
    merge_lcd_out(file_name, save_cnt); //lcd display result file name ,and save count
    fclose(fp[0]);
    fclose(fp[1]);
    fclose(new_fp);
    //remove 2 file
    for(i = 0;i<2;i++)
    {
        printf("%s\n",rm_file[i]);
        int nResult = remove( rm_file[i] );

        if( nResult == 0 )
        {
            printf( "파일 삭제 성공" );
        }
        else if( nResult == -1 )
        {
            perror( "파일 삭제 실패" );
        }

    }


}
void merge_lcd_out(char *file_name,int save_cnt)//if merge success display lcd file name and save count
{
    char out[100] = {0,};
    int i;
    for(i=0;i<strlen(file_name);i++)
        out[i] = file_name[i];
    out[i] = ' ';
    out[i+1] = save_cnt + 0x30;
    out[i+2] = '\0';

    write(devices[TEXT_LCD], out,strlen(out));	
}
int is_same_key(int key,char*ch_key)
{//key value compare function
// becuase compare with int key, char*key 
//ex 1234 <-> '1''2''3''4'
    int i =0;
    int div = 1000;
    for(i =0;i< strlen(ch_key);i++)
    {
        if((key / div) != (ch_key[i] -0x30) )
            return 0;
        key %= div;
        div /= 10;
    }
    return 1;
}
void motor(void){
//motor riding function
    unsigned char motor_state[3];
	
	memset(motor_state,0,sizeof(motor_state));
    printf("motor");
	motor_state[0]=MOTOR_START;//
	motor_state[1]=MOTOR_RIGHT;
	motor_state[2]=1;
	sleep(2);
	write(devices[STEP_MOTOR],motor_state,3);	
    motor_state[0]=MOTOR_STOP;//
	motor_state[1]=MOTOR_RIGHT;
	motor_state[2]=1;
    sleep(3);
    write(devices[STEP_MOTOR],motor_state,3);	
	return 0;

}




void make_file_name(int num)//make file name funciton
{
    if(num>=100)
    {
        file_name[0] = num / 100 + 0x30;
        file_name[1] = num %100 / 10 + 0x30;
        file_name[2] = num%100%10 +0x30;
        file_name[3] = '.'; file_name[4] = 's';file_name[5] = 's'; file_name[6] ='t';
        file_name[7] = '\0';

    }
    else if(num >= 10)
    {
        file_name[0] = num / 10 + 0x30;
        file_name[1] = num %10 + 0x30;
        file_name[2] = '.'; file_name[3] = 's';file_name[4] = 's'; file_name[5] ='t';
        file_name[6] = '\0';
    }
    else{
        file_name[0] = num + 0x30;
        file_name[1] = '.'; file_name[2] = 's';file_name[3] = 's'; file_name[4] ='t';
        file_name[5] = '\0';
    }
}

