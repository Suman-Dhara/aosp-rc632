/*
 * rc632_input.c
 *
 *  Created on: 17-Apr-2018
 *      Author: suman-dhara<dhara_suman@yahoo.in>
 */
#include "include/rc632_input.h"
///#include "../include/rc632.h"
#include "../rc632_driver.c"
#include <linux/module.h>
//#include "../poller/include/input-polldev.h"
//#include "../chdriver/include/rc632_driver.h"
#define DEBUG 1
static bool open_flag = true;

extern int smart_card_request(struct spi_proxd *rc632, u_int8_t *ret_atqa);
extern int smart_card_halt(struct spi_proxd *rc632);
struct spi_proxd *rc632 = NULL;
int polled_rc632_probe(struct spi_proxd *rc632_ptr)
{
	int retval;
	//struct input_polled_dev **poll_dev_ptrr = &poll_dev_ptr;
	//rc632 = container_of(&poll_dev_ptrr, struct spi_proxd, poll_dev);
	//struct input_polled_dev *poll_dev_ptr = rc632_ptr->poll_dev;
	rc632 = rc632_ptr;
	{
		//poll_dev = devm_kzalloc(poll_dev->spi->dev,sizeof(*poll_dev),GFP_KERNEL);
		rc632_ptr->poll_dev = input_allocate_polled_device();
		if(!rc632_ptr->poll_dev)return -ENOMEM;
	}

	//poll_dev->private;
	rc632_ptr->poll_dev->poll = polled_rc632_poll;
	rc632_ptr->poll_dev->poll_interval = 10000;
	rc632_ptr->poll_dev->open = NULL;//polled_rc632_open;
	rc632_ptr->poll_dev->close = NULL;//polled_rc632_close;
	rc632_ptr->poll_dev->input->name = RC632_INPUT_DEV_NAME;

//	set_bit(EV_MSC,poll_dev_ptr->input->evbit);
//	set_bit(MSC_RC632,poll_dev_ptr->input->mscbit);

//	rc632_ptr->poll_dev->input->evbit[0] = BIT_MASK(EV_KEY);
//	rc632_ptr->poll_dev->input->keybit[BIT_WORD(BTN_0)] = BIT_MASK(BTN_0);

	input_set_capability(rc632_ptr->poll_dev->input, EV_MSC, MSC_SCAN);

	retval = input_register_polled_device(rc632_ptr->poll_dev);
	if(retval)
	{
		pr_err("Failed to register RC632 input device \n");
		return retval;
	}
	//reg_flag = true;
	return retval;
}

int polled_rc632_remove(struct input_polled_dev *poll_dev)
{
	if(poll_dev != NULL)
	{
		input_unregister_polled_device(poll_dev);
		input_free_polled_device(poll_dev);
		return 0;
	}
	return 1;
}

void polled_rc632_poll(struct input_polled_dev *poll_dev_ptr)
{
	u_int8_t	atqa[2]={0,0}; /* card request return ATQA always 2 byte */
	u_int16_t atqaval;
	int 	 	ret;
	mutex_lock(&rc632->buf_lock);
	if(DEBUG) printk(KERN_ERR "RC632: call polled_rc632_poll \n");
	//struct spi_proxd *rc632 = NULL;//container_of(poll_dev_ptr, struct spi_proxd, poll_dev);
	if(rc632 != NULL){
		ret = smart_card_halt(rc632);
			if(ret != 0)
			{
				if(DEBUG)printk(KERN_ERR "RC632: error at pcci Halt command \n");
			}else{
				//ret = rc632->input_dev_calback(rc632,atqa); //smart_card_request(rc632,atqa);
				ret = smart_card_request(rc632,atqa);
				if(ret == 0)
				{
					//input_event(poll_dev_ptr->input, EV_MSC, MSC_RC632, ((atqa[1]<<0x08)|atqa[0]));
					atqaval = ((atqa[1]<<0x08)|atqa[0]);
					if(atqaval != 0){
						if(DEBUG)printk(KERN_ERR "RC632: RC632: detect PICC ATQA:%X \n",atqaval);
						//input_report_key(rc632->poll_dev->input,BTN_0, 1);
						//input_report_key(rc632->poll_dev->input,BTN_0, 0);
						input_event(rc632->poll_dev->input, EV_MSC, MSC_SCAN, atqaval);
						input_sync(rc632->poll_dev->input);
					}
					//input_report_key(poll_dev->input,BTN_0,10);
				}else{
						if(DEBUG)printk(KERN_ERR "RC632: Not detect any PICC \n");
				}
			}


	}else{
		if(DEBUG)printk(KERN_ERR "RC632: spi_proxd is null \n");
	}
	mutex_unlock(&rc632->buf_lock);
}

void polled_rc632_open(struct input_polled_dev *poll_dev)
{
	int retval =0;
	printk(" call  polled_rc632_open\n");
	if(!open_flag){
		retval = schedule_delayed_work(&poll_dev->work,10000);
		open_flag = true;
	}

	//if(!reg_flag)
	{
		//printk(" call  polled_rc632_open again reg. \n");
		//poll_dev->input->open(poll_dev->input);
		//retval = input_register_polled_device(poll_dev);
		if(retval)
		{
			pr_err("Work is not in queue and added to queue %d \n",retval);
		}else{
			//reg_flag = true;
		}
	}
}

void polled_rc632_close(struct input_polled_dev *poll_dev)
{
	printk(" call  polled_rc632_close. \n");
	if(open_flag){
		cancel_delayed_work_sync(&poll_dev->work);
		open_flag = false;
	}
	//if(reg_flag)
	{
		//cancel_delayed_work_sync(poll_dev->work);
		//poll_dev->input->close(poll_dev->input);
		//input_unregister_polled_device(poll_dev);
		//reg_flag = false;
	}
}

