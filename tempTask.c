#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "threads.h"
#include "signals.h"


void *tempTask(void *pthread_inf) {
        threadInfo *ppthread_info = (threadInfo *)pthread_inf;
        int ret;
        ret = setTempTimer(); //sets up timer to periodically signal and wake this thread
        if(ret ==-1) return NULL;
        else printf("Periodic Timer set for Temperature Task\n");

/****************Do this periodically*******************************/
        while(1) {


                pthread_mutex_lock(&gtemp_mutex);
                while(gtemp_flag == 0) {
                        pthread_cond_wait(&gtemp_condition,&gtemp_mutex);
                }

                pthread_mutex_unlock(&gtemp_mutex);
                gtemp_flag = 0;
        }

        return NULL;
}
