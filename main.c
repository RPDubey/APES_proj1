/*******************************************************************************
   @Filename:main.c
   @Brief:
   @Author:Ravi Dubey
   @Date:3/10/2018
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "includes.h"
#include "threads.h"
#include "signals.h"
#include "messageQue.h"
#include <mqueue.h>
#include "notification.h"
#define SLEEP(t)      struct timespec current,remaining; \
        current.tv_nsec = 0; current.tv_sec = t; \
        do { \
                ret = nanosleep(&current,&remaining); \
                if(ret == -1) { \
                        current.tv_sec = remaining.tv_sec; \
                        current.tv_nsec = remaining.tv_nsec;} \
        } while(ret != 0);  \


sig_atomic_t glight_HB_flag;
sig_atomic_t gtemp_HB_flag;
sig_atomic_t glog_HB_flag;

void LightHBhandler(int sig){
        if(sig == SIGLIGHT_HB)
        {
                //  printf("L");
                glight_HB_flag = 1;
        }
}

void TempHBhandler(int sig){
        if(sig == SIGTEMP_HB)
        {
                //  printf("T");
                gtemp_HB_flag = 1;
        }
}

void LogHBhandler(int sig){
        if(sig == SIGLOG_HB)
        {
                //    printf("l");
                glog_HB_flag = 1;
        }
}


int main()
{
        int ret;
        msg_pack = (notify_pack*)malloc(sizeof(notify_pack));
//pthread_mutex_init
        if(msg_pack == NULL) {perror("malloc-main"); return -1;}
        printf("Entering Main- PID:%d\n",getpid());
        gclose_app = 1; gclose_light = 1; gclose_temp = 1; gclose_log = 1;
        gclose_socket = 1; gtemp_HB_flag = 0; glight_HB_flag = 0;

/*******************Masking SIgnals***********************/

//this thread will be inherited by all
        sigset_t mask, mask_all; //set of signals
        sigfillset(&mask_all);
        ret = pthread_sigmask(
                SIG_SETMASK, //block the signals in the set argument
                &mask_all, //set argument has list of blocked signals
                NULL); //if non NULL prev val of signal mask stored here
        if(ret == -1) { printf("Main pthread_sigmask:%s\n",strerror(errno)); return -1; }

/**************install SIGINT handler to close application through ctrl + c*************/
        signal(SIGINT,SIGINT_handler);

/******initialize notification msgq*************************************/
        mqd_t notify_msgq;
        int msg_prio_err = MSG_PRIO_ERR;
        int num_bytes_err;
        struct mq_attr msgq_attr_err = {.mq_maxmsg = MQ_MAXMSG, //max # msg in queue
                                        .mq_msgsize = BUF_SIZE,//max size of msg in bytes
                                        .mq_flags = 0};

        notify_msgq = mq_open(NOTIFY_MQ, //name
                              O_CREAT | O_RDWR, //flags. create a new if dosent already exist
                              S_IRWXU,  //mode-read,write and execute permission
                              &msgq_attr_err);  //attribute
        if(notify_msgq < 0) {perror("Main mq_open"); return -1;}
//set up notification for this error

        sig_ev_err.sigev_notify=SIGEV_THREAD;      //notify by signal in sigev_signo
        sig_ev_err.sigev_notify_function = notifyRcvThread;
        sig_ev_err.sigev_notify_attributes=NULL;
        sig_ev_err.sigev_value.sival_ptr=&notify_msgq;  //data passed with notification

        //register for notification
        ret  = mq_notify(notify_msgq,&sig_ev_err);
        if(ret == -1) {perror("Main-mq_notify"); return -1;}

//Register Light HB signal
        struct sigaction action;
        sigemptyset(&action.sa_mask);
        action.sa_handler = LightHBhandler;
        ret = sigaction(SIGLIGHT_HB,&action,NULL);
        if(ret == -1) { perror("Main sigaction"); return -1; }

//Register Temp HB signal
        sigemptyset(&action.sa_mask);
        action.sa_handler = TempHBhandler;
        ret = sigaction(SIGTEMP_HB,&action,NULL);
        if(ret == -1) { perror("Main sigaction"); return -1; }

