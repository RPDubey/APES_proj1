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
#include "./sensors/adps9301Sensor.h"
#define FREQ_NSEC (1000000000)

sig_atomic_t light_IPC_flag;

void LightIPChandler(int sig){
        if(sig == SIGLIGHT_IPC)
        {printf("Caught signal LightIPChandler\n");
         light_IPC_flag = 1;}
}

void *lightTask(void *pthread_inf) {

        uint8_t init_state = 0;
        char init_message[5][ sizeof(err_msg_pack) ];

        light_IPC_flag = 0;
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

        if(msgq_err < 0) {
                init_state =0;
                sprintf(&(init_message[0][0]),"mq_open-error_mq-lightTask %s\n",strerror(errno));
        }
        else {
                init_state =0;
                sprintf(&(init_message[0][0]),"error MessgaeQ Opened in lightTask:%s\n",strerror(errno));
        }


/********set periodic timer**********/
        ret = setLightTimer();
        if(ret ==-1) return NULL;
        else printf("Periodic Timer set for Light Task\n");

        ret = pthread_mutex_init(&glight_mutex,NULL);
        if(ret == -1) {
                init_state =0;
                sprintf(&(init_message[1][0]),"lightTask-pthread_mutex_init %s\n",strerror(errno));
        }
        else {
                init_state =0;
                sprintf(&(init_message[1][0]),"lightTaskpthread_mutex_init:%s\n",strerror(errno));
        }
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
        if(msgq < 0) {
                init_state =0;
                sprintf(&(init_message[2][0]),"lightTask-mq_open-loggerQ %s\n",strerror(errno));
        }
        else {
                init_state =0;
                sprintf(&(init_message[2][0]),"lightTask-mq_open-loggerQ %s\n",strerror(errno));
        }
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
        if(IPCmsgq < 0) {
                init_state =0;
                sprintf(&(init_message[3][0]),"lightTask-mq_open-IPCQ %s\n",strerror(errno));
        }
        else {
                init_state =1;
                sprintf(&(init_message[3][0]),"lightTask-mq_open-IPCQ %s\n",strerror(errno));
        }
//set up the signal to request data
        struct sigaction action;
        sigemptyset(&action.sa_mask);
        action.sa_handler = LightIPChandler;
        ret = sigaction(SIGLIGHT_IPC,&action,NULL);
        if(ret  == -1) {
                init_state =0;
                sprintf(&(init_message[4][0]),"sigaction lightTask %s\n",strerror(errno));
        }
        else {
                init_state =1;
                sprintf(&(init_message[4][0]),"sigaction lightTask %s\n",strerror(errno));
        }

/************Init light sensor********************/
        int light = initializeLight();
        char lightbuffer[1];
        commandReg(light,CONTROL,WRITE);
        controlReg(light,WRITE,ENABLE,lightbuffer);

        float lumen;
        char data_lumen_str[BUF_SIZE-200];
        uint16_t ch0,ch1;

        sleep(2);//allow other threads to Initialize


/*****************Mask SIGNALS********************/
        sigset_t mask; //set of signals
        sigemptyset(&mask);
        sigaddset(&mask,SIGTEMP); sigaddset(&mask,SIGTEMP_HB);
        sigaddset(&mask,SIGLOG); sigaddset(&mask,SIGLIGHT_HB);
        sigaddset(&mask,SIGLOG_HB); sigaddset(&mask,SIGCONT);

//unblocking for test
//sigaddset(&mask,SIGTEMP_IPC); sigaddset(&mask,SIGLIGHT_IPC);

        ret = pthread_sigmask(
                SIG_SETMASK, //block the signals in the set argument
                &mask, //set argument has list of blocked signals
                NULL); //if non NULL prev val of signal mask stored here
        if(ret == -1) { printf("Error:%s\n",strerror(errno)); return NULL; }

//send initialization status
        handle_err(&init_message[0][0],msgq_err,msgq,init);
        handle_err(&init_message[1][0],msgq_err,msgq,init);
        handle_err(&init_message[2][0],msgq_err,msgq,init);
        handle_err(&init_message[3][0],msgq_err,msgq,init);
        handle_err(&init_message[4][0],msgq_err,msgq,init);

/************Creating logpacket*******************/
        log_pack light_log ={.log_level=1,.log_source = light_Task};
        struct timespec now,expire;


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
                ch0=adcDataRead(light,0);
                ch1=adcDataRead(light,1);
                lumen = reportLumen(ch0, ch1);

/************populate the log packet*********/
                time_t t = time(NULL); struct tm *tm = localtime(&t);
                strcpy(light_log.time_stamp, asctime(tm));
                sprintf(data_lumen_str,"lumen %f", lumen);
                strcpy(light_log.log_msg, data_lumen_str);

/************Log messages on Que*************/
                //  sleep(30);
                clock_gettime(CLOCK_MONOTONIC,&now);
                expire.tv_sec = now.tv_sec+2;
                expire.tv_nsec = now.tv_nsec;
                num_bytes = mq_timedsend(msgq,
                                         (const char*)&light_log,
                                         sizeof(log_pack),
                                         msg_prio,
                                         &expire);
                if(num_bytes<0) {handle_err("mq_send-Log Q-lightTask",msgq_err,msgq,error);}
/******Log data on IPC Que if requested******/

                if(light_IPC_flag == 1) {
                        light_IPC_flag = 0;
//set up time for timed send

                        clock_gettime(CLOCK_MONOTONIC,&now);
                        expire.tv_sec = now.tv_sec+2;
                        expire.tv_nsec = now.tv_nsec;
                        num_bytes = mq_timedsend(IPCmsgq,
                                                 (const char*)&light_log,
                                                 sizeof(log_pack),
                                                 IPCmsg_prio,
                                                 &expire);
                        if(num_bytes<0) {handle_err("mq_send-IPC-lightTask Error",msgq_err,msgq,error);}
                        else printf("data put on IPC msg Q\n");
                }


        }
        printf("exiting light task\n");
        mq_close(msgq);
        mq_close(msgq_err);
        mq_close(IPCmsgq);
        return NULL;

}
