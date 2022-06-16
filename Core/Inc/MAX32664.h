/*
 * MAX3664.h
 *
 *  Created on: Jun 8, 2022
 *      Author: annin
 */

#ifndef INC_MAX32664_H_
#define INC_MAX32664_H_



#endif /* INC_MAX32664_H_ */
#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "stdint.h"
#include "i2c.h"
#include "usart.h"
#include "string.h"
#include "stdio.h"

/* MAX32664 ADDRESSES */
#define MAX32664_Address 0x55
#define MAX32664_I2C_WriteAddress 0xAA
#define MAX32664_I2C_ReadAddress 0xAB

/* DELAY CONSTANT */
#define CMD_DELAY 6
#define CMD_DELAY_DEFAULT 2
#define CMD_DELAY_MAX30101 45 /* for the MAX30101 the delay should be 40ms; standard value is 2 ms. */

/*STATUS CONSTANTS*/
#define SUCCESS 0X00
#define ERR_UNAVAIL_CMD 0x01 		/* Wrong Family Byte or Index Byte was used */
#define ERR_UNAVAIL_FUNC 0x02 		/* Function not implemented. Index Byte or Write byte could not be valid */
#define ERR_DATA_FORMAT 0x03 		/* Incorrect number of bytes sent for the requested family byte */
#define ERR_INPUT_VALUE 0x04 		/* Illegal Configuration */
#define ERR_BTLDR_TRY_AGAIN 0x05 	/* In bootloader mode, the device is busy - try with a delay */
#define ERR_BTLDR_GENERAL 0x80 		/* General Error during bootloader mode */
#define ERR_BTLDR_CHECKSUM 0x81 	/* Bootloader checksum error while checking page data*/
#define ERR_BTLDR_AUTH 0x82 		/* Bootloader authorization error */
#define ERR_BTLDR_INVALIDE_APP 0x83 /* Bootloader detected that the application is not valid */
#define ERR_TRY_AGAIN 0xFE 			/* Device is busy, try again. Increase the general delay and CMD_DELAY */
#define ERR_UNKNOWN 0xFF			/* Unknown error */

/*DEVICE MODE CONSTANTS - BOTH REQUESTS & RESPONSES */
#define APPLICATION_MODE 0x00
#define RESET_MODE 0x02
#define BOOTLOADER_MODE 0x08
#define SHUTDOWN_REQUEST_MODE 0X01

/*OUTPUT MODE CONSTANTS - BOTH REQUESTS & RESPONSES */
#define PAUSE 0x00 			/* no data */
#define SENSOR_DATA 0x01
#define ALGORITHM_DATA 0x02
#define SENSOR_ALGORITHM_DATA 0x03
#define SENSOR_SAMPLE_COUNTER_BYTE 0x05
#define ALGORITHM_SAMPLE_COUNTER_BYTE 0x06
#define SENSOR_ALGORITHM_SAMPLE_COUNTER_BYTE 0x07

/*CONSTANTS - BOTH REQUESTS & RESPONSES */
#define ENABLE 0x01
#define DISABLE 0x00

/* CONSTANTS */
#define OK 0
#define ERROR (-1)


struct MAX32664_sensor{
	uint16_t reset;
	uint16_t mfio;
	GPIO_TypeDef* reset_gpio;
	GPIO_TypeDef* mfio_gpio;
	I2C_HandleTypeDef *i2c_port;
	uint32_t infrared_led;
	uint32_t red_led;
	uint16_t heart_rate;
	uint8_t confidence;
	uint16_t oxygen;
	uint8_t status;
	uint8_t maxim_fast_status;
	uint8_t algorithm_status;
	uint8_t algorithm_state;
	uint16_t ib_interval;
	uint8_t mode;
	float oxygen_algorithm;
};
typedef struct MAX32664_sensor MAX32664;

/* funzioni controllate*/
void mfio(MAX32664* max32664);
uint8_t begin(MAX32664* sensor,I2C_HandleTypeDef *i2c_port, GPIO_TypeDef* reset_pin_gpio,GPIO_TypeDef* mfio_pin_gpio, uint16_t reset_pin, uint16_t mfio_pin);
uint8_t read_status(MAX32664* max32664);
uint8_t write(MAX32664 *max32664, uint8_t family_byte, uint8_t index_byte, uint8_t write_byte);
uint8_t enable_write(MAX32664 *max32664, uint8_t family_byte, uint8_t index_byte, uint8_t write_byte);
uint8_t read(MAX32664 *max32664, uint8_t family_byte, uint8_t index_byte);
uint8_t set_device_mode(MAX32664 *max32664, uint8_t mode);
uint8_t get_device_mode(MAX32664 *max32664);
uint8_t set_output_mode(MAX32664 *max32664, uint8_t mode);
uint8_t get_output_mode(MAX32664 *max32664);
uint8_t MAX30101_enable(MAX32664 *max32664, uint8_t enable);
uint8_t get_number_of_samples_FIFO(MAX32664 *max32664);
uint8_t read_sensor(MAX32664 *max32664);
uint8_t AGC_enable(MAX32664 *max32664, uint8_t enable);
uint8_t MaximFast_enable(MAX32664 *max32664, uint8_t enable);
uint8_t set_fifo_threshold(MAX32664 *max32664, uint8_t threshold);
uint8_t config_sensor(MAX32664 *max32664, uint8_t mode);
uint8_t read_sensor1(MAX32664 *max32664);

//------------------------------------ NON CONTROLLATE ------------------------------------------//

uint8_t write_bytes(MAX32664 *max32664, uint8_t family_byte, uint8_t index_byte, uint8_t write_byte0,uint8_t write_byte1);
uint8_t MAX30101_get_sensor_mode(MAX32664 *max32664);
uint8_t get_samples_FIFO(MAX32664 *max32664);
uint8_t AGC_configuration(MAX32664 *max32664, uint8_t range,uint8_t step, uint8_t sensitivity, uint8_t samples_to_average);
uint8_t read_fill_array(MAX32664* max32664,uint8_t family_byte, uint8_t index_byte, uint8_t num_reads, uint8_t *samples);
