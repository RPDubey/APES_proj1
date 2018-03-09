#include <stdio.h>
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
        printf("Entering Main\n");
        gclose_app = 1; gclose_light = 1; gclose_temp = 1; gclose_log = 1;
        gtemp_HB_flag = 0; glight_HB_flag = 0;
/***install SIGINT handler to close application*******/
        signal(SIGINT,SIGINT_handler);
/*******************Masking SIgnals***********************/
        // sigset_t mask; //set of signals
        // sigemptyset(&mask); sigaddset(&mask,SIGTEMP_IPC);
        // ret = pthread_sigmask(
        //         SIG_SETMASK, //block the signals in the set argument
        //         &mask, //set argument has list of blocked signals
        //         NULL); //if non NULL prev val of signal mask stored here
        // if(ret == -1) { printf("Error:%s\n",strerror(errno)); return -1; }

        int ret;
        pthread_t temp,light,log;
        threadInfo temp_info; temp_info.thread_id = 1; temp_info.main=pthread_self();
        threadInfo light_info; light_info.thread_id = 2; light_info.main=pthread_self();
        threadInfo log_info; log_info.thread_id = 3; log_info.main=pthread_self();

//Register Light HB signal
        struct sigaction action;
        sigemptyset(&action.sa_mask);
        action.sa_handler = LightHBhandler;
        ret = sigaction(SIGLIGHT_HB,&action,NULL);
        if(ret == -1) { perror("sigaction main"); return -1; }

//Register Temp HB signal
        sigemptyset(&action.sa_mask);
        action.sa_handler = TempHBhandler;
        ret = sigaction(SIGTEMP_HB,&action,NULL);
        if(ret == -1) { perror("sigaction main"); return -1; }

//Register Log HB signal
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        action.sa_handler = LogHBhandler;
        ret = sigaction(SIGLOG_HB,&action,NULL);
        if(ret == -1) { perror("sigaction main"); return -1; }



        ret = pthread_create(  &temp,
                               DEFAULT_THREAD_ATTR,
                               tempTask,
                               (void *)&(temp_info) );
        if (ret != 0) {  printf("Error:%s\n",strerror(errno)); return -1;}

        ret = pthread_create(  &light,
                               DEFAULT_THREAD_ATTR,
                               lightTask,
                               (void *)&(light_info) );
        if (ret != 0) {  printf("Error:%s\n",strerror(errno)); return -1;}

        ret = pthread_create(  &log,
                               DEFAULT_THREAD_ATTR,
                               logTask,
                               (void *)&(log_info) );
        if (ret != 0) {  printf("Error:%s\n",strerror(errno)); return -1;}
        uint8_t read_bytes; char choice;
        uint8_t light_cancelled=0; uint8_t temp_cancelled=0; uint8_t log_cancelled=0;
        while (gclose_app) {
                SLEEP(10);
//check HB signals

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
                        else {printf("l"); glog_HB_flag = 0;}
                }
                fflush(stdout);

                printf("\nEnter thread to close 1-temp; 2-light; 3-log; 4-application\n");

                read_bytes=read(0,&choice,sizeof(char));
                if(read_bytes == 1) {
                        printf("choice:%c\n",choice);
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
                                printf("Closing application\n");
                                pthread_cancel(temp); pthread_cancel(light);
                                pthread_cancel(log); gclose_app = 0;
                                break;
                        }
                        read_bytes = 0;
                }
        }
        pthread_join(temp, NULL);
        pthread_join(light, NULL);
        pthread_join(log, NULL);
        printf("Joined all threads\n");

/*********destroy message Ques***********************/
        mq_unlink(IPC_TEMP_MQ);
        mq_unlink(IPC_LIGHT_MQ);
        mq_unlink(MY_MQ);
        printf("Destroyed all opened Msg Ques\n");

        printf("***************Exiting Main***************\n");
        return 0;
}
