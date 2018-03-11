#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "includes.h"
#include "threads.h"
#include "signals.h"
#include "messageQue.h"
#include <mqueue.h>
#include "errorhandling.h"


void handle_err(char* arg_msg,mqd_t msgq_err,mqd_t msgq){
//log on error msg q
        char msg[BUF_SIZE];
        sprintf(msg,"%s:%s",arg_msg,strerror(errno));
        struct timespec now,expire;
        int num_bytes;
//log on logegr q
        clock_gettime(CLOCK_MONOTONIC,&now);
        expire.tv_sec = now.tv_sec+2;
        expire.tv_nsec = now.tv_nsec;
        num_bytes = mq_timedsend(msgq_err,
                                 msg,
                                 sizeof(msg),
                                 MSG_PRIO_ERR,
                                 &expire);
        if(num_bytes<0) {perror("mq_send to error Q in handle_err");}

        time_t t = time(NULL); struct tm* tm = localtime(&t);
        log_pack err_log ={.log_level=2,.log_source = error_handler};
        strcpy(err_log.time_stamp, asctime(tm));
        strcpy(err_log.log_msg, "Test Error:");

        clock_gettime(CLOCK_MONOTONIC,&now);
        expire.tv_sec = now.tv_sec+5;
        expire.tv_nsec = now.tv_nsec;

        num_bytes = mq_timedsend(msgq,
                                 (const char*)&err_log,
                                 sizeof(log_pack),
                                 MSG_PRIO_ERR,
                                 &expire);
        if(num_bytes<0) {perror("mq_send to log q in handle_err");}


        return;
}


//this thread is created for mq_notify event on error message que
void errorFunction(union sigval sv){
//read and print the error
        if(sv.sival_ptr == NULL) {printf("errorFunction argument\n"); return;}
        mqd_t msgq_err = *((mqd_t*)sv.sival_ptr);
        struct timespec now,expire;
        clock_gettime(CLOCK_MONOTONIC,&now);
        expire.tv_sec = now.tv_sec+2;
        expire.tv_nsec = now.tv_nsec;
        int num_bytes;
//empty the msgq - notification only for an empty q
        do {
                num_bytes = mq_timedreceive(msgq_err,
                                            err_msg,
                                            BUF_SIZE,
                                            NULL,
                                            &expire);
                if(num_bytes>0) {printf("*%s\n",err_msg);}

        } while(num_bytes>0);
//reregister for notification
        int ret  = mq_notify(msgq_err,&sig_ev_err);
        if(ret == -1) {perror("mq_notify-errorFunction"); return;}

//change led status???????????????????????????
//left to do
        return;
}
