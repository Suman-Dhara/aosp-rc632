/*
 * rc632_new.c
 *
 *  Created on: 13-Mar-2018
 *      Author: suman-dhara <dhara_suman@yahoo.in>
 */


#include <linux/init.h>
#include <linux/module.h>
#include <linux/amba/pl022.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/completion.h>

#include <linux/kernel.h>
#include <mach/platform.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>                 /* Required for the GPIO functions */
#include <linux/interrupt.h>            /* Required for the IRQ code */


#include <linux/spi/spi.h>
#include "include/rc632.h"
//#include "include/rc632_driver.h"
#include "indriver/include/rc632_input.h"
#include <asm/uaccess.h>



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define	PICC_KEY_A 	0
#define	PICC_KEY_B	1
#define BLOCK_R	0
#define	BLOCK_W	1
#define UID_LENGTH_WITH_BCC	5 /* 4 + 1CRC*/
#define UID_LENGTH (UID_LENGTH_WITH_BCC - 1)

/*...........................................................................................................*/


static LIST_HEAD(device_list);
static DEFINE_MUTEX(device_list_lock);

struct class *prox_class;
dev_t devt = 0;

/*...........................................................................................................*/
/*...........................................................................................................*/

/* @spidev, it is RC632 spi device port
 * @addr, is an RC632 internal register address,
 * @value it is value for register.
 *
 * This writes the buffer and returns zero or a negative error code.
 * Callable only from contexts that can sleep.
 * */
int rc632_byte_out(struct spi_device *spidev, u_int8_t addr, u_int8_t value)
{
	u_int8_t buff[2];
	buff[0] = RC632_WRITE_REG(addr);
	buff[1] = value;
	return spi_write(spidev,buff,sizeof(buff));
}

/*
 * @spidev, it is RC632 spi device port
 * @addr, is an RC632 internal register address,
 *
 * This returns the (unsigned) eight bit number returned by the
 * device, or else a negative error code.
 * Callable only from contexts that can sleep.
 *
 * */
int rc632_byte_in(struct spi_device *spidev, u_int8_t addr)
{
	return spi_w8r8(spidev,RC632_READ_REG(addr));
}

/*
 * @spidev, it is RC632 spi device port
 * @buff, it is a read buffer pointer.
 * @length, byte number that we want to read.
 *
 * It returns zero on success, else a negative error code.
 * */
int rc632_fifo_read(struct spi_device *spidev, u_int8_t *buff, int length)
{
	u_int8_t reg_add;

	int i	=	0;
	int ret;
	struct spi_message	message;
    struct spi_transfer	t;
	u_int8_t			*local_buf;
	u8 					rx_local_buf_start_index, tx_local_buf_start_index, tx_local_buf_size, rx_local_buf_size, local_buf_size;

	tx_local_buf_start_index = 0;
	rx_local_buf_start_index = length+1;
	tx_local_buf_size = rx_local_buf_size = length+1;
	local_buf_size = tx_local_buf_size + rx_local_buf_size;

	local_buf = kcalloc(local_buf_size, sizeof(u_int8_t), GFP_KERNEL);
	spi_message_init(&message);
	printk("FIFO-LENGTH:%02X \n",rc632_byte_in(spidev,RC632_REG_FIFO_LENGTH));

	reg_add = RC632_READ_REG(RC632_REG_FIFO_DATA);	//address of the fifo
	memset(&t, 0, sizeof t);
	memset(local_buf, reg_add, length); 		// first n positions
	local_buf[length+1] = 0x00; // as we get n'th  value at n+1'th position, it is a dummy for n+1'th place

	if (buff) {
		t.len = tx_local_buf_size;
		t.tx_buf = local_buf + tx_local_buf_start_index;
		t.rx_buf = local_buf + rx_local_buf_start_index;
		spi_message_add_tail(&t, &message);
	}
	/* do the i/o */
	ret = spi_sync(spidev, &message);
	if (ret == 0)
		memcpy(buff, (local_buf + rx_local_buf_start_index + 1), length);
	kfree(local_buf);
/*	for(i=0;i<length;i++)
	{
		ret = spi_w8r8(spidev,reg_add);
		if(ret < 0) return ret;
		buff[i] = ret;
	}*/

	if(ret < 0)
	{
		printk(KERN_ERR "RC632: error at spi buffer read: %d \n",ret);
		return ret;
	}
	for(i=0;i<length;i++)
	{
		printk("FIFO-READ RX[%d]:%02X\n",i,buff[i]);
	}
return ret;
}

/*
 * @spidev, it is RC632 spi device port
 * @buff, it is a write buffer pointer.
 * @length, It is a write buffer length.
 *
 * This writes the buffer and returns zero or a negative error code.
 * Callable only from contexts that can sleep.
 * */
int rc632_fifo_write(struct spi_device *spidev, u_int8_t *buff, int length)
{
	u_int8_t *tx_buf;
	int tx_buff_length, i, ret;

	tx_buff_length = length+1;
	tx_buf = kcalloc(tx_buff_length, sizeof(u_int8_t), GFP_KERNEL);
	*tx_buf = RC632_WRITE_REG(RC632_REG_FIFO_DATA);  //Reg. add. Masking for write

	for(i=1; i<tx_buff_length; i++)
	{
		*(tx_buf+i) = buff[i-1];
	}

	ret = spi_write(spidev, tx_buf, tx_buff_length);
	kfree(tx_buf);
	return ret;
}

/**
 * @spidev, it is RC632 spi device port
 * @addr, is an RC632 internal register address,
 * @val, masking value with register value,
 *
 * This logical or with the reg. value to set and returns zero or a negative error code.
 * Callable only from contexts that can sleep.
 * */
int rc632_set_value_mask(struct spi_device *spidev, u_int8_t adr,u_int8_t val)
{
	u_int8_t temp_buff[2];

	val =  ( val | spi_w8r8(spidev,RC632_READ_REG(adr)));
	temp_buff[0] = RC632_WRITE_REG(adr); temp_buff[1] = val;
	return spi_write(spidev, temp_buff, sizeof(temp_buff));
}


/**
 * @spidev, it is RC632 spi device port
 *
 * This writes the buffer and returns zero or a negative error code.
 * Callable only from contexts that can sleep.
 * */
int rc632_flash_fifo(struct spi_device *spidev)
{
	return rc632_set_value_mask(spidev,RC632_REG_CONTROL, 0x01);
}


/**
 * @spidev, it is RC632 spi device port
 * @addr, is an RC632 internal register address,
 * @val, masking value with register value,
 *
 * This logical and with the reg. value to clear and returns zero or a negative error code.
 * Callable only from contexts that can sleep.
 * */
