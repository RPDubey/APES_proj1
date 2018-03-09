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
                printf("Light HB\n");
                glight_HB_flag = 1;
        }
}

void TempHBhandler(int sig){
        if(sig == SIGTEMP_HB)
        {
                printf("Temp HB\n");
                gtemp_HB_flag = 1;
        }
}

void LogHBhandler(int sig){
        if(sig == SIGLOG_HB)
        {
                printf("LOG HB\n");
                glog_HB_flag = 1;
        }
}

int main()
{
        printf("Entering Main\n");
        gclose_app = 1; gclose_light = 1; gclose_temp = 1; gclose_log = 1;
        gtemp_HB_flag = 0; glight_HB_flag = 0;
//install SIGINT handler to close application
        signal(SIGINT,SIGINT_handler);
        int ret;
        pthread_t temp,light,log;
        threadInfo temp_info; temp_info.thread_id = 1; temp_info.main=pthread_self();
        threadInfo light_info; light_info.thread_id = 2; light_info.main=pthread_self();
        threadInfo log_info; log_info.thread_id = 3; log_info.main=pthread_self();

/*SIGTEMP should be received by thread1 only. so blocking it for main thread*/
// sigset_t mask; //set of signals
// sigemptyset(&mask); sigaddset(&mask,SIGTEMP); sigaddset(&mask,SIGLIGHT);
// ret = pthread_sigmask(
//         SIG_SETMASK, //block the signals in the set argument
//         &mask, //set argument has list of blocked signals
//         NULL); //if non NULL prev val of signal mask stored here
// if(ret == -1) { printf("Error:%s\n",strerror(errno)); return -1; }

//Register Light HB signal
        struct sigaction action;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        action.sa_handler = LightHBhandler;
        ret = sigaction(SIGLIGHT_HB,&action,NULL);
        if(ret == -1) { perror("sigaction main"); return -1; }

//Register Temp HB signal
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
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

        while (gclose_app) {
//check HB signals
                SLEEP(10);
                printf("HB check\n");

                if(gtemp_HB_flag == 0) printf("send close signal to temp task\n");
                else if(gtemp_HB_flag == 1) { gtemp_HB_flag = 0;}

                if(glight_HB_flag == 0) printf("send close signal to light task\n");
                else if(glight_flag == 1) { glight_HB_flag = 0;}

                if(glog_HB_flag == 0) printf("send close signal to log task\n");
                else if(glog_HB_flag == 1) { glog_HB_flag = 0;}

        }
        pthread_join(temp, NULL);
        pthread_join(light, NULL);
        pthread_join(log, NULL);
        printf("Exiting Main\n");
        return 0;
}
