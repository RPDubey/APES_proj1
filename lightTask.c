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
#define FREQ_NSEC (1000000000)

/**
 * Thread 1 function
 */

void *lightTask(void *pthread_inf) {
        int ret;
        threadInfo *ppthread_info = (threadInfo *)pthread_inf;
/********set periodic timer**********/
        ret = setLightTimer();
        if(ret ==-1) return NULL;
        else printf("Periodic Timer set for Light Task\n");

        ret = pthread_mutex_init(&glight_mutex,NULL);
        if(ret == -1) { printf("Error:%s\n",strerror(errno)); return NULL; }

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

/************Creating logpacket*******************/
        log_pack light_log ={.log_level=1,.log_source = light_Task};
        time_t t = time(NULL);

/****************Do this periodically*******************************/
        while(gclose_light & gclose_app) {


                pthread_mutex_lock(&glight_mutex);
                while(glight_flag == 0) {
                        pthread_cond_wait(&glight_condition,&glight_mutex);
                }

                pthread_mutex_unlock(&glight_mutex);
                glight_flag = 0;
/***********collect data*****************/

/************populate the log packet*********/
                struct tm *tm = localtime(&t);
                strcpy(light_log.time_stamp, asctime(tm));
                strcpy(light_log.log_msg, "lightTask");
/*******Log messages on Que*************/
                num_bytes = mq_send(msgq,
                                    (const char*)&light_log,
                                    sizeof(log_pack),
                                    msg_prio);
                if(num_bytes<0) {perror("mq_send-lightTask Err"); gclose_light=0;}

        }
        printf("exiting light task\n");
        //timer_delete(timerid);
        mq_unlink(MY_MQ);
        mq_close(msgq);
        return NULL;

}
