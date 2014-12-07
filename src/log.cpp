/*===========================================
*   Copyright (C) 2014 All rights reserved.
*   
*   company      : xiaomi
*   author       : work
*   email        : work@xiaomi.com
*   date         ：2014-07-26 16:52:04
*   description  ：
*
=============================================*/

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#include "log.h"

#define XM_LOG_DEFAULT_CHECK_INTERVAL 10
#define XM_LOG_CHECK_INIT_STATUS -10001

xm_log_t* g_log_handle = NULL;

static int xm_log_open(xm_log_t* log_handle)
{
    if (log_handle->fp) {
        fclose(g_log_handle->fp);
        log_handle->fp = NULL;
    }

    if (log_handle->fp_err) {
        fclose(g_log_handle->fp_err);
        log_handle->fp_err = NULL;
    }

    log_handle->fp = fopen(g_log_handle->normal_filename, "a+");
    if (log_handle->fp == NULL) {
        char cmd[MAX_PATH];
        snprintf(cmd, sizeof(cmd), "mkdir -p %s", log_handle->log_path);
        system(cmd);

        log_handle->fp = fopen(g_log_handle->normal_filename, "a+");
    }

    if (log_handle->fp == NULL) {
        return XM_LOG_OPEN_FILE_FAILED;
    }

    log_handle->fp_err = fopen(g_log_handle->warn_filename, "a+");
    if (log_handle->fp_err == NULL) {
        fclose(log_handle->fp);
        log_handle->fp = NULL;
        return XM_LOG_OPEN_FILE_FAILED;
    }

    return XM_LOG_SUCCESS;
}

int xm_log_init(const char* log_path, const char* log_name, 
                uint32_t log_level, uint32_t log_mode)
{
    if (log_path == NULL || log_name == NULL) {
        return XM_LOG_PARAM_ERROR;
    }

    if (g_log_handle) {
        xm_log_close();
    }

    g_log_handle = (xm_log_t*) malloc(sizeof(xm_log_t));
    if (g_log_handle == NULL) {
        return XM_LOG_MALLOC_FAILED;
    }

    memset(g_log_handle, 0, sizeof(xm_log_t));
    snprintf(g_log_handle->log_path, sizeof(g_log_handle->log_path),
            "%s", log_path);

    snprintf(g_log_handle->normal_filename, sizeof(g_log_handle->normal_filename),
            "%s/%s.log", log_path, log_name);

    snprintf(g_log_handle->warn_filename, sizeof(g_log_handle->warn_filename),
            "%s/%s.log.wf", log_path, log_name);

    g_log_handle->last_check_time = time(NULL);
    g_log_handle->last_check_hours = XM_LOG_CHECK_INIT_STATUS;
    g_log_handle->log_level = log_level;
    //g_log_handle->log_buf_size = XM_MAX_LOG_BUF;
    g_log_handle->log_mode = log_mode;
    g_log_handle->auto_backup = false;

    int ret = xm_log_open(g_log_handle);
    if (ret != XM_LOG_SUCCESS) {
        xm_log_close();
        return ret;
    }

    pthread_key_create (&g_log_handle->thread_key, NULL);

    //XM_LOG_NOTICE("init log succ");
    //XM_LOG_WARN("log_path:%s", log_path);
    //XM_LOG_WARN("log_name:%s", log_name);
    //XM_LOG_WARN("log_level:%u", log_level);

    pthread_mutex_init(&g_log_handle->_lock, NULL);

    return XM_LOG_SUCCESS;
}

void check_backup(struct tm tm_now)
{
    if (g_log_handle == NULL || g_log_handle->auto_backup == false) {
        return;
    }

    pthread_mutex_lock(&g_log_handle->_lock);
    if (g_log_handle->last_check_hours == XM_LOG_CHECK_INIT_STATUS) {
        g_log_handle->last_check_hours = tm_now.tm_hour;
        pthread_mutex_unlock(&g_log_handle->_lock);
        return;
    }

    if (g_log_handle->last_check_hours == tm_now.tm_hour) {
        pthread_mutex_unlock(&g_log_handle->_lock);
        return;
    }

    g_log_handle->last_check_hours = tm_now.tm_hour;
    pthread_mutex_unlock(&g_log_handle->_lock);

    char backup_normal[MAX_PATH];
    char backup_warn[MAX_PATH];

    snprintf(backup_normal, sizeof(backup_normal),
            "%s.%04d%02d%02d%02d", g_log_handle->normal_filename,
            tm_now.tm_year + 1900, tm_now.tm_mon + 1, 
            tm_now.tm_mday, tm_now.tm_hour);

    snprintf(backup_warn, sizeof(backup_warn),
            "%s.%04d%02d%02d%2d", g_log_handle->warn_filename,
            tm_now.tm_year + 1970, tm_now.tm_mon + 1, 
            tm_now.tm_mday, tm_now.tm_hour);

    rename(g_log_handle->normal_filename, backup_normal);
    rename(g_log_handle->warn_filename, backup_warn);

    xm_log_open(g_log_handle);
}

