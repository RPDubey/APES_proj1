/*******************************************************************************
   @Filename:notification.h
   @Brief: declaration for notification functions
   @Author:Ravi Dubey
   @Date:3/14/2018
 ******************************************************************************/
#ifndef ERRORHANDLING_H
#define ERRORHANDLING_H

#include"includes.h"

notify_pack *msg_pack;

struct sigevent sig_ev_err;

/**
*@brief:
*this thread is created for mq_notify event on error message que
*@param:
*@return:
*/
void notifyRcvThread(union sigval sv);

/**
*@brief:
*
*@param:
*@return:
*/
void notify(char* msg,mqd_t notify_msgq,mqd_t msgq_log,msg_type type);

#endif
