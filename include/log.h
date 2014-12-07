/*===========================================
*   Copyright (C) 2014 All rights reserved.
*   
*   company      : xiaomi
*   author       : sherlockhua
*   email        : sherlockhua@xiaomi.com
*   date         ：2014-07-09 10:57:13
*   description  ：
*
=============================================*/
#ifndef _LOG_H_
#define _LOG_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <pthread.h>
#include <stdint.h>





#define XM_LOG_LEVEL_DEBUG         16   
#define XM_LOG_LEVEL_TRACE         8   
#define XM_LOG_LEVEL_NOTICE        4  
#define XM_LOG_LEVEL_WARN          2 
#define XM_LOG_LEVEL_FATAL         1 

/*立即刷盘*/
#define XM_LOG_MODE_FLUSH_NOW      1
/*每秒刷一次*/
#define XM_LOG_MODE_FLUSH_SEC      2
/*操作系统自动刷盘*/
#define XM_LOG_MODE_FLUSH_SYS      3


#define MAX_PATH 1024
#define XM_MAX_LOG_BUF 8192

typedef struct _xm_log_t
{
    FILE* fp;
    FILE* fp_err;
    int log_level;
    int log_size;

    char log_path[MAX_PATH];
    char log_name[MAX_PATH];

    int log_buf_size;
    pthread_key_t thread_key;
    /**/
    int log_mode;
    time_t last_flush_time;

    time_t last_check_time;
    int last_check_hours;

    char normal_filename[MAX_PATH];
    char warn_filename[MAX_PATH];

    bool auto_backup;
    pthread_mutex_t _lock;
} xm_log_t;

typedef struct _xm_notice_t
{
    char buf[XM_MAX_LOG_BUF];
    uint32_t pos;
    uint32_t size;
    char logid[16];
}xm_notice_t;

extern xm_log_t* g_log_handle;

#define XM_LOG_DEBUG(format, arg...) \
         if (g_log_handle && XM_LOG_LEVEL_DEBUG <= g_log_handle->log_level) { \
                xm_log(XM_LOG_LEVEL_DEBUG, format " %s:%d", ##arg, __FILE__, __LINE__); \
         } 

#define XM_LOG_WARN(format, arg...) \
         if (g_log_handle && XM_LOG_LEVEL_WARN <= g_log_handle->log_level) { \
                xm_log(XM_LOG_LEVEL_WARN, format " %s:%d", ##arg, __FILE__, __LINE__); \
         } 

#define XM_LOG_FATAL(format, arg...) \
         if (g_log_handle && XM_LOG_LEVEL_FATAL <= g_log_handle->log_level) { \
                xm_log(XM_LOG_LEVEL_FATAL, format " %s:%d", ##arg, __FILE__, __LINE__); \
         }

#define XM_LOG_NOTICE(format, arg...) \
         if (g_log_handle && XM_LOG_LEVEL_NOTICE <= g_log_handle->log_level) { \
                 xm_log(XM_LOG_LEVEL_NOTICE, format, ##arg); \
         } 

#define XM_LOG_PUSH_NOTICE(format, arg...) \
        xm_log_push_notice(format, ##arg)

#define XM_LOG_TRACE(format, arg...) \
         if (g_log_handle && XM_LOG_LEVEL_TRACE <= g_log_handle->log_level) { \
                 xm_log(XM_LOG_LEVEL_TRACE, format " %s:%d", ##arg, __FILE__, __LINE__); \
         } 

#define XM_LOG_SUCCESS              0
#define XM_LOG_PACK_FAILED         -1001
#define XM_LOG_CONNECT_FAILED      -1002
#define XM_LOG_PARAM_ERROR         -1003
#define XM_LOG_FAILED              -1004
#define XM_LOG_SYS_CALL_FAILED     -1005
#define XM_LOG_CONNECT_TIMEOUT     -1006
#define XM_LOG_NOT_ENOUGH_SLOT     -1007
#define XM_LOG_UNKNOWN_TRANSPORT   -1008
#define XM_LOG_INVALID_METHOD      -1009
#define XM_LOG_INVALID_PAYLOAD     -1010
#define XM_LOG_WRITE_FAILED        -1011
#define XM_LOG_SELECT_FAILED       -1012
#define XM_LOG_WRITE_TIMEOUT       -1013
#define XM_LOG_READ_TIMEOUT        -1014
#define XM_LOG_CLIENT_CLOSED       -1015
#define XM_LOG_READ_FAILED         -1016
#define XM_LOG_CONF_DUPLICATE      -1017
#define XM_LOG_CONF_NOT_EXIST      -1018
#define XM_LOG_CONF_SYNAX_ERROR    -1019
#define XM_LOG_NOT_HAVE_MACHINE    -1020
#define XM_LOG_CURL_CALL_FAILED    -1021
#define XM_LOG_OUT_OF_BOUND        -1022
#define XM_LOG_CURL_MULTI_ADD_FAILED -1023
#define XM_LOG_CURL_FAILED           -1024
#define XM_LOG_NOT_FOUND_BALANCE     -1025
#define XM_LOG_NOT_FOUND_PROTOCOL    -1026
#define XM_LOG_INVALID_INPUT_HINTS   -1027
#define XM_LOG_UNPACK_FAILED         -1028
#define XM_LOG_MALLOC_FAILED         -1029
#define XM_LOG_OPEN_FILE_FAILED      -1030
#define XM_LOG_FLOCK_FAILED          -1031

int xm_log_init(const char* log_path, const char* log_name, 
                uint32_t log_level, uint32_t log_mode=XM_LOG_MODE_FLUSH_NOW);

int xm_log(int log_type, const char *msg,...);

int xm_log_push_notice(const char* msg, ...);

int xm_log_set_logid(const char* logid);

int xm_log_set_log_level(int level);

int xm_log_set_auto_backup(bool auto_backup);

int xm_log_close();

#ifdef __cplusplus
}
#endif

#endif //_LOG_H
