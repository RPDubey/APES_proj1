#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "threads.h"
#include "signals.h"

#define FREQ_NSEC (1000000000)

void *tempTask(void *pthread_inf) {

        threadInfo *ppthread_info = (threadInfo *)pthread_inf;

        int ret;
/****** creating RT signal SIGTEMP with SA_RESTART flag for which 1 parameter
 *** .sa_handler should be used for handler function(not .sa_sigaction)********/

        struct sigaction sig_act ={
                .sa_flags = SA_RESTART,                  //three arguments
                .sa_handler = temp_sig_handler        //one arg method
        };
        sigemptyset(&sig_act.sa_mask);
        ret = sigaction(SIGTEMP,&sig_act,NULL);
        if(ret == -1) { printf("Error:%s\n",strerror(errno)); return NULL; }

/************Blocking the signal temporarily while creating the timer*********/
        sigset_t mask;         //set of signals
        sigemptyset(&mask); sigaddset(&mask,SIGTEMP);
        ret = pthread_sigmask(
                SIG_SETMASK,      //block the signals in the set argument
                &mask,          //set argument has list of blocked signals
                NULL);          //if non NULL prev val of signal mask stored here
        if(ret == -1) { printf("Error:%s\n",strerror(errno)); return NULL; }


/***********************Creating the timer*********************/

        /*sigevent struct specifies how the caller should be notified on timer expiry*/
        timer_t timerid;
        struct sigevent sig_ev ={
                .sigev_notify=SIGEV_SIGNAL,      //notify by signal in sigev_signo
                .sigev_signo = SIGTEMP,      //Notification Signal
                .sigev_value.sival_ptr=&timerid      //data passed with notification
        };

        ret = timer_create(CLOCK_MONOTONIC,
                           &sig_ev,      //signal notification on timer expiry
                           &timerid      //function places the id of the timer here
                           );
        if(ret == -1) { printf("Error:%s\n",strerror(errno)); return NULL; }

/******************* start the timer*******************/
        struct itimerspec its={
                .it_value.tv_sec=2,      //start after 2 seconds(initial value)
                .it_value.tv_nsec=0,
                .it_interval.tv_sec=FREQ_NSEC/1000000000,
                .it_interval.tv_nsec=FREQ_NSEC%1000000000
        };
        ret = timer_settime(timerid,
                            0,          //No flags
                            &its,       //timer specs
                            NULL        //if non null, old timer specs returned here
                            );
        if(ret == -1) { printf("Error:%s\n",strerror(errno)); return NULL; }

/************Unblocking the signal after setting the timer***********/
        ret = pthread_sigmask(
                SIG_UNBLOCK,        //unblock the signals in the set argument
                &mask,         //set argument has list of signals being unblocked
                NULL);         //if non NULL prev val of signal mask stored here
        if(ret == -1) { printf("Error:%s\n",strerror(errno)); return NULL; }


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
