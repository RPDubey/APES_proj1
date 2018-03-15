/*******************************************************************************
   @Filename:signals.c
   @Brief:
   @Author:Ravi Dubey
   @Date:3/14/2018
 ******************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "signals.h"
#include <errno.h>
#include <string.h>
#include "threads.h"


void SIGINT_handler(int sig){
        if(sig == SIGINT) {
                gclose_app   = 0;
                printf("\ncleared gclose_app flag\n");
        }
}

#define FREQ_NSEC (1000000000)

void temp_sig_handler(int sig){
        //printf("caught temp signal %d\n",sig);
        pthread_mutex_lock(&gtemp_mutex);
        gtemp_flag = 1;
        pthread_cond_signal(&gtemp_condition);
        pthread_mutex_unlock(&gtemp_mutex);

}


void light_sig_handler(int sig){
        //    printf("caught light signal %d\n",sig);
        pthread_mutex_lock(&glight_mutex);
        glight_flag = 1;
        pthread_cond_signal(&glight_condition);
        pthread_mutex_unlock(&glight_mutex);

}

int setTempTimer(){
        int ret;
/****** creating RT signal SIGTEMP with SA_RESTART flag for which 1 parameter
 *** .sa_handler should be used for handler function(not .sa_sigaction)********/

        struct sigaction sig_act ={
                .sa_flags = SA_RESTART,                  //three arguments
                .sa_handler = temp_sig_handler        //one arg method
        };
//        sig_act.sa_mask = mask;
        ret = sigaction(SIGTEMP,&sig_act,NULL);
        if(ret == -1) { perror("sigaction setTempTimer"); return -1; }
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
        if(ret == -1) { perror("timer_create setTempTimer"); return -1; }

/******************* start the timer*******************/
        struct itimerspec its={
                .it_value.tv_sec=3,      //start after 3 seconds(initial value)
                .it_value.tv_nsec=0,
                .it_interval.tv_sec=FREQ_NSEC/1000000000,
                .it_interval.tv_nsec=FREQ_NSEC%1000000000
        };
        ret = timer_settime(timerid,
                            0,          //No flags
                            &its,       //timer specs
                            NULL        //if non null, old timer specs returned here
                            );
        if(ret == -1)  { perror("timer_settime setTempTimer"); return -1; }

        return 1;
}


int setLightTimer(){
        int ret;
/****** creating RT signal SIGRTMIN+1 with SA_RESTART flag for which 1 parameter
   .sa_handler should be used for handler function(not .sa_sigaction)********/

        struct sigaction sig_act ={
                .sa_flags = SA_RESTART,          //three arguments
                .sa_handler = light_sig_handler //one arg method
        };
        //  sig_act.sa_mask=mask;
        ret = sigaction(SIGLIGHT,&sig_act,NULL);
        if(ret == -1) { perror("sigaction setLightTimer"); return -1; }

/***********************Creating the timer*********************/

        timer_t timerid;
/*sigevent struct specifies how the caller should be notified on timer expiry*/
        struct sigevent sig_ev ={
                .sigev_notify=SIGEV_SIGNAL, //notify by signal in sigev_signo
                .sigev_signo = SIGLIGHT, //Notification Signal
                .sigev_value.sival_ptr=&timerid //data passed with notification
        };

        ret = timer_create(CLOCK_MONOTONIC,
                           &sig_ev, //signal notification on timer expiry
                           &timerid //function places the id of the timer here
                           );
        if(ret == -1) { perror("timer_create setLightTimer"); return -1; }

/******************* start the timer*******************/
        struct itimerspec its={
                .it_value.tv_sec=3, //start after 3 seconds(initial value)
                .it_value.tv_nsec=0,
                .it_interval.tv_sec=FREQ_NSEC/1000000000,
                .it_interval.tv_nsec=FREQ_NSEC%1000000000
        };
        ret = timer_settime(timerid,
                            0,  //No flags
                            &its, //timer specs
                            NULL //if non null, old timer specs returned here
                            );
        if(ret == -1)  { perror("timer_settime setLightTimer"); return -1; }
        return 1;
}
