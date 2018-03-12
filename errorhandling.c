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

static pthread_mutex_t error_msg_lock = PTHREAD_MUTEX_INITIALIZER;

void handle_err(char* arg_msg,mqd_t msgq_err,mqd_t msgq,msg_type type){
        char msg[BUF_SIZE];
        sprintf(msg,"%s:%s",arg_msg,strerror(errno));

        err_msg_pack err_pack={.type=type};
        strcpy(err_pack.msg, msg);
        struct timespec now,expire;
        int num_bytes;
        clock_gettime(CLOCK_MONOTONIC,&now);
        expire.tv_sec = now.tv_sec+2;
        expire.tv_nsec = now.tv_nsec;
        int prio;
        if (type==init) prio = MSG_PRIO_INIT;
        else if (type==error) prio = MSG_PRIO_ERR;

//log on error msg q
        num_bytes = mq_timedsend(msgq_err,
                                 (const char*)&err_pack,
                                 sizeof(err_pack),
                                 prio,
                                 &expire);
        if(num_bytes<0) {perror("mq_send to error Q in handle_err");}

        time_t t = time(NULL); struct tm* tm = localtime(&t);
        log_pack err_log ={.log_level=2,.log_source = error_handler};
        strcpy(err_log.time_stamp, asctime(tm));
        strcpy(err_log.log_msg, msg);

//log on logegr q
        clock_gettime(CLOCK_MONOTONIC,&now);
        expire.tv_sec = now.tv_sec+5;
        expire.tv_nsec = now.tv_nsec;

        num_bytes = mq_timedsend(msgq,
                                 (const char*)&err_log,
                                 sizeof(log_pack),
                                 prio,
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

        int ret =  pthread_mutex_lock(&error_msg_lock);
        if(ret !=0 ) {perror("errorFunction-mutexlock"); return;}
//empty the msgq - notification only for an empty q
        do {
                num_bytes = mq_timedreceive(msgq_err,
                                            (char*)msg_pack,
                                            BUF_SIZE,
                                            NULL,
                                            &expire);
                if(num_bytes>0) {
                        printf("#%s",( (err_msg_pack*)msg_pack)->msg);
                        bzero(msg_pack,sizeof(msg_pack));
                }

        } while(num_bytes>0);
        ret =  pthread_mutex_unlock(&error_msg_lock);
        if(ret !=0 ) {perror("errorFunction-mutex unlock"); return;}

//reregister for notification
        ret  = mq_notify(msgq_err,&sig_ev_err);
        if(ret == -1) {perror("mq_notify-errorFunction"); return;}
//what are we supposed to do if initializatio complete
//change led status???????????????????????????
//left to do based on err_pack type
        return;
}
