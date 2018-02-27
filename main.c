#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "threads.h"

int main()
{
        printf("Entering Main\n");
        int ret;
        pthread_t light; threadInfo light_info; light_info.thread_id = 1;
        pthread_t temp; threadInfo temp_info; temp_info.thread_id = 2;

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

/*SIGRTMIN should be received by thread1 only. so blocking it for main thread*/
        sigset_t mask; //set of signals
        sigemptyset(&mask); sigaddset(&mask,SIGRTMIN);
        ret = pthread_sigmask(
                SIG_SETMASK,  //block the signals in the set argument
                &mask,   //set argument has list of blocked signals
                NULL);   //if non NULL prev val of signal mask stored here
        if(ret == -1) { printf("Error:%s\n",strerror(errno)); return -1; }


        pthread_join(temp, NULL);
        pthread_join(light, NULL);
        printf("Exiting Main\n");
        return 0;
}
