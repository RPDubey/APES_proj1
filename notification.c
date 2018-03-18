/*******************************************************************************
   @Filename:notification.c
   @Brief: definition for notification(init and error) threads
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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static pthread_mutex_t error_msg_lock = PTHREAD_MUTEX_INITIALIZER;

/****************puts initialization messages and errors on notify msg q and
 * logger msg q. notify msg q is registered to notify main, which creates a
 * thread to handle these conditions appropriately*************/
void notify(char *msg, mqd_t notify_msgq, mqd_t logger_msgq, msg_type type) {
  // char msg[BUF_SIZE];
  // sprintf(msg,"%s:%s",arg_msg,strerror(errno));

  notify_pack err_pack = {.type = type};
  strcpy(err_pack.msg, msg);
  struct timespec now, expire;
  int num_bytes;
  clock_gettime(CLOCK_MONOTONIC, &now);
  expire.tv_sec = now.tv_sec + 2;
  expire.tv_nsec = now.tv_nsec;
  int prio;
  if (type == init)
    prio = MSG_PRIO_INIT;
  else if (type == error)
    prio = MSG_PRIO_ERR;

  // log on notify msg q
  if (notify_msgq >= 0) {
    num_bytes = mq_timedsend(notify_msgq, (const char *)&err_pack,
                             sizeof(err_pack), prio, &expire);
    if (num_bytes < 0) {
      perror("mq_send to notify_msgq in notify");
    }
  }
  if (logger_msgq >= 0) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    log_pack err_log = {.log_level = 2, .log_source = error_handler};
    strcpy(err_log.time_stamp, asctime(tm));
    strcpy(err_log.log_msg, msg);

    // log on logegr q
    clock_gettime(CLOCK_MONOTONIC, &now);
    expire.tv_sec = now.tv_sec + 5;
    expire.tv_nsec = now.tv_nsec;

    num_bytes = mq_timedsend(logger_msgq, (const char *)&err_log,
                             sizeof(log_pack), prio, &expire);
    if (num_bytes < 0) {
      perror("mq_send to logger_msgq in notify");
    }
  }

  return;
}

/*This thread is invoked via mq_notify on main once any initialization or error
 * message is put on notify_msgq
 */
void notifyRcvThread(union sigval sv) {
  // read and print the error
  if (sv.sival_ptr == NULL) {
    printf("notifyRcvThread argument\n");
    return;
  }
  mqd_t notify_msgq = *((mqd_t *)sv.sival_ptr);
  struct timespec now, expire;
  clock_gettime(CLOCK_MONOTONIC, &now);
  expire.tv_sec = now.tv_sec + 2;
  expire.tv_nsec = now.tv_nsec;
  int num_bytes;
  int error_flag = 0;
  int ret = pthread_mutex_lock(&error_msg_lock);
  if (ret != 0) {
    perror("notifyRcvThread-mutexlock");
    return;
  }
  // empty the logger_msgq - notification only for an empty q
  do {
    bzero(msg_pack, sizeof(msg_pack));

    num_bytes = mq_timedreceive(notify_msgq, (char *)msg_pack,
                                sizeof(notify_pack), NULL, &expire);
    if (num_bytes > 0) {
      printf("%s", ((notify_pack *)msg_pack)->msg);
      if (((notify_pack *)msg_pack)->type == error)
        error_flag = 1;
    }

  } while (num_bytes > 0);
  ret = pthread_mutex_unlock(&error_msg_lock);
  if (ret != 0) {
    perror("notifyRcvThread-mutex unlock");
    return;
  }

  // reregister for notification
  ret = mq_notify(notify_msgq, &sig_ev_err);
  if (ret == -1) {
    perror("mq_notify-notifyRcvThread");
    return;
  }

// For error condition turn on the LED
#ifdef BBB
  if (error_flag == 1)
    LED_CONTROL(1);
#endif

  return;
}
