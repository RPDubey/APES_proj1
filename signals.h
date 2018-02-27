
#ifndef SIGNALS_H
#define SIGNALS_H

/**
*@brief:Signal handler for
*Wakes up periodically to read light data from light sensor via I2c,sends data *to Logger, sends HB to main and handles IPC socket requests
*@param:pointer to thread info structure
*@return: returns NULL pointer
*/
void thread1_sig_handler(int sig);

#endif
