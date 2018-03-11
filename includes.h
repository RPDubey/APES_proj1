
#ifndef includes_H
#define includes_H

#include<stdint.h>

typedef enum {
  temperatue_Task,
  light_Task,
  RemoteRequestSocket_Task,
  error_handler
}task_type;

typedef struct{
char time_stamp[32];
uint8_t log_level;
task_type log_source;
char log_msg[16];
}log_pack;

#endif
