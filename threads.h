
#ifndef THREAH_H
#define THREAD_H

#include <signal.h>
#include <unistd.h>

#define DEFAULT_THREAD_ATTR ((void *)0)

pthread_mutex_t gtemp_mutex ;//= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t gtemp_condition;
sig_atomic_t gtemp_flag;

pthread_mutex_t glight_mutex ;//= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t glight_condition;
sig_atomic_t glight_flag;

/**
*structure to pass arguments and data to thread function
*/
typedef struct
{
        int thread_id;
        char* plog_file;
        pthread_t main;
} threadInfo;

/**
*@brief:Implements Light Task
*Wakes up periodically to read light data from light sensor via I2c,sends data *to Logger, sends HB to main and handles IPC socket requests
*@param:pointer to thread info structure
*@return: returns NULL pointer
*/
void *lightTask(void *pthread_inf);

/**
*@brief:Implements Temp Task
*Wakes up periodically to read temp data from light sensor via I2c,sends data *to Logger, sends HB to main and handles IPC socket requests
*@param:pointer to thread info structure
*@return: returns NULL pointer
*/
void *tempTask(void *pthread_inf);

/**
*@brief:Implements log Task
*Requests and receives logs from other tasks and writes them synchronously on file
*@param:pointer to thread info structure
*@return: returns NULL pointer
*/
void *logTask(void *pthread_inf);



/**
*@brief:Implements socketTask
*Receives socket requests,based on the request probes the light or temp sensor for data *and sends it back to the requestee.
*@param:pointer to thread info structure
*@return: returns NULL pointer
*/
void *socketTask(void *pthread_inf);

#endif
