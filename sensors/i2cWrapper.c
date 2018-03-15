/*******************************************************************************
   @Filename:i2cWrappper.c
   @Brief:
   @Author:Ravi Dubey
   @Date:3/14/2018
 ******************************************************************************/
/****Reference: https://www.kernel.org/doc/Documentation/i2c/dev-interface******/

#include "i2cWrappper.h"

int i2cInit(char *dev_path, int i2c_file_handler, int slave_addr)
{
								int rc;

								i2c_file_handler = open(dev_path, O_RDWR);
								if (i2c_file_handler < 0) {
																err(errno, "Tried to open '%s'", dev_path);
																return -1;
								}

								rc = ioctl(i2c_file_handler, I2C_SLAVE, slave_addr);
								if (rc < 0) {
																err(errno, "Tried to set device address '0x%02x'", slave_addr);
																return -1;
								}

								return i2c_file_handler;
}

int i2cRead(int i2c_file_handler, char *buffer, int num_bytes)
{
								if( read(i2c_file_handler, buffer, num_bytes) != num_bytes) return -1;
								else return 0;
}

int i2cWrite(int i2c_file_handler, char *buffer, int num_bytes)
{
								if( write(i2c_file_handler, buffer, num_bytes) != num_bytes) return -1;
								else return 0;

}
