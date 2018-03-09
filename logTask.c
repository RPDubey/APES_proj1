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

void *logTask(void *pthread_inf) {

        threadInfo *ppthread_info = (threadInfo *)pthread_inf;
/*************create a message q****************/
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
        while(gclose_log & gclose_app)
        {
                pthread_kill(ppthread_info->main,SIGLOG_HB);//send HB
//read from queue
                num_bytes=mq_receive(msgq,
                                     (char*)log,
                                     BUF_SIZE,
                                     &msg_prio);
                if(num_bytes<0) {perror("mq_rcv-logTask Error"); gclose_log = 0; }
//write to a log file

                fprintf(pfd,"%s  %d  %d  %s\n\n",
                        ((log_pack*)log)->time_stamp,((log_pack*)log)->log_level,
                        ((log_pack*)log)->log_source,((log_pack*)log)->log_msg );
                fflush(pfd);

                //  printf("Written to Log file:%s\n",message);
        }
        printf("exiting Log task\n");
        fclose(pfd);
        mq_unlink(MY_MQ);
        mq_close(msgq);
        free(log);
        return NULL;
}
