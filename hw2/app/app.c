#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>


#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "chardev.h"



int main(int argc, char* argv[]) {

    int interval;
    int count;
    int init;
    int i;

    char data[15] = {'\0'};

    // Argument 개수 check
    if (argc != 4) {
        printf("argument error!\n");
        return -1;
    }

    //interval, count, init 변수 할당
    interval = atoi(argv[1]);
    count = atoi(argv[2]);
    init = atoi(argv[3]);

    //만약 값이 0 이면 error!
    if (interval == 0 || count == 0)
    {
        printf("Error parsing arguments!\n");
        return -1;
    }

    // interval boundary check
    if (interval < 1 || interval > 100)
    {
        printf("time interval value error!\n");
        return -1;
    }

    // count boundary check
    if (count < 1 || count > 100)
    {
        printf(" timerCount value error!(1~100)\n");
        return -1;
    }

    // init 값 check
    int flag = 0; 
    if (strlen(argv[3]) != 4) {
       printf(" timer init length error!\n");
        return -1;
    }
    for (i = 0; i < 4; i++) {
        //값의 boundary 0~8 인지 check
        if ((argv[3][i] -'0') < 0 ||(argv[3][i] -'0') > 8) {
            printf(" timer init value error!(0~8)\n");
            return -1;
        }
        if (argv[3][i] != '0') {
            //0이 아닌 값이 두개 이상인지 check
            if (flag != 0) {
                printf("timer init too many non-zero digit.\n");
                return -1;
            }
            flag = 1;
        }
    }
    if (flag == 0) {
        printf("timer init no non-zero digit.\n");
        return -1;
    }




    int fd = open(DEVICE_PATH, O_WRONLY);
    //device driver check

    if (fd == -1) {
        printf("file open error\n");
        return -1;
    }

    sprintf(data, "%03d%03d%04d", interval, count, init);

    //ioctl로 전송
    ioctl(fd, IOCTL_SET_OPTION, data);
    ioctl(fd, IOCTL_COMMAND);

    close(fd);

    return 0;

}
