/*
 * Copyright (C) 2018 FORTUNA IMPLEX PVT. LTD.
 * Written by SUMAN DHARA @ Embedded System R&D Team <dhara_suman@yahoo.in>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>

#define LOG_TAG "rc632hw"

#include <cutils/log.h>


#include <linux/types.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <hardware/rc632hw.h>

#define RC632HW_DEBUG 1
#ifndef RC632_INPUT_DEV_NAME
#define RC632_INPUT_DEV_NAME	"rc632 picc evn"
#endif

#if RC632HW_DEBUG
/*#define D(...) ((void)ALOG(LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#else
#define D(...) ((void)0) */
#endif
struct input_event;

int dev_fd = -1;
int input_dev_fd = -1;

int rc632hw__block_read(char *buffer, __u16 start_block_no, __u16 block_count, picckey_t read_key)
{
    int retval;

    struct rc632_ioc_transfer rc632_tr = {
        .rx_buf = (unsigned long) buffer,
        .tx_buf = (unsigned long) NULL,
        .len = (block_count * PICC_BLOCK_LEN),
        .start_block = start_block_no,
        .block_count = block_count,
        .block_size = PICC_BLOCK_LEN,
        .picc_key = (unsigned long) &read_key.picc_key,
        .picc_key_size = read_key.picc_key_size,
        .key_type = read_key.picc_key_type
	};
    ALOGD("RC632 HW - block_read()");
    ALOGD("RC632 HW - block_read()for %d bytes called", (block_count * PICC_BLOCK_LEN));
    ALOGD("RC632 HW - block_read()for %d start_block_no", start_block_no);
    ALOGD("RC632 HW - block_read()for %d block_count", block_count);
  /*  buffer[0] ='1';
    buffer[1] ='2';
    buffer[2] ='E';
    */
    if(dev_fd <= 0){
    	ALOGE("Error at fd ");
    }
    errno = 0;
    retval = TEMP_FAILURE_RETRY(ioctl(dev_fd, RC632HW_IOC_READBLOCK, &rc632_tr));
    if(retval < 0){
        	ALOGE("RC632 HW ioctl failed :%s",strerror(errno));
        	return -errno;
        }
    return retval;
}

int rc632hw__block_write(char *buffer, __u16 start_block_no, __u16 block_count, picckey_t read_key)
{
    int retval;
    struct rc632_ioc_transfer rc632_tr = {
        .rx_buf = (unsigned long) NULL,
        .tx_buf = (unsigned long) buffer,
        .len = (block_count * PICC_BLOCK_LEN),
        .start_block = start_block_no,
        .block_count = block_count,
        .block_size = PICC_BLOCK_LEN,
        .picc_key = (unsigned long) &read_key.picc_key,
        .picc_key_size = read_key.picc_key_size,
        .key_type = read_key.picc_key_type
	};
    ALOGD("RC632 HW - block_write()for %d bytes called", (block_count * PICC_BLOCK_LEN));

    retval = ioctl(dev_fd, RC632HW_IOC_WRITEBLOCK, &rc632_tr);

    return retval;
}


int rc632hw__scan_on(unsigned int poll_interval, unsigned int poll_interval_min, unsigned int poll_interval_max)
{
    int retval;
    struct rc632_ioc_transfer rc632_tr = {
        .poll_interval = poll_interval,
        .poll_interval_min = poll_interval_min,
        .poll_interval_max = poll_interval_max
    };

    ALOGD("RC632 HW - rc632hw__scan_on()for %d poll interval", poll_interval);

    retval = ioctl(dev_fd, RC632HW_IOC_SCANMODEON, &rc632_tr);
    return retval;
}

int rc632hw__scan_off(void)
{
int retval;
    struct rc632_ioc_transfer rc632_tr = {0};

    retval = ioctl(dev_fd, RC632HW_IOC_SCANMODEOFF, &rc632_tr);
    if (retval == 0)
    {
    	ALOGD("RC632 HW - rc632hw__scan_off() called \n");
    }
    return retval;
}

int rc632hw__readEvent (void)
{
	struct input_event event;
	if(input_dev_fd <0 ) return -1;
	errno = 0;
	//ALOGE("RC632 HW input_dev_fd :%d",input_dev_fd);
	const ssize_t nread = read(input_dev_fd, &event, sizeof(struct input_event));
	//ALOGE("RC632 HW read failed :%s",strerror(errno));
	if(nread == sizeof(struct input_event))
	//if(nread > 0)
	{
		return (int) event.value;
	}
    return -1;

}

int openInput(const char* inputName) {
    int fd = -1;
    const char *dirname = "/dev/input";
    char devname[PATH_MAX];
    char *filename;
    DIR *dir;
    struct dirent *de;
    dir = opendir(dirname);
    if(dir == NULL)
        return -1;
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    while((de = readdir(dir))) {
        if(de->d_name[0] == '.' &&
                (de->d_name[1] == '\0' ||
                        (de->d_name[1] == '.' && de->d_name[2] == '\0')))
            continue;
        strcpy(filename, de->d_name);
        fd = open(devname, O_RDONLY);
        if (fd>=0) {
            char name[80];
            if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
                name[0] = '\0';
            }
            if (!strcmp(name, inputName)) {
            	ALOGE("RC632 HW p\event path :%s",devname);
                //strcpy(input_name, filename);
                break;
            } else {
                close(fd);
                fd = -1;
            }
        }
    }
    closedir(dir);
    ALOGE_IF(fd<0, "Rc632Event couldn't find '%s' input device", inputName);
    ALOGE_IF(fd>0, "Rc632Event find '%s' input device fd %d", inputName,fd);
    return fd;
}

static int open_rc632hw(const struct hw_module_t *module, char const *name,
                              struct hw_device_t **device)
{
    struct rc632hw_device_t *dev = malloc(sizeof(struct rc632hw_device_t));
    memset(dev, 0, sizeof(*dev));
    errno = 0;
    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t *)module;
    dev->rc632hw_block_read = rc632hw__block_read;
    dev->rc632hw_block_write = rc632hw__block_write;
    dev->rc632hw_scan_on = rc632hw__scan_on;
    dev->rc632hw_scan_off= rc632hw__scan_off;
    dev->rc632hw_readEvent= rc632hw__readEvent;

    *device = (struct hw_device_t *)dev;

    dev_fd = TEMP_FAILURE_RETRY(open("/dev/rc632_cdev", O_RDWR));
    if(dev_fd < 0){
    	ALOGE("RC632 HW open(/dev/rc632_cdev) failed :%s",strerror(errno));
    	return -errno;
    }
    errno = 0;
    input_dev_fd = TEMP_FAILURE_RETRY(openInput(RC632_INPUT_DEV_NAME));
    if(dev_fd < 0 || input_dev_fd < 0){
    	ALOGD("RC632 HW openInput() failed: %s",strerror(errno));
    	return -errno;
    }
    ALOGD("RC632 HW has been initialized");

    return 0;
}

static struct hw_module_methods_t rc632hw_module_methods = {
    .open = open_rc632hw
};

rc632_module_t HAL_MODULE_INFO_SYM = {
		.common = {
		        .tag                = HARDWARE_MODULE_TAG,
		        .module_api_version = RC632_MODULE_API_VERSION_1_0,
		        .hal_api_version    = HARDWARE_HAL_API_VERSION,
		        .id                 = RC632_HARDWARE_MODULE_ID,
		        .name               = "RC632 HW Module",
		        .author             = "SUMAN DHARA @ FORTUNA IMPLEX PVT. LTD(R&D TEAM)",
		        .methods            = &rc632hw_module_methods,
		    },
};
