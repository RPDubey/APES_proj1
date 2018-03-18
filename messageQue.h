/*******************************************************************************
   @Filename:messageQue.h
   @Brief: contains message que macros
   @Author:Ravi Dubey
   @Date:3/14/2018
 ******************************************************************************/
#ifndef messageQue_H
#define messageQue_H
#include <mqueue.h>


#define BUF_SIZE 4096
#define MQ_MAXMSG 10
#define MSG_PRIO  30
#define MSG_PRIO_INIT 25
#define MSG_PRIO_ERR 20

#define LOGGER_MQ   "/logger_MQ"
#define NOTIFY_MQ   "/error_MQ6"
#define IPC_TEMP_MQ   "/IPC_TEMP_MQ"
#define IPC_LIGHT_MQ   "/IPC_LIGHT_MQ"

#endif
