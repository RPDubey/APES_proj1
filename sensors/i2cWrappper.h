/*******************************************************************************
   @Filename:i2cWrappper.h
   @Brief:declaration for i2c wrapper functions
   @Author:Ravi Dubey
   @Date:3/14/2018
 ******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/**
*@brief:i2c initialization
*Initializes a file handler for i2c
*@param:path to device on the file system, file handler, slave adress of the i2c device
*@return: file handler
*/
int i2cInit(char *dev_path, int i2c_file_handler, int slave_addr);

/**
*@brief:i2c Read
*Reads from a i2c slave, given number of bytes
*@param:file handler, pointer to buffer to read data into and num bytes
*@return: 0 on success , -1 on failure
*/
int i2cRead(int i2c_file_handler, char *buffer, int num_bytes);


/**
*@brief:i2c Write
*Writes to a i2c slave, given number of bytes
*@param:file handler, pointer to buffer to write from and num bytes
*@return: 0 on success , -1 on failure
*/
int i2cWrite(int i2c_file_handler, char *buffer, int num_bytes);