int rc632_clear_value_mask(struct spi_device *spidev, u_int8_t adr, u_int8_t val)
{
	u_int8_t buff[2];
	buff[1] = spi_w8r8(spidev,RC632_READ_REG(adr));
	printk("Creat mask reg.:%02X:%02X",adr,buff[1]);
	val = ( ~val & buff[1]);
	printk("mask val:%02X \n",val);
	buff[0] = RC632_WRITE_REG(adr); buff[1] = val;
	return spi_write(spidev,buff,sizeof(buff));
}

/* x:1 = 1 ms, x:2 = 1.5 ms, x:3 = 6 ms, default = 9.6 ms */
int rc632_timer_start(struct spi_device *spidev,int x)
{
	int ret;
	ret = rc632_byte_out(spidev,RC632_REG_TIMER_CONTROL,0x02);
	if(ret < 0){
		printk(KERN_ERR "RC632: error at timer control reg. initializetion \n");
		return ret;
	}
	switch(x)
	{
		case 1:
			ret = rc632_byte_out(spidev,RC632_REG_TIMER_CLOCK,0x07);
			if (ret < 0) return ret;

			ret = rc632_byte_out(spidev,RC632_REG_TIMER_RELOAD,0x6A);
			if (ret < 0) return ret;
			break;
		case 2:
			ret = rc632_byte_out(spidev,RC632_REG_TIMER_CLOCK,0x07);
			if (ret < 0) return ret;

			ret = rc632_byte_out(spidev,RC632_REG_TIMER_RELOAD,0x6A);
			if (ret < 0) return ret;
			break;
		case 3:
			ret = rc632_byte_out(spidev,RC632_REG_TIMER_CLOCK,0x09);
			if (ret < 0) return ret;

			ret = rc632_byte_out(spidev,RC632_REG_TIMER_RELOAD,0xA0);
			if (ret < 0) return ret;
			break;
		default:
			ret = rc632_byte_out(spidev,RC632_REG_TIMER_CLOCK,0x09);
			if (ret < 0) return ret;

			ret = rc632_byte_out(spidev,RC632_REG_TIMER_RELOAD,0xFF);
			if (ret < 0) return ret;

	}
	return ret;
}
void rc632_timer_stop(struct spi_device *spidev)
{
	rc632_set_value_mask(spidev,RC632_REG_CONTROL,0x04);
}

/* Function prototype for the custom IRQ handler function  */
static irq_handler_t  rc632_irq_handler(unsigned int irq, void *dev_id)
{
	struct spi_proxd *rc632 = dev_id;
	//u_int8_t rc632_irq;
	//spin_lock_irq(&rc632->spi_lock);
	//rc632_irq = gpio_get_value(rc632->spi->irq);
	printk(KERN_INFO "RC632: IRQ call IRQ.%d",rc632->irq_number);
	//if(rc632_irq == 0x00)
	{
		complete(&rc632->rc632_work_done);
	}
	//spin_unlock_irq(&rc632->spi_lock);
	return (irq_handler_t)IRQ_HANDLED;
}

/*...........................................................................................................*/
/*...........................................................................................................*/

int smart_card_halt(struct spi_proxd *rc632)
{
	u_int8_t buff[2];
	int ret;
	struct spi_device *rc632_spi_port;

	rc632_spi_port = rc632->spi;
	//-----------------------------------------------------//
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_EN, 0x3F);
	if(ret != 0) return ret;

	ret = rc632_byte_out(rc632_spi_port,RC632_REG_INTERRUPT_RQ,0x3F);
	if(ret != 0) return ret;

	rc632_flash_fifo(rc632_spi_port);
	/* ISO/IEC 14443 HALT command */
	buff[0] = PICC_HALT_H;
	buff[1] = PICC_HALT_L;
	printk("VAL:%02X\n",buff[0]);
	ret = rc632_fifo_write(rc632_spi_port, buff, sizeof(buff));
	if(ret != 0) return ret;

	ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_EN, 0x8C);
	if(ret != 0) return ret;
	printk("RC632: INTR_EN val: %02X",rc632_byte_in(rc632_spi_port,RC632_REG_INTERRUPT_EN));

	ret = rc632_byte_out(rc632_spi_port,RC632_REG_INTERRUPT_RQ,0x3F);
	if(ret != 0) return ret;

	ret = rc632_byte_out(rc632_spi_port,RC632_REG_COMMAND,RC632_CMD_TRANSCEIVE);
	if(ret != 0) return ret;

	//-----------------------------------------------------//
	//smart_card_get_event(SMART_ENENT_TIME);
	//ret = smart_card_get_event(SMART_ENENT_TIME);
	ret = wait_for_completion_timeout(&rc632->rc632_work_done,rc632->pcci_wait_time);
	if(ret == 0){
		rc632_flash_fifo(rc632_spi_port);
		rc632_byte_out(rc632_spi_port,RC632_REG_COMMAND, RC632_CMD_IDLE);
		//return EBUSY;
	}
	//-----------------------------------------------------//

	ret = rc632_byte_out(rc632_spi_port,RC632_REG_INTERRUPT_EN, 0x3f);
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_INTERRUPT_RQ, 0x3f);
	ret = rc632_flash_fifo(rc632_spi_port);
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_COMMAND, RC632_CMD_IDLE);
	//-----------------------------------------------------//
	return ret;
}

int smart_card_request(struct spi_proxd *rc632, u_int8_t *ret_atqa)
{
	int ret;
	struct spi_device *rc632_spi_port;
	rc632_spi_port = rc632->spi;
	//-----------------------------------------------------//
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_CHANNEL_REDUNDANCY, 0x03);
	if(ret != 0) return ret;
	ret = rc632_clear_value_mask(rc632_spi_port,RC632_REG_CONTROL,0x08);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_BIT_FRAMING, 0x07);
	if(ret != 0) return ret;
	ret = rc632_set_value_mask(rc632_spi_port,RC632_REG_TX_CONTROL,0x03);
	if(ret != 0) return ret;
	rc632_byte_out(rc632_spi_port,RC632_REG_INTERRUPT_EN, 0x3f);
	rc632_byte_out(rc632_spi_port,RC632_REG_INTERRUPT_RQ, 0x3f);
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_COMMAND, RC632_CMD_IDLE);
	if(ret != 0) return ret;

	ret = rc632_flash_fifo(rc632_spi_port);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_FIFO_DATA, PICC_WUPA); /* Wake-up command */
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_INTERRUPT_EN, 0x88);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_INTERRUPT_RQ, 0x3F);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_COMMAND, RC632_CMD_TRANSCEIVE);
	if(ret != 0) return ret;
	//-----------------------------------------------------//
	//ret = smart_card_get_event(SMART_ENENT_TIME);
	ret = 1;wait_for_completion_interruptible_timeout(&rc632->rc632_work_done,rc632->pcci_wait_time);
	//while( gpio_get_value(rc632_spi_port->irq) == 0);
	if(ret == 0) return EBUSY;
	//-----------------------------------------------------//
	if(rc632_byte_in(rc632_spi_port,RC632_REG_FIFO_LENGTH) == 0x02)
	{
		ret = rc632_fifo_read(rc632_spi_port, ret_atqa ,2);
	}else{
		ret = 1;
	}
	//-----------------------------------------------------//
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_COMMAND, RC632_CMD_IDLE);
	//-----------------------------------------------------//
	return ret;
	//-----------------------------------------------------//

}

