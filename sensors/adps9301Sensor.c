/*******************************************************************************
   @Filename:adps9301Sensor.c
   @Brief:This file implements the wrapper methods to write/read from the apds light sensor via I2C.
   @Author:Ravi Dubey
   @Date:3/14/2018
 ******************************************************************************/
#include "adps9301Sensor.h"
#include "../includes.h"
#include <pthread.h>

int initializeLight(void)
{
								int light;
								light = i2cInit("/dev/i2c-2", light, LIGHT_ADDR);
								return light;
}

void commandReg(int file_handler, adps9301_regs adps9301_reg, apds9301_opt op)
{
								char CMD, CLEAR, WORDS, RES;

								CMD = 1; CLEAR = 0;
								WORDS = 0; RES = 0;

								char data = CMD << 7 | CLEAR << 6 | WORDS << 5 | RES << 4 | adps9301_reg;

								i2cWrite(file_handler, &data, 1);

}

void controlReg(int file_handler, apds9301_opt op1, apds9301_opt op2, char *buffer)
{
								char data;
								if(op1 == READ)
																i2cRead(file_handler, buffer, 1);
								else if(op1 == WRITE)
								{
																if(op2 == ENABLE) {
																								data = POWERUP;
																								//	pthread_mutex_lock(&light_i2c_mutex);

																								i2cWrite(file_handler, &data, 1);
																								//pthread_mutex_unlock(&light_i2c_mutex);

																}
																else if(op2 == DISABLE) {
																								data = POWERDOWN;
																								//	pthread_mutex_lock(&light_i2c_mutex);

																								i2cWrite(file_handler, &data, 1);
																								//pthread_mutex_unlock(&light_i2c_mutex);

																}
								}
								else
																printf("ERROR: invalid operation\n");
}

void timingReg(int file_handler, apds9301_opt op1, apds9301_opt op2, apds9301_opt op3, char *buffer )
{
								char GAIN, INTEG;
								static char data = 0x00;
								if(op1 == WRITE)
								{
																if(op2 == SET_GAIN)
																{
																								if(op3 == LOW_GAIN) {
																																GAIN = 0;
																																data |=  GAIN <<4;
																																//pthread_mutex_lock(&light_i2c_mutex);

																																i2cWrite(file_handler, &data, 1);
																																//pthread_mutex_unlock(&light_i2c_mutex);

																								}
																								else if(op3 == HIGH_GAIN) {
																																GAIN = 1;
																																data |= GAIN << 4;
																																//pthread_mutex_lock(&light_i2c_mutex);
																																i2cWrite(file_handler, &data, 1);
																																//pthread_mutex_unlock(&light_i2c_mutex);

																								}
																}
																else if(op2 == SET_INTEGTIME)
																{
																								switch(op3)
																								{
																								case INTEG0:
																																INTEG = 0;
																																data |= INTEG;
																																break;

																								case INTEG1:
																																INTEG = 1;
																																data |= INTEG;
																																break;

																								case INTEG2:
																																INTEG = 2;
																																data |= INTEG;
																																break;

																								case INTEG3:
																																INTEG = 3;
																																data |= INTEG;
																																break;
																								}
																								//	pthread_mutex_lock(&light_i2c_mutex);
																								i2cWrite(file_handler, &data, 1);
																								//	pthread_mutex_unlock(&light_i2c_mutex);

																}
								}
								else if(op1 == READ)
								{
																//pthread_mutex_lock(&light_i2c_mutex);

																i2cRead(file_handler, buffer, 1);
																//pthread_mutex_unlock(&light_i2c_mutex);

								}
								else
																printf("ERROR: invalid operation\n");

}

void interrupReg(int file_handler, apds9301_opt op)
{
								char data;
								if(op == ENABLE) {
																data = 0x10;
																//pthread_mutex_lock(&light_i2c_mutex);

																i2cWrite(file_handler, &data, 1);
//																pthread_mutex_unlock(&light_i2c_mutex);

								}
								else if(op == DISABLE) {
																data = 0x00;
																//pthread_mutex_lock(&light_i2c_mutex);

																i2cWrite(file_handler, &data, 1);
																//pthread_mutex_unlock(&light_i2c_mutex);

								}
								else
																printf("ERROR: invalid operation\n");

}

