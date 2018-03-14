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

#define TEMP_ADDR 			(0x48)
#define TEMP_READ_REG 	(0x00)
#define TEMP_CONFIG_REG (0x01)
#define TEMP_TLOW_REG 	(0x10)
#define TEMP_THIGH_REG 	(0x11)
#define TEMP_THIGH_VAL 	(0x15)
#define TEMP_TLOW_VAL 	(0x10)

#define TEMP_CONFIG_VAL (0x60)


typedef enum{
	CELCIUS,
	FARENHEIT,
	KELVIN
} temp_unit;


float temperatureConv(temp_unit unit, char *buffer);

void temperatureRead(int i2c_file_handler, char *buffer);

int initializeTemp();

void configRegWrite(int i2c_file_handler, char *buffer);

void configRegRead(int i2c_file_handler, char *buffer);

void tlowRead(int i2c_file_handler, char *buffer);

void thighRead(int i2c_file_handler, char *buffer);

void thighWrite(int i2c_file_handler, char *buffer);

void tlowWrite(int i2c_file_handler, char *buffer);
