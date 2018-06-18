/*
 * rc632.h
 *
 *  Created on: 05-Mar-2018
 *      Author: suman-dhara<dhara_suman@yahoo.in>
 */

#ifndef RC632_H_
#define RC632_H_

#include<linux/cdev.h>
#include <mach/platform.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include "../poller/include/input-polldev.h"


struct spi_proxd {
//	dev_t				devt;
	struct cdev 		rc632_cdev;
	struct input_polled_dev		* poll_dev;
	spinlock_t			spi_lock;
	struct spi_device	*spi;
//	struct list_head	device_entry;
	u_int 				irq_number;
	u_int				reset_pin;
	/* buffer is NULL unless this device is open (users > 0) */
	struct mutex		buf_lock;
	struct completion 	rc632_work_done;
	int					pcci_wait_time;
	u8					*pcci_key;
	u8					pcci_key_type; /* KEY-A or KEY-B */
	u8					pcci_key_size;
//	unsigned			users;
	u8					*buffer;

	//int (*input_dev_calback)(struct spi_proxd *rc632, u_int8_t *ret_atqa);

};

int smart_card_request(struct spi_proxd *rc632, u_int8_t *ret_atqa);

/* RC632 register definition */

enum rc632_registers {
	RC632_REG_PAGE0			= 0x00,
	RC632_REG_COMMAND		= 0x01,
	RC632_REG_FIFO_DATA		= 0x02,
	RC632_REG_PRIMARY_STATUS	= 0x03,
	RC632_REG_FIFO_LENGTH		= 0x04,
	RC632_REG_SECONDARY_STATUS	= 0x05,
	RC632_REG_INTERRUPT_EN		= 0x06,
	RC632_REG_INTERRUPT_RQ		= 0x07,

	RC632_REG_PAGE1			= 0x08,
	RC632_REG_CONTROL		= 0x09,
	RC632_REG_ERROR_FLAG		= 0x0a,
	RC632_REG_COLL_POS		= 0x0b,
	RC632_REG_TIMER_VALUE		= 0x0c,
	RC632_REG_CRC_RESULT_LSB	= 0x0d,
	RC632_REG_CRC_RESULT_MSB	= 0x0e,
	RC632_REG_BIT_FRAMING		= 0x0f,

	RC632_REG_PAGE2			= 0x10,
	RC632_REG_TX_CONTROL		= 0x11,
	RC632_REG_CW_CONDUCTANCE	= 0x12,
	RC632_REG_MOD_CONDUCTANCE	= 0x13,
	RC632_REG_CODER_CONTROL		= 0x14,
	RC632_REG_MOD_WIDTH		= 0x15,
	RC632_REG_MOD_WIDTH_SOF		= 0x16,
	RC632_REG_TYPE_B_FRAMING	= 0x17,

	RC632_REG_PAGE3			= 0x18,
	RC632_REG_RX_CONTROL1		= 0x19,
	RC632_REG_DECODER_CONTROL	= 0x1a,
	RC632_REG_BIT_PHASE		= 0x1b,
	RC632_REG_RX_THRESHOLD		= 0x1c,
	RC632_REG_BPSK_DEM_CONTROL	= 0x1d,
	RC632_REG_RX_CONTROL2		= 0x1e,
	RC632_REG_CLOCK_Q_CONTROL	= 0x1f,

	RC632_REG_PAGE4			= 0x20,
	RC632_REG_RX_WAIT		= 0x21,
	RC632_REG_CHANNEL_REDUNDANCY	= 0x22,
	RC632_REG_CRC_PRESET_LSB	= 0x23,
	RC632_REG_CRC_PRESET_MSB	= 0x24,
	RC632_REG_TIME_SLOT_PERIOD	= 0x25,
	RC632_REG_MFOUT_SELECT		= 0x26,
	RC632_REG_PRESET_27		= 0x27,

