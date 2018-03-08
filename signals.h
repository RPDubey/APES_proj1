
#ifndef SIGNALS_H
#define SIGNALS_H


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
