/*===========================================
*   Copyright (C) 2014 All rights reserved.
*   
*   company      : xiaomi
*   author       : sherlockhua
*   email        : sherlockhua@xiaomi.com
*   date         ：2014-12-07 22:20:53
*   description  ：
*
=============================================*/
#ifndef _EPROXY_H_
#define _EPROXY_H_

#include "define.h"

typedef struct _eproxy_conf_t 
{
    char conf_file[EPROXY_MAX_PATH];
    int daemon;
    int thread_num;
    int listen_port;

    char log_path[EPROXY_MAX_PATH];
    char log_file[EPROXY_MAX_PATH];
    int log_level;
} eproxy_conf_t;





#endif //_EPROXY_H
