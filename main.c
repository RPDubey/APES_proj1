#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "signals.h"
#include "threads.h"


void LightHBhandler(int sig){
        if(sig == SIGLIGHT_HB)
                printf("caught HB\n");
}
int main()
{
        printf("Entering Main\n");
        int ret;
        pthread_t temp,light,log;
        threadInfo temp_info; temp_info.thread_id = 1; temp_info.main=pthread_self();
        threadInfo light_info; light_info.thread_id = 2; light_info.main=pthread_self();
        threadInfo log_info; log_info.thread_id = 3; log_info.main=pthread_self();

/*SIGTEMP should be received by thread1 only. so blocking it for main thread*/
        sigset_t mask; //set of signals
// sigemptyset(&mask); sigaddset(&mask,SIGTEMP);
// ret = pthread_sigmask(
//         SIG_SETMASK, //block the signals in the set argument
//         &mask, //set argument has list of blocked signals
//         NULL); //if non NULL prev val of signal mask stored here
// if(ret == -1) { printf("Error:%s\n",strerror(errno)); return -1; }
// /*SIGLIGHTshould be received by thread1 only. so blocking it for main thread*/
// sigemptyset(&mask); sigaddset(&mask,SIGLIGHT);
// ret = pthread_sigmask(
//         SIG_SETMASK,         //block the signals in the set argument
//         &mask,         //set argument has list of blocked signals
//         NULL);         //if non NULL prev val of signal mask stored here
// if(ret == -1) { printf("Error:%s\n",strerror(errno)); return -1; }

        struct sigaction action;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        action.sa_handler = LightHBhandler;
        ret = sigaction(SIGLIGHT_HB,&action,NULL);
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


        pthread_join(temp, NULL);
        pthread_join(light, NULL);
        pthread_join(log, NULL);
        printf("Exiting Main\n");
        return 0;
}
