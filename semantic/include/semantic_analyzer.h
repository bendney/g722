/*
 * semantic_analyzer.h
 */

#ifndef _SEMANTIC_ANALYZER_H_
#define _SEMANTIC_ANALYZER_H_

#include "logger.h"
#include <sqlcipher/sqlite3.h>

#include "semantic.h"

#define CONTEXT_SIZE        64
#define CHILD_CONTEXT_MAX   5
#define WORDS_SIZE          512
#define WORDS_MAX_COUNT     12

#define HOME "./default_template"
#define DB_NAME "aibeta.db"

#define TIMEOUT_PRIORITY    1
#define NOTFIND_PRIORITY    2
#define GENERAL_PRIORITY    5
#define BREAK_PRIORITY      9

#define NETWORK_ABNORMAL_LOWER  400
#define Q_NOTFIND_MAX_FLOOR     10
#define SKIP_TIMES_LIMITED		30

#define IVR_MAX_TIMES	3
#define TTS_FORMAT	".mp3"

typedef enum {
    wait_answer,
    auto_trigger,
    decision,
}trigger_t;

typedef enum {
    ws_null,
    ws_ask,
    ws_special,
    ws_sure,
    ws_negtive,
    ws_nuner,
}words_type_t;

typedef enum {
    fr_null,
    fr_1wd_ignore,
    fr_qign,
    fr_qany,
    fr_question1,
    fr_question2,
    fr_special,
    fr_sure,
    fr_neuter,
    fr_negtive,
    fr_refuse,
    fr_busy,
    fr_again,
    fr_auto,
    fr_auto0,
    fr_any,
    fr_question_notfind,
    fr_other_notfind,
    fr_sil_timeout,
    fr_ans_timeout,
    fr_ans_timeout_purpose,
	fr_time_all,
	fr_time_ai2human,
	fr_skip_session_fail,
	fr_skip_session_success,
    fr_start,
	fr_skip,
}find_result_type_t;

typedef struct _words {
    char words[WORDS_SIZE];
    char next_context[CONTEXT_SIZE]; /*next context name*/
    char curt_context[CONTEXT_SIZE]; /*current context name*/
    trigger_t trigger_condition;
    char over_words[WORDS_SIZE];
    words_type_t ws_type;
}words_t;


typedef struct context {
    char c_name[CONTEXT_SIZE];
    int  c_level;
    char c_parent[CONTEXT_SIZE];
    int  c_child_nub;
    char c_child[CHILD_CONTEXT_MAX][CONTEXT_SIZE];
}context_object;

typedef struct correct {
	int (*init)(void);
	int (*correct)(char* db_path, char* src, char* result, float *distance);
	void (*correct_free)(void);
	short enable_flag;
}correct_opt;


typedef enum {
    evt_null,
    evt_pre_answer, /* customer answer */
    evt_pre_ask, /* customer ask */
    evt_ai_play,
    evt_ai_answer_timeout, /* customer answer timeout */
    evt_ai_silent_timeout,
    evt_ai_telphone,
    evt_ai_hang_up,
    evt_sa_send_mesg,
    evt_sa_invite,
    evt_sa_invite_middle,
    evt_sa_invite_high,
    evt_continue_run,
    evt_start_talk,
    evt_success,
    evt_before_call,
    evt_wait,
    evt_ignore,
	evt_ai_skip_session_fail,
	evt_ai_skip_session_success,
	evt_ai_183,
}semantic_event_t;

typedef enum {
    type_null,
    type_negtive,
    type_sure,
    type_neuter,
    type_question,
    type_inconvenient,
    type_again,
	type_refuse,
}answer_type_t;

typedef enum {
    ai_event,
    pre_event,
    sa_event,
}event_type_t;

typedef enum {
	s_null,
    s_before_call,
    s_start,
    s_running,
    s_hangup,
    s_wait,
    s_ivr,
	s_session,
} system_status_t;

typedef struct _word {
    char word[WORDS_MAX_COUNT][WORDS_SIZE];
    int word_nub;
    int cur_word;
} word_t;