void idRegRead(int file_handler)
{
								char part_num, rev_num, id;
								//pthread_mutex_lock(&light_i2c_mutex);

								i2cRead(file_handler, &id, 1);
								//	pthread_mutex_unlock(&light_i2c_mutex);

								part_num = id >> 4;
								rev_num = 0x0F & id;

								printf("PART NUMBER: %d\n", part_num);
								printf("REV NUMBER: %d\n", rev_num);
}

void interruptThreshReg(int file_handler, apds9301_opt op, char *buffer)
{
								if(op == READ) {

																//		pthread_mutex_lock(&light_i2c_mutex);
																i2cRead(file_handler, buffer, 1);
																//	pthread_mutex_unlock(&light_i2c_mutex);
								}
								else if(op == WRITE) {
																//	pthread_mutex_lock(&light_i2c_mutex);

																i2cWrite(file_handler, buffer, 1);
																//		pthread_mutex_unlock(&light_i2c_mutex);
								}
								else
																printf("ERROR: invalid operation\n");
}

uint16_t adcDataRead(int file_handler, int channel)
{
								char buffer[2];
								uint16_t adc_data;

								if(channel == 0) {
																pthread_mutex_lock(&light_i2c_mutex);

																commandReg(file_handler, DATA0LOW, WRITE);
																i2cRead(file_handler, &buffer[0], 1);
																pthread_mutex_unlock(&light_i2c_mutex);

																pthread_mutex_lock(&light_i2c_mutex);

																commandReg(file_handler, DATA0HIGH, WRITE);
																i2cRead(file_handler, &buffer[1], 1);

																pthread_mutex_unlock(&light_i2c_mutex);

								}
								else if (channel == 1) {
																pthread_mutex_lock(&light_i2c_mutex);

																commandReg(file_handler, DATA1LOW, WRITE);
																i2cRead(file_handler, &buffer[0], 1);
																pthread_mutex_unlock(&light_i2c_mutex);

																pthread_mutex_lock(&light_i2c_mutex);

																commandReg(file_handler, DATA1HIGH, WRITE);
																i2cRead(file_handler, &buffer[1], 1);
																pthread_mutex_unlock(&light_i2c_mutex);

								}
								else
																printf("ERROR: invalid operation\n");
								adc_data = (buffer[1] << 8 | buffer[0]);
								return adc_data;
}

float reportLumen(uint16_t adc_data_ch0, uint16_t adc_data_ch1)
{
								float lumen, ratio;
								ratio = (float)adc_data_ch1/adc_data_ch0;
								if(ratio > 0 && ratio <= 0.50)
								{
																double power=pow(ratio,1.4);
																lumen = (0.0304 * adc_data_ch0) - (0.062 * adc_data_ch0 * power);
								}
								else if(ratio > 0.50 && ratio <= 0.61)
																lumen = (0.0224 * adc_data_ch0) - (0.031 * adc_data_ch1);
								else if(ratio > 0.61 && ratio <= 0.80)
																lumen = (0.0128 * adc_data_ch0) - (0.0153 * adc_data_ch1);
								else if(ratio > 0.80 && ratio <= 1.30)
																lumen = (0.00146 * adc_data_ch0) - (0.00112 * adc_data_ch1);
								else
																lumen = 0;
								return lumen;
}

status reportStatus(int file_handler)
{
								status tod;
								uint16_t adc_ch0, adc_ch1;
								//pthread_mutex_lock(&light_i2c_mutex);

								adc_ch0 = adcDataRead(file_handler, 0);
								adc_ch1 = adcDataRead(file_handler, 1);
								//pthread_mutex_unlock(&light_i2c_mutex);

								float lumen = reportLumen(adc_ch0, adc_ch1);

								if(lumen <= LUMENS_NIGHT)
																tod = NIGHT;
								else
																tod = DAY;
								return tod;
}
