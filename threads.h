
#ifndef THREAH_H
#define THREAD_H

#define DEFAULT_THREAD_ATTR ((void *)0)

pthread_mutex_t gtemp_mutex;
pthread_cond_t gtemp_condition;
sig_atomic_t gtemp_flag;

/**
*structure to pass arguments and data to thread function
*/
typedef struct
{
        int thread_id;
        char* plog_file;
        //  pthread_mutex_t* fp_mutex;
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

#endif
