/*
 * MAX32664.c
 *
 *  Created on: Jun 8, 2022
 *      Author: annin
 */
#include "MAX32664.h"

void mfio(MAX32664* max32664){
GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin =max32664->mfio;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;

	HAL_GPIO_Init(max32664->mfio_gpio, &GPIO_InitStruct);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);
}

/* FUNZIONI CONTROLLATE */
uint8_t begin(MAX32664* sensor,I2C_HandleTypeDef *i2c_port, GPIO_TypeDef* reset_pin_gpio,GPIO_TypeDef* mfio_pin_gpio, uint16_t reset_pin, uint16_t mfio_pin){
	sensor->reset = reset_pin;
	sensor->mfio = mfio_pin;
	sensor->reset_gpio = reset_pin_gpio;
	sensor->mfio_gpio = mfio_pin_gpio;
	sensor->i2c_port = i2c_port;
	sensor->infrared_led = -1;
	sensor->red_led = -1;
	sensor->heart_rate = -1;
	sensor->confidence = -1;
	sensor->oxygen = -1;
	sensor->status = -1;
	sensor->algorithm_state = -1;
	sensor->algorithm_status = -1;
	sensor->ib_interval = -1;
	sensor->mode = -1; //initialized to a non acceptable value;

	//set_device_mode(sensor, RESET_MODE);
	//set_device_mode(sensor, APPLICATION_MODE);

	HAL_GPIO_WritePin(sensor->mfio_gpio, sensor->mfio, GPIO_PIN_SET);
	HAL_GPIO_WritePin(sensor->reset_gpio, sensor->reset, GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(sensor->reset_gpio, sensor->reset, GPIO_PIN_SET);
	HAL_Delay(1000);
	mfio(sensor);// To be used as an interrupt later


	sensor->mode = get_device_mode(sensor);

	if((sensor->status == SUCCESS) & (sensor->mode == APPLICATION_MODE))
		return OK;
	else
		return ERROR;
}

uint8_t read_status(MAX32664* max32664){
	uint8_t FAMILY_BYTE = 0x00;
	uint8_t INDEX_BYTE = 0x00;


	read(max32664, FAMILY_BYTE, INDEX_BYTE);
	if(max32664->status != SUCCESS)
		return ERROR;

	return OK;
}

uint8_t write(MAX32664 *max32664, uint8_t family_byte, uint8_t index_byte, uint8_t write_byte){
	uint8_t array[] = {-1,-1,-1};
	array[0]=family_byte;
	array[1]= index_byte;
	array[2] = write_byte;

	if(HAL_I2C_Master_Transmit(max32664->i2c_port, MAX32664_I2C_WriteAddress, array, 3, HAL_MAX_DELAY) != HAL_OK)
		return ERROR;

	HAL_Delay(CMD_DELAY);

	return read_status(max32664);

}

uint8_t enable_write(MAX32664 *max32664, uint8_t family_byte, uint8_t index_byte, uint8_t write_byte){
	uint8_t array[] = {-1,-1,-1};
	array[0]=family_byte;
	array[1]= index_byte;
	array[2] = write_byte;

	if(HAL_I2C_Master_Transmit(max32664->i2c_port, MAX32664_I2C_WriteAddress, array, 3, HAL_MAX_DELAY) != HAL_OK)
		return ERROR;

	HAL_Delay(CMD_DELAY_MAX30101);

	return read_status(max32664);

}

uint8_t read(MAX32664 *max32664, uint8_t family_byte, uint8_t index_byte){
	uint8_t address[] = {-1, -1};
	uint8_t read[] ={-1, -1};

	address[0]= family_byte;
	address[1]= index_byte;

	if(HAL_I2C_Master_Transmit(max32664->i2c_port, MAX32664_I2C_WriteAddress, address, 2, HAL_MAX_DELAY) != HAL_OK)
		return ERROR;

	HAL_Delay(CMD_DELAY);

	if(HAL_I2C_Master_Receive(max32664->i2c_port, MAX32664_I2C_ReadAddress, read, 2, HAL_MAX_DELAY) != HAL_OK)
		return ERROR;

	max32664->status = read[0];

	if(max32664->status == SUCCESS)
		return read[1];
	else
		return ERROR;
}

uint8_t set_device_mode(MAX32664 *max32664, uint8_t mode){
	uint8_t FAMILY_BYTE = 0x01;
	uint8_t INDEX_BYTE = 0x00;

	switch (mode) {
		case APPLICATION_MODE:
			return write(max32664, FAMILY_BYTE,INDEX_BYTE, APPLICATION_MODE);
		case BOOTLOADER_MODE:
			return write(max32664, FAMILY_BYTE,INDEX_BYTE, BOOTLOADER_MODE);
		case RESET_MODE:
			return write(max32664, FAMILY_BYTE,INDEX_BYTE, RESET_MODE);
		case SHUTDOWN_REQUEST_MODE:
			return write(max32664, FAMILY_BYTE,INDEX_BYTE, SHUTDOWN_REQUEST_MODE);
	}

	return ERROR;
}

uint8_t	get_device_mode(MAX32664 *max32664){

	uint8_t FAMILY_BYTE = 0x02;
	uint8_t INDEX_BYTE = 0x00;

	max32664->mode = read(max32664, FAMILY_BYTE, INDEX_BYTE);
	return OK;


}

uint8_t set_output_mode(MAX32664 *max32664, uint8_t mode){
	uint8_t FAMILY_BYTE = 0x10;
	uint8_t INDEX_BYTE = 0x00;

	if(mode > SENSOR_ALGORITHM_SAMPLE_COUNTER_BYTE)
		return ERR_INPUT_VALUE;

	return write(max32664, FAMILY_BYTE, INDEX_BYTE, mode);

}

uint8_t get_output_mode(MAX32664 *max32664){
	uint8_t FAMILY_BYTE = 0x11;
	uint8_t INDEX_BYTE = 0x00;

	return read(max32664, FAMILY_BYTE, INDEX_BYTE);
}

uint8_t MAX30101_enable(MAX32664 *max32664, uint8_t enable){
	uint8_t FAMILY_BYTE = 0x44;
	uint8_t INDEX_BYTE = 0x03;

	if(enable == ENABLE)
		return enable_write(max32664, FAMILY_BYTE, INDEX_BYTE, ENABLE);
	else
		return enable_write(max32664, FAMILY_BYTE, INDEX_BYTE, DISABLE);

	return ERR_INPUT_VALUE;


}

uint8_t get_number_of_samples_FIFO(MAX32664 *max32664){
	uint8_t FAMILY_BYTE = 0x12;
	uint8_t INDEX_BYTE = 0x00;

	return read(max32664, FAMILY_BYTE, INDEX_BYTE);
}


uint8_t AGC_enable(MAX32664 *max32664, uint8_t enable){
	uint8_t FAMILY_BYTE = 0x52;
	uint8_t INDEX_BYTE = 0x00;

	if(enable == ENABLE)
		return enable_write(max32664, FAMILY_BYTE, INDEX_BYTE, ENABLE);
	else
		return enable_write(max32664, FAMILY_BYTE, INDEX_BYTE, DISABLE);

	return ERR_INPUT_VALUE;

}

uint8_t MaximFast_enable(MAX32664 *max32664, uint8_t enable){

	uint8_t FAMILY_BYTE = 0x52;
	uint8_t INDEX_BYTE = 0x02;

		if(enable == ENABLE)
			return enable_write(max32664, FAMILY_BYTE, INDEX_BYTE, ENABLE);
		else
			return enable_write(max32664, FAMILY_BYTE, INDEX_BYTE, DISABLE);

	return ERR_INPUT_VALUE;

}


uint8_t set_fifo_threshold(MAX32664 *max32664, uint8_t threshold){
	uint8_t FAMILY_BYTE = 0x10;
	uint8_t INDEX_BYTE = 0x01;

	return write(max32664, FAMILY_BYTE, INDEX_BYTE, threshold);

}


uint8_t config_sensor(MAX32664 *max32664, uint8_t mode){

	//if(mode == RESET_MODE || mode == SHUTDOWN_REQUEST_MODE)
		//return ERR_INPUT_VALUE;

	if(set_output_mode(max32664, SENSOR_ALGORITHM_DATA) !=  OK)
		return max32664->status;

    if(set_fifo_threshold(max32664, 0x01) !=  OK)
    	return max32664->status;

    if(AGC_enable(max32664, ENABLE) !=  OK)
    	 return max32664->status;

    if(MAX30101_enable(max32664, ENABLE) !=  OK)
		return max32664->status;

	if(MaximFast_enable(max32664, mode) !=  OK)
		return max32664->status;

	HAL_Delay(1000);

	return OK;
}

//-------------------------------------------------------------------------------------------------------------------//
/* non controllate */
uint8_t write_bytes(MAX32664 *max32664, uint8_t family_byte, uint8_t index_byte, uint8_t write_byte0,uint8_t write_byte1){
	uint8_t array[4];
	array[0]=family_byte;
	array[1]= index_byte;;
	array[2] = write_byte0;
	array[3] = write_byte1;

	if(HAL_I2C_Master_Transmit(max32664->i2c_port, MAX32664_I2C_WriteAddress, array, 4, HAL_MAX_DELAY) != HAL_OK)
		return ERROR;

	return read_status(max32664);

}

uint8_t MAX30101_get_sensor_mode(MAX32664 *max32664){
	uint8_t FAMILY_BYTE = 0x45;
	uint8_t INDEX_BYTE = 0x03;

	return read(max32664, FAMILY_BYTE, INDEX_BYTE);
}


uint8_t AGC_configuration(MAX32664 *max32664, uint8_t range, uint8_t step, uint8_t sensitivity, uint8_t samples_to_average){
	/* the parameters are in the range of 0 to 100 percent*/
	uint8_t FAMILY_BYTE = 0x50;
	uint8_t INDEX_BYTE = 0x00;


	/* write range */
	if(write_bytes(max32664, FAMILY_BYTE, INDEX_BYTE, 0x00, range) != OK)
		return ERROR;

	/* write step */
	if(write_bytes(max32664, FAMILY_BYTE, INDEX_BYTE, 0x01, step) != OK)
		return ERROR;

	/* write sensitivity */
	if(write_bytes(max32664, FAMILY_BYTE, INDEX_BYTE, 0x02, sensitivity) != OK)
		return ERROR;

	/* number of write samples to average */
	if(write_bytes(max32664, FAMILY_BYTE, INDEX_BYTE, 0x03, samples_to_average) != OK)
			return ERROR;

	return OK;

}


uint8_t read_sensor(MAX32664 *max32664){
	uint8_t FAMILY_BYTE = 0x12;
	uint8_t INDEX_BYTE = 0x01;
	uint8_t samples[32];

	uint8_t address[] = {-1, -1};

	address[0]= FAMILY_BYTE;
	address[1]= INDEX_BYTE;

	if(HAL_I2C_Master_Transmit(max32664->i2c_port, MAX32664_I2C_WriteAddress, address, 2, HAL_MAX_DELAY) != HAL_OK)
		return ERROR;
	if(HAL_I2C_Master_Receive(max32664->i2c_port,MAX32664_I2C_ReadAddress, samples, 32, HAL_MAX_DELAY) != OK)
		return ERROR;


	/* Infrared raw value */
	max32664->infrared_led = (uint32_t)(samples[0] <<16);
	max32664->infrared_led |= (uint32_t)(samples[1] <<8);
	max32664->infrared_led |= samples[2];

	/* Red Led raw value */
	max32664->red_led = (uint32_t)(samples[3] <<16);
	max32664->red_led |= (uint32_t)(samples[4] <<8);
	max32664->red_led |= samples[5];


	/* Values from samples [6:11] are values for leds that are not available on the MAX30101, so they're left empty.
	 * Value from sample [12] is a value for the empty accelerometer  */

	/* heart rate values */
	max32664->heart_rate = (uint16_t)(samples[13] << 8);
	max32664->heart_rate |= (samples[14]);
	max32664->heart_rate /= 10;

	/* confidence */
	max32664->confidence =(uint8_t) samples[15];

	/* oxygen */
	max32664->oxygen = (uint16_t)(samples[16] <<8);
	max32664->oxygen |= (samples[17]);
	max32664->oxygen /= 10;

	max32664->algorithm_state = (uint8_t) samples[18];

	max32664->algorithm_status = (uint8_t) samples[19];

	max32664->ib_interval = (uint16_t)(samples[20] << 8);
	max32664->ib_interval |= (samples[21]);
	max32664->ib_interval /= 1000;

	return OK;

}

uint8_t read_sensor1(MAX32664 *max32664){
	uint8_t FAMILY_BYTE = 0x12;
	uint8_t INDEX_BYTE = 0x01;
	uint8_t samples[16];
	char str[100];

	uint8_t address[] = {200, 200};

	address[0]= FAMILY_BYTE;
	address[1]= INDEX_BYTE;

	if(read_status(max32664) != OK){
		max32664->heart_rate = 0;
		max32664->confidence = 0;
		max32664->oxygen = 0;
		return max32664->status;
	}
	uint8_t num_samples = get_number_of_samples_FIFO(max32664);

	if(HAL_I2C_Master_Transmit(max32664->i2c_port, MAX32664_I2C_WriteAddress, address, 2, HAL_MAX_DELAY) != HAL_OK)
			return ERROR;
	if(HAL_I2C_Master_Receive(max32664->i2c_port,MAX32664_I2C_ReadAddress, samples, 16, HAL_MAX_DELAY) != OK)
			return ERROR;

	/* Infrared raw value */
	/*max32664->infrared_led = (uint32_t)(samples[0] << 16);
	max32664->infrared_led |= (uint32_t)(samples[1] << 8);
	max32664->infrared_led |= samples[2];

	sprintf(str,"Infrared_led: %lu\r\n", max32664->infrared_led);
    HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str),HAL_MAX_DELAY);



	//  Red Led raw value
	max32664->red_led = (uint32_t)(samples[3] << 16);
	max32664->red_led |= (uint32_t)(samples[4] << 8);
	max32664->red_led |= samples[5];

	sprintf(str,"red_led: %lu\r\n", max32664->red_led);
	HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str),HAL_MAX_DELAY);

	uint32_t extraled;
	extraled = (uint32_t)(samples[6] << 16);
	extraled= (uint32_t)(samples[7] << 8);
	extraled |= samples[8];
	sprintf(str,"Extraled1: %lu\r\n", extraled);
	HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str),HAL_MAX_DELAY);

	extraled = (uint32_t)(samples[9] << 16);
	extraled= (uint32_t)(samples[10] << 8);
	extraled |= samples[11];
	sprintf(str,"Extraled2: %lu\r\n", extraled);
	HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str),HAL_MAX_DELAY);

	//Values from samples [6:11] are values for leds that are not available on the MAX30101, so they're left empty.
    * Value from sample [12] is a value for the empty accelerometer  */

	/*sprintf(str,"Accel_value: %d\r\n", samples[12]);
	HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str),HAL_MAX_DELAY);

	/ heart rate values */
	max32664->heart_rate = (uint16_t)(samples[0] << 8);
	max32664->heart_rate |= (samples[1]);
	max32664->heart_rate /= 10;

	sprintf(str,"Heart_rate: %d\r\n", max32664->heart_rate);
	HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str),HAL_MAX_DELAY);


	/* confidence */
	max32664->confidence = (uint8_t) samples[2];

	sprintf(str,"Confidence: %d\r\n", max32664->confidence);
	HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str),HAL_MAX_DELAY);

	/* oxygen */
	max32664->oxygen = (uint16_t)(samples[3] << 8);
	max32664->oxygen |= (samples[4]);
	max32664->oxygen /= 10;

	sprintf(str,"Oxygen %d\r\n", max32664->oxygen);
	HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str),HAL_MAX_DELAY);

	/* state = finger detected? */
	max32664->maxim_fast_status = samples[5];
	sprintf(str,"Algorithm_state: %d\r\n", max32664->maxim_fast_status);
	HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str),HAL_MAX_DELAY);

	uint16_t tempVal=(uint16_t)(samples[6] << 8);
	tempVal |= samples[7];
	max32664->oxygen_algorithm = tempVal;
	max32664->oxygen_algorithm /= 1000;
	sprintf(str,"R_value: %f\r\n", max32664->oxygen_algorithm);
	HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str),HAL_MAX_DELAY);

	sprintf(str,"Algorithm_status: %d\r\n", samples[8]);
	HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str),HAL_MAX_DELAY);

	sprintf(str,"Motion_Value: %d\r\n", samples[9]);
	HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str),HAL_MAX_DELAY);

	uint16_t perfusion;
	perfusion = (uint16_t)(samples[10] << 8);
	perfusion |= (samples[11]);
	perfusion /= 10;
	sprintf(str,"Perfusion: %d\r\n", perfusion);
	HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str),HAL_MAX_DELAY);

	uint16_t interbeat;
	interbeat = (uint16_t)(samples[12] << 8);
	interbeat |= (samples[13]);
	interbeat /= 1000;
	sprintf(str,"Interbeat: %d\r\n", interbeat);
	HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str),HAL_MAX_DELAY);

    char *s = "-------------\n";
	HAL_UART_Transmit(&huart2, (uint8_t*) s, strlen(s),HAL_MAX_DELAY);
	return OK;
	/*
	 char str[400];
	 sprintf(str, "Samples:\r\nled: %d %d %d %d %d %d %d %d %d %d %d %d\r\n"
	    "accel: %d\r\n"// %d %d %d %d %d\r\n"
	    "heart rate: %d %d \r\n"
	    "confidence: %d \r\n"
	    "spO2: %d %d\r\n"
	    "state: %d\r\n"
	    "R: %d %d\r\n"
	    "status: %d\r\n"
	    "motion flag: %d\n\r "
	    "perfusion%d %d\r\n"
	    "interbeat: %d %d\r\n",
	    samples[0], samples[1], samples[2], samples[3], samples[4], samples[5],
	    samples[6], samples[7],
	    samples[8], samples[9],
	    samples[10], samples[11],
	    samples[12], samples[13],
	    samples[14], samples[15],
	    samples[16], samples[17],
	    samples[18], samples[19],
	    samples[20], samples[21],
	    samples[22], samples[23],
	    samples[24], samples[25],
	    samples[26], samples[27],
	    samples[28], samples[29],
	    samples[30], samples[31]
	    );

	 HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str),HAL_MAX_DELAY);
	 */
}
