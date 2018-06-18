/*
 * rc632io.c
 *
 *  Created on: 06-Mar-2018
 *      Author: suman-dhara<dhara_suman@yahoo.in>
 */

#include <linux/gpio.h>                 // Required for the GPIO functions
#include <linux/interrupt.h>            // Required for the IRQ code
#include <linux/delay.h>
#include <linux/kernel.h>				/* Needed for KERN_INFO */
#include <linux/interrupt.h>            // Required for the IRQ code

#include <linux/spi/spi.h>
#include "rc632.h"


/*...................................................................................................................*/

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required. Used to release the
 *  GPIOs and display cleanup messages.
 */
/*void rc632_gpio_unrgister(unsigned gpio){
   gpio_free(rc632_device->dev.platform_data->reset_pin);                      // Free the reset_pin GPIO

}*/


static irq_handler_t rc632_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);

static int rc632_rst_pin_init(u_int reset_pin) {
	int result;
	printk(KERN_INFO "RC632: Initializing the reset_pin LKM\n");

	if (!gpio_is_valid(reset_pin)) {
		printk(KERN_INFO "RC632: invalid reset_pin GPIO\n");
		return -ENODEV;
	}
	gpio_request(reset_pin, "sysfs");
	gpio_direction_output(reset_pin, 1); /**/
	return 0;
}

static void reset_rc632(){
	int reset_pin_state = 0;
	long int reset_sate_stay_time = 200;
	gpio_set_value(gpioLED, reset_pin_state);
	udelay(reset_sate_stay_time);
	gpio_set_value(gpioLED, !reset_pin_state);
	printk(KERN_INFO "RC632: reset success LKM\n");
	return;
}

irq_handler_t rc632_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs) {
	printk(KERN_INFO "GPIO_TEST: Interrupt! (button state is %d)\n", gpio_get_value(irq_pin));
	return (irq_handler_t) IRQ_HANDLED; /* Announce that the IRQ has been handled correctly */
}
/*...................................................................................................................*/

/*...................................................................................................................*/

/*@brief this will write value to the particular RC632 register and read 8 bit after value write.
 * @reg is a RC632 register address
 * @value is given register value
 * @rtx is a return 8bit value after write register and value
 *
 * this will return zero at success and return negative error value at non success.
 *
 * */
static int write_value_to_reg_then_read(byte reg, byte value , byte *rtx){
	int ret;
	reg  = ((reg << 1) & 0x7E);
	ret = spi_write(rc632_device, &value, sizeof(value));
	if(ret < 0){
		return ret;
	}
	ret = spi_w8r8(rc632_device, value);
	if(ret < 0) return ret;
	if(rtx != NULL)*rtx = ret;
	return 0;
}

static int read_value_from_reg(byte reg, byte *buf, byte buf_len){
	int ret;
	reg  = (((reg << 1) & 0x7E) | 0x80);
	ret = spi_write(rc632_device, &reg, sizeof(reg));
	if(ret < 0) return ret;
	ret = spi_read(rc632_device,buf,buf_len);
	return 0;
}

int rc632_init(void){
	byte buf;
	int ret;

	ret = rc632_rst_pin_init();
	if(ret != 0){
		printk(KERN_ERR "error at rc632_init\n");
		return -EIO;
	}

	reset_rc632();
	ret = read_value_from_reg(COMMAND,&buf,sizeof(buf));
	if(ret != 0){
		printk(KERN_ERR "error at rc632_init\n");
		return ret;
	}
	printk(KERN_INFO "Command reg val:%d\n",&buf);
	ret = write_value_to_reg_then_read(PAGE0,0x00,NULL);
	if(ret != 0){
		printk(KERN_ERR "error to write PAGE0 register at RC632 \n");
		return ret;
	}
	ret = read_value_from_reg(IRQ_PIN_CONFIG,&buf,sizeof(1));
	if(ret != 0){
		printk(KERN_ERR "error to read IRQ_PIN_CONFIG register at RC632 \n");
		return ret;
	}
	printk(KERN_INFO,"RC632 IRQ_PIN_CONFIG  value:%d\n",buf);
	buf = ((buf | 0x01) & 0xfd);
	ret = write_value_to_reg_then_read(IRQ_PIN_CONFIG, buf, NULL);
	if(ret != 0){
		printk(KERN_ERR "error to write IRQ_PIN_CONFIG register at RC632 \n");
		return ret;
	}
	printk(KERN_INFO "RC632 initialize success..\n");
	return ret;
}

/*...................................................................................................................*/
