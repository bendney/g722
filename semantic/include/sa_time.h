/*
 * sa_time.h
 *
 *  Created on: 2016年9月28日
 *      Author: suifeng
 */

#ifndef SA_TIME_H_
#define SA_TIME_H_

void timer_start(void);
void timer_stop(void);
void is_timeout(void);

/* * * * * * * * * * * * * * * * * *
 * struct timeval data compare
 * return:  1   t1 > t2
 *          0   t1 <= t2
 * * * * * * * * * * * * * * * * * */
int timeval_compare(struct timeval t1, struct timeval t2);

/* t1 - t2
 * return  n ms*/
int timeval_sub(struct timeval t1, struct timeval t2);
int is_phone_timeout(struct timeval phone_start, int time_all);


#endif /* SA_TIME_H_ */
