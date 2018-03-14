
#include "tmp102Sensor.h"


// The Pointer Register uses the two least-significant bytes (LSBs) to identify
// which of the data registers must respond to a read or write command.

void tlowRead(int i2c_file_handler, char *buffer)
{
        char P1P0= TEMP_TLOW_REG;
        i2cWrite(i2c_file_handler, &P1P0, 1);
        i2cRead(i2c_file_handler, buffer,2);
}

void thighRead(int i2c_file_handler, char *buffer)
{
        char P1P0= TEMP_THIGH_REG;
        i2cWrite(i2c_file_handler, &P1P0, 1);
        i2cRead(i2c_file_handler, buffer,2);
}

void tlowWrite(int i2c_file_handler, char *buffer)
{
        buffer[0]= TEMP_TLOW_REG; buffer[1]= TEMP_TLOW_VAL;
        i2cWrite(i2c_file_handler, buffer, 2);
}

void thighWrite(int i2c_file_handler, char *buffer)
{
        buffer[0]= TEMP_THIGH_REG; buffer[1]= TEMP_THIGH_VAL;
        i2cWrite(i2c_file_handler, buffer, 2);
}


//  byte 1 is MSB,  byte 2LSB. First 12 bits indicate temperature.
void temperatureRead(int i2c_file_handler, char *buffer)
{
        char P1P0= TEMP_READ_REG;
        i2cWrite(i2c_file_handler, &P1P0, 1);
        i2cRead(i2c_file_handler, buffer,2);
}

int initializeTemp()
{
        int temp;
        temp=i2cInit("/dev/i2c-2",temp,TEMP_ADDR);
        return temp;
}


void configRegWrite(int file_handler, char *buffer)
{
        buffer[0]= TEMP_CONFIG_REG; buffer[1]= TEMP_CONFIG_VAL;
        i2cWrite(file_handler, buffer, 2);
}

void configRegRead(int file_handler, char *buffer)
{
        char P1P0 = TEMP_CONFIG_REG;
        i2cWrite(file_handler, &P1P0, 1);
        i2cRead(file_handler, buffer,2);
}

float temperatureConv(temp_unit unit, char *buffer)
{
        float temperature;
        unsigned char MSB, LSB;
        int temp_12b;
        // Reference: http://bildr.org/2011/01/tmp102-arduino
        MSB = buffer[0];
        LSB = buffer[1];
        // 12 bit result
        temp_12b = ((MSB << 8) | LSB) >> 4;

        switch(unit)
        {
        case CELCIUS:
                temperature = temp_12b * 0.0625;
                break;

        case FARENHEIT:
                temperature = 1.8 * (temp_12b * 0.0625) + 32;
                break;

        case KELVIN:
                temperature = 273.15 + (temp_12b * 0.0625);
                break;

        default:
                temperature = 0;
                break;
        }
        return temperature;
}
