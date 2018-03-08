#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "threads.h"
#include "signals.h"

void *lightTask(void *pthread_inf) {
        threadInfo *ppthread_info = (threadInfo *)pthread_inf;
        int ret;
        ret = setLightTimer();
        if(ret ==-1) return NULL;
        else printf("Periodic Timer set for Light Task\n");

/****************Do this periodically*******************************/
        while(1) {


                pthread_mutex_lock(&glight_mutex);
                while(glight_flag == 0) {
                        pthread_cond_wait(&glight_condition,&glight_mutex);
                }

                pthread_mutex_unlock(&glight_mutex);
                glight_flag = 0;
        }

        return NULL;

}
