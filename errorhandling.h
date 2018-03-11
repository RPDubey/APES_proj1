
#ifndef ERRORHANDLING_H
#define ERRORHANDLING_H

#include"includes.h"

err_msg_pack *msg_pack;

struct sigevent sig_ev_err;


//this thread is created for mq_notify event on error message que
void errorFunction(union sigval sv);

void handle_err(char* msg,mqd_t msgq_err,mqd_t msgq_log,msg_type type);

#endif
