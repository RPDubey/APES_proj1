/*******************************************************************************
   @Filename:logTask.c
   @Brief: implements logger task functions
   @Author:Ravi Dubey
   @Date:3/14/2018
 ******************************************************************************/
#include "includes.h"
#include "messageQue.h"
#include "notification.h"
#include "signals.h"
#include "threads.h"
#include <errno.h>
#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

sig_atomic_t log_data_flag;

void *logTask(void *pthread_inf) {

  int ret;
  uint8_t init_state = 1;
  char init_message[4][sizeof(notify_pack)];

  // log_data_flag=0;
  threadInfo *ppthread_info = (threadInfo *)pthread_inf;

  /*******Initialize Notification Message Que*****************/
  mqd_t notify_msgq;
  int msg_prio_err = MSG_PRIO_ERR;
  int num_bytes_err;
  struct mq_attr msgq_attr_err = {.mq_maxmsg = MQ_MAXMSG, // max # msg in queue
                                  .mq_msgsize =
                                      BUF_SIZE, // max size of msg in bytes
                                  .mq_flags = 0};

  notify_msgq =
      mq_open(NOTIFY_MQ,        // name
              O_CREAT | O_RDWR, // flags. create a new if dosent already exist
              S_IRWXU,          // mode-read,write and execute permission
              &msgq_attr_err);  // attribute
  if (notify_msgq < 0) {
    init_state = 0;
    sprintf(&(init_message[1][0]), "Log Task-mq_open-notify_mq %s\n",
            strerror(errno));
  } else {
    sprintf(&(init_message[1][0]), "Log Task-mq_open-notify_mq %s\n",
            strerror(errno));
  }

  /*************create a logger message q****************/
  mqd_t logger_msgq;
  int msg_prio;
  int num_bytes;
  // char message[BUF_SIZE];
  log_pack *log = (log_pack *)malloc(sizeof(log_pack));
  if (log == NULL) {
    perror("Log Task malloc Error");
    return NULL;
  }

  struct mq_attr msgq_attr = {.mq_maxmsg = MQ_MAXMSG, // max # msg in queue
                              .mq_msgsize =
                                  BUF_SIZE, // max size of msg in bytes
                              .mq_flags = 0};

  logger_msgq =
      mq_open(LOGGER_MQ,        // name
              O_CREAT | O_RDWR, // flags. create a new if dosent already exist
              S_IRWXU,          // mode-read,write and execute permission
              &msgq_attr);      // attribute
  if (logger_msgq < 0) {
    init_state = 0;
    sprintf(&(init_message[2][0]), "LogTask-mq_open-loggerQ %s\n",
            strerror(errno));
  } else {
    sprintf(&(init_message[2][0]), "LogTask-mq_open-loggerQ %s\n",
            strerror(errno));
  }

  // open a file to write to
  FILE *pfd = fopen(filename, "w");
  if (pfd == NULL) {
    init_state = 0;
    sprintf(&(init_message[3][0]), "Log Task-fopen %s\n", strerror(errno));
  } else {

    sprintf(&(init_message[3][0]), "Log Task-fopen %s\n", strerror(errno));
  }

  /*****************Mask SIGNALS********************/
  sigset_t mask; // set of signals
  sigemptyset(&mask);
  sigaddset(&mask, SIGLIGHT);
  sigaddset(&mask, SIGLIGHT_HB);
  sigaddset(&mask, SIGLOG_HB);
  sigaddset(&mask, SIGTEMP_HB);
  sigaddset(&mask, SIGLOG);
  sigaddset(&mask, SIGCONT);
  sigaddset(&mask, SIGSOCKET_HB);

  ret =
      pthread_sigmask(SIG_SETMASK, // block the signals in the set argument
                      &mask,       // set argument has list of blocked signals
                      NULL); // if non NULL prev val of signal mask stored here
  if (ret == -1) {
    init_state = 0;
    sprintf(&(init_message[0][0]), "LogTask Sigmask %s\n", strerror(errno));
  } else {

    sprintf(&(init_message[0][0]), "LogTask Sigmask:%s\n", strerror(errno));
  }
/*******&&&&&&&&&&&&&& msg Q to test IPC
 * transfer&&&&&&&&&&&&&&&&&&**********************/
#ifdef PRINT_IPC_MSGQ

  mqd_t IPCmsgqT, IPCmsgqL;
  int IPCmsg_prio = 20;
  int IPCnum_bytes;
  char IPCmessage[BUF_SIZE];

  struct mq_attr IPCmsgq_attr = {.mq_maxmsg = MQ_MAXMSG, // max # msg in queue
                                 .mq_msgsize =
                                     BUF_SIZE, // max size of msg in bytes
                                 .mq_flags = 0};
  // temp
  IPCmsgqT =
      mq_open(IPC_TEMP_MQ,      // name
              O_CREAT | O_RDWR, // flags. create a new if dosent already exist
              S_IRWXU,          // mode-read,write and execute permission
              &IPCmsgq_attr);   // attribute
  if (IPCmsgqT < 0) {
    perror("mq_open-logTask Error:");
    return NULL;
  } else
    printf("IPC temp Messgae Que Opened in logTask\n");
  // light
  IPCmsgqL =
      mq_open(IPC_LIGHT_MQ,     // name
              O_CREAT | O_RDWR, // flags. create a new if dosent already exist
              S_IRWXU,          // mode-read,write and execute permission
              &IPCmsgq_attr);   // attribute
  if (IPCmsgqL < 0) {
    perror("mq_open-logTask Error:");
    return NULL;
  } else
    printf("IPC light Messgae Que Opened in logTask\n");
#endif
  struct timespec now, expire;

  // send initialization message
  notify(&init_message[0][0], notify_msgq, -1, init);
  notify(&init_message[1][0], notify_msgq, -1, init);
  notify(&init_message[2][0], notify_msgq, -1, init);
  notify(&init_message[3][0], notify_msgq, -1, init);

  if (init_state == 0) {
    notify("##All elements not initialized in Log Task. Not proceeding with "
           "it##\n",
           notify_msgq, logger_msgq, error);
    while (gclose_log & gclose_app) {
      sleep(1);
    };
    return NULL;
  }

  else if (init_state == 1)
    notify("##All elements initialized in Log Task, proceeding with it##\n",
           notify_msgq, logger_msgq, init);

  /*******************Do this in LOOP************************************/
  while (gclose_log & gclose_app) {

    pthread_kill(ppthread_info->main, SIGLOG_HB); // send HB
    // empty the message que
    do {
      // read from queue
      clock_gettime(CLOCK_MONOTONIC, &now);
      expire.tv_sec = now.tv_sec + 5;
      expire.tv_nsec = now.tv_nsec;

      num_bytes = mq_timedreceive(logger_msgq, (char *)log, BUF_SIZE, &msg_prio,
                                  &expire);

      // write to a log file
      if (num_bytes > 0) {
        fprintf(pfd, "%s  %d  %d  %s\n\n", ((log_pack *)log)->time_stamp,
                ((log_pack *)log)->log_level, ((log_pack *)log)->log_source,
                ((log_pack *)log)->log_msg);
        fflush(pfd);
      }
    } while (num_bytes > 0);
// reregister after emptying the que
// ret  = mq_notify(logger_msgq,&sig_ev);
// if(ret == -1) {perror("mq_notify-main"); return NULL;}

/*&&&&&&&&&&& printing data for IPC message Q test&&&&&&&&&&&&&&*/

#ifdef PRINT_IPC_MSGQ
    clock_gettime(CLOCK_MONOTONIC, &now);
    expire.tv_sec = now.tv_sec + 1;
    expire.tv_nsec = now.tv_nsec;
    num_bytes =
        mq_timedreceive(IPCmsgqT, (char *)log, BUF_SIZE, &IPCmsg_prio, &expire);
    if (num_bytes > 0) {
      printf("data read on IPC msg Q:%s\n", log->time_stamp);
    }

    clock_gettime(CLOCK_MONOTONIC, &now);
    expire.tv_sec = now.tv_sec + 1;
    expire.tv_nsec = now.tv_nsec;
    num_bytes =
        mq_timedreceive(IPCmsgqL, (char *)log, BUF_SIZE, &IPCmsg_prio, &expire);
    if (num_bytes > 0) {
      printf("data read on IPC msg Q:%s\n", log->time_stamp);
    }
#endif
    sleep(1);
  }

  printf("exiting Log task\n");
  fclose(pfd);
  mq_close(logger_msgq);
  mq_close(notify_msgq);
#ifdef PRINT_IPC_MSGQ
  mq_close(IPCmsgqT);
  mq_close(IPCmsgqL);
#endif
  free(log);
  return NULL;
}
