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
#include "includes.h"


void *tempTask(void *pthread_inf) {
        int ret;
        threadInfo *ppthread_info = (threadInfo *)pthread_inf;

/******set periodic timer**************/
        ret = setTempTimer(); //sets up timer to periodically signal and wake this thread
        if(ret ==-1) return NULL;
        else printf("Periodic Timer set for Temperature Task\n");

        ret = pthread_mutex_init(&gtemp_mutex,NULL);
        if(ret == -1) { printf("Error:%s\n",strerror(errno)); return NULL; }

/*******Initialize Message Que*****************/
        mqd_t msgq;
        int msg_prio = 30;
        int num_bytes;
        char message[BUF_SIZE];
        struct mq_attr msgq_attr = {.mq_maxmsg = MQ_MAXMSG, //max # msg in queue
                                    .mq_msgsize = BUF_SIZE,//max size of msg in bytes
                                    .mq_flags = 0};

        msgq = mq_open(MY_MQ, //name
                       O_CREAT | O_RDWR,//flags. create a new if dosent already exist
                       S_IRWXU, //mode-read,write and execute permission
                       &msgq_attr); //attribute
        if(msgq < 0) {perror("mq_open-tempTask Error:"); return NULL;}
        else printf("Messgae Que Opened in tempTask\n");

/************Creating logpacket*******************/
        log_pack temp_log ={.log_level=1,.log_source = temperatue_Task};
        time_t t = time(NULL);

/****************Do this periodically*******************************/
        while(gclose_temp & gclose_app) {


                pthread_mutex_lock(&gtemp_mutex);
                while(gtemp_flag == 0) {
                        pthread_cond_wait(&gtemp_condition,&gtemp_mutex);
                }

                pthread_mutex_unlock(&gtemp_mutex);
                gtemp_flag = 0;
//collect temperatue

/************populate the log packet*********/
                struct tm *tm = localtime(&t);
                strcpy(temp_log.time_stamp, asctime(tm));
                strcpy(temp_log.log_msg, "tempTask");

//

/*******Log messages on Que*************/
                num_bytes = mq_send(msgq,
                                    (const char*)&temp_log,
                                    sizeof(log_pack),
                                    msg_prio);
                if(num_bytes<0) {perror("mq_sen-tempTask Error"); gclose_temp = 0;}

        }
        printf("exiting Temp task\n");
        //  timer_delete(timerid);
        mq_unlink(MY_MQ);
        mq_close(msgq);
        return NULL;
}
