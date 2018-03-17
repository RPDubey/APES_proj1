/*******************************************************************************
   @Filename:client.c
   @Brief:
   @Author:Ravi Dubey
   @Date:3/14/2018
 ******************************************************************************/
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
#include <mqueue.h>
#include "../includes.h"

int main(){
        int ret;
        int sockfd;              // listening FD
        int newsockfd;             // Client connected FD
        int num_char;              //No. of characters red/written
        struct sockaddr_in server_addr; //structure containing internet addresss.
        printf("Enter 1 for temp in F, 2 for temp in C, 3 for Kelvin. 4 for DAY/NIGHT, 5 for lumens");
        int input;
        scanf("%d",&input);


        sock_req* request = (sock_req*)malloc(sizeof(sock_req));
        if(request==NULL) {printf("malloc Error: %s\n", strerror(errno)); return -1;}

        log_pack* response = (log_pack*)malloc(sizeof(log_pack));
        if(response==NULL) {printf("malloc Error: %s\n", strerror(errno)); return -1;}
/****create socket********/
        sockfd = socket(
                AF_INET,     // com domain - IPv4
                SOCK_STREAM,     //com type - TCP
                0);     //protocol
        if(sockfd < 0) {printf("socket Error:%s\n",strerror(errno)); return -1;}
/*****clear and initialize the server address structure*****/
        memset(&server_addr,'0',sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);

/*convert IP addr from text to binary*/
        ret = inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr);
        if(ret<0) {printf("inet_pton Error:%s\n",strerror(errno)); return -1;}

/****connect socket to the address specified in server_addr******/
        ret = connect(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr));
        if(ret<0) {printf("connect Error:%s\n",strerror(errno)); return -1;}

        switch(input) {
        case 1:
                request->sensor = temp;
                request->tunit = FARENHEIT;
                break;
        case 2:
                request->sensor = temp;
                request->tunit = CELCIUS;
                break;
        case 3:
                request->sensor = temp;
                request->tunit = KELVIN;
                break;
        case 4:
                request->sensor = light;
                request->lunit = DAY_NIGHT;
                break;
        case 5:
                request->sensor = light;
                request->lunit = LUMEN;
                break;

        default:
                request->sensor = temp;
                request->tunit = CELCIUS;

        }

/******write to socket******/

        num_char =  send(sockfd,request,sizeof(sock_req),0 );
        printf("message sent from child has %d bytes\n",num_char);

/*****read from socket*****/
        num_char = read(sockfd,(char*)response,sizeof(log_pack));

        printf("\nRead in Client\n");
        printf("time     :%s\n",((log_pack*)response)->time_stamp);
        printf("source   :%d\n",((log_pack*)response)->log_source);
        printf("message  :%s\n",((log_pack*)response)->log_msg);
        printf("log level:%d\n",((log_pack*)response)->log_level);

        printf("Exiting Client\n");
        free(request);
        free(response);
        return 0;
}
