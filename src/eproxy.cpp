/*===========================================
*   Copyright (C) 2014 All rights reserved.
*   
*   company      : xiaomi
*   author       : sherlockhua
*   email        : sherlockhua@xiaomi.com
*   date         ：2014-12-07 21:22:57
*   description  ：
*
=============================================*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ev.h>

#include "eproxy.h"
#include "log.h"

int initialize(eproxy_conf_t* conf)
{
    int ret = xm_log_init(conf->log_path, conf->log_file, conf->log_level);
    if (ret != EPROXY_SUCCESS) {
        printf("init log failed, ret:%d\n", ret);
        return EPROXY_FAILED;
    }

    XM_LOG_DEBUG("init log succ, path:%s file:%s level:%d", 
            conf->log_path, conf->log_file, conf->log_level);

    return EPROXY_SUCCESS;
}

int daemon()
{
    return EPROXY_SUCCESS;
}

int set_default_conf(eproxy_conf_t* conf)
{
    conf->daemon = 0;
    snprintf(conf->conf_file, sizeof(conf->conf_file), "./etc/eproxy.yaml");
    conf->thread_num = EPROXY_DEF_THREAD_NUM;
    conf->listen_port = EPROXY_DEF_LISTEN_PORT;

    snprintf(conf->log_path, sizeof(conf->log_path), "./logs");
    snprintf(conf->log_file, sizeof(conf->log_file), "eproxy");
    conf->log_level = EPROXY_DEF_LOG_LEVEL;

    return EPROXY_SUCCESS;
}

int main(int argc, char** argv)
{
    int ch;
    eproxy_conf_t conf;
    set_default_conf(&conf);

    while ((ch = getopt(argc, argv, "c:dt:")) != -1) {
        switch (ch) {
            case 'c':
                snprintf(conf.conf_file, sizeof(conf.conf_file), "%s", optarg);
                printf("config file is %s\n", conf.conf_file);
                break;
            case 'd':
                conf.daemon = 1;
                printf("deamon: true\n");
                break;
            case 't':
                conf.thread_num = atoi(optarg);
                printf("thread num:%d\n", conf.thread_num);
                break;
            case '?':
                printf("unknown option:%c\n", (char)optopt);
                break;

        }
    }

    if (conf.daemon) {
        daemon();
    }

    int ret = initialize(&conf);
    if (ret != EPROXY_SUCCESS) {
        printf("initialize failed, ret:%d", ret);
        exit(0);
    }

    return 0;
}

