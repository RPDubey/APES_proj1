/*******************************************************************************
   @Filename:tempTask.c
   @Brief:
   @Author:Ravi Dubey
   @Date:3/12/2018
 ******************************************************************************/
#include "./sensors/tmp102Sensor.h"
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
#include <string.h>
#include <time.h>
#include <unistd.h>

// #define TEMP_THIGH_VAL 	(0x15)
// #define TEMP_TLOW_VAL 	(0x10)
// #define TEMP_CONFIG_VAL (0x60)

#define GET_TIME                                                               \
  clock_gettime(CLOCK_MONOTONIC, &now);                                        \
  expire.tv_sec = now.tv_sec + 2;                                              \
  expire.tv_nsec = now.tv_nsec;

void *tempTask(void *pthread_inf) {

  uint8_t init_state = 1;
  char init_message[8][sizeof(notify_pack)];
  temp_IPC_flag = 0;
  int ret;
  threadInfo *ppthread_info = (threadInfo *)pthread_inf;

  /*******Initialize ERROR Message Que*****************/
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
    sprintf(&(init_message[0][0]), "Temp Task mq_open-notify_mq  %s\n",
            strerror(errno));
  } else {
    sprintf(&(init_message[0][0]), "Temp Task mq_open-notify_mq %s\n",
            strerror(errno));
  }

  /******set periodic timer**************/
  ret = setTempTimer(); // sets up timer to periodically signal and wake this
                        // thread
  if (ret == -1) {
    init_state = 0;
    sprintf(&(init_message[1][0]), "Temptask-setTempTimer Error\n");
  } else {
    sprintf(&(init_message[1][0]), "Temptask-setTempTimer Success\n");
  }

  ret = pthread_mutex_init(&gtemp_mutex, NULL);
  if (ret == -1) {
    init_state = 0;
    sprintf(&(init_message[2][0]), "Temptask-pthread_mutex_init %s\n",
            strerror(errno));
  } else {
    sprintf(&(init_message[2][0]), "Temptask-pthread_mutex_init:%s\n",
            strerror(errno));
  }
  /*******Initialize Logger Message Que*****************/
  mqd_t logger_msgq;
  int msg_prio = 30;
  int num_bytes;
  char message[BUF_SIZE];
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
    sprintf(&(init_message[3][0]), "Temp Task-mq_open-loggerQ %s\n",
            strerror(errno));
  } else {
    sprintf(&(init_message[3][0]), "Temp Task-mq_open-loggerQ %s\n",
            strerror(errno));
  }
  /***************setting logger_msgq for IPC data Request******************/
  mqd_t IPCmsgq;
  int IPCmsg_prio = 20;
  int IPCnum_bytes;
  char IPCmessage[BUF_SIZE];

  struct mq_attr IPCmsgq_attr = {.mq_maxmsg = MQ_MAXMSG, // max # msg in queue
                                 .mq_msgsize =
                                     BUF_SIZE, // max size of msg in bytes
                                 .mq_flags = 0};

  IPCmsgq =
      mq_open(IPC_TEMP_MQ,      // name
              O_CREAT | O_RDWR, // flags. create a new if dosent already exist
              S_IRWXU,          // mode-read,write and execute permission
              &IPCmsgq_attr);   // attribute
  if (IPCmsgq < 0) {
    init_state = 0;
    sprintf(&(init_message[4][0]), "Temptask-mq_open-IPCQ %s\n",
            strerror(errno));
  } else {
    sprintf(&(init_message[4][0]), "Temptask-mq_open-IPCQ %s\n",
            strerror(errno));
  }
  // set up the signal to request data
  struct sigaction action;
  sigemptyset(&action.sa_mask);
  action.sa_handler = TemptIPChandler;
  ret = sigaction(SIGTEMP_IPC, &action, NULL);

  if (ret == -1) {
    init_state = 0;
    sprintf(&(init_message[5][0]), "Temp Task sigaction %s\n", strerror(errno));
  } else {
    sprintf(&(init_message[5][0]), "Temp task sigaction %s\n", strerror(errno));
  }

/************Initialize temperatue sensor******************/
#ifdef BBB
  int temp = initializeTemp();
  char temp_data[2], data_cel_str[BUF_SIZE - 200];
  float data_cel;

  if (temp == -1) {
    init_state = 0;
    sprintf(&(init_message[6][0]), "Temp Sensor init Error\n");
  } else {
    sprintf(&(init_message[6][0]), "Temp Sensor init Success\n");
  }
#endif

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
    sprintf(&(init_message[7][0]), "Temp task-pthread_sigmask %s\n",
            strerror(errno));
  } else {
    sprintf(&(init_message[7][0]), "Temp task-pthread_sigmask %s\n",
            strerror(errno));
  }

  // send initialization status
  notify(&init_message[0][0], notify_msgq, logger_msgq, init);
  notify(&init_message[1][0], notify_msgq, logger_msgq, init);
  notify(&init_message[2][0], notify_msgq, logger_msgq, init);
  notify(&init_message[3][0], notify_msgq, logger_msgq, init);
  notify(&init_message[4][0], notify_msgq, logger_msgq, init);
#ifdef BBB
  notify(&init_message[5][0], notify_msgq, logger_msgq, init);
#endif
  notify(&init_message[6][0], notify_msgq, logger_msgq, init);
  notify(&init_message[7][0], notify_msgq, logger_msgq, init);

  if (init_state == 0) {
    notify("##All elements not initialized in Temp Task, Not proceeding with "
           "it##\n",
           notify_msgq, logger_msgq, error);
    while (gclose_temp & gclose_app) {
      sleep(1);
    };
    return NULL;
  }

  else if (init_state == 1)
    notify("##All elements initialized in Temp Task, proceeding with it##\n",
           notify_msgq, logger_msgq, init);

  /************Creating logpacket*******************/
  log_pack temp_log = {.log_level = 1, .log_source = temperatue_Task};
  struct timespec now, expire;

  sleep(1);

