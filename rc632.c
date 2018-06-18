/*
 * rc632.c
 *
 *  Created on: 05-Mar-2018
 *      Author: suman-dhara<dhara_suman@yahoo.in>
 */


#include <linux/init.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/amba/pl022.h>
#include <mach/soc.h>
#include "include/rc632.h"


struct spi_plat_data rc632_plat_data = {
	.reset_pin = RC632_RST_PIN,
};

static void spi0_cs(u32 chipselect)
{
#if (CFG_SPI0_CS_GPIO_MODE)
	if(nxp_soc_gpio_get_io_func( CFG_SPI0_CS )!= nxp_soc_gpio_get_altnum( CFG_SPI0_CS))
		nxp_soc_gpio_set_io_func( CFG_SPI0_CS, nxp_soc_gpio_get_altnum( CFG_SPI0_CS));

	nxp_soc_gpio_set_io_dir( CFG_SPI0_CS,1);
	nxp_soc_gpio_set_out_value(	 CFG_SPI0_CS , chipselect);
	//printk(KERN_INFO "RC632: CS:%d call\n",chipselect);
#else
	;
#endif
}
struct pl022_config_chip spi0_info = {
    /* available POLLING_TRANSFER, INTERRUPT_TRANSFER, DMA_TRANSFER */
    .com_mode = CFG_SPI0_COM_MODE,
    .iface = SSP_INTERFACE_MOTOROLA_SPI,
    /* We can only act as master but SSP_SLAVE is possible in theory */
    .hierarchy = SSP_MASTER,
    /* 0 = drive TX even as slave, 1 = do not drive TX as slave */
    .slave_tx_disable = 1,
    .rx_lev_trig = SSP_RX_4_OR_MORE_ELEM,
    .tx_lev_trig = SSP_TX_4_OR_MORE_EMPTY_LOC,
    .ctrl_len = SSP_BITS_8,
    .wait_state = SSP_MWIRE_WAIT_ZERO,
    .duplex = SSP_MICROWIRE_CHANNEL_FULL_DUPLEX,
    /*
     * This is where you insert a call to a function to enable CS
     * (usually GPIO) for a certain chip.
     */
#if (CFG_SPI0_CS_GPIO_MODE)
    .cs_control = spi0_cs,
#endif
    .clkdelay = SSP_FEEDBACK_CLK_DELAY_1T,

};


struct spi_board_info rc632_device_info = {
		.modalias ="rc632",
		.max_speed_hz = 3125000,
		.mode = SPI_MODE_0,//SPI_MODE_0 ,// | SPI_CPOL | SPI_CPHA,
		.bus_num = MY_BUS_NUM,
		.chip_select = 0,
		.irq = RC632_IRQ_PIN,//GPIO_IRQ(RC632_IRQ_PIN),
		.controller_data = &spi0_info,
		.platform_data = &rc632_plat_data
};

struct spi_device *rc632_device;
//static struct spi_device *spidev;
static int __init rc632_module_init(void){
	struct spi_master *master ;
	int ret;
	//spi_unregister_device(spidev);
	master = spi_busnum_to_master(rc632_device_info.bus_num);
	
	if(master == NULL){
		printk(KERN_ERR "error to get spi master for RS632\n");
		return -ENXIO;
	}
	printk(KERN_INFO "RC632: Sucessfully get SPI master LKM\n");

	rc632_device = spi_new_device(master, &rc632_device_info);
	if(rc632_device == NULL){
		printk(KERN_ERR "error to create new SPI RC632 device \n");
		return -ENODEV;
	}
	printk(KERN_INFO "RC632: Sucessfully add rc632 device to spi master LKM\n");

	rc632_device->bits_per_word = 8;
	//rc632_device->max_speed_hz = 500000;
	ret = spi_setup(rc632_device);

	if(ret){
		 printk(KERN_ERR "FAILED to setup slave.\n");
		 spi_unregister_device( rc632_device );
		 return -ENODEV;
	}
	printk(KERN_INFO "RC632: Sucessfully update controller master LKM\n");

	return 0;
}


static void __exit  rc632_module_exit(void)
{

    if( rc632_device ){
        spi_unregister_device( rc632_device );
        gpio_free(RC632_RST_PIN);                      // Free the reset_pin GPIO
        //rc632_gpio_unrgister();
        printk(KERN_INFO "RC632: Uninitializing success LKM\n");
    }
}

module_init(rc632_module_init);
module_exit(rc632_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Suman Dhara <dhara_suman@yahoo.in>");
MODULE_DESCRIPTION("RC632 SPI module");
MODULE_VERSION("0.1");
