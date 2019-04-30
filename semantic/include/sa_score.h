/*
 * sa_score.h
 *
 *  Created on: 2016年6月22日
 *      Author: 云分身
 */

#ifndef INCLUDE_SA_SCORE_H_
#define INCLUDE_SA_SCORE_H_

#include "semantic.h"
#include "semantic_analyzer.h"

struct custom_info {
    char level;
    char info[512];
    custom_info_t c_info;
    char record[13][64];
};

extern struct custom_info custom;


/* ************************* *
 * function: set sql db path *
 * return:   0,  success,    *
 *           -1, error       *
 * ************************* */
int set_db_path(char *path);

//int count_score(int value, float weight);
int count_score(int total, int value, float weight);

int sa_info_init(void);

int sa_score_process(event_t *event);

void set_customer_level(char level, char *info);

int get_customer_level(char *level);

int set_hangup_status(int status);

int set_custom_info(custom_info_t custom_info);

int set_log_path(char *path);

int get_hangup_status(int *status);

int get_custom_record(struct custom_info *custom);

int get_hangup_desc(char *desc);

int get_score_threshold(event_t *event);

int get_init_arg(event_t *event);

int get_ef_level_item(event_t *event);

int word_to_filename(char (*filename)[FILENAME_SIZE], int *sum, char *word);

#endif /* INCLUDE_SA_SCORE_H_ */