#ifdef BBB
/*****Logging BBB configurations*******/
#ifdef REG_MANIPULATE
  char buffer[2];

  buffer[0] = TEMP_TLOW_VAL_B1;
  buffer[1] = TEMP_TLOW_VAL_B2;
  tlowWrite(temp, buffer);
  buffer[0] = buffer[1] = 0;
  tlowRead(temp, buffer);
  printf("Temp Sensor TLOW Read %x %x \n", buffer[0], buffer[1]);

  float val = temperatureConv(CELCIUS, buffer);
  printf("Temp Sensor TLOW Read in cecius %f\n", val);

  buffer[0] = TEMP_THIGH_VAL_B1;
  buffer[1] = TEMP_THIGH_VAL_B2;
  thighWrite(temp, buffer);
  buffer[0] = buffer[1] = 0;
  thighRead(temp, buffer);
  printf("Temp Sensor THIGH Read %x %x \n", buffer[0], buffer[1]);
  val = temperatureConv(CELCIUS, buffer);
  printf("Temp Sensor THIGH Read Celcius %f \n", val);

  buffer[0] = buffer[1] = 0;
  configRegRead(temp, buffer);
  printf("Config Register Default Val %x %x \n", buffer[0], buffer[1]);

  buffer[0] |= SHUTDOWN_EN;
  configRegWrite(temp, buffer);

  buffer[0] = buffer[1] = 0;
  configRegRead(temp, buffer);
  printf("Config Register Val after shutdown Enable %x %x \n", buffer[0],
         buffer[1]);

  buffer[0] &= SHUTDOWN_DI;
  configRegWrite(temp, buffer);

  buffer[0] = buffer[1] = 0;
  configRegRead(temp, buffer);
  printf("Config Register after shutdown disable %x %x \n", buffer[0],
         buffer[1]);

  buffer[1] |= EMMODE_EN;
  configRegWrite(temp, buffer);

  buffer[0] = buffer[1] = 0;
  configRegRead(temp, buffer);
  printf("Config Register after setting EM mode %x %x \n", buffer[0],
         buffer[1]);

  buffer[1] &= EMMODE_DI;
  configRegWrite(temp, buffer);

  buffer[0] = buffer[1] = 0;
  configRegRead(temp, buffer);
  printf("Config Register after disabling EM mode %x %x \n", buffer[0],
         buffer[1]);

  buffer[1] |= CONVRATE3;
  configRegWrite(temp, buffer);

  buffer[0] = buffer[1] = 0;
  configRegRead(temp, buffer);
  printf("Config Register after setting conversion rate to 8 HZ %x %x \n",
         buffer[0], buffer[1]);

  buffer[0] |= TM_EN;
  configRegWrite(temp, buffer);

  buffer[0] = buffer[1] = 0;
  configRegRead(temp, buffer);
  printf("Config Register val after setting interrupt mode %x %x \n", buffer[0],
         buffer[1]);

#endif

#endif

  /****************Do this periodically*******************************/
  while (gclose_temp & gclose_app) {
#ifdef BBB
    INTR_LED_OFF;
#endif
    // wait for next second
    pthread_mutex_lock(&gtemp_mutex);
    while (gtemp_flag == 0) {
      pthread_cond_wait(&gtemp_condition, &gtemp_mutex);
    }

    pthread_mutex_unlock(&gtemp_mutex);
    gtemp_flag = 0;
    // send HB
    pthread_kill(ppthread_info->main, SIGTEMP_HB);
// collect temperatue
#ifdef BBB
    temperatureRead(temp, temp_data);
    data_cel = temperatureConv(temp_format, temp_data);

    /************populate the log packet*********/
    sprintf(data_cel_str, "temperature %f", data_cel);
    strcpy(temp_log.log_msg, data_cel_str);
#else
    strcpy(temp_log.log_msg, "Mock temp");

#endif
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strcpy(temp_log.time_stamp, asctime(tm));

    /*******Log messages on Que*************/
    // set up time for timed send

    clock_gettime(CLOCK_MONOTONIC, &now);
    expire.tv_sec = now.tv_sec + 2;
    expire.tv_nsec = now.tv_nsec;
    num_bytes = mq_timedsend(logger_msgq, (const char *)&temp_log,
                             sizeof(log_pack), msg_prio, &expire);
    if (num_bytes < 0) {
      notify("mq_send to Log Q in tempTask", notify_msgq, logger_msgq, error);
    }
    /******Log data on IPC Que if requested******/

    if (temp_IPC_flag == 1) {
      temp_IPC_flag = 0;
      // set up time for timed send
      clock_gettime(CLOCK_MONOTONIC, &now);
      expire.tv_sec = now.tv_sec + 2;
      expire.tv_nsec = now.tv_nsec;
      num_bytes = mq_timedsend(IPCmsgq, (const char *)&temp_log,
                               sizeof(log_pack), IPCmsg_prio, &expire);
      if (num_bytes < 0) {
        notify("mq_send-IPC Q-tempTask Error", notify_msgq, logger_msgq, error);
      }
    }
    // printf("hi\n");
    //  notify("Test Error",notify_msgq,logger_msgq,error);
  }
  printf("exiting Temp task\n");
  mq_close(logger_msgq);
  mq_close(notify_msgq);
  mq_close(IPCmsgq);

  return NULL;
}