int smart_card_anticollision(struct spi_proxd *rc632, u_int8_t *ret_uid_buff, u_int8_t *ret_uid_buff_length_with_bcc)
{
	int ret;
	struct spi_device *rc632_spi_port;
	u_int8_t command_buff[2];
	rc632_spi_port = rc632->spi;
	//-----------------------------------------------------//
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_DECODER_CONTROL, 0x28);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_CHANNEL_REDUNDANCY, 0x03);
	if(ret != 0) return ret;
	ret = rc632_clear_value_mask(rc632_spi_port,RC632_REG_CONTROL,0x08);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_INTERRUPT_EN, 0x3f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_INTERRUPT_RQ, 0x3f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_COMMAND, RC632_CMD_IDLE);
	if(ret != 0) return ret;
	ret = rc632_flash_fifo(rc632_spi_port);
	if(ret != 0) return ret;
	command_buff[0] = PICC_ANTICOLLISION_CL1_H;
	command_buff[1] = PICC_ANTICOLLISION_CL1_L;
	//ret = spi_write(rc632_spi_port, command_buff, );
	ret = rc632_fifo_write(rc632_spi_port,command_buff,sizeof(command_buff));
	if(ret != 0) return ret;

	ret = rc632_byte_out(rc632_spi_port,RC632_REG_INTERRUPT_EN, 0x88);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_INTERRUPT_RQ, 0x3f);
	if(ret != 0) return ret;

	printk("send command \n");
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_COMMAND, RC632_CMD_TRANSCEIVE);
	if(ret != 0) return ret;
	
	
	//-----------------------------------------------------//
	ret = wait_for_completion_interruptible_timeout(&rc632->rc632_work_done,(rc632->pcci_wait_time));
	//if(ret == 0) return EBUSY;
	//-----------------------------------------------------//

		if((ret_uid_buff_length_with_bcc != NULL) && (*ret_uid_buff_length_with_bcc = rc632_byte_in(rc632_spi_port,RC632_REG_FIFO_LENGTH)) == UID_LENGTH_WITH_BCC && ret_uid_buff != NULL)
		{
			rc632_fifo_read(rc632_spi_port, ret_uid_buff, *ret_uid_buff_length_with_bcc);
			printk("UID[0] :%02X\n",ret_uid_buff[0]);
			printk("UID[1] :%02X\n",ret_uid_buff[1]);
			printk("UID[2] :%02X\n",ret_uid_buff[2]);
			printk("UID[3] :%02X\n",ret_uid_buff[3]);
			printk("UID[4] :%02X\n",ret_uid_buff[4]);
			ret = 0;
		}
		else{	ret = 1;	}
	//-----------------------------------------------------//
	rc632_clear_value_mask(rc632_spi_port,RC632_REG_DECODER_CONTROL, 0x20);
	rc632_byte_out(rc632_spi_port,RC632_REG_COMMAND, RC632_CMD_IDLE);
	//-----------------------------------------------------//
	return ret;
	//-----------------------------------------------------//
}
/**
 * Select AcKnowledge (SAK) code which determines the type of the selected card.
 */
int smart_card_select(struct spi_proxd *rc632, u_int8_t *uid_buff, u_int8_t uid_buff_length_with_bcc, u_int8_t *ret_sak)
{
	int ret;
	struct spi_device *rc632_spi_port;
	u_int8_t *tmp_buff = kcalloc((uid_buff_length_with_bcc+2), sizeof(u_int8_t), GFP_KERNEL);; //uid_length+2;
	rc632_spi_port = rc632->spi;

	ret = rc632_byte_out(rc632_spi_port,RC632_REG_CHANNEL_REDUNDANCY, 0x0f);
	if(ret != 0) return ret;
	ret = rc632_clear_value_mask(rc632_spi_port,RC632_REG_CONTROL,0x08);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_INTERRUPT_EN, 0x3f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_INTERRUPT_RQ, 0x3f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_COMMAND, RC632_CMD_IDLE);
	if(ret != 0) return ret;

	ret = rc632_flash_fifo(rc632_spi_port);
	if(ret != 0) return ret;
	tmp_buff[0] = PICC_SELECT_CL1_H;
	tmp_buff[1] = PICC_SELECT_CL1_L;
	tmp_buff[2] = uid_buff[0];
	tmp_buff[3] = uid_buff[1];
	tmp_buff[4] = uid_buff[2];
	tmp_buff[5] = uid_buff[3];
	tmp_buff[6] = uid_buff[4];

/*	for(i =2,j=0; j < uid_length; j++,i++)
	{
		tmp_buff[i] = uid_buff[j];
	}

	tmp_buff[2] = uid_buff[UID_LENGTH-5]; //0
	tmp_buff[3] = uid_buff[UID_LENGTH-4]; //1
	tmp_buff[4] = uid_buff[UID_LENGTH-3]; //2
	tmp_buff[5] = uid_buff[UID_LENGTH-2]; //3
	tmp_buff[6] = uid_buff[UID_LENGTH-1]; // snr_chk; 4
*/
	printk("call fifo_write start\n");
	ret = rc632_fifo_write(rc632_spi_port,tmp_buff,(uid_buff_length_with_bcc+2));
	printk("call fifo_write end\n");
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_EN, 0x88);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_RQ, 0x3f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_COMMAND, RC632_CMD_TRANSCEIVE);
	if(ret != 0) return ret;
	//-----------------------------------------------------//
	ret = wait_for_completion_timeout(&rc632->rc632_work_done,rc632->pcci_wait_time);
	//if(ret == 0) return EBUSY;
	//-----------------------------------------------------//
	ret = 0; /* no error */

	tmp_buff[0] = rc632_byte_in(rc632_spi_port,RC632_REG_FIFO_LENGTH);
	printk("RC632: FIFOLEN:%02X",tmp_buff[0]);
	if(tmp_buff[0] == 1)
	{
		if(ret_sak != NULL)*ret_sak = rc632_byte_in(rc632_spi_port,RC632_REG_FIFO_DATA);	//get ATS (card memory type)
	}
	else{	ret = 1;	}

	//-----------------------------------------------------//
	//rc632_byte_out(cmnd, 0x00);
	kfree(tmp_buff);
	//-----------------------------------------------------//
	return ret;
	//---------------------
}