	RC632_REG_PAGE5			= 0x28,
	RC632_REG_FIFO_LEVEL		= 0x29,
	RC632_REG_TIMER_CLOCK		= 0x2a,
	RC632_REG_TIMER_CONTROL		= 0x2b,
	RC632_REG_TIMER_RELOAD		= 0x2c,
	RC632_REG_IRQ_PIN_CONFIG	= 0x2d,
	RC632_REG_PRESET_2E		= 0x2e,
	RC632_REG_PRESET_2F		= 0x2f,

	RC632_REG_PAGE6			= 0x30,

	RC632_REG_PAGE7			= 0x38,
	RC632_REG_TEST_ANA_SELECT	= 0x3a,
	RC632_REG_TEST_DIGI_SELECT	= 0x3d,
};

enum rc632_reg_status {
	RC632_STAT_LOALERT		= 0x01,
	RC632_STAT_HIALERT		= 0x02,
	RC632_STAT_ERR			= 0x04,
	RC632_STAT_IRQ			= 0x08,
#define RC632_STAT_MODEM_MASK		0x70
	RC632_STAT_MODEM_IDLE		= 0x00,
	RC632_STAT_MODEM_TXSOF		= 0x10,
	RC632_STAT_MODEM_TXDATA		= 0x20,
	RC632_STAT_MODEM_TXEOF		= 0x30,
	RC632_STAT_MODEM_GOTORX		= 0x40,
	RC632_STAT_MODEM_PREPARERX	= 0x50,
	RC632_STAT_MODEM_AWAITINGRX	= 0x60,
	RC632_STAT_MODEM_RECV		= 0x70,
};

enum rc632_reg_command {
	RC632_CMD_IDLE			= 0x00,
	RC632_CMD_WRITE_E2		= 0x01,
	RC632_CMD_READ_E2		= 0x03,
	RC632_CMD_LOAD_CONFIG		= 0x07,
	RC632_CMD_LOAD_KEY_E2		= 0x0b,
	RC632_CMD_AUTHENT1		= 0x0c,
	RC632_CMD_CALC_CRC		= 0x12,
	RC632_CMD_AUTHENT2		= 0x14,
	RC632_CMD_RECEIVE		= 0x16,
	RC632_CMD_LOAD_KEY		= 0x19,
	RC632_CMD_TRANSMIT		= 0x1a,
	RC632_CMD_TRANSCEIVE		= 0x1e,
	RC632_CMD_STARTUP		= 0x3f,
};

enum rc632_reg_interrupt {
	RC632_INT_LOALERT		= 0x01,
	RC632_INT_HIALERT		= 0x02,
	RC632_INT_IDLE			= 0x04,
	RC632_INT_RX			= 0x08,
	RC632_INT_TX			= 0x10,
	RC632_INT_TIMER			= 0x20,
	RC632_INT_SET			= 0x80,
};

enum rc632_reg_control {
	RC632_CONTROL_FIFO_FLUSH	= 0x01,
	RC632_CONTROL_TIMER_START	= 0x02,
	RC632_CONTROL_TIMER_STOP	= 0x04,
	RC632_CONTROL_CRYPTO1_ON	= 0x08,
	RC632_CONTROL_POWERDOWN		= 0x10,
	RC632_CONTROL_STANDBY		= 0x20,
};

enum rc632_reg_error_flag {
	RC632_ERR_FLAG_COL_ERR		= 0x01,
	RC632_ERR_FLAG_PARITY_ERR	= 0x02,
	RC632_ERR_FLAG_FRAMING_ERR	= 0x04,
	RC632_ERR_FLAG_CRC_ERR		= 0x08,
	RC632_ERR_FLAG_FIFO_OVERFLOW	= 0x10,
	RC632_ERR_FLAG_ACCESS_ERR	= 0x20,
	RC632_ERR_FLAG_KEY_ERR		= 0x40,
};

