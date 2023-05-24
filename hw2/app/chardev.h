#ifndef CHARDEV_H
#define CHARDEV_H

#include <linux/ioctl.h>

#define MAJOR_NUM 242

#define IOCTL_SET_OPTION _IOW(MAJOR_NUM, 0, char*)
#define IOCTL_COMMAND _IO(MAJOR_NUM, 0)

#define DEVICE_FILE_NAME "dev_driver"
#define DEVICE_PATH "/dev/dev_driver"


#endif