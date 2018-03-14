
#include <math.h>
#include "i2cWrappper.h"

#define LIGHT_ADDR	(0x39)
#define POWERUP 	  (0x03)
#define POWERDOWN 	(0x00)
#define LUMENS_NIGHT (10)

//sensor registers
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


int initializeLight(void);

void commandReg(int file_handler, adps9301_regs reg, apds9301_opt op);

void controlReg(int file_handler, apds9301_opt opt1, apds9301_opt op2, char *buffer);

void timingReg(int file_handler, apds9301_opt op1, apds9301_opt op2,
 							 apds9301_opt op3, char *buffer );

void interrupReg(int file_handler, apds9301_opt op);

void idRegRead(int file_handler);

void interruptThreshReg(int file_handler, apds9301_opt op, char *buffer);

uint16_t adcDataRead(int file_handler, int channel);

float reportLumen(uint16_t adc_data_ch0, uint16_t adc_data_ch1);

status reportStatus(int file_handler);
