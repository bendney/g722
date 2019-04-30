#pragma once
#include <stdio.h>
#include <string.h>

#include "semantic_analyzer.h"

//typedef enum {
//    evt_answer,
//    evt_ask,
//    evt_play,
//    evt_answer_timeout,
//    evt_silent_timeout,
//    evt_telphone,
//    evt_hang_up,
//    evt_send_mesg,
//    evt_invite,
//}semantic_event_t;
//
//typedef enum {
//    type_negtive,
//    type_sure,
//    type_neuter,
//}answer_type_t;
//
//typedef struct semantic_info {
//    semantic_event_t event;
//    char *word;
//    answer_type_t answer_type;
//}semantic_info_t;

int set_envent(event_t *semantic_info,int flag, sqlite3 *db, char *word);
int normative_word(char *result, char *src, sqlite3 *db);
int analy_word(char (*word)[WORD_SIZE], int *sum, char *src);