enum rc632_reg_tx_control {
	RC632_TXCTRL_TX1_RF_EN		= 0x01,
	RC632_TXCTRL_TX2_RF_EN		= 0x02,
	RC632_TXCTRL_TX2_CW		= 0x04,
	RC632_TXCTRL_TX2_INV		= 0x08,
	RC632_TXCTRL_FORCE_100_ASK	= 0x10,

	RC632_TXCTRL_MOD_SRC_LOW	= 0x00,
	RC632_TXCTRL_MOD_SRC_HIGH	= 0x20,
	RC632_TXCTRL_MOD_SRC_INT	= 0x40,
	RC632_TXCTRL_MOD_SRC_MFIN	= 0x60,
};

enum rc632_reg_coder_control {
	 /* bit 2-0 TXCoding */
#define RC632_CDRCTRL_TXCD_MASK		0x07
	RC632_CDRCTRL_TXCD_NRZ		= 0x00,
	RC632_CDRCTRL_TXCD_14443A	= 0x01,
	RC632_CDRCTRL_TXCD_ICODE_STD	= 0x04,
	RC632_CDRCTRL_TXCD_ICODE_FAST	= 0x05,
	RC632_CDRCTRL_TXCD_15693_STD	= 0x06,
	RC632_CDRCTRL_TXCD_15693_FAST	= 0x07,

	/* bit5-3 CoderRate*/
#define RC632_CDRCTRL_RATE_MASK		0x38
	RC632_CDRCTRL_RATE_848K		= 0x00,
	RC632_CDRCTRL_RATE_424K		= 0x08,
	RC632_CDRCTRL_RATE_212K		= 0x10,
	RC632_CDRCTRL_RATE_106K		= 0x18,
	RC632_CDRCTRL_RATE_14443B	= 0x20,
	RC632_CDRCTRL_RATE_15693	= 0x28,
	RC632_CDRCTRL_RATE_ICODE_FAST	= 0x30,

	/* bit 7 SendOnePuls */
	RC632_CDRCTRL_15693_EOF_PULSE	= 0x80,
};

enum rc632_erg_type_b_framing {
	RC632_TBFRAMING_SOF_10L_2H	= 0x00,
	RC632_TBFRAMING_SOF_10L_3H	= 0x01,
	RC632_TBFRAMING_SOF_11L_2H	= 0x02,
	RC632_TBFRAMING_SOF_11L_3H	= 0x03,

	RC632_TBFRAMING_EOF_10		= 0x00,
	RC632_TBFRAMING_EOF_11		= 0x20,

	RC632_TBFRAMING_NO_TX_SOF	= 0x80,
	RC632_TBFRAMING_NO_TX_EOF	= 0x40,
};
#define	RC632_TBFRAMING_SPACE_SHIFT	2
#define RC632_TBFRAMING_SPACE_MASK	7

enum rc632_reg_rx_control1 {
	RC632_RXCTRL1_GAIN_20DB		= 0x00,
	RC632_RXCTRL1_GAIN_24DB		= 0x01,
	RC632_RXCTRL1_GAIN_31DB		= 0x02,
	RC632_RXCTRL1_GAIN_35DB		= 0x03,

	RC632_RXCTRL1_LP_OFF		= 0x04,
	RC632_RXCTRL1_ISO15693		= 0x08,
	RC632_RXCTRL1_ISO14443		= 0x10,

#define RC632_RXCTRL1_SUBCP_MASK	0xe0
	RC632_RXCTRL1_SUBCP_1		= 0x00,
	RC632_RXCTRL1_SUBCP_2		= 0x20,
	RC632_RXCTRL1_SUBCP_4		= 0x40,
	RC632_RXCTRL1_SUBCP_8		= 0x60,
	RC632_RXCTRL1_SUBCP_16		= 0x80,
};

enum rc632_reg_decoder_control {
	RC632_DECCTRL_MANCHESTER	= 0x00,
	RC632_DECCTRL_BPSK		= 0x01,

	RC632_DECCTRL_RX_INVERT		= 0x04,

