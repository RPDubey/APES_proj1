/*******************************************************************************
   @Filename:errorHandling.h
   @Brief:
   @Author:Ravi Dubey
   @Date:3/14/2018
 ******************************************************************************/
#ifndef ERRORHANDLING_H
#define ERRORHANDLING_H

#include"includes.h"

err_msg_pack *msg_pack;

struct sigevent sig_ev_err;

/**
*@brief:
*this thread is created for mq_notify event on error message que
*@param:
*@return:
*/
void errorFunction(union sigval sv);

/**
*@brief:
*
*@param:
*@return:
*/
void handle_err(char* msg,mqd_t msgq_err,mqd_t msgq_log,msg_type type);

#endif