void smart_card_format_key(u_int8_t *key , int size, u_int8_t *ret_key, int *ret_size)
{

	unsigned char x = 0,y = 0,z = 0;

	for(x = 0; x < size; x++)
	{
		y = ((*key >> 4) & 0x0F);
		z = (((~(*key)) & 0xF0) | y);
		ret_key[2 * x] = z;
		y = (*key & 0x0F);
	    z =  ((~(*key << 4) & 0xF0) | y);
	    ret_key[2 * x + 1] = z;key++;
	}
	*ret_size = 12;
}

int smart_card_load_key(struct spi_proxd *rc632, u_int8_t *key, int size)
{
	int ret;
	u_int8_t formeted_key_buffer[12];
	int formeted_key_size;
	struct spi_device *rc632_spi_port;
	rc632_spi_port = rc632->spi;
	//-----------------------------------------------------//
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_INTERRUPT_EN, 0x3f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_INTERRUPT_RQ, 0x3f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_COMMAND, RC632_CMD_IDLE);
	if(ret != 0) return ret;

	ret = rc632_flash_fifo(rc632_spi_port);
	if(ret != 0) return ret;
	smart_card_format_key(key,size, formeted_key_buffer,&formeted_key_size);
	if(ret != 0) return ret;
	ret = rc632_fifo_write(rc632_spi_port,formeted_key_buffer, formeted_key_size);//sm_key_buffer
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_INTERRUPT_EN, 0x84);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_INTERRUPT_RQ, 0x3f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port,RC632_REG_COMMAND, RC632_CMD_LOAD_KEY);
	if(ret != 0) return ret;

	//-----------------------------------------------------//
	ret = wait_for_completion_timeout(&rc632->rc632_work_done,rc632->pcci_wait_time);
	//if(ret == 0) return EBUSY;
	//-----------------------------------------------------//
	ret = 0; /* no error */

	if((rc632_byte_in(rc632_spi_port,RC632_REG_ERROR_FLAG) & 0x40) == 0x40)
	{
		ret = 1;
	}
	else{	ret = 0;	}
	//-----------------------------------------------------//
	rc632_byte_out(rc632_spi_port,RC632_REG_COMMAND, RC632_CMD_IDLE);
	//-----------------------------------------------------//
	return ret;
	//-----------------------------------------------------//
}

int smart_card_authourization_1(struct spi_proxd *rc632, u_int8_t blk_no, u_int8_t pcci_key_type, u_int8_t *uid_buff,u_int8_t uid_length)
{
	int ret, i, j;
	struct spi_device *rc632_spi_port;
	u_int8_t *tmp_buff = kcalloc((uid_length+2), sizeof(u_int8_t), GFP_KERNEL);; //uid_length+2;
	rc632_spi_port = rc632->spi;
	//-----------------------------------------------------//
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_EN, 0x3f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_RQ, 0x3f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_COMMAND, RC632_CMD_IDLE);
	if(ret != 0) return ret;
	ret = rc632_flash_fifo(rc632_spi_port);
	if(ret != 0) return ret;
	//-----------------------------------------------------//
	if(pcci_key_type == PICC_KEY_A)
	{ 	tmp_buff[0] = PICC_AUTH_WITH_KEY_A;	}
	else
	{ 	tmp_buff[0] = PICC_AUTH_WITH_KEY_B; }
	//-----------------------------------------------------//

	tmp_buff[1] = blk_no;
	for(i=2,j=0; j< uid_length ; i++,j++)
	{
		tmp_buff[i] = uid_buff[j];
		printk(KERN_INFO"auth[%d]:%02X \n",i,tmp_buff[i]);
	}

/*	tmp_buff[3] = rc632->snr[1];
	tmp_buff[4] = rc632->snr[2];
	tmp_buff[5] = rc632->snr[3];*/
	ret = rc632_fifo_write(rc632_spi_port,tmp_buff, (uid_length+2));
	if(ret != 0) return ret;

	ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_EN, 0x84);
	if(ret != 0) return ret;

	ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_RQ, 0x3f);
	if(ret != 0) return ret;

	ret = rc632_byte_out(rc632_spi_port, RC632_REG_COMMAND, RC632_CMD_AUTHENT1);
	if(ret != 0) return ret;
	//-----------------------------------------------------//
	//error = smart_card_get_event(SMART_ENENT_TIME);
	ret = wait_for_completion_timeout(&rc632->rc632_work_done,rc632->pcci_wait_time);
	//if(ret == 0) return EBUSY;
	//-----------------------------------------------------//
	ret = 0; /* no error */
//	if(!ret)
//	{
		if((ret=rc632_byte_in(rc632_spi_port, RC632_REG_SECONDARY_STATUS) & 0x07) != 0x00)
		{	printk(KERN_INFO"SECONDARY_STATUS :%02X\n",ret);ret = 1;	}
		else{	ret = 0;	}
//	}
	//-----------------------------------------------------//
	//rc632_byte_out(spidev, cmnd, 0x00);
	kfree(tmp_buff);
	//-----------------------------------------------------//
	return ret;
	//-----------------------------------------------------//
}

int smart_card_authourization_2(struct spi_proxd *rc632)
{
	int ret;
	struct spi_device *rc632_spi_port;
	rc632_spi_port = rc632->spi;
	//-----------------------------------------------------//
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_EN, 0x3f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_RQ, 0x3f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_COMMAND, RC632_CMD_IDLE);
	if(ret != 0) return ret;

	ret = rc632_flash_fifo(rc632_spi_port);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_EN, 0x84);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_RQ, 0x3f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_COMMAND, RC632_CMD_AUTHENT2);
	if(ret != 0) return ret;

	//-----------------------------------------------------//
	//error = smart_card_get_event(SMART_ENENT_TIME);
	ret = wait_for_completion_timeout(&rc632->rc632_work_done,rc632->pcci_wait_time);
	//if(ret == 0) return EBUSY;
	//-----------------------------------------------------//

	if((ret = rc632_byte_in(rc632_spi_port, RC632_REG_CONTROL) & 0x08) != 0x08)
	{
		printk(KERN_INFO"REG_CONTROL :%02X\n",ret);
		ret = 1;
	}
	else
	{	ret = 0;	}

	//-----------------------------------------------------//
	rc632_byte_out(rc632_spi_port, RC632_REG_COMMAND, RC632_CMD_IDLE);
	//-----------------------------------------------------//
	return ret;
	//-----------------------------------------------------//
}

