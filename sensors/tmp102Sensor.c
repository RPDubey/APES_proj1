/*******************************************************************************
   @Filename:tmp102Sensor.c
   @Brief:
   @Author:Ravi Dubey
   @Date:3/14/2018
 ******************************************************************************/
#include "tmp102Sensor.h"
#include <pthread.h>

void tlowRead(int i2c_file_handler, char *buffer) {
  char P1P0 = TEMP_TLOW_REG;
  pthread_mutex_lock(&temp_i2c_mutex);
  i2cWrite(i2c_file_handler, &P1P0, 1);
  i2cRead(i2c_file_handler, buffer, 2);
  pthread_mutex_unlock(&temp_i2c_mutex);
}

void thighRead(int i2c_file_handler, char *buffer) {
  char P1P0 = TEMP_THIGH_REG;
  pthread_mutex_lock(&temp_i2c_mutex);
  i2cWrite(i2c_file_handler, &P1P0, 1);
  i2cRead(i2c_file_handler, buffer, 2);
  pthread_mutex_unlock(&temp_i2c_mutex);
}

void tlowWrite(int i2c_file_handler, char *buffer) {
  char send[3];
  send[0] = TEMP_TLOW_REG;
  send[1] = buffer[0];
  send[2] = buffer[1];
  pthread_mutex_lock(&temp_i2c_mutex);
  i2cWrite(i2c_file_handler, send, 3);
  pthread_mutex_unlock(&temp_i2c_mutex);
}

void thighWrite(int i2c_file_handler, char *buffer) {
  char send[3];
  send[0] = TEMP_THIGH_REG;
  send[1] = buffer[0];
  send[2] = buffer[1];
  pthread_mutex_lock(&temp_i2c_mutex);
  i2cWrite(i2c_file_handler, send, 3);
  pthread_mutex_unlock(&temp_i2c_mutex);
}

void temperatureRead(int i2c_file_handler, char *buffer) {
  char P1P0 = TEMP_READ_REG;
  pthread_mutex_lock(&temp_i2c_mutex);
  i2cWrite(i2c_file_handler, &P1P0, 1);
  i2cRead(i2c_file_handler, buffer, 2);
  pthread_mutex_unlock(&temp_i2c_mutex);
}

int initializeTemp() {
  int temp;
  pthread_mutex_lock(&temp_i2c_mutex);
  temp = i2cInit("/dev/i2c-2", temp, TEMP_ADDR);
  pthread_mutex_unlock(&temp_i2c_mutex);
  return temp;
}

void configRegWrite(int i2c_file_handler, char *buffer) {
  char send[3];
  send[0] = TEMP_CONFIG_REG;
  send[1] = buffer[0];
  send[2] = buffer[1];
  pthread_mutex_lock(&temp_i2c_mutex);
  i2cWrite(i2c_file_handler, send, 3);
  pthread_mutex_unlock(&temp_i2c_mutex);
}

void configRegRead(int file_handler, char *buffer) {
  char P1P0 = TEMP_CONFIG_REG;
  pthread_mutex_lock(&temp_i2c_mutex);
  i2cWrite(file_handler, &P1P0, 1);
  i2cRead(file_handler, buffer, 2);
  pthread_mutex_unlock(&temp_i2c_mutex);
}

float temperatureConv(temp_unit unit, char *buffer) {
  float temperature;
  unsigned char MSB, LSB;
  int temp_12b;
  // Reference: http://bildr.org/2011/01/tmp102-arduino
  MSB = buffer[0];
  LSB = buffer[1];
  // 12 bit result
  temp_12b = ((MSB << 8) | LSB);
  int negative = temp_12b & 0x8000;
  float multiplier = 0.0625;
  temp_12b = temp_12b >> 4;
  if (negative == 0x8000) {
    temp_12b = 0x0FFF - (temp_12b - 1);
    multiplier = -0.0625;
  }
  switch (unit) {
  case CELCIUS:
    temperature = temp_12b * multiplier;
    break;

  case FARENHEIT:
    temperature = 1.8 * (temp_12b * multiplier) + 32;
    break;

  case KELVIN:
    temperature = 273.15 + (temp_12b * multiplier);
    break;

  default:
    temperature = temp_12b * multiplier;
  }
  return temperature;
}
