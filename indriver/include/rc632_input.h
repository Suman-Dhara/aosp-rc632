/*
 * rc632_input.h
 *
 *  Created on: 17-Apr-2018
 *      Author: suman-dhara<dhara_suman@yahoo.in>
 */

#ifndef RC632_INPUT_H_
#define RC632_INPUT_H_

#include "../../poller/include/input-polldev.h"
#include "../../include/rc632.h"

#define RC632_INPUT_DEV_NAME "rc632 picc evn"
#define MSC_RC632	0x05

void polled_rc632_open(struct input_polled_dev *poll_dev);
void polled_rc632_close(struct input_polled_dev *poll_dev);
void polled_rc632_poll(struct input_polled_dev *poll_dev);

int polled_rc632_probe(struct spi_proxd *rc632_ptr);
int polled_rc632_remove(struct input_polled_dev *poll_dev);



#endif /* KERNEL_KERNEL_3_4_39_DRIVERS_RC632_RC632_INPUT_H_ */
