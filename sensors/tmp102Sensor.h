/*******************************************************************************
   @Filename:tmp102Sensor.h
   @Brief:
   @Author:Ravi Dubey
   @Date:3/14/2018
 ******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "i2cWrappper.h"
#include "../includes.h"

#define TEMP_ADDR 			(0x48)
#define TEMP_READ_REG 	(0x00)
#define TEMP_CONFIG_REG (0x01)
#define TEMP_TLOW_REG 	(0x10)
#define TEMP_THIGH_REG 	(0x11)
#define TEMP_THIGH_VAL 	(0x15)
#define TEMP_TLOW_VAL 	(0x10)

#define TEMP_CONFIG_VAL (0x60)


#define SHUTDOWN_EN 	0x01
#define SHUTDOWN_DI	0x00
#define RES_9BIT	0x00
#define RES_10BIT	0x20
#define	RES_11BIT	0x40
#define RES_12BIT	0x60
#define FAULT1		0x00
#define FAULT2		0x08
#define FAULT4		0x10
#define FAULT6		0x18

//2nd byte
#define EMMODE		0x10
#define CONVRATE0	0x00
#define CONVRATE1	0x40
#define CONVRATE2	0x80
#define CONVRATE3	0xC0


/**
*@brief:
*
*@param:
*@return:
*/
float temperatureConv(temp_unit unit, char *buffer);

/**
*@brief:
*byte 1 is MSB,  byte 2LSB. First 12 bits indicate temperature.
*@param:
*@return:
*/
void temperatureRead(int i2c_file_handler, char *buffer);

/**
*@brief:
*
*@param:
*@return:
*/
int initializeTemp();

/**
*@brief:
*
*@param:
*@return:
*/
void configRegWrite(int i2c_file_handler, char *buffer);

/**
*@brief:
*
*@param:
*@return:
*/
void configRegRead(int i2c_file_handler, char *buffer);

/**
*@brief:
*The Pointer Register uses the two least-significant bytes (LSBs) to identify
*which of the data registers must respond to a read or write command.
*@param:
*@return:
*/
void tlowRead(int i2c_file_handler, char *buffer);

/**
*@brief:
*
*@param:
*@return:
*/
void thighRead(int i2c_file_handler, char *buffer);

/**
*@brief:
*
*@param:
*@return:
*/
void thighWrite(int i2c_file_handler, char *buffer);

/**
*@brief:
*
*@param:
*@return:
*/
void tlowWrite(int i2c_file_handler, char *buffer);
