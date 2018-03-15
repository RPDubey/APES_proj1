/*******************************************************************************
   @Filename:signals.h
   @Brief:
   @Author:Ravi Dubey
   @Date:3/14/2018
 ******************************************************************************/
#ifndef SIGNALS_H
#define SIGNALS_H

#define SIGTEMP     (SIGRTMAX)
#define SIGLIGHT    (SIGRTMAX-1)

#define SIGLIGHT_HB (SIGRTMAX-2)
#define SIGTEMP_HB  (SIGRTMAX-3)
#define SIGLOG_HB  (SIGRTMAX-4)

#define SIGTEMP_IPC (SIGRTMAX-5)
#define SIGLIGHT_IPC (SIGRTMAX-6)

#define SIGLOG (SIGRTMAX-7)


sig_atomic_t gclose_light;
sig_atomic_t gclose_log;
sig_atomic_t gclose_temp;
sig_atomic_t gclose_socket;
sig_atomic_t gclose_app;

/**
*@brief:Signal handler for SIGINT
*clears the global flag atomically to allow tasks to exit from while(1) loop and close *ques, file descriptors etc before exiting
*@param:signal no.
*@return: no returns
*/
void SIGINT_handler(int sig);


/**
*@brief:sets timer and signal handler for timer notification for temp task
*@param:void
*@return: int success/failure
*/
int setTempTimer();

/**
*@brief:sets timer and signal handler for light task
*@param:void
*@return: int success/failure
*/
int setLightTimer();


/**
*@brief:Signal handler for temp task
*sets the global flag atomically and signals the temp task throught a condition *variable to read next data
*@param:signal no.
*@return: no returns
*/
void temp_sig_handler(int sig);

/**
*@brief:Signal handler for light task
*sets the global flag atomically and signals the light task throught a *condition variable to read next data
*@param:signal no.
*@return: no returns
*/
void light_sig_handler(int sig);


#endif
