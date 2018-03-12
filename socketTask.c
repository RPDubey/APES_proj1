#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "threads.h"
#include "signals.h"
#include "messageQue.h"
#include <mqueue.h>
#include "includes.h"
#include "errorhandling.h"

void *socketTask(void *pthread_inf) {
        int ret;
        threadInfo *ppthread_info = (threadInfo *)pthread_inf;

/*************Sockets*****************************/
//user defined data structures for data read and write
        sock_req* request = (sock_req*)malloc(sizeof(sock_req));
        if(request==NULL) {printf("malloc Error: %s\n", strerror(errno)); return NULL;}

        log_pack* response = (log_pack*)malloc(sizeof(log_pack));
        if(response==NULL) {printf("malloc Error: %s\n", strerror(errno)); return NULL;}

        int sockfd;          // listening FD
        int newsockfd;           // Client connected FD
        int num_char;            //No. of characters red/written
        struct sockaddr_in server_addr; //structure containing internet addresss.

        int opt = 1;
/****Create a new socket*******/
        sockfd = socket(
                AF_INET,         // com domain - IPv4
                SOCK_STREAM,         //com type - TCP
                0);         //protocol
        if(sockfd < 0) {printf("fork Error:%s\n",strerror(errno)); return NULL;}

/*****set options for the socket***********/
        ret = setsockopt(sockfd,
                         SOL_SOCKET,            //Socket Level Protocol
                         SO_REUSEADDR|SO_REUSEPORT,
                         &opt,         //option is enabled
                         sizeof(opt) );

        if(ret==-1) {printf("setsockopt Error:%s\n",strerror(errno)); return NULL;}
/***initialize the address structure and bind socket ****/
        bzero((char*)&server_addr, sizeof(server_addr));         //sets all val to 0
        server_addr.sin_family=AF_INET;
        server_addr.sin_addr.s_addr=INADDR_ANY;
        server_addr.sin_port=htons(PORT);
        ret = bind(sockfd,
                   (struct sockaddr*)&server_addr,
                   sizeof(server_addr));
        if(ret == -1) {printf("bind Error:%s\n",strerror(errno)); return NULL;}

/**listen on socket for connections**/
        ret = listen(sockfd,5);
        if(ret == -1) {printf("listen Error:%s\n",strerror(errno)); return NULL;}

/****block until the client connects to the server and gets its address*****/
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);         //size of address of client

        sleep(1);//allow other threads to initialize
/*****************Mask SIGNALS********************/
        sigset_t mask; //set of signals
        sigemptyset(&mask);
        sigaddset(&mask,SIGTEMP); sigaddset(&mask,SIGTEMP_HB);
        sigaddset(&mask,SIGLIGHT_HB); sigaddset(&mask,SIGLIGHT);
        sigaddset(&mask,SIGLOG_HB);  sigaddset(&mask,SIGLOG);
        sigaddset(&mask,SIGTEMP_IPC);  sigaddset(&mask,SIGLIGHT_IPC);

        ret = pthread_sigmask(
                SIG_SETMASK, //block the signals in the set argument
                &mask, //set argument has list of blocked signals
                NULL); //if non NULL prev val of signal mask stored here
        if(ret == -1) { printf("Error:%s\n",strerror(errno)); return NULL; }



//keep doing this repeatedly
        while(gclose_socket & gclose_app) {

                newsockfd = accept(sockfd,
                                   (struct sockaddr*)&client_addr,
                                   (socklen_t*)&addrlen);
                if(newsockfd<0) {printf("accept Error:%s\n",strerror(errno)); return NULL;}

/*****beyond this, execution happens only after client is connected******/
//prepopulate static elements of response packet
                response->log_source = RemoteRequestSocket_Task;
                response->log_level = 3;

                /****read from the client and write to it*******/

                bzero(request,sizeof(sock_req));

                num_char = read(newsockfd,(char*)request,sizeof(sock_req));
                if(num_char<0) {printf("read Error:%s\n",strerror(errno)); break;}
//                if(num_char>0) printf("read request:%d\n",request->sensor);

//find the sensor requested to to probe and probe the sensor
                time_t t = time(NULL); struct tm *tm = localtime(&t);        strcpy(response->time_stamp, asctime(tm));
//collect data and plugin
                if(request->sensor==(sensor_type)temp) strcpy(response->log_msg, "TEMP");
                if(request->sensor==(sensor_type)light) strcpy(response->log_msg, "LIGHT");

//send the read data
                num_char = write(newsockfd,response,sizeof(log_pack));
                if(num_char<0) {printf("write Error:%s\n",strerror(errno)); break;}
                if(num_char>0) printf("Message sent to client\n");
                sleep(2);
        }
        printf("Exiting Socket Task\n");
        free(request);
        free(response);


        return NULL;
}