int xm_log(int log_level, const char *format,...)
{
    if (g_log_handle == NULL || format == NULL) {
        return XM_LOG_PARAM_ERROR;
    }

    /*
    if (log_level > g_log_handle->log_level) {
        return XM_LOG_SUCCESS;
    }*/

    char msg_buf[XM_MAX_LOG_BUF];

    va_list va;
    va_start(va, format);
    int ret = vsnprintf(msg_buf, XM_MAX_LOG_BUF, format, va);        
    va_end(va); 

    const char* log_level_str = "";
    const char* logid = "";
    FILE* fp = NULL;
    bool print_notice = false;

    switch(log_level) {
        case XM_LOG_LEVEL_DEBUG:
            log_level_str = "DEBUG:";
            fp = g_log_handle->fp;
            break;
        case XM_LOG_LEVEL_TRACE:
            log_level_str = "TRACE:";
            fp = g_log_handle->fp;
            break;
        case XM_LOG_LEVEL_NOTICE:
            print_notice = true;
            log_level_str = "NOTICE:";
            fp = g_log_handle->fp;
            break;
        case XM_LOG_LEVEL_WARN:
            log_level_str = "WARN:";
            fp = g_log_handle->fp_err;
            break;
        case XM_LOG_LEVEL_FATAL:
            log_level_str = "FATAL:";
            fp = g_log_handle->fp_err;
            break;
        default:
            fp = g_log_handle->fp_err;
            break;
    }

    if (fp == NULL) {
        int ret = xm_log_open(g_log_handle);
        if (ret != XM_LOG_SUCCESS) {
            return ret;
        }
    }

    const char* notice = "";
    xm_notice_t* notice_buf = NULL;
    notice_buf = (xm_notice_t*) pthread_getspecific(g_log_handle->thread_key);
    if (print_notice) {
        if (notice_buf) {
            int cur_pos = notice_buf->pos;
            if (cur_pos >= (int)(notice_buf->size)) {
                cur_pos = notice_buf->size -1;
            }
            if (cur_pos > 0) {
                notice_buf->buf[cur_pos] = '\0';
                notice = notice_buf->buf;
            } else {
                notice_buf->pos = 0;
                print_notice = false;
            }
        } else {
            print_notice = false;
        }
    }

    if (notice_buf) {
        logid = notice_buf->logid;
    }

    struct timeval now;
    gettimeofday(&now, NULL);
    struct tm tmp;

    time_t time_now = now.tv_sec;

    localtime_r(&time_now, &tmp);
    if (print_notice) {
        ret = fprintf(fp, "%-6s [%4d-%02d-%02d %02d:%02d:%02d %d][%d] [logid:%s] [%s] %s\n", 
            log_level_str,
            tmp.tm_year + 1900, tmp.tm_mon  + 1, 
            tmp.tm_mday,  tmp.tm_hour, 
            tmp.tm_min, tmp.tm_sec, (int)now.tv_usec, getpid(),  
            logid, notice, msg_buf);
    } else {
        ret = fprintf(fp, "%-6s [%4d-%02d-%02d %02d:%02d:%02d %d][%d] [logid:%s] %s\n", 
            log_level_str,
            tmp.tm_year + 1900, tmp.tm_mon  + 1, 
            tmp.tm_mday,  tmp.tm_hour, 
            tmp.tm_min, tmp.tm_sec, (int)now.tv_usec,  getpid(),  
            logid, msg_buf);
    }

    if (ret < 0) {
        xm_log_open(g_log_handle);
        fp = NULL;
    }

    if (notice_buf && log_level == XM_LOG_LEVEL_NOTICE) {
        notice_buf->buf[0] = '\0';
        notice_buf->pos = 0;
    }

    if (g_log_handle->log_mode == XM_LOG_MODE_FLUSH_NOW) {
        if (fp) {
            fflush(fp);
        }
    } else if (g_log_handle->log_mode == XM_LOG_MODE_FLUSH_SEC) {
        if (time_now - g_log_handle->last_flush_time >= 1) {
            g_log_handle->last_flush_time = time_now;
            if (g_log_handle->fp) {
                fflush(g_log_handle->fp);
            }

            if (g_log_handle->fp_err) {
                fflush(g_log_handle->fp_err);
            }
        }
    }

    if (time_now - g_log_handle->last_check_time > XM_LOG_DEFAULT_CHECK_INTERVAL) {
        g_log_handle->last_check_time = time_now;
        check_backup(tmp);
    }

    return XM_LOG_SUCCESS;
}