	RC632_DECCTRL_RXFR_ICODE	= 0x00,
	RC632_DECCTRL_RXFR_14443A	= 0x08,
	RC632_DECCTRL_RXFR_15693	= 0x10,
	RC632_DECCTRL_RXFR_14443B	= 0x18,

	RC632_DECCTRL_ZEROAFTERCOL	= 0x20,

	RC632_DECCTRL_RX_MULTIPLE	= 0x40,
};

enum rc632_reg_bpsk_dem_control {
	RC632_BPSKD_TAUB_SHIFT		= 0x00,
	RC632_BPSKD_TAUB_MASK		= 0x03,

	RC632_BPSKD_TAUD_SHIFT		= 0x02,
	RC632_BPSKD_TAUD_MASK		= 0x03,

	RC632_BPSKD_FILTER_AMP_DETECT	= 0x10,
	RC632_BPSKD_NO_RX_EOF		= 0x20,
	RC632_BPSKD_NO_RX_EGT		= 0x40,
	RC632_BPSKD_NO_RX_SOF		= 0x80,
};

enum rc632_reg_rx_control2 {
	RC632_RXCTRL2_DECSRC_LOW	= 0x00,
	RC632_RXCTRL2_DECSRC_INT	= 0x01,
	RC632_RXCTRL2_DECSRC_SUBC_MFIN	= 0x10,
	RC632_RXCTRL2_DECSRC_BASE_MFIN	= 0x11,

	RC632_RXCTRL2_AUTO_PD		= 0x40,
	RC632_RXCTRL2_CLK_I		= 0x80,
	RC632_RXCTRL2_CLK_Q		= 0x00,
};

enum rc632_reg_channel_redundancy {
	RC632_CR_PARITY_ENABLE		= 0x01,
	RC632_CR_PARITY_ODD		= 0x02,
	RC632_CR_TX_CRC_ENABLE		= 0x04,
	RC632_CR_RX_CRC_ENABLE		= 0x08,
	RC632_CR_CRC8			= 0x10,
	RC632_CR_CRC3309		= 0x20,
};

enum rc632_reg_timer_control {
	RC632_TMR_START_TX_BEGIN	= 0x01,
	RC632_TMR_START_TX_END		= 0x02,
	RC632_TMR_STOP_RX_BEGIN		= 0x04,
	RC632_TMR_STOP_RX_END		= 0x08,
};

enum rc632_reg_timer_irq {
	RC632_IRQ_LO_ALERT		= 0x01,
	RC632_IRQ_HI_ALERT		= 0x02,
	RC632_IRQ_IDLE			= 0x04,
	RC632_IRQ_RX			= 0x08,
	RC632_IRQ_TX			= 0x10,
	RC632_IRQ_TIMER			= 0x20,

	RC632_IRQ_SET			= 0x80,
};

enum rc632_reg_secondary_status {
	RC632_SEC_ST_TMR_RUNNING	= 0x80,
	RC632_SEC_ST_E2_READY		= 0x40,
	RC632_SEC_ST_CRC_READY		= 0x20,
};


/*----------------------------------------------------------------------------------------------------------------------*/

/* Block for IOCTL communication   */

struct rc632_ioc_transfer {
	__u64		tx_buf;
	__u64		rx_buf;
	__u64		pcci_key;

