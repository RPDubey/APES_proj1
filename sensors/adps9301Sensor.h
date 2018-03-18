/*******************************************************************************
   @Filename:adps9301Sensor.h
   @Brief:This file declares the wrapper methods to write/read from the apds light sensor via I2C.
   @Author:Ravi Dubey
   @Date:3/14/2018
 ******************************************************************************/
#include <math.h>
#include "i2cWrappper.h"

#define LIGHT_ADDR	(0x39)
#define POWERUP 	  (0x03)
#define POWERDOWN 	(0x00)
#define LUMENS_NIGHT (10)

//s Temperature ensor registers
typedef enum{
	CONTROL,
	TIMING,
	THRESHLOWLOW,
	THRESHLOWHIGH,
	THRESHHIGHLOW,
	THRESHHIGHIGH,
	INTERRUPT,
	RES0,
	RES1,
	RES2,
	CRC,
	ID,
	DATA0LOW,
	DATA0HIGH,
	DATA1LOW,
	DATA1HIGH
}adps9301_regs;

//light sensor registers
typedef enum{
	READ,
	WRITE,
	SET_GAIN,
	SET_INTEGTIME,
	ENABLE,
	DISABLE,
	LOW_GAIN,
	HIGH_GAIN,
	INTEG0,
	INTEG1,
	INTEG2,
	INTEG3,
	NA
}apds9301_opt;

typedef enum{
	NIGHT,
	DAY
}status;

/**
*@brief:Initialize write sensor
*
*@param:
*@return:
*/
int initializeLight(void);

/**
*@brief:
*op-> READ or WRITE
*data is 8 bits
*@param:
*@return:
*/
void commandReg(int file_handler, adps9301_regs reg, apds9301_opt op);

/**
*@brief:
*op1-> READ or WRITE
*op2-> ENABLE or DISABLE
*buffer is 8 bit
*@param:
*@return:
*/
void controlReg(int file_handler, apds9301_opt opt1, apds9301_opt op2, char *buffer);

/**
*@brief:
*op1-> READ or WRITE
*op2-> SET_GAIN or SET_INTEGTIME
*op3-> LOW_GAIN, HIGH_GAIN | INTEG0 or INTEG1 or INTEG2 or INTEG3
*buffer is 8 bits
*@param:
*@return:
*/
void timingReg(int file_handler, apds9301_opt op1, apds9301_opt op2,
 							 apds9301_opt op3, char *buffer );

/**
*@brief:
*op-> ENABLE or DISABLE
*@param:
*@return:
*/
void interrupReg(int file_handler, apds9301_opt op1, apds9301_opt op2,
                 char *buffer);
/**
*@brief:
*
*@param:
*@return:
*/
void idRegRead(int file_handler);

/**
*@brief:
*op-> READ or WRITE
*@param:
*@return:
*/
void interruptThreshReg(int file_handler, apds9301_opt op, char *buffer);

/**
*@brief:
*buffer-> buffer[0](lsb) and buffer[1](msb)
*channel
*@param:
*@return:
*/
uint16_t adcDataRead(int file_handler, int channel);

/**
*@brief:
*
*@param:
*@return:
*/
float reportLumen(uint16_t adc_data_ch0, uint16_t adc_data_ch1);

/**
*@brief:
*
*@param:
*@return:
*/
status reportStatus(int file_handler);