int xm_log_set_logid(const char* logid)
{
    if (logid == NULL||g_log_handle == NULL) {
        return XM_LOG_FAILED;
    }

    xm_notice_t* notice_buf;
    notice_buf = (xm_notice_t*) pthread_getspecific(g_log_handle->thread_key);
    if (notice_buf == NULL) {
        notice_buf = (xm_notice_t*)malloc(sizeof(xm_notice_t));
        if (notice_buf == NULL) {
            return XM_LOG_MALLOC_FAILED;
        }

        notice_buf->buf[0] = '\0';
        notice_buf->pos = 0;
        notice_buf->size = sizeof(notice_buf->buf);
        notice_buf->logid[0] = '\0';
        pthread_setspecific(g_log_handle->thread_key, notice_buf);
    }

    if (notice_buf == NULL) {
        return XM_LOG_MALLOC_FAILED;
    }

    snprintf(notice_buf->logid, sizeof(notice_buf->logid),"%s", logid);
    return XM_LOG_SUCCESS;
}

int xm_log_push_notice(const char* format, ...)
{
    if (g_log_handle == NULL) {
        return -1;
    }

    xm_notice_t* notice_buf;
    notice_buf = (xm_notice_t*) pthread_getspecific(g_log_handle->thread_key);
    if (notice_buf == NULL) {
        notice_buf = (xm_notice_t*)malloc(sizeof(xm_notice_t));
        if (notice_buf == NULL) {
            return XM_LOG_MALLOC_FAILED;
        }

        notice_buf->buf[0] = '\0';
        notice_buf->pos = 0;
        notice_buf->size = sizeof(notice_buf->buf);
        notice_buf->logid[0] = '\0';
        pthread_setspecific(g_log_handle->thread_key, notice_buf);
    }

    if (notice_buf == NULL) {
        return XM_LOG_MALLOC_FAILED;
    }

    char msg_buf[XM_MAX_LOG_BUF];

    va_list va;
    va_start(va, format);
    int len = vsnprintf(msg_buf, XM_MAX_LOG_BUF, format, va);        
    va_end(va); 

    if (len <= 0) {
        return XM_LOG_SYS_CALL_FAILED;
    }

    if (notice_buf->pos + len + 1 > notice_buf->size) {
        return XM_LOG_NOT_ENOUGH_SLOT;
    }

    int left = notice_buf->size - notice_buf->pos;
    char* pos = notice_buf->buf + notice_buf->pos;
    len = snprintf(pos, left, "%s ", msg_buf);
    if (len <= 0) {
        return XM_LOG_SYS_CALL_FAILED;
    }

    notice_buf->pos += len;
    return XM_LOG_SUCCESS;
}

int xm_log_close()
{
    if (g_log_handle == NULL) {
        return XM_LOG_SUCCESS;
    }

    pthread_mutex_destroy(&g_log_handle->_lock);
    if (g_log_handle->fp) {
        fclose(g_log_handle->fp);
        g_log_handle->fp = NULL;
    }

    if (g_log_handle->fp_err) {
        fclose(g_log_handle->fp_err);
        g_log_handle->fp_err = NULL;
    }

    free(g_log_handle);
    g_log_handle = NULL;

    return XM_LOG_SUCCESS;
}

int xm_log_set_log_level(int level)
{
    if (g_log_handle == NULL) {
        return XM_LOG_PARAM_ERROR;
    }

    g_log_handle->log_level = level;
    return XM_LOG_SUCCESS;
}

int xm_log_set_auto_backup(bool auto_backup)
{
    if (g_log_handle == NULL) {
        return XM_LOG_PARAM_ERROR;
    }

    g_log_handle->auto_backup = auto_backup;
    return XM_LOG_SUCCESS;
}