int smart_card_read(struct spi_proxd *rc632, u_int8_t blk_no, u_int8_t *ret_buff , int *ret_size)
{
	int ret;
	u_int8_t tmp_buff[2];
	struct spi_device *rc632_spi_port;
	rc632_spi_port = rc632->spi;
	//-----------------------------------------------------//
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_CHANNEL_REDUNDANCY, 0x0f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_EN, 0x3f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_RQ, 0x3f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_COMMAND, RC632_CMD_IDLE);
	if(ret != 0) return ret;

	ret = rc632_flash_fifo(rc632_spi_port);
	if(ret != 0) return ret;
	tmp_buff[0] = PICC_MIFARE_READ;
	tmp_buff[1] = blk_no;
	ret = rc632_fifo_write(rc632_spi_port, tmp_buff, sizeof(tmp_buff));
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_EN, 0x8c);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_RQ, 0x3f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_COMMAND, RC632_CMD_TRANSCEIVE);
	if(ret != 0) return ret;

	//-----------------------------------------------------//
	ret = wait_for_completion_timeout(&rc632->rc632_work_done,rc632->pcci_wait_time);
	//if(ret == 0) return EBUSY;
	//-----------------------------------------------------//

	ret = 0; /* no error */
	*ret_size = rc632_byte_in(rc632_spi_port, RC632_REG_FIFO_LENGTH);
	if(*ret_size == 0x10)
	{	ret = 0;rc632_fifo_read(rc632_spi_port, ret_buff, *ret_size);	}
	else
	{	ret = 1;	}

	//-----------------------------------------------------//
	rc632_byte_out(rc632_spi_port, RC632_REG_COMMAND, RC632_CMD_IDLE);
	//-----------------------------------------------------//
	return ret;
	//-----------------------------------------------------//
}

int smart_card_write(struct spi_proxd *rc632, u_int8_t blk_no, u_int8_t *buff, int size )
{
	int ret = 0;
	u_int8_t tmp_buff[2];

	struct spi_device *rc632_spi_port;
	rc632_spi_port = rc632->spi;

	//-----------------------------------------------------//
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_CHANNEL_REDUNDANCY, 0x0f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_EN, 0x3f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_RQ, 0x3f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_COMMAND, RC632_CMD_IDLE);
	if(ret != 0) return ret;

	ret = rc632_flash_fifo(rc632_spi_port);
	if(ret != 0) return ret;
	tmp_buff[0] = PICC_MIFARE_WRITE;
	tmp_buff[1] = blk_no;
	ret = rc632_fifo_write(rc632_spi_port, tmp_buff, sizeof(tmp_buff));
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_EN, 0x84);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_RQ, 0x3f);
	if(ret != 0) return ret;
	ret = rc632_byte_out(rc632_spi_port, RC632_REG_COMMAND, RC632_CMD_TRANSCEIVE);
	if(ret != 0) return ret;
	//-----------------------------------------------------//

	//error = smart_card_get_event(SMART_ENENT_TIME);
	ret = wait_for_completion_timeout(&rc632->rc632_work_done,rc632->pcci_wait_time);
	//if(ret == 0) return EBUSY;
	//-----------------------------------------------------//
	ret = 0; /* no error */
	if(!((rc632_byte_in(rc632_spi_port, RC632_REG_FIFO_DATA) & 0x0f) == 0x0a))
	{
		rc632_byte_out(rc632_spi_port, RC632_REG_COMMAND, RC632_CMD_IDLE);
		return -1; /* return error */
	}

		ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_EN, 0x3f);
		if(ret != 0) return ret;
		ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_RQ, 0x3f);
		if(ret != 0) return ret;
		ret = rc632_byte_out(rc632_spi_port, RC632_REG_COMMAND, RC632_CMD_IDLE);
		if(ret != 0) return ret;

		ret = rc632_flash_fifo(rc632_spi_port);
		if(ret != 0) return ret;
		ret = rc632_fifo_write(rc632_spi_port,buff,size);
		if(ret != 0) return ret;
		ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_EN, 0x84);
		if(ret != 0) return ret;
		ret = rc632_byte_out(rc632_spi_port, RC632_REG_INTERRUPT_RQ, 0x3f);
		if(ret != 0) return ret;
		ret = rc632_byte_out(rc632_spi_port, RC632_REG_COMMAND, RC632_CMD_TRANSCEIVE);
		if(ret != 0) return ret;

		//-----------------------------------------------------//
		//error = smart_card_get_event(SMART_ENENT_TIME);
		ret = wait_for_completion_timeout(&rc632->rc632_work_done,rc632->pcci_wait_time);
		//if(ret == 0) return EBUSY;
		//-----------------------------------------------------//
		ret = 0 ; /* no error */
		if( (rc632_byte_in(rc632_spi_port, RC632_REG_FIFO_DATA) & 0x0f) == 0x0a)
		{	ret = 0;	}
		else
		{	ret = 1;	}

	//-----------------------------------------------------//
	rc632_byte_out(rc632_spi_port, RC632_REG_COMMAND, RC632_CMD_IDLE);
	//-----------------------------------------------------//
	return ret;
	//-----------------------------------------------------//
}


