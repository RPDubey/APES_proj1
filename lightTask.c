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
#define FREQ_NSEC (1000000000)

sig_atomic_t light_IPC_flag;

void LightIPChandler(int sig){
        if(sig == SIGLIGHT_IPC)
        {printf("Caught signal LightIPChandler\n");
         light_IPC_flag = 1;}
}

void *lightTask(void *pthread_inf) {
        light_IPC_flag = 0;
        int ret;
        threadInfo *ppthread_info = (threadInfo *)pthread_inf;
/********set periodic timer**********/
        ret = setLightTimer();
        if(ret ==-1) return NULL;
        else printf("Periodic Timer set for Light Task\n");

        ret = pthread_mutex_init(&glight_mutex,NULL);
        if(ret == -1) { printf("Error:%s\n",strerror(errno)); return NULL; }

/*******Initialize Logger Message Que*****************/
        mqd_t msgq;
        int msg_prio = MSG_PRIO;
        int num_bytes;
        char message[BUF_SIZE];
        struct mq_attr msgq_attr = {.mq_maxmsg = MQ_MAXMSG, //max # msg in queue
                                    .mq_msgsize = BUF_SIZE,//max size of msg in bytes
                                    .mq_flags = 0};

        msgq = mq_open(MY_MQ, //name
                       O_CREAT | O_RDWR,//flags. create a new if dosent already exist
                       S_IRWXU, //mode-read,write and execute permission
                       &msgq_attr); //attribute
        if(msgq < 0) {perror("mq_open-lightTask Error:"); return NULL;}
        else printf("Messgae Que Opened in lightTask\n");

/***************setting msgq for IPC data Request******************/
        mqd_t IPCmsgq;
        int IPCmsg_prio = 20;
        int IPCnum_bytes;

        struct mq_attr IPCmsgq_attr = {.mq_maxmsg = MQ_MAXMSG, //max # msg in queue
                                       .mq_msgsize = BUF_SIZE, //max size of msg in bytes
                                       .mq_flags = 0};

        IPCmsgq = mq_open(IPC_LIGHT_MQ, //name
                          O_CREAT | O_RDWR, //flags. create a new if dosent already exist
                          S_IRWXU, //mode-read,write and execute permission
                          &IPCmsgq_attr); //attribute
        if(IPCmsgq < 0) {perror("mq_open-lightTask Error:"); return NULL;}
        else printf("IPC Messgae Que Opened in lightTask\n");

//set up the signal to request data
        struct sigaction action;
        sigemptyset(&action.sa_mask);
        action.sa_handler = LightIPChandler;
        ret = sigaction(SIGLIGHT_IPC,&action,NULL);
        if(ret == -1) { perror("sigaction lightTask"); return NULL; }
        printf("pid:%d\n",getpid());

/************Creating logpacket*******************/
        log_pack light_log ={.log_level=1,.log_source = light_Task};

/****************Do this periodically*******************************/
        while(gclose_light & gclose_app) {

                pthread_kill(ppthread_info->main,SIGLIGHT_HB);//send HB

                pthread_mutex_lock(&glight_mutex);
                while(glight_flag == 0) {
                        pthread_cond_wait(&glight_condition,&glight_mutex);
                }
                pthread_mutex_unlock(&glight_mutex);
                glight_flag = 0;
/*************collect data*****************/

/************populate the log packet*********/
                time_t t = time(NULL); struct tm *tm = localtime(&t);
                strcpy(light_log.time_stamp, asctime(tm));
                strcpy(light_log.log_msg, "lightTask");
/************Log messages on Que*************/
                //    sleep(30);
                num_bytes = mq_send(msgq,
                                    (const char*)&light_log,
                                    sizeof(log_pack),
                                    msg_prio);
                if(num_bytes<0) {perror("mq_send-lightTask Err"); gclose_light=0;}
/******Log data on IPC Que if requested******/

                if(light_IPC_flag == 1) {
                        light_IPC_flag = 0;
//set up time for timed send
                        struct timespec now,expire;
                        clock_gettime(CLOCK_MONOTONIC,&now);
                        expire.tv_sec = now.tv_sec+2;
                        expire.tv_nsec = now.tv_nsec;
                        num_bytes = mq_timedsend(IPCmsgq,
                                                 (const char*)&light_log,
                                                 sizeof(log_pack),
                                                 IPCmsg_prio,
                                                 &expire);
                        if(num_bytes<0) {perror("mq_send-IPC tempTask Error");}
                        else printf("data put on IPC msg Q\n");
                }


        }
        printf("exiting light task\n");
        mq_close(msgq);
        mq_close(IPCmsgq);
        return NULL;

}
