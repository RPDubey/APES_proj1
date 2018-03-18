/*******************************************************************************
   @Filename:includes.h
   @Brief:
   @Author:Ravi Dubey
   @Date:3/14/2018
 ******************************************************************************/
#ifndef includes_H
#define includes_H

#include<stdint.h>
#include"messageQue.h"
#define PORT 8080
#define DEFAULT_FILE_NAME ("logfile.txt")

#define SLEEP(t)                                                               \
  struct timespec current, remaining;                                          \
  current.tv_nsec = 0;                                                         \
  current.tv_sec = t;                                                          \
  do {                                                                         \
    ret = nanosleep(&current, &remaining);                                     \
    if (ret == -1) {                                                           \
      current.tv_sec = remaining.tv_sec;                                       \
      current.tv_nsec = remaining.tv_nsec;                                     \
    }                                                                          \
  } while (ret != 0);
char* filename;

#define BBB
//#define TEST
#define REG_MANIPULATE

#ifdef Test
#undef BBB
#endif

#ifdef BBB
#undef TEST
#define LED_ON (system("echo 1 > /sys/class/leds/beaglebone:green:usr0/brightness"))
#define LED_OFF (system("echo 0 > /sys/class/leds/beaglebone:green:usr0/brightness"))
#define LED_CONTROL(status) {if(status == 1) LED_ON; \
                             if(status == 0) LED_OFF; }

#define READY_LED 		(system("echo none >/sys/class/leds/beaglebone:green:usr1/trigger"))
#define INTR_LED_ON 		(system("echo 1 > /sys/class/leds/beaglebone:green:usr1/brightness"))

#define INTR_LED_OFF		(system("echo 0 > /sys/class/leds/beaglebone:green:usr1/brightness"))



#endif

pthread_mutex_t light_i2c_mutex;
pthread_mutex_t temp_i2c_mutex;
pthread_mutex_t i2c_mutex;

typedef enum{
	CELCIUS,
	FARENHEIT,
	KELVIN
} temp_unit;

typedef enum{
	LUMEN,
	DAY_NIGHT
} light_unit;

temp_unit temp_format;


typedef enum{
init,
error,
notification
}msg_type;

typedef struct{
char msg[BUF_SIZE-sizeof(msg_type)];
msg_type type;
}notify_pack;

typedef enum {
  temperatue_Task,
  light_Task,
  RemoteRequestSocket_Task,
  error_handler
}task_type;


typedef struct{
char time_stamp[32];
task_type log_source;
char log_msg[BUF_SIZE -100];
uint8_t log_level;
}log_pack;

typedef enum{
temp,
light
}sensor_type;

//request structure
  typedef struct {
  sensor_type sensor;
  temp_unit tunit;//if sensor is temperature
  light_unit lunit;
}sock_req;

#endif
