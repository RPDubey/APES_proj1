
#ifndef includes_H
#define includes_H

#include<stdint.h>
#include"messageQue.h"

#define PORT 8080

typedef enum{
init,
error
}msg_type;

typedef struct{
char msg[BUF_SIZE-sizeof(msg_type)];
msg_type type;
}err_msg_pack;

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
  }sock_req;

#endif