int pcci_operation(struct spi_proxd *rc632, u_int8_t r_or_w, u_int8_t blk_no ,u_int8_t *rw_buff , int *rw_buff_len)
{
	u_int8_t	atqa[2]={0,0}; /* card request return ATQA always 2 byte */
	u_int8_t 	uid_buff[UID_LENGTH_WITH_BCC]; /* snr[4] = snr_chk */
	u_int8_t 	sak; 	/* card memory type .ATS*/
	u_int8_t 	uid_buff_length_with_bcc;
	int 	 	ret;

	if(blk_no == 0 && r_or_w == BLOCK_W)
	{
		pr_err("Block:%d not writable \n",blk_no);
		return -1;
	}

	ret = smart_card_halt(rc632);
	if(ret != 0)
	{
		printk(KERN_ERR "RC632: error at pcci Halt command \n");
		return ret;
	}

	ret = smart_card_request(rc632,atqa);
	if(ret != 0)
	{
		printk(KERN_ERR "RC632: error at pcci card request command \n");
		return ret;
	}
	printk(KERN_ERR "RC632: after card_request ATQA[0]:%02X::ATQA[1]:%02X \n",atqa[0],atqa[1]);

	ret = smart_card_anticollision(rc632, uid_buff, &uid_buff_length_with_bcc);
	if(ret != 0)
	{
		printk(KERN_ERR "RC632: error at pcci anticollision command \n");
		return ret;
	}

	ret = smart_card_select(rc632,uid_buff,uid_buff_length_with_bcc,&sak);
	if(ret != 0)
	{
		printk(KERN_ERR "RC632: error at pcci card select command \n");
		return ret;
	}
	printk(KERN_INFO "RC632: card select SAK:%02X\n",sak);
	ret = smart_card_load_key(rc632, rc632->pcci_key, rc632->pcci_key_size);
	if(ret != 0)
	{
		printk(KERN_ERR "RC632: error at pcci load key command \n");
		return ret;
	}

	ret = smart_card_authourization_1(rc632, blk_no, rc632->pcci_key_type, uid_buff, UID_LENGTH);
	if(ret != 0)
	{
		printk(KERN_ERR "RC632: error at pcci authourization-1 command \n");
		return ret;
	}

	ret = smart_card_authourization_2(rc632);
	if(ret != 0)
	{
		printk(KERN_ERR "RC632: error at pcci authourization-2 command \n");
		return ret;
	}

	if(r_or_w == BLOCK_W)
	{
		ret = smart_card_write(rc632,blk_no,rw_buff,*rw_buff_len);
		if(ret != 0)
		{
			printk(KERN_ERR "RC632: error at pcci write command \n");
			return ret;
		}

	}
	else
	{
		ret = smart_card_read(rc632,blk_no,rw_buff,rw_buff_len);
		if(ret != 0)
		{
			printk(KERN_ERR "RC632: error at pcci read command \n");
			return ret;
		}

	}
return ret;
}
/*...........................................................................................................*/
/*...........................................................................................................*/

static int open_rc632(struct inode *inode, struct file *filp)
{
	struct spi_proxd *rc632 = NULL;
	int			status;
	printk(KERN_INFO "RC632: open_rc632 \n");
	rc632 = container_of(inode->i_cdev,struct spi_proxd,rc632_cdev);
	if(rc632 == NULL ) return -ENODEV;
	filp->private_data = rc632;
	status = nonseekable_open(inode, filp);
	printk(KERN_INFO "RC632: nonseekable_open status: %d \n",status);
	return status;
}

static int release_rc632(struct inode *inode, struct file *filp)
{
	if(filp->private_data != NULL)
	{
		filp->private_data = NULL;
	}
	return 0;
}

static long rc632_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int	err = 0;
	struct spi_proxd	*rc632;
	struct spi_device	*spi;
	struct	rc632_ioc_transfer *rc632_tr;
	u32 temp;

	printk(KERN_INFO "RC632: rc632_ioctl cmd %d \n",cmd);
	/* Check type and command number */
	if (_IOC_TYPE(cmd) != RC632HW_IOC_MAGIC)
		return -ENOTTY;

	/* Check access direction once here; don't repeat below.
	 * IOC_DIR is from the user perspective, while access_ok is
	 * from the kernel perspective; so they look reversed.
	 * */
		if (_IOC_DIR(cmd) & _IOC_READ)
			err = !access_ok(VERIFY_WRITE,
					(void __user *)arg, _IOC_SIZE(cmd));
		if (err == 0 && _IOC_DIR(cmd) & _IOC_WRITE)
			err = !access_ok(VERIFY_READ,
					(void __user *)arg, _IOC_SIZE(cmd));
		if (err)
			return -EFAULT;

		/* guard against device removal before, or while,
		 * we issue this ioctl.
		 */

		rc632 = (struct spi_proxd *) filp->private_data;
		spin_lock_irq(&rc632->spi_lock);
		spi = spi_dev_get(rc632->spi);
		spin_unlock_irq(&rc632->spi_lock);

		if (spi == NULL)
			err = -ESHUTDOWN;

		temp = _IOC_SIZE(cmd);
		if ((temp % sizeof(struct rc632_ioc_transfer)) != 0) {
			err = -EINVAL;
		}
		printk(KERN_INFO "RC632: rc632_ioc_transfer size %d \n",temp);
		/* use the buffer lock here for triple duty:
		 *  - prevent I/O (from us) so calling spi_setup() is safe;
		 *  - prevent concurrent SPI_IOC_WR_* from morphing
		 *    data fields while RC632_IOC_RD_* reads them;
		 *
		 */
		mutex_lock(&rc632->buf_lock);

		err = 0;
		rc632_tr = kmalloc(sizeof(struct rc632_ioc_transfer),GFP_KERNEL);

		if(__copy_from_user(rc632_tr,(void __user *)arg, temp))
		{
			err = -EFAULT;
			goto err_with_dy_mem;
		}
		printk(KERN_INFO "RC632: __copy_from_user ok  \n");
		if((cmd == RC632HW_IOC_READBLOCK) || (cmd == RC632HW_IOC_WRITEBLOCK))
		{
			temp = (rc632_tr->block_count*rc632_tr->block_size);

			rc632->buffer = kcalloc(temp, sizeof(u8), GFP_KERNEL);
			rc632->pcci_key = kcalloc(rc632_tr->pcci_key_size, sizeof(u8), GFP_KERNEL);

			if ((!rc632_tr) || (!rc632->buffer) || (!rc632->pcci_key)) {
				err = -ENOMEM;
				goto err_with_nody_mem;
			}

		}


		switch(cmd)
		{
			case RC632HW_IOC_READBLOCK:
				polled_rc632_close(rc632->poll_dev);
				printk(KERN_INFO "RC632: __RC632HW_IOC_READBLOCK \n");
				rc632->pcci_key_type = rc632_tr->key_type;

				if (__copy_from_user(rc632->pcci_key, (const u8 __user *) (uintptr_t) rc632_tr->pcci_key, rc632_tr->pcci_key_size)) {
					err = -EFAULT;
					goto err_with_dy_mem;
				}

				printk(KERN_INFO "RC632: rc632->pcci_key[0]:%x \n",rc632->pcci_key[0]);
				printk(KERN_INFO "RC632: rc632->pcci_key[1]:%x \n",rc632->pcci_key[1]);
				printk(KERN_INFO "RC632: rc632->pcci_key[2]:%x \n",rc632->pcci_key[2]);
				printk(KERN_INFO "RC632: rc632->pcci_key[3]:%x \n",rc632->pcci_key[3]);

				temp = (rc632_tr->block_count*rc632_tr->block_size);
				printk(KERN_INFO "RC632: Read Block No: %d \n",rc632_tr->start_block);
				printk(KERN_INFO "RC632: Reab Buffer Len: %d \n",temp);

				err = pcci_operation(rc632, BLOCK_R, rc632_tr->start_block, rc632->buffer, &temp);

				if(err != 0) goto err_with_dy_mem;

				if (__copy_to_user((u8 __user *)
									(uintptr_t) rc632_tr->rx_buf, rc632->buffer,
									rc632_tr->len)) {
					err = -EFAULT;
				}
				polled_rc632_open(rc632->poll_dev);
				break;

			case RC632HW_IOC_WRITEBLOCK:
				polled_rc632_close(rc632->poll_dev);
				printk(KERN_INFO "RC632: RC632HW_IOC_WRITEBLOCK \n");
				rc632->pcci_key_type = rc632_tr->key_type;

				if (__copy_from_user(rc632->pcci_key, (const u8 __user *) (uintptr_t) rc632_tr->pcci_key, rc632_tr->pcci_key_size))
				{
					err = -EFAULT;
					goto err_with_dy_mem;
				}

				temp = (rc632_tr->block_count*rc632_tr->block_size);
				if (__copy_from_user(rc632->buffer, (const u8 __user *) (uintptr_t) rc632_tr->tx_buf, temp))
				{
					err = -EFAULT;
					goto err_with_dy_mem;
				}
				err = pcci_operation(rc632, BLOCK_W, rc632_tr->start_block, rc632->buffer, &temp);
				polled_rc632_open(rc632->poll_dev);

				if(err != 0) goto err_with_dy_mem;
				break;

			case RC632HW_IOC_SCANMODEON:
				printk(KERN_INFO "RC632: RC632HW_IOC_SCANMODEON \n");
				//rc632->poll_dev->poll_interval = (rc632_tr->poll_interval)?rc632_tr->poll_interval:rc632->poll_dev->poll_interval;
				//rc632->poll_dev->poll_interval_max = (rc632_tr->poll_interval_max)?rc632_tr->poll_interval_max:rc632->poll_dev->poll_interval_max;
				//rc632->poll_dev->poll_interval_min = (rc632_tr->poll_interval_min)?rc632_tr->poll_interval_min:rc632->poll_dev->poll_interval_min;
				polled_rc632_open(rc632->poll_dev);
				goto scan_mod;
				break;

			case RC632HW_IOC_SCANMODEOFF:
				printk(KERN_INFO "RC632: RC632HW_IOC_SCANMODEOFF \n");
				polled_rc632_close(rc632->poll_dev);
				goto scan_mod;
				break;
		}
