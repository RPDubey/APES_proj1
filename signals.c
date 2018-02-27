#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "signals.h"

#include "threads.h"

void thread1_sig_handler(int sig){
        printf("caught signal %d\n",sig);
        pthread_mutex_lock(&gtemp_mutex);
        gtemp_flag = 1;
        pthread_cond_signal(&gtemp_condition);
        pthread_mutex_unlock(&gtemp_mutex);

}
