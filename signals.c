#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "signals.h"

#include "threads.h"

void SIGINT_handler(int sig){
        if(sig == SIGINT) {
                gclose_app   = 0;
                printf("\ncleared gclose flag\n");
        }
}


void temp_sig_handler(int sig){
        printf("caught temp signal %d\n",sig);
        pthread_mutex_lock(&gtemp_mutex);
        gtemp_flag = 1;
        pthread_cond_signal(&gtemp_condition);
        pthread_mutex_unlock(&gtemp_mutex);

}


void light_sig_handler(int sig){
        printf("caught light signal %d\n",sig);
        pthread_mutex_lock(&glight_mutex);
        glight_flag = 1;
        pthread_cond_signal(&glight_condition);
        pthread_mutex_unlock(&glight_mutex);

}
