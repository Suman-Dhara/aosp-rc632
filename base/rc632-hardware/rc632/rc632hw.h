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

#ifndef __ANDROID_RC632HR_H__
#define __ANDROID_RC632HR_H__

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/ioctl.h>
#include <linux/types.h>

#include <hardware/hardware.h>

__BEGIN_DECLS

#define RC632_HARDWARE_MODULE_ID "rc632hw"


/* Block for IOCTL communication   */

struct rc632_ioc_transfer {
	__u64		tx_buf;
	__u64		rx_buf;
	__u64		picc_key;

	__u16		len;
	__u16		picc_key_size;
	__u16		block_count;
	__u16		block_size;

	__u16		start_block;
	__u16		key_type;
	__u32 poll_interval;

	__u32 poll_interval_max;
	__u32 poll_interval_min;

	/* If the contents of 'struct spi_ioc_transfer' ever change
	 * incompatibly, then the ioctl number (currently 0) must change;
	 * ioctls with constant size fields get a bit more in the way of
	 * error checking than ones (like this) where that field varies.
	 *
	 * NOTE: struct layout is the same in 64bit and 32bit userspace.
	 */
};


#define PICC_KEY_SIZE	6

/* 16+16=32
 *
 * 8*6= 48 [32+16]
 *  */
typedef struct {
	__u16 picc_key_size ;//= PICC_KEY_SIZE;
	__u16 picc_key_type;

	__u8 picc_key[PICC_KEY_SIZE];
	__u8 pad;
} picckey_t;

enum rc632_mode
{
    NORM_MODE = 0,
    SCAN_MODE 
};

#define PICC_BLOCK_LEN  16 
/* IOCTL commands */
#define RC632HW_IOC_MAGIC			'r'
/* not all platforms use <asm-generic/ioctl.h> or _IOC_TYPECHECK() ... */
#define RC632HW_MSGSIZE     (sizeof (struct rc632_ioc_transfer))

#define RC632HW_IOC_READBLOCK   _IOR(RC632HW_IOC_MAGIC, 0, char[RC632HW_MSGSIZE])
#define RC632HW_IOC_WRITEBLOCK  _IOW(RC632HW_IOC_MAGIC, 0, char[RC632HW_MSGSIZE])

#define RC632HW_IOC_SCANMODEON    _IOW(RC632HW_IOC_MAGIC, 2, char[RC632HW_MSGSIZE])
#define RC632HW_IOC_SCANMODEOFF    _IOW(RC632HW_IOC_MAGIC, 3, char[RC632HW_MSGSIZE])

/* End Block for IOCTL communication   */


struct rc632hw_device_t {
    struct hw_device_t common;

    int (*rc632hw_block_read)(char *buffer, uint16_t start_block_no, uint16_t block_count, picckey_t read_key);
    int (*rc632hw_block_write)(char *buffer, uint16_t start_block_no, uint16_t block_count, picckey_t read_key);

    int (*rc632hw_scan_on)(unsigned int poll_interval, unsigned int poll_interval_min, unsigned int poll_interval_max);
    int (*rc632hw_scan_off)(void);

    int (*rc632hw_readEvent)(void);

};

__END_DECLS

#endif // ANDROID_OPERSYSHW_INTERFACE_H
