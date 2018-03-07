#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "threads.h"
#include "signals.h"
#include "messageQue.h"
#include <mqueue.h>

#define FREQ_NSEC (1000000000)

/**
 * Thread 1 function
 */

void *lightTask(void *pthread_inf) {

        threadInfo *ppthread_info = (threadInfo *)pthread_inf;

/*******Initialize Message Que*****************/
        mqd_t msgq;
        int msg_prio = MSG_PRIO;
        int num_bytes;
        char message[BUF_SIZE];
        struct mq_attr msgq_attr = {.mq_maxmsg = MQ_MAXMSG, //max # msg in queue
                                    .mq_msgsize = BUF_SIZE,//max size of msg in bytes
                                    .mq_flags = 0};

        msgq = mq_open(MY_MQ, //name
                       O_CREAT | O_RDWR,//flags. create a new if dosent already exist
                       S_IRWXU, //mode-read,write and execute permission
                       &msgq_attr); //attribute
        if(msgq < 0) {perror("mq_open-lightTask Error:"); return NULL;}
        else printf("Messgae Que Opened in lightTask\n");

        int ret;
        ret = pthread_mutex_init(&glight_mutex,NULL);
        if(ret == -1) { printf("Error:%s\n",strerror(errno)); return NULL; }
/****** creating RT signal SIGRTMIN+1 with SA_RESTART flag for which 1 parameter
   .sa_handler should be used for handler function(not .sa_sigaction)********/

        struct sigaction sig_act ={
                .sa_flags = SA_RESTART,          //three arguments
                .sa_handler = light_sig_handler //one arg method
        };
        sigemptyset(&sig_act.sa_mask);
        ret = sigaction(SIGLIGHT,&sig_act,NULL);
        if(ret == -1) { printf("Error:%s\n",strerror(errno)); return NULL; }

/************Blocking the signal temporarily while creating the timer*********/
        sigset_t mask; //set of signals
        sigemptyset(&mask); sigaddset(&mask,SIGLIGHT);
        ret = pthread_sigmask(
                SIG_SETMASK,//block the signals in the set argument
                &mask,  //set argument has list of blocked signals
                NULL);  //if non NULL prev val of signal mask stored here
        if(ret == -1) { printf("Error:%s\n",strerror(errno)); return NULL; }


/***********************Creating the timer*********************/

        timer_t timerid;
/*sigevent struct specifies how the caller should be notified on timer expiry*/
        struct sigevent sig_ev ={
                .sigev_notify=SIGEV_SIGNAL,//notify by signal in sigev_signo
                .sigev_signo = SIGLIGHT,//Notification Signal
                .sigev_value.sival_ptr=&timerid//data passed with notification
        };

        ret = timer_create(CLOCK_MONOTONIC,
                           &sig_ev,//signal notification on timer expiry
                           &timerid//function places the id of the timer here
                           );
        if(ret == -1) { printf("Error:%s\n",strerror(errno)); return NULL; }

/******************* start the timer*******************/
        struct itimerspec its={
                .it_value.tv_sec=2,//start after 2 seconds(initial value)
                .it_value.tv_nsec=0,
                .it_interval.tv_sec=FREQ_NSEC/1000000000,
                .it_interval.tv_nsec=FREQ_NSEC%1000000000
        };
        ret = timer_settime(timerid,
                            0,  //No flags
                            &its, //timer specs
                            NULL //if non null, old timer specs returned here
                            );
        if(ret == -1) { printf("Error:%s\n",strerror(errno)); return NULL; }

/************Unblocking the signal after setting the timer***********/
        ret = pthread_sigmask(
                SIG_UNBLOCK, //unblock the signals in the set argument
                &mask, //set argument has list of signals being unblocked
                NULL); //if non NULL prev val of signal mask stored here
        if(ret == -1) { printf("Error:%s\n",strerror(errno)); return NULL; }




/****************Do this periodically*******************************/
        while(1) {


                pthread_mutex_lock(&glight_mutex);
                while(glight_flag == 0) {
                        pthread_cond_wait(&glight_condition,&glight_mutex);
                }

                pthread_mutex_unlock(&glight_mutex);
                glight_flag = 0;
/*******Log messages on Que*************/
                num_bytes = mq_send(msgq,
                                    "lightTask",
                                    sizeof("lightTask"),
                                    msg_prio);
                if(num_bytes<0) {perror("mq_send-lightTask Error"); return NULL;}

        }
        mq_unlink(MY_MQ);
        return NULL;

}
