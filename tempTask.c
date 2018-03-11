#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "threads.h"
#include "signals.h"
#include "messageQue.h"
#include <mqueue.h>
#include "includes.h"
#include "errorhandling.h"

sig_atomic_t temp_IPC_flag;

void TemptIPChandler(int sig){
        if(sig == SIGTEMP_IPC)
        {printf("Caught signal TemptIPChandler\n");
         temp_IPC_flag = 1;}
}

void *tempTask(void *pthread_inf) {

        temp_IPC_flag = 0;
        int ret;
        threadInfo *ppthread_info = (threadInfo *)pthread_inf;



/*******Initialize ERROR Message Que*****************/
        mqd_t msgq_err;
        int msg_prio_err = MSG_PRIO_ERR;
        int num_bytes_err;
        struct mq_attr msgq_attr_err = {.mq_maxmsg = MQ_MAXMSG, //max # msg in queue
                                        .mq_msgsize = BUF_SIZE,//max size of msg in bytes
                                        .mq_flags = 0};

        msgq_err = mq_open(MY_MQ_ERR, //name
                           O_CREAT | O_RDWR,//flags. create a new if dosent already exist
                           S_IRWXU, //mode-read,write and execute permission
                           &msgq_attr_err); //attribute
        if(msgq_err < 0) {perror("mq_open-error_mq-tempTask "); return NULL;}
        else printf("Messgae Que Opened in tempTask\n");


/******set periodic timer**************/
        ret = setTempTimer(); //sets up timer to periodically signal and wake this thread
        if(ret ==-1) return NULL;
        else printf("Periodic Timer set for Temperature Task\n");

        ret = pthread_mutex_init(&gtemp_mutex,NULL);
        if(ret == -1) { printf("Error:%s\n",strerror(errno)); return NULL; }

/*******Initialize Logger Message Que*****************/
        mqd_t msgq;
        int msg_prio = 30;
        int num_bytes;
        char message[BUF_SIZE];
        struct mq_attr msgq_attr = {.mq_maxmsg = MQ_MAXMSG, //max # msg in queue
                                    .mq_msgsize = BUF_SIZE,//max size of msg in bytes
                                    .mq_flags = 0};

        msgq = mq_open(MY_MQ, //name
                       O_CREAT | O_RDWR,//flags. create a new if dosent already exist
                       S_IRWXU, //mode-read,write and execute permission
                       &msgq_attr); //attribute
        if(msgq < 0) {perror("mq_open-tempTask Error:"); return NULL;}
        else printf("Messgae Que Opened in tempTask\n");
/***************setting msgq for IPC data Request******************/
        mqd_t IPCmsgq;
        int IPCmsg_prio = 20;
        int IPCnum_bytes;
        char IPCmessage[BUF_SIZE];

        struct mq_attr IPCmsgq_attr = {.mq_maxmsg = MQ_MAXMSG, //max # msg in queue
                                       .mq_msgsize = BUF_SIZE, //max size of msg in bytes
                                       .mq_flags = 0};

        IPCmsgq = mq_open(IPC_TEMP_MQ, //name
                          O_CREAT | O_RDWR, //flags. create a new if dosent already exist
                          S_IRWXU, //mode-read,write and execute permission
                          &IPCmsgq_attr); //attribute
        if(IPCmsgq < 0) {perror("mq_open-tempTask Error:"); return NULL;}
        else printf("IPC Messgae Que Opened in tempTask\n");

//set up the signal to request data
        struct sigaction action;
        sigemptyset(&action.sa_mask);
        action.sa_handler = TemptIPChandler;
        ret = sigaction(SIGTEMP_IPC,&action,NULL);
        if(ret == -1) { perror("sigaction temptask"); return NULL; }
//        printf("pid:%d\n",getpid());

        struct timespec now,expire;

/************Creating logpacket*******************/
        log_pack temp_log ={.log_level=1,.log_source = temperatue_Task};

/****************Do this periodically*******************************/
        while(gclose_temp & gclose_app) {

                pthread_kill(ppthread_info->main,SIGTEMP_HB);//send HB
                pthread_mutex_lock(&gtemp_mutex);
                while(gtemp_flag == 0) {
                        pthread_cond_wait(&gtemp_condition,&gtemp_mutex);
                }

                pthread_mutex_unlock(&gtemp_mutex);
                gtemp_flag = 0;
//collect temperatue

/************populate the log packet*********/

                time_t t = time(NULL); struct tm *tm = localtime(&t);
                strcpy(temp_log.time_stamp, asctime(tm));
                strcpy(temp_log.log_msg, "tempTask");

/*******Log messages on Que*************/
//set up time for timed send

                clock_gettime(CLOCK_MONOTONIC,&now);
                expire.tv_sec = now.tv_sec+2;
                expire.tv_nsec = now.tv_nsec;
                num_bytes = mq_timedsend(msgq,
                                         (const char*)&temp_log,
                                         sizeof(log_pack),
                                         msg_prio,
                                         &expire);
                if(num_bytes<0) {handle_err("mq_send to Log Q in tempTask", msgq_err,msgq,error);}
/******Log data on IPC Que if requested******/

                if(temp_IPC_flag == 1) {
                        temp_IPC_flag = 0;
//set up time for timed send
                        clock_gettime(CLOCK_MONOTONIC,&now);
                        expire.tv_sec = now.tv_sec+2;
                        expire.tv_nsec = now.tv_nsec;
                        num_bytes = mq_timedsend(IPCmsgq,
                                                 (const char*)&temp_log,
                                                 sizeof(log_pack),
                                                 IPCmsg_prio,
                                                 &expire);
                        if(num_bytes<0) {handle_err("mq_send-IPC Q-tempTask Error",msgq_err,msgq,error);}
                        else printf("data put on IPC msg Q\n");
                }
                //printf("hi\n");
                handle_err("Test Error:",msgq_err,msgq,error);


        }
        printf("exiting Temp task\n");
        mq_close(msgq);
        mq_close(msgq_err);
        mq_close(IPCmsgq);

        return NULL;
}
