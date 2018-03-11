#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "threads.h"
#include "messageQue.h"
#include <mqueue.h>
#include "includes.h"
#include "signals.h"
#include "errorhandling.h"

// void LogQNotifyhandler(int sig){
//         if(sig == SIGLOG)
//                 printf("received data\n");
//
// }

sig_atomic_t log_data_flag;

void *logTask(void *pthread_inf) {
        int ret;
//log_data_flag=0;
        threadInfo *ppthread_info = (threadInfo *)pthread_inf;

/*****************Mask SIGNALS********************/
        sigset_t mask; //set of signals
        sigemptyset(&mask);
        sigaddset(&mask,SIGTEMP); sigaddset(&mask,SIGTEMP_HB);
        sigaddset(&mask,SIGLIGHT); sigaddset(&mask,SIGLIGHT_HB);
        sigaddset(&mask,SIGLOG_HB);

//unblocking for test
//sigaddset(&mask,SIGTEMP_IPC); sigaddset(&mask,SIGLIGHT_IPC);

        ret = pthread_sigmask(
                SIG_SETMASK, //block the signals in the set argument
                &mask, //set argument has list of blocked signals
                NULL); //if non NULL prev val of signal mask stored here
        if(ret == -1) { printf("Error:%s\n",strerror(errno)); return NULL; }

/*******Initialize ERROR Message Que*****************/
        mqd_t msgq_err;
        int msg_prio_err = MSG_PRIO_ERR;
        int num_bytes_err;
        struct mq_attr msgq_attr_err = {.mq_maxmsg = MQ_MAXMSG, //max # msg in queue
                                        .mq_msgsize = BUF_SIZE,//max size of msg in bytes
                                        .mq_flags = 0};

        msgq_err = mq_open(MY_MQ_ERR, //name
                           O_CREAT | O_RDWR,//flags. create a new if dosent already exist
                           S_IRWXU, //mode-read,write and execute permission
                           &msgq_attr_err); //attribute
        if(msgq_err < 0) {perror("mq_open-error_mq-tempTask "); return NULL;}
        else printf("Messgae Que Opened in tempTask\n");



/*************create a logger message q****************/
        mqd_t msgq;
        int msg_prio;
        int num_bytes;
        //char message[BUF_SIZE];
        log_pack* log = (log_pack*)malloc(sizeof(log_pack));
        if(log == NULL) {perror("malloc-logTask Err"); return NULL;}
        struct mq_attr msgq_attr = {.mq_maxmsg = MQ_MAXMSG, //max # msg in queue
                                    .mq_msgsize = BUF_SIZE,//max size of msg in bytes
                                    .mq_flags = 0};

        msgq = mq_open(MY_MQ, //name
                       O_CREAT | O_RDWR,//flags. create a new if dosent already exist
                       S_IRWXU, //mode-read,write and execute permission
                       &msgq_attr); //attribute
        if(msgq < 0) {perror("mq_open-logTask Error:"); return NULL;}
        else printf("Messgae Que Opened in logTask\n");
//open a file to write to
        FILE* pfd = fopen("logfile.txt","w");
        if(pfd==NULL) {perror("fopen-logTask Error:"); return NULL;}
        else printf("log file opened in logTask\n");


/*******&&&&&&&&&&&&&& msg Q to test IPC transfer&&&&&&&&&&&&&&&&&&**********************/
        mqd_t IPCmsgqT, IPCmsgqL;
        int IPCmsg_prio = 20;
        int IPCnum_bytes;
        char IPCmessage[BUF_SIZE];

        struct mq_attr IPCmsgq_attr = {.mq_maxmsg = MQ_MAXMSG, //max # msg in queue
                                       .mq_msgsize = BUF_SIZE, //max size of msg in bytes
                                       .mq_flags = 0};
//temp
        IPCmsgqT = mq_open(IPC_TEMP_MQ, //name
                           O_CREAT | O_RDWR, //flags. create a new if dosent already exist
                           S_IRWXU, //mode-read,write and execute permission
                           &IPCmsgq_attr); //attribute
        if(IPCmsgqT < 0) {perror("mq_open-logTask Error:"); return NULL;}
        else printf("IPC temp Messgae Que Opened in logTask\n");
//light
        IPCmsgqL = mq_open(IPC_LIGHT_MQ, //name
                           O_CREAT | O_RDWR, //flags. create a new if dosent already exist
                           S_IRWXU, //mode-read,write and execute permission
                           &IPCmsgq_attr); //attribute
        if(IPCmsgqL < 0) {perror("mq_open-logTask Error:"); return NULL;}
        else printf("IPC light Messgae Que Opened in logTask\n");
        struct timespec now,expire;

/*******************Do this in LOOP************************************/
        while(gclose_log & gclose_app)
        {

                pthread_kill(ppthread_info->main,SIGLOG_HB);//send HB
//read from queue
                clock_gettime(CLOCK_MONOTONIC,&now);
                expire.tv_sec = now.tv_sec+5;
                expire.tv_nsec = now.tv_nsec;
                //  printf("hi\n");
                num_bytes=mq_timedreceive(msgq,
                                          (char*)log,
                                          BUF_SIZE,
                                          &msg_prio,
                                          &expire);
                //  sleep(1);
                if(num_bytes<0) {
                        handle_err("mq_rcv-Log Q-logTask Error",msgq_err,msgq);
                }
                //  else printf("hii\n");

//write to a log file
                fprintf(pfd,"%s  %d  %d  %s\n\n",
                        ((log_pack*)log)->time_stamp,((log_pack*)log)->log_level,
                        ((log_pack*)log)->log_source,((log_pack*)log)->log_msg );
                fflush(pfd);

/*****&&&&&&&&&&&&&&&&&&& printing data for IPC message Q test &&&&&&&&&&&&&&&&&*******/


                clock_gettime(CLOCK_MONOTONIC,&now);
                expire.tv_sec = now.tv_sec+1;
                expire.tv_nsec = now.tv_nsec;
                num_bytes = mq_timedreceive(IPCmsgqT,
                                            (char*)log,
                                            BUF_SIZE,
                                            &IPCmsg_prio,
                                            &expire);
                if(num_bytes>0) {printf("data read on IPC msg Q:%s\n",log->time_stamp);}

                clock_gettime(CLOCK_MONOTONIC,&now);
                expire.tv_sec = now.tv_sec+1;
                expire.tv_nsec = now.tv_nsec;
                num_bytes = mq_timedreceive(IPCmsgqL,
                                            (char*)log,
                                            BUF_SIZE,
                                            &IPCmsg_prio,
                                            &expire);
                if(num_bytes>0) {printf("data read on IPC msg Q:%s\n",log->time_stamp);}
                //      sleep(10);
        }
        printf("exiting Log task\n");
        fclose(pfd);
        mq_close(msgq);
        mq_close(msgq_err);
        mq_close(IPCmsgqT);
        mq_close(IPCmsgqL);

        free(log);
        return NULL;
}
