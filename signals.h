
#ifndef SIGNALS_H
#define SIGNALS_H

#define SIGTEMP  (SIGRTMIN)
#define SIGLIGHT (SIGRTMIN+1)
#define SIGLIGHT_HB (SIGRTMIN+2)

sig_atomic_t gclose_light;
sig_atomic_t gclose_log;
sig_atomic_t gclose_temp;
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