//Register Log HB signal
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        action.sa_handler = LogHBhandler;
        ret = sigaction(SIGLOG_HB,&action,NULL);
        if(ret == -1) { perror("Main sigaction"); return -1; }

        //create task threads
        pthread_t temp,light,log,socket;
        threadInfo temp_info; temp_info.thread_id = 1; temp_info.main=pthread_self();
        threadInfo light_info; light_info.thread_id = 2; light_info.main=pthread_self();
        threadInfo log_info; log_info.thread_id = 3; log_info.main=pthread_self();
        threadInfo socket_info; socket_info.thread_id = 3; socket_info.main=pthread_self();

        ret = pthread_create(  &log,
                               DEFAULT_THREAD_ATTR,
                               logTask,
                               (void *)&(log_info) );
        if (ret != 0) {  printf("Main pthread_create:%s\n",strerror(errno)); return -1;}
        sleep(1);
        ret = pthread_create(  &light,
                               DEFAULT_THREAD_ATTR,
                               lightTask,
                               (void *)&(light_info) );
        if (ret != 0) {  printf("Main pthread_create:%s\n",strerror(errno)); return -1;}
        sleep(1);
        ret = pthread_create(  &temp,
                               DEFAULT_THREAD_ATTR,
                               tempTask,
                               (void *)&(temp_info) );
        if (ret != 0) {  printf("Main pthread_create:%s\n",strerror(errno)); return -1;}
        sleep(1);
        ret = pthread_create(  &socket,
                               DEFAULT_THREAD_ATTR,
                               socketTask,
                               (void *)&(socket_info) );
        if (ret != 0) {  printf("Main pthread_create:%s\n",strerror(errno)); return -1;}


        uint8_t read_bytes; char choice;
        uint8_t light_cancelled=0; uint8_t temp_cancelled=0;
        uint8_t log_cancelled=0; uint8_t socket_cancelled=0;
        SLEEP(1);//allow other threads to initialize

        // required masks for main
        sigemptyset(&mask);
        sigaddset(&mask,SIGTEMP); sigaddset(&mask,SIGLIGHT);
        sigaddset(&mask,SIGTEMP_IPC); sigaddset(&mask,SIGLIGHT_IPC);
        sigaddset(&mask,SIGLOG); sigaddset(&mask,SIGCONT);

        ret = pthread_sigmask(
                SIG_SETMASK, //block the signals in the set argument
                &mask, //set argument has list of blocked signals
                NULL); //if non NULL prev val of signal mask stored here
        if(ret == -1) { printf("Main pthread_sigmask:%s\n",strerror(errno)); return -1; }


        printf("\n*******************************************************************\n");
        printf(" Enter thread to close 1-temp 2-light 3-log 4-socket 5-application\n");
        printf("*******************************************************************\n");

        while (gclose_app) {

//check HB signals every 5 seconds
                SLEEP(5);

                printf("M");

                if(light_cancelled == 0) {
                        if(glight_HB_flag == 0) printf("NO HB from Light Task\n");
                        else {printf("L"); glight_HB_flag = 0;}
                }

                if(temp_cancelled == 0) {
                        if(gtemp_HB_flag == 0) printf("NO HB from Temp Task\n");
                        else {printf("T"); gtemp_HB_flag = 0;}
                }

                if(log_cancelled == 0) {
                        if(glog_HB_flag == 0) printf("NO HB from Log Task\n");
                        else {printf("l*"); glog_HB_flag = 0;}
                }
                fflush(stdout);

                read_bytes=read(0,&choice,sizeof(char));
                if(read_bytes == 1) {
                        printf("\nchoice:%c\n",choice);
                        switch(choice) {
                        case '1':
                                if(temp_cancelled == 0) {
                                        printf("sending close signal to temp task\n");
                                        gclose_temp = 0; temp_cancelled = 1;
                                }
                                break;
                        case '2':
                                if(light_cancelled == 0) {
                                        printf("sending close signal to light task\n");
                                        gclose_light = 0; light_cancelled = 1;
                                }
                                break;
                        case '3':
                                if(log_cancelled == 0) {
                                        printf("sending close signal to log task\n");
                                        gclose_log = 0; log_cancelled = 1;
                                }
                                break;
                        case '4':
                                if(socket_cancelled == 0) {
                                        printf("sending close signal to socket task\n");
                                        gclose_socket = 0; pthread_kill(socket,SIGCONT);
                                        socket_cancelled = 1;
                                }
                                break;

                        case '5':
                                printf("Closing application\n");
                                gclose_temp = 0; gclose_light = 0;  gclose_log = 0;
                                gclose_socket = 0; pthread_kill(socket,SIGCONT);
                                //pthread_cancel(temp); pthread_cancel(light);
                                //pthread_cancel(log);
                                gclose_app = 0;
                                break;
                        }
                        read_bytes = 0;
                }

        }
        pthread_join(temp, NULL);
        pthread_join(light, NULL);
        pthread_join(log, NULL);
        printf("Joined all threads\n");
        mq_close(notify_msgq);

/*********destroy message Ques***********************/
        mq_unlink(IPC_TEMP_MQ);
        mq_unlink(IPC_LIGHT_MQ);
        mq_unlink(LOGGER_MQ);
        mq_unlink(NOTIFY_MQ);
        free(msg_pack);

        printf("Destroyed all opened Msg Ques\n");

        printf("***************Exiting***************\n");
        return 0;
}
