/*
 * sa_time.c
 *
 *  Created on: 2016年9月28日
 *      Author: suifeng
 */
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include "logger.h"

#include "semantic_analyzer.h"

static struct timeval g_tv0, g_tv1;
static int time_start_flag = 0;

void timer_start(void)
{
    time_start_flag = 1;
    gettimeofday(&g_tv0, NULL);
}

void timer_stop(void)
{
    time_start_flag = 0;
}

int is_timeout(void)
{
    if (time_start_flag == 0) {
        return 0;
    }

    gettimeofday(&g_tv1, NULL);
    if ((g_tv1.tv_sec - g_tv0.tv_sec) * 1000 + (g_tv1.tv_usec - g_tv0.tv_usec) / 1000 > 1) {
        return 1;
    }
    return 0;
}


/* * * * * * * * * * * * * * * * * *
 * struct timeval data compare
 * return:  1   t1 > t2
 *          0   t1 <= t2
 * * * * * * * * * * * * * * * * * */
int timeval_compare(struct timeval t1, struct timeval t2)
{
    if ((t1.tv_sec - t2.tv_sec) * 1000000 + (t1.tv_usec - t2.tv_usec) > 0) {
        return 1;
    }

    return 0;
}

/*return  n ms*/
int timeval_sub(struct timeval t1, struct timeval t2)
{
    return ((t1.tv_sec - t2.tv_sec) * 1000000 + t1.tv_usec - t2.tv_usec) / 1000;
}

int is_phone_timeout(struct timeval phone_start, int time_all)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    if (phone_start.tv_sec == 0) {
    	log_error("same abnormal! ignore it!");
    } else {
		if (timeval_sub(now, phone_start) > time_all * 1000) {
			log_info("time is all: now:%ld, start:%ld, sub:%ld",
					now.tv_sec, phone_start.tv_sec, now.tv_sec - phone_start.tv_sec);
			return 1;
		}
    }
    return 0;
}