typedef enum {
    ai_null,
    ai_play,
    ai_play_end,
} ai_status_t;

typedef enum {
    text_null,
    text_tips,
    text_words,
    text_break,
} text_type_t;

typedef struct wait_play_queue {
    int w_id;
    int w_priority;
    find_result_type_t w_fr_type;
} wait_play_queue_t;

typedef struct system_conut {
    int total;
    int normal;
    int question;
    int question_notfind;
    int one_word_ignore;
    int not_recognize;
    int time_ignore;
} system_conut_t;

typedef enum {
    recognize_nothing,
    recognize_normal,
    recognize_ignore,
    recognize_one_word_ignore,
    recognize_timeout,
    recognize_question_notfind,
    recognize_other_notfind,
} recognize_type_t;

typedef enum {
    result_ignore,
    result_wait,
    result_answer,
} resutl_type;

typedef struct {
    int score_a;
    int score_b;
    int score_c;
    int score_d;
    int score_p;
    int e_level_item[20];//add by pxj e level
    int f_level_item[20];//add by pxj f level
} score_threshold_t;

typedef struct semanticevent {
    semantic_event_t event; /* event */
    word_t word; /* after normal word */
    answer_type_t answer_type; /* answer type */
//  event_type_t event_type; /* event type */
    int question_value; /* question value */
    int total_score; /* total score */
    context_t context; /* context */
    system_status_t system_status; /*system status*/
//  int question_status; /*question status :hold*/
    sqlite3 *db; /* point to sql */
//    int question_flag;
    int answer_flag;
    semantic_event_t continue_event;
    int question_cnt;/* if > 2, clear question wait flag */
    int play_response_id; /*the play id or the last play id*/
    int last_id;
    int not_memery_id; /*not memory this id, such as tip*/
    int timeout_cnt; /*timeout count, if over threshold, we hang up */
    int timeout; /* help timeout count */
    int timeout_flag; /* timeout flag */
    ai_status_t ai_status; /*ai is play or not play*/
    int continue_run_flag;
    int notfind_times;
    int qustion_notfind_times;
    int noting_notfind_times;
    int reason;
    int hangup_status;
    char hangup_desc[YFS_HANGUP_DESC_LEN];
    int wait_play_id_queue;
    text_type_t text_type;
    int reponse_ignore;
    int cur_priority;
    struct timeval play_start_time;
    struct timeval play_end_time;
    wait_play_queue_t wait_queue;
    int wait_cnt;
    int tmp_effect_flag;
    struct timeval call_start;
    struct timeval call_end;
    int phone_time_all; /* max time(s) of one phone */
    system_conut_t system_conut; /* count system one phone's result */
    recognize_type_t recognize_type;
    score_threshold_t score_threshold;
    int notclean_waitqueue;
    int notfind_nub;
    int ansout_nub;
    int timeout_nub;
    int question_notfind_floor;
    int question_notfind_cur_floor;
    int question_notfind_nub[Q_NOTFIND_MAX_FLOOR];
    short time_ai2human;
    find_result_type_t fr_type;
    find_result_type_t prev_fr_type;
    char ai_version_of_db_depend[24];
    char db_name[64];
    short this_word_say_times;
    short purpose_flag; /* 语境曾经走到目标语境的标志  add by SuiFeng 2016.11.29 */
	short ivr_cnt; /*记录转接的次数*/
	short times_nfr_ignore; /* (ms) 刚说完然后不识别之间的忽略时间   */  /* add by suifeng 2017.09.15 */
    char record_file[FILENAME_SIZE];/* add by SuiFeng 2017.10.10 */
    char industry[36];
    char area[36];
    correct_opt correct;
    char corrected_word[WORDS_SIZE];
    float correct_distance;
} event_t;

extern event_t sa_info;

int set_context_name(context_t *context);

int get_time_all_answer(event_t *event);

int get_time_ai2human(event_t *event);

int semantic_analyzer(semantic_request_t *request, semantic_answer_t *response);

#endif /* _SEMANTIC_ANALYZER_H_ */