	__u16		len;
	__u16		pcci_key_size;
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


enum rc632_mode
{
    NORM_MODE = 0,
    SCAN_MODE
};

#define PICC_BLOCK_LEN = 16
/* IOCTL commands */
#define RC632HW_IOC_MAGIC			'r'
/* not all platforms use <asm-generic/ioctl.h> or _IOC_TYPECHECK() ... */
#define RC632HW_MSGSIZE     (sizeof(struct rc632_ioc_transfer))

#define RC632HW_IOC_READBLOCK   _IOR(RC632HW_IOC_MAGIC, 0, char[RC632HW_MSGSIZE])
#define RC632HW_IOC_WRITEBLOCK  _IOW(RC632HW_IOC_MAGIC, 0, char[RC632HW_MSGSIZE])

#define RC632HW_IOC_READKEY     _IOR(RC632HW_IOC_MAGIC, 1, char[RC632HW_MSGSIZE])
#define RC632HW_IOC_WRITEKEY    _IOW(RC632HW_IOC_MAGIC, 1, char[RC632HW_MSGSIZE])

#define RC632HW_IOC_SCANMODEON    _IOW(RC632HW_IOC_MAGIC, 2, char[RC632HW_MSGSIZE])
#define RC632HW_IOC_SCANMODEOFF    _IOW(RC632HW_IOC_MAGIC, 3, char[RC632HW_MSGSIZE])

/*----------------------------------------------------------------------------------------------------------------------*/

/*

 end RC632 definition

typedef u_int8_t u_int8_t;
 ;-------rc632 registers----------------
  Page 0: Command and status
#define RC632_REG_PAGE0  					0x00				 selects the page register
#define RC632_REG_COMMAND					0x01				 starts and stops command execution
#define RC632_REG_FIFO					0x02				 input and output of 64-u_int8_t FIFO buffer
#define RC632_REG_PRIMARY_STATUS			0x03				 receiver and transmitter and FIFO buffer status flags
#define RC632_REG_FIFO_LEN   				0x04				 number of u_int8_ts buffered in the FIFO buffer
#define RC632_REG_SECONDARY_STATTUS	    0x05
#define RC632_REG_INTR_EN					0x06				 enable and disable interrupt request control bits
#define RC632_REG_INTR_REQ				0x07				 interrupt request flags
 Page 1: Control and status
#define RC632_REG_CONTROL					0x09				 control flags for timer unit, power saving etc
#define RC632_REG_ERR_FLG					0x0a				 show the error status of the last command executed
#define RC632_REG_COLL_POS				0x0b				 bit position of the first bit-collision detected on the RF interface
#define RC632_REG_TRIMER_VAL				0x0c				 value of the timer
#define RC632_REG_CRC_RESULT_LSB			0x0d				 LSB of the CRC coprocessor register
#define RC632_REG_CRC_RESULT_MSB			0x0e				 MSB of the CRC coprocessor register
#define RC632_REG_BIT_FRAMING				0x0f				 adjustments for bit oriented frames
 Page 2: Transmitter and coder control
#define RC632_TX_CONTROL				0x11				 controls the operation of the antenna driver pins TX1 and TX2
#define RC632_REG_ANT_CONDUCTANCE			0x12				 selects the conductance of the antenna driver pins TX1 and TX2
#define RC632_REG_MOD_WIDTH				0x15				 selects the modulation pulse width
 Page 3: Receiver and decoder control
#define RC632_REG_RX_CONTROL1				0x19				 controls receiver behavior
#define RC632_REG_DECODER_CONTROL			0x1a				 controls decoder behavior
#define RC632_REG_BIT_PHASE				0x1b				 selects the bit-phase between transmitter and receiver clock
#define RC632_REG_RX_THRESHOLD			0x1c				 selects thresholds for the bit decoder
#define RC632_REG_RX_CONTROL2				0x1e				 controls decoder and defines the receiver input source
#define RC632_REG_CLOCK_Q_CONTROL			0x1f				 clock control for the 9ODEG phase-shifted Q-channel clock
 Page 4: RF Timing and channel redundancy
#define RC632_REG_RX_WAIT					0x21				 selects the interval after transmission before the receiver starts
#define RC632_REG_CHANNEL_REDUNDENCY		0x22				 selects the method and mode used to check data integrity on the RF channel
#define RC632_REG_CRC_PRESET_LSB			0x23				 preset LSB value for the CRC register
#define RC632_REG_CRC_PRESET_MSB			0x24				 preset MSB value for the CRC register
#define RC632_REG_MFOUT_SELECT			0x26				 selects internal signal applied to pin MFOUT, includes the MSB of value TimeSlotPeriod
 Page 5: FIFO, timer and IRQ pin configuration
#define RC632_REG_FIFO_LEVEL				0x29				 defines the FIFO buffer overflow and underflow warning levels
#define RC632_REG_TIMER_CLOCK				0x2a				 selects the timer clock divider
#define RC632_REG_TIMER_CONTROL			0x2b				 selects the timer start and stop conditions
#define RC632_REG_TIMER_RELOAD			0x2c				 defines the timer preset value
#define RC632_REG_IRQ_PIN_CONFIG			0x2d				 configures pin IRQ output stage
 Page 7: Test contro
#define RC632_REG_TEST_DIGI_SELECT		0x3d				 selects digital test mode

 ....................................................................................................................

  CLRC632 command set
#define	CMD_START_UP				Ox3F		 Runs the reset and initialization phase
#define CMD_IDLE					0x00		 no action; cancels execution of the current command.
#define	CMD_TRANSMIT				0x1A		 transmits data from the FIFO buffer to the card
#define	CMD_RECEIVE 				0x16		 activates receiver circuitry. Before the receiver starts, the state machine waits until the time defined in the RxWait register has elapsed
#define	CMD_TRANSCEIVE				0x1E		 transmits data from FIFO buffer to the card and automatically activates the receiver after transmission. The receiver waits until the time defined in the RxWait register has elapsed before starting.
#define	CMD_WRITE_E2				0x01		 reads data from the FIFO buffer and writes it to the EEPROM.
#define	CMD_READ_E2					0x03		 reads data from the EEPROM and sends it to the FIFO buffer.
#define	CMD_LOAD_KEY_E2				0x0B		 copies a key from the EEPROM into the key buffer
#define	CMD_LOAD_KEY				0x19		 reads a key from the FIFO buffer and loads it into the key buffer
#define	CMD_AUTHENT1				0x0C		 performs the first part of card authentication using the Crypto1 algorithm
#define	CMD_AUTHENT2				0x14		 performs the second part of card authentication using the Crypto1 algorithm
#define	CMD_LOAD_CONFIG				0x07		 reads data from EEPROM and initializes the CLRC632 registers
#define	CMD_CALC_CRC				0x12		 activates the CRC coprocessor
													Remark:
														 The result of the CRC calculation is read
														 from the CRCResultLSB and CRCResultMSB registers

*/



/*----------------------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------------------*/
u_int8_t RC632_WRITE_REG(u_int8_t reg) { return ((reg << 1) & 0x7E);}
u_int8_t RC632_READ_REG(u_int8_t reg){ return (((reg << 1) & 0x7E)|0x80);}
/*----------------------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------------------*/

#define MY_BUS_NUM		0
#define RC632_IRQ_PIN	(PAD_GPIO_D + 27)
#define RC632_RST_PIN	(PAD_GPIO_D + 26)

/* .................................................................................................................... */
/* .................................................................................................................... */

#define PICC_REQA	0x26
#define	PICC_WUPA	0x52

#define	PICC_ANTICOLLISION_CL1_H	0x93
#define	PICC_ANTICOLLISION_CL1_L	0x20

#define	PICC_SELECT_CL1_H		0x93
#define	PICC_SELECT_CL1_L		0x70

#define	PICC_AUTH_WITH_KEY_A	0x60
#define	PICC_AUTH_WITH_KEY_B	0x61

#define PICC_HALT_H		0x50
#define PICC_HALT_L		0x00

#define PICC_MIFARE_READ	0x30
#define PICC_MIFARE_WRITE	0xA0


/* .................................................................................................................... */
/*
 * init gpio for rc632 reset and interrupt pin, also assign rc632 interrupt received routine with interrupt handler
 * @return returns 0 if successful
 * */
//static int rc632_gpio_init(void);

/*@brief
 *  do hard reset rc632
 *
 * */
//int reset_rc632(void);
/*
 *  Function prototype for the custom IRQ handler function
 * */
/* .................................................................................................................... */
/*
 * do register initialize for rc632
 * */


struct spi_plat_data {
	u_int reset_pin;
};

int rc632_rst_pin_init(u_int reset_pin) {
	//int result;
	printk(KERN_INFO "RC632: Initializing the reset_pin LKM\n");

	if (!gpio_is_valid(reset_pin)) {
		printk(KERN_INFO "RC632: invalid reset_pin GPIO\n");
		return -ENODEV;
	}
	gpio_request(reset_pin, "sysfs");
	gpio_export(reset_pin,NULL);
	gpio_direction_output(reset_pin, 1); /**/
	return 0;
}


/*u_int8_t read_value_from_reg(u_int8_t reg){
	reg  = (((reg << 1) & 0x7E) | 0x80);
	return spi_w8r8(rc632_device, reg);
	//return ret;
	if(ret < 0) return ret;
	ret = spi_read(rc632_device,buf,buf_len);
	return 0;
}*/


int write_value_to_reg_then_read(struct spi_device *spi,u_int8_t reg, u_int8_t value , u_int8_t *rtx){
	int ret;
	u_int8_t buff[2];
	buff[0] = RC632_WRITE_REG(reg); buff[1] = value;
	ret = spi_write(spi, buff, sizeof(buff));
	if(ret < 0){
		return ret;
	}
	buff[0] = spi_w8r8(spi, RC632_READ_REG(reg));
	if(ret < 0) return ret;
	if(rtx != NULL)*rtx = buff[0];
	return 0;
}

void reset_rc632(void){
	int reset_pin_state = 1;
	long int reset_sate_stay_time = 200;
	udelay(reset_sate_stay_time);
	gpio_set_value(RC632_RST_PIN, reset_pin_state);
	udelay(reset_sate_stay_time);
	gpio_set_value(RC632_RST_PIN, !reset_pin_state);
	printk(KERN_INFO "RC632: reset success LKM\n");
	return;
}



/*
int rc632_init(struct spi_device *spi)
{
	u_int8_t buf, buf2;
	u_int8_t ret;

	rc632_device = spi;
	ret = rc632_rst_pin_init(RC632_RST_PIN);
	if(ret != 0){
		printk(KERN_ERR "error at rc632_init\n");
		return -EIO;
	}

	reset_rc632();
	ret = read_value_from_reg(RC632_REG_COMMAND);
	if(ret < 0){
		printk(KERN_ERR "error at rc632_init\n");
		return ret;
	}
	printk(KERN_INFO "Command reg val:%02X\n",ret);
	ret = write_value_to_reg_then_read(RC632_REG_PAGE0,0x00,&buf);
	printk(KERN_INFO "PAGE0 reg val:%02X\n",buf);
	if(ret != 0){
		printk(KERN_ERR "error to write PAGE0 register at RC632 \n");
		return ret;
	} 
	buf = read_value_from_reg(RC632_REG_IRQ_PIN_CONFIG);
	if(buf < 0){
		printk(KERN_ERR "error to read IRQ_PIN_CONFIG register at RC632 \n");
		return buf;
	}
	printk(KERN_INFO "RC632 IRQ_PIN_CONFIG  value:%02X\n",buf);
	buf = ((buf | 0x01) & 0xfd);
	printk(KERN_INFO "RC632 edited IRQ_PIN_CONFIG  value:%02X\n",buf);
	ret = write_value_to_reg_then_read(RC632_REG_IRQ_PIN_CONFIG, buf, &buf2);
	printk(KERN_INFO "IRQ_PIN_CONFIG reg val:%02X\n",buf2);
	printk(KERN_INFO "IRQ_PIN_CONFIG reg val:%02X\n",read_value_from_reg(RC632_REG_IRQ_PIN_CONFIG));
	if(ret != 0){
		printk(KERN_ERR "error to write IRQ_PIN_CONFIG register at RC632 \n");
		return ret;
	} 
	printk(KERN_INFO "RC632 initialize success..\n");
	return ret;
}
*/


#endif /* RC632_H_ */