err_with_dy_mem:
		//printk(KERN_INFO "RC632: kfree(rc632_tr) \n");
		if(rc632->buffer) kfree(rc632->buffer);
		//printk(KERN_INFO "RC632: kfree(rc632->buffer) \n");
		if(rc632->pcci_key) kfree(rc632->pcci_key);
scan_mod:	//printk(KERN_INFO "RC632:  kfree(rc632->pcci_key)\n");
		if(rc632_tr)kfree(rc632_tr);
err_with_nody_mem:
		mutex_unlock(&rc632->buf_lock);
		//printk(KERN_INFO "RC632: mutex_unlock(&rc632->buf_lock); \n");
		return err;
}

#ifdef CONFIG_COMPAT
static long
rc632_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return rc632_ioctl(filp, cmd, (unsigned long)compat_ptr(arg));
}
#else
#define rc632_compat_ioctl NULL
#endif /* CONFIG_COMPAT */

struct file_operations rc632_fops = {
	.owner =    THIS_MODULE,
	.read =     NULL,
	.write =    NULL,
	.open =     open_rc632,
	.release =  release_rc632,
	.unlocked_ioctl = rc632_ioctl,
	.compat_ioctl = rc632_compat_ioctl
};

/*...........................................................................................................*/

int rc632_init(struct spi_device *spi)
{
	u_int8_t buf, buf2;
	u_int8_t ret;

	//rc632_device = spi;
	ret = rc632_rst_pin_init(RC632_RST_PIN);
	if(ret != 0){
		printk(KERN_ERR "error at rc632_init\n");
		return -EIO;
	}

	reset_rc632();
	ret = rc632_byte_in(spi,RC632_REG_COMMAND);
	if(ret < 0){
		printk(KERN_ERR "error at rc632_init\n");
		return ret;
	}
	printk(KERN_INFO "Command reg val:%02X\n",ret);
	ret = write_value_to_reg_then_read(spi,RC632_REG_PAGE0,0x00,&buf);
	printk(KERN_INFO "PAGE0 reg val:%02X\n",buf);
	if(ret != 0){
		printk(KERN_ERR "error to write PAGE0 register at RC632 \n");
		return ret;
	}
	buf = rc632_byte_in(spi,RC632_REG_IRQ_PIN_CONFIG);
	if(buf < 0){
		printk(KERN_ERR "error to read IRQ_PIN_CONFIG register at RC632 \n");
		return buf;
	}
	printk(KERN_INFO "RC632 IRQ_PIN_CONFIG  value:%02X\n",buf);
	buf = ((buf | 0x01) & 0xfd);
	printk(KERN_INFO "RC632 edited IRQ_PIN_CONFIG  value:%02X\n",buf);
	ret = write_value_to_reg_then_read(spi,RC632_REG_IRQ_PIN_CONFIG, buf, &buf2);
	printk(KERN_INFO "IRQ_PIN_CONFIG reg val:%02X\n",buf2);
	printk(KERN_INFO "IRQ_PIN_CONFIG reg val:%02X\n",rc632_byte_in(spi,RC632_REG_IRQ_PIN_CONFIG));
	if(ret != 0){
		printk(KERN_ERR "error to write IRQ_PIN_CONFIG register at RC632 \n");
		return ret;
	}
	printk(KERN_INFO "RC632 initialize success..\n");
	return ret;
}

/*...........................................................................................................*/

