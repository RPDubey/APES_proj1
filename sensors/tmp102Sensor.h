/*******************************************************************************
   @Filename:tmp102Sensor.h
   @Brief:declarations for tempsensor i2c wrappers
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
#define TEMP_TLOW_REG 	(0x02)
#define TEMP_THIGH_REG 	(0x03)


#define SHUTDOWN_EN 	0x01
#define SHUTDOWN_DI	0xFE
#define FAULT1		0x00
#define FAULT2		0x08
#define FAULT4		0x10
#define FAULT6		0x18

#define TM_EN		0x02
#define TM_DI		0xFD

//2nd byte

#define EMMODE_EN	0x10
#define EMMODE_DI 0xEF

#define CONVRATE0	0x00
#define CONVRATE1	0x40
#define CONVRATE2	0x80
#define CONVRATE3	0xC0

#define TEMP_THIGH_VAL_B1 	(0x20)
#define TEMP_TLOW_VAL_B1 	(0x19)
#define TEMP_THIGH_VAL_B2 	(0x50)
#define TEMP_TLOW_VAL_B2	(0x50)
#define TEMP_CONFIG_VAL (0x60)

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