static int __devinit   rc632_probe(struct spi_device *spi)
{
	struct spi_proxd	*rc632;
	int				status;
	struct spi_plat_data *rc632_plat_data;
	struct device *dev;

	u_int8_t key_buff[6]={0x43,0x50,0x53,0x20,0x49,0x44}; /* CPS ID */
	u_int8_t csn_blk = 4 ;
	u_int8_t rw_buff[16] ;
	int rw_buff_len	=16 ;

	printk(KERN_INFO "RC632: driver probe\n");
	/* Allocate driver data */
	rc632 = kzalloc(sizeof(*rc632), GFP_KERNEL);
	if (!rc632)
		return -ENOMEM;

	spi_set_drvdata(spi,rc632);
	/*----------------------------*/

	/* Initialization  */
	spin_lock_init(&rc632->spi_lock);
	mutex_init(&rc632->buf_lock);
	init_completion(&rc632->rc632_work_done);
	/*----------------------------*/

	rc632->spi = spi;
	if (!gpio_is_valid(spi->irq)){
	        printk(KERN_INFO "RC632: invalid IRQ pin\n");
        	return -ENODEV;
   	}
    	status = gpio_request(spi->irq, "sysfs");
	printk(KERN_INFO "RC632: gpio_request STATUS:%d\n",status);
    	status = gpio_direction_input(spi->irq); /* pull up */
	printk(KERN_INFO "RC632: gpio_direction_input STATUS:%d\n",status);

	rc632->irq_number = gpio_to_irq(rc632->spi->irq);
	//gpio_export(spi->irq,false);
	printk(KERN_INFO "RC632: irq-pin:%d\n",spi->irq);
	printk(KERN_INFO "RC632: IRQ Number: %d\n",rc632->irq_number);
	/*----------------------------*/

	/* irq handler Initialization  */
	status= request_irq(rc632->irq_number,(irq_handler_t)  rc632_irq_handler,IRQF_TRIGGER_FALLING,"rc632_irq_handler",rc632);
	printk(KERN_INFO "RC632: request_irq STATUS: %d\n",status);

	/*----------------------------*/

	/* reset pin initialization.. */
	//rc632_plat_data	= dev_get_platform_data(rc632->spi);
	rc632_plat_data	= (struct spi_plat_data *)spi->dev.platform_data;
	rc632->reset_pin = rc632_plat_data->reset_pin;
	rc632_rst_pin_init(rc632->reset_pin);
	printk(KERN_INFO "RC632: reset pin: %d\n",rc632->reset_pin);
	/*----------------------------*/

	/*.............................................................................................................................*/
		/* Get a range of minor numbers (starting with 0) to work with */
		status = alloc_chrdev_region(&devt, 0, 1, "rc632_cdev");
		if (status < 0) {
			pr_err("Can't get major number\n");
		    return status;
		}
		prox_class = class_create(THIS_MODULE,"prox-class");
		if(IS_ERR(prox_class))
		{
			pr_err("Error creating dummy char class.\n");
			unregister_chrdev_region(MKDEV(MAJOR(devt), 0), 1);
			return PTR_ERR(prox_class);
		}
		/* Initialize the char device and tie a file_operations to it */
		cdev_init(&rc632->rc632_cdev, &rc632_fops);
		rc632->rc632_cdev.owner = THIS_MODULE;
		/* Now make the device live for the users to access */
		cdev_add(&rc632->rc632_cdev, devt, 1);
		dev = device_create(prox_class,
							&spi->dev,   	/* no parent device */
		                    devt,    		/* associated dev_t */
							NULL,   		/* no additional data */
							"rc632_cdev"); 	/* device name */
		printk(KERN_INFO "RC632: Device Created \n");
	/*.............................................................................................................................*/
	/*.............................................................................................................................*/
		printk(KERN_INFO "RC632: polled_rc632_probe \n");
		status = polled_rc632_probe(rc632);
		if(status < 0) {
			printk(KERN_ERR "RC632: error at polled_rc632_probe:%d\n",status);
		}
		printk(KERN_INFO "RC632: polled_rc632_probe Out Ok \n");
	/*.............................................................................................................................*/


	/* PCCI value Initialization  */
	rc632->pcci_wait_time = 50 ;//always 5000;;
	rc632->pcci_key = key_buff;
	rc632->pcci_key_type = PICC_KEY_B;
	rc632->pcci_key_size = 6;
	/*----------------------------*/

	printk(KERN_INFO "RC632: Initialize \n");
	status = rc632_init(spi);
	if(status != 0)
	{
		printk(KERN_ERR "RC632: error at Initialize:%d\n",status);
	}
	/*  */
	printk(KERN_INFO "RC632: CSN read \n");
	status = 0;//pcci_operation(rc632, BLOCK_R,csn_blk ,rw_buff , &rw_buff_len);
	if(status != 0)
	{
		printk(KERN_ERR "RC632: error at CSN read erro:%d\n",status);
	}
	return status;
}

static int __devexit  rc632_remove(struct spi_device *spi)
{
	struct spi_proxd *rc632;
	rc632 = spi_get_drvdata(spi);
	free_irq(rc632->irq_number, rc632);
	//free_irq(rc632->irq_number, rc632);
	gpio_free(spi->irq);
	rc632->spi = NULL;
	spi_set_drvdata(spi, NULL);


    //kfree(eep_device->data);
    mutex_destroy(&rc632->buf_lock);
    //spinlock_destroy(&rc632->spi_lock);
    /*....................................................................................................................*/
    polled_rc632_remove(rc632->poll_dev);
    kfree(rc632);
    /*....................................................................................................................*/
	printk(KERN_INFO "RC632: remove driver success LKM\n");
	return 0;
}

static struct spi_driver rc632_spi_driver = {
	.driver = {
		.name =		"rc632",
		.owner =	THIS_MODULE,
	},
	.probe 	=	rc632_probe,
	.remove =	rc632_remove,

	/* NOTE:  suspend/resume methods are not necessary here.
	 * We don't do anything except pass the requests to/from
	 * the underlying controller.  The refrigerator handles
	 * most issues; the controller driver handles the rest.
	 */
};


static int __init rc632driver_init(void)
{
	int status;
	status = spi_register_driver(&rc632_spi_driver);
	return status;
}

static void  __exit  rc632driver_exit(void)
{

	spi_unregister_driver(&rc632_spi_driver);

	unregister_chrdev_region(devt, 1);
	device_destroy(prox_class, devt);
    class_destroy(prox_class);

	printk(KERN_INFO "RC632: remove driver success LKM module\n");

}

module_init(rc632driver_init);
module_exit(rc632driver_exit);

MODULE_AUTHOR("Suman Dhara, <dhara_suman@yahoo.in>");
MODULE_DESCRIPTION("User mode RC632_SPI device interface");
MODULE_LICENSE("GPL");
MODULE_ALIAS("spi:rc632");
