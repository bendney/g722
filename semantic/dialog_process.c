/*
 * semantic_analyzer.c
 *
 *  Created on: 2016楠烇拷6閺堬拷21閺冿拷
 *      Author: 娴滄垵鍨庨煬锟?
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "sa_score.h"
#include "sa_sql.h"
#include "sa_suport.h"
#include "word_to_pinyin.h"
#include "sa_api.h"
#include "sa_time.h"
#include "sa_correct.h"
#include "config_parser.h"
#include "common_type.h"
#include "semantic_analyzer.h"
#include "semantic_pretreatment.h"
#include "pinyin.h"

#define ERROR_ANSWER			-1
#define VALID_ANSWER			0
#define READ_ANSWER				1
#define CLOSE_ANSWER			2
#define CLEAN_ANSWER			3
#define KEEP_ANSWER				4
#define SKIP_OPERATOR_ANSWER	5
#define SKIP_IVR_ANSWER			6

//extern int yfs_is_ars_busy(void);
event_t sa_info;


static void update_system(event_t *event)
{
    switch (event->fr_type) {
    case fr_null:
        break;
    case fr_1wd_ignore:

        break;
    case fr_qign:

        break;
    case fr_qany:

        break;
    case fr_question1:
    case fr_question2:
        if (event->context.priority == BREAK_PRIORITY) {
            event->not_memery_id = 2;
        }
        break;
    case fr_special:

        break;
    case fr_sure:
        event->context.words_score ? 0 : (event->context.words_score = 15);
        break;
    case fr_neuter:
        event->context.words_score ? 0 : (event->context.words_score = 10);
        break;
    case fr_negtive:
        event->context.words_score ? 0 : (event->context.words_score = 5);
        break;
    case fr_refuse:

        break;
    case fr_busy:
        event->context.words_score ? 0 : (event->context.words_score = 10);
        break;
    case fr_again:
        event->context.words_score ? 0 : (event->context.words_score = 10);
        break;
    case fr_auto:
        event->not_memery_id = 1;
        break;
    case fr_auto0:

        break;
    case fr_any:

        break;
    case fr_question_notfind:
        /* 统一不记忆的ID add by suifeng2016.10.19 */
        /* all of this type is tips, not memory modify by suifeng2016.11.02 */
//        if (event->qustion_notfind_times == 0) {
        if (event->context.priority < GENERAL_PRIORITY - 1) {
            event->not_memery_id = 1;
        }

        event->qustion_notfind_times++;
        break;
    case fr_other_notfind:
        /* 统一不记忆的ID add by suifeng 2016.10.19 */
//        if (event->notfind_nub == 3 && event->noting_notfind_times != 2) {
        /* all of this type is tips, not memory modify by suifeng2016.11.02 */
        if (event->context.priority < GENERAL_PRIORITY - 1) {
            event->not_memery_id = 1;
        }

        event->noting_notfind_times++;
        break;
    case fr_sil_timeout:
        event->timeout_flag = 1;
//        event->timeout_cnt++;
        event->not_memery_id = 1;
        break;
    case fr_ans_timeout:
        event->timeout_flag = 1;
//        event->timeout_cnt++;
        event->not_memery_id = 1;
        break;
    case fr_ans_timeout_purpose:
        event->timeout_flag = 1;
        break;
    case fr_time_ai2human:
    	event->context.words_score = 20;
    	break;
    case fr_time_all:
    	event->context.words_score = 0;
    	break;
    case fr_skip_session_fail:
        if (event->system_status == s_session) {
        	event->system_status = s_running;
        }
    	break;
    case fr_skip_session_success:
    	event->system_status = s_session;
    	break;
	case fr_start:
        break;
    default:
        break;
    }
}

static int is_hello(void)
{
	return !strcmp(sa_info.context.cur_context, "1");
}

static int wildcard_replace(semantic_answer_t *response)
{
	char *pword = NULL;
	char *pos = NULL;
	char tmp[512];

	pos = strchr(response->context.record, '%');

	while (pos != NULL) {
	    log_debug("replace: %c", pos[1]);
		switch (pos[1]) {
		case 'n':/*name*/
		    log_debug("replace name");
			pword = strstr(response->word, "[%n]");
			if (pword == NULL) {
				log_error("db data format error!");
			    pos += 1;
			    break;
			} else {
				/* word replace */
				strcpy(tmp, pword + 4);
				strcpy(pword, custom.c_info.name);
				strcat(pword, tmp);

//				log_debug("word: %s", response->word);

				/* record replace */
//				log_debug("src record: %s", response->context.record);
//				log_debug("name record: %s", custom.record[0]);

				strcpy(tmp, pos + 2);
				strcpy(pos, custom.record[0]);
				strcat(pos, tmp);

//				log_debug("record: %s", response->context.record);
			}
			break;
		case '0':/*option 1*/
		    log_debug("replace option 0");
			pword = strstr(response->word, "[%0]");
			if (pword == NULL) {
				log_error("db data format error!");
			    pos += 1;
			    break;
			} else {
				/* word replace */
				strcpy(tmp, pword + 4);
				strcpy(pword, custom.c_info.option[0]);
				strcat(pword, tmp);

				/* record replace */
				strcpy(tmp, pos + 2);
				strcpy(pos, custom.record[0]);
				strcat(pos, tmp);
			}

			break;
		case '1':/*option 1*/
		    log_debug("replace option 1");
			pword = strstr(response->word, "[%1]");
			if (pword == NULL) {
				log_error("db data format error!");
			    pos += 1;
			    break;
			} else {
				/* word replace */
				strcpy(tmp, pword + 4);
				strcpy(pword, custom.c_info.option[1]);
				strcat(pword, tmp);

				/* record replace */
				strcpy(tmp, pos + 2);
				strcpy(pos, custom.record[1]);
				strcat(pos, tmp);
			}

			break;
		case '2':/*option 2*/
		    log_debug("replace option 2");
			pword = strstr(response->word, "[%2]");
			if (pword == NULL) {
				log_error("db data format error!");
			    pos += 1;
			    break;
			} else {
				/* word replace */
				strcpy(tmp, pword + 4);
				strcpy(pword, custom.c_info.option[2]);
				strcat(pword, tmp);

				/* record replace */
				strcpy(tmp, pos + 2);
				strcpy(pos, custom.record[2]);
				strcat(pos, tmp);
			}
			break;
		case '3':/*option 3*/
		    log_debug("replace option 3");
			pword = strstr(response->word, "[%3]");
			if (pword == NULL) {
				log_error("db data format error!");
			    pos += 1;
			    break;
			} else {
				/* word replace */
				strcpy(tmp, pword + 4);
				strcpy(pword, custom.c_info.option[3]);
				strcat(pword, tmp);

				/* record replace */
				strcpy(tmp, pos + 2);
				strcpy(pos, custom.record[3]);
				strcat(pos, tmp);

				log_debug("record: %s", response->context.record);
			}
			break;
		case '4':/*option 4*/
		    log_debug("replace option 4");
			pword = strstr(response->word, "[%4]");
			if (pword == NULL) {
				log_error("db data format error!");
			    pos += 1;
			    break;
			} else {
				/* word replace */
				strcpy(tmp, pword + 4);
				strcpy(pword, custom.c_info.option[4]);
				strcat(pword, tmp);

				/* record replace */
				strcpy(tmp, pos + 2);
				strcpy(pos, custom.record[4]);
				strcat(pos, tmp);
			}
			break;
		case '5':/*option 5*/
		    log_debug("replace option 5");
			pword = strstr(response->word, "[%5]");
			if (pword == NULL) {
				log_error("db data format error!");
			    pos += 1;
			    break;
			} else {
				/* word replace */
				strcpy(tmp, pword + 4);
				strcpy(pword, custom.c_info.option[5]);
				strcat(pword, tmp);

				/* record replace */
				strcpy(tmp, pos + 2);
				strcpy(pos, custom.record[5]);
				strcat(pos, tmp);
			}
			break;
		case '6':/*option 5*/
		    log_debug("replace option 6");
			pword = strstr(response->word, "[%6]");
			if (pword == NULL) {
				log_error("db data format error!");
			    pos += 1;
			    break;
			} else {
				/* word replace */
				strcpy(tmp, pword + 4);
				strcpy(pword, custom.c_info.option[6]);
				strcat(pword, tmp);

				/* record replace */
				strcpy(tmp, pos + 2);
				strcpy(pos, custom.record[6]);
				strcat(pos, tmp);
			}
			break;
		case '7':/*option 5*/
		    log_debug("replace option 7");
			pword = strstr(response->word, "[%7]");
			if (pword == NULL) {
				log_error("db data format error!");
			    pos += 1;
			    break;
			} else {
				/* word replace */
				strcpy(tmp, pword + 4);
				strcpy(pword, custom.c_info.option[7]);
				strcat(pword, tmp);

				/* record replace */
				strcpy(tmp, pos + 2);
				strcpy(pos, custom.record[7]);
				strcat(pos, tmp);
			}
			break;
		case '8':/*option 5*/
		    log_debug("replace option 8");
			pword = strstr(response->word, "[%8]");
			if (pword == NULL) {
				log_error("db data format error!");
			    pos += 1;
			    break;
			} else {
				/* word replace */
				strcpy(tmp, pword + 4);
				strcpy(pword, custom.c_info.option[8]);
				strcat(pword, tmp);

				/* record replace */
				strcpy(tmp, pos + 2);
				strcpy(pos, custom.record[8]);
				strcat(pos, tmp);
			}
			break;
		case '9':/*option 5*/
		    log_debug("replace option 9");
			pword = strstr(response->word, "[%9]");
			if (pword == NULL) {
				log_error("db data format error!");
			    pos += 1;
			    break;
			} else {
				/* word replace */
				strcpy(tmp, pword + 4);
				strcpy(pword, custom.c_info.option[9]);
				strcat(pword, tmp);

				/* record replace */
				strcpy(tmp, pos + 2);
				strcpy(pos, custom.record[9]);
				strcat(pos, tmp);
			}
			break;
		case 'a':/*option 5*/
		    log_debug("replace option 10");
			pword = strstr(response->word, "[%a]");
			if (pword == NULL) {
				log_error("db data format error!");
			    pos += 1;
			    break;
			} else {
				/* word replace */
				strcpy(tmp, pword + 4);
				strcpy(pword, custom.c_info.option[10]);
				strcat(pword, tmp);

				/* record replace */
				strcpy(tmp, pos + 2);
				strcpy(pos, custom.record[10]);
				strcat(pos, tmp);
			}
			break;
		case 'b':/*option 5*/
		    log_debug("replace option 11");
			pword = strstr(response->word, "[%b]");
			if (pword == NULL) {
				log_error("db data format error!");
			    pos += 1;
			    break;
			} else {
				/* word replace */
				strcpy(tmp, pword + 4);
				strcpy(pword, custom.c_info.option[11]);
				strcat(pword, tmp);

				/* record replace */
				strcpy(tmp, pos + 2);
				strcpy(pos, custom.record[11]);
				strcat(pos, tmp);
			}
			break;
		case 'c':/*option 5*/
		    log_debug("replace option 12");
			pword = strstr(response->word, "[%c]");
			if (pword == NULL) {
				log_error("db data format error!");
			    pos += 1;
			    break;
			} else {
				/* word replace */
				strcpy(tmp, pword + 4);
				strcpy(pword, custom.c_info.option[12]);
				strcat(pword, tmp);

				/* record replace */
				strcpy(tmp, pos + 2);
				strcpy(pos, custom.record[12]);
				strcat(pos, tmp);
			}
			break;
		default:
		    log_error("not suports data format!");
		    pos += 1;
		    break;
		}
		pos = strchr(pos, '%');
	}

	return 0;
}

char * get_cur_context(void)
{
    return sa_info.context.context;
}

static int set_end_status(void)
{
    log_info("Play end, set status");
    sa_info.ai_status = ai_play_end;

    /* 获取当前时间 */
    gettimeofday(&sa_info.play_end_time, NULL);

    //sa_info.text_type = text_null;

	//timer_start();

    return 0;
}

static void clean_event(event_t *event)
{
    if (event->continue_event != evt_null) {
        event->event = event->continue_event;
        event->continue_event = evt_null;
    } else {
        event->event = evt_null;
    }
}


static void clean_wait_queue(event_t *event)
{
    event->wait_queue.w_priority = 0;
    event->wait_queue.w_id = 0;
    event->wait_queue.w_fr_type = fr_null;
    event->notclean_waitqueue = 0;
}

static int get_anser_from_wait_queue(event_t *event)
{
    int ret = 0;

    if (event->wait_queue.w_id > 0) {

        log_info("get answer from wait queue");

        event->fr_type = event->wait_queue.w_fr_type;
        if (event->fr_type == fr_sil_timeout || event->fr_type == fr_ans_timeout || event->fr_type == fr_other_notfind) {
            ret = sql_get_public_by_id(event, event->wait_queue.w_id);
        } else {
            ret = sql_get_by_id(event, event->wait_queue.w_id);
        }
        clean_wait_queue(event);
    }

    return ret;
}


static int put_anser_in_wait_queue(event_t *event)
{
	int ret = -1;

	/* 将 play_response_id 改为 last_id， 解决记忆ID与当前ID重合时，不缓存的BUG  add by suifeng (2017.07.13) */
    if (event->context.response_id != event->last_id) {
        if (event->context.priority >= event->wait_queue.w_priority && event->context.priority > TIMEOUT_PRIORITY) {
			//由 TIMEOUT_PRIORITY + 1 更改为 TIMEOUT_PRIORITY （2016.10.10 suifeng） －－更改回去（2017.05.24 suifeng）
			//if (event->context.priority >= event->wait_queue.w_priority) {//更改为所有都缓存 （2016.11.03 suifeng）
            //如果缓存的为问题，当非问题过来时，不覆盖，直接丢弃非问题 （2017.05.24 suifeng）
            if (!((event->wait_queue.w_fr_type == fr_question1 || event->wait_queue.w_fr_type == fr_question2) &&
                    (event->fr_type != fr_question1 && event->fr_type != fr_question2))) {
	            log_info("put answer into wait queue");
	            event->wait_queue.w_priority = event->context.priority;
	            event->wait_queue.w_id = event->context.response_id;
	            event->wait_queue.w_fr_type = event->fr_type;
	            ret = 0;
			}
        }
    }

    return ret;
}


static int find_hello(event_t *event)
{
    strcpy(event->context.tmp_context, "nihao");
    event->context.context = event->context.tmp_context;
    return sql_get_response(event);
}


static int start_talk_handle(event_t *event)
{
    int ret = 0;

    ret = find_hello(event);
    if (ret != 1) {
        ret = sql_get_by_id(event, 1);
    }

    if (ret != 1) {
        log_error("SQL No Start Talk Data!");
    } else {
        sa_info.system_status = s_running;
        event->fr_type = fr_start;
    }

    return ret;
}


#if 0
static int get_en(event_t *event)
{
    strcpy(event->context.tmp_context, ".en");
    return sql_get_response_by_context_only(event, event->context.tmp_context);
}
#endif

static int format_request(event_t *event, semantic_request_t *request)
{
    memset(&event->word, 0, sizeof(event->word));

	switch (request->type) {
		case WITH_SPEECH:
			if (strcmp(event->context.cur_context, "success") == 0 ||
					strcmp(event->context.cur_context, "bye") == 0) {
				event->event = evt_success;
			} else {
				event->fr_type = fr_null;
				analy_word(event->word.word, &event->word.word_nub, request->word);
				strcpy(event->record_file, request->record_file);
				log_debug("cur_context: %s, status: %d", event->context.cur_context, event->system_status);

				if (event->system_status == s_before_call) {
					event->event = evt_before_call;
				} else if (event->system_status == s_start) {
					if (timeval_compare(request->start, event->call_start) == 0) {
						event->event = evt_ignore;
					} else {
						event->event = evt_start_talk;
					}
				} else {
					if (timeval_compare(request->start, event->call_start) == 0) {
						event->event = evt_ignore;
					} else if (event->context.next_context[0] == '!') {
						event->event = evt_continue_run;
					} else {
						event->event = evt_null;
					}
				}
			}
			break;
		case WITH_TIMEOUT:
			if (event->system_status == s_wait) {
				event->event = evt_wait;
			} else if (event->system_status == s_before_call) {
				event->event = evt_before_call;
			} else if (event->context.next_context[0] == '!') {
				event->event = evt_continue_run;
			} else {
				if (event->system_status == s_start) {
					event->event = evt_ai_silent_timeout;
				} else if (event->system_status == s_running){
					event->event = evt_ai_answer_timeout;
				}
			}
			break;
		case WITH_CONNECT:
			event->event = evt_ai_telphone;
			gettimeofday(&event->call_start, NULL);
			break;
		case WITH_SESSION:
			//    	event->system_status = s_session;
			event->event = evt_ai_skip_session_success;
			break;
		case WITH_SESSION_FAIL:
			event->event = evt_ai_skip_session_fail;
			break;
		case WITH_183:
			event->event = evt_ai_183;
			break;
		default:
			log_error("undefine type");
			break;
	}

	return 0;
}

static void format_response(event_t *event, semantic_request_t *request, semantic_answer_t *response)
{
    //int ret = 0;
    response->type = KEEP_ANSWER;

    if (event->reponse_ignore > 0) {
        return;
    }

    memset(response->word, 0, sizeof(response->word));

	if (is_hello()) {
		clean_wait_queue(event);
		response->type = CLEAN_ANSWER;
	}
	else if (event->ai_status == ai_play) {
    	if (event->context.priority >= BREAK_PRIORITY) {
            clean_wait_queue(event);
            if (event->text_type != text_break) {
                log_debug("play id: %d, current id: %d", event->play_response_id, event->context.response_id);
                log_debug("play priority: %d, current priority: %d", event->cur_priority, event->context.priority);
                log_info("high priority, answer!");
                response->type = CLEAN_ANSWER;
            } else {
                log_debug("play id: %d, current id: %d", event->play_response_id, event->context.response_id);
                log_debug("play priority: %d, current priority: %d", event->cur_priority, event->context.priority);
                log_info("same id, ignore!");
            }
    	} else /* 在播放的时候，过滤掉之前播放之前说的结果 */
    		if (event->text_type == text_words && (timeval_compare(request->start, event->play_start_time) == 0 ||
                timeval_sub(request->start, event->play_start_time) < 1500)) {
    		    log_info("is playing, ignore front the start word!\n");
    		    clean_wait_queue(event);
        } else {
            if (event->context.priority > event->cur_priority) {
                response->type = CLEAN_ANSWER;
            }
			else if (event->context.priority == event->cur_priority) {/* add break, modify by suifeng 2018.04.20 */
				if (event->context.priority >= GENERAL_PRIORITY &&
				                    (event->fr_type == fr_question1 ||
				                     event->fr_type == fr_question2 ||
				                     event->fr_type == fr_busy ||
				                     event->fr_type == fr_again ||
				                     event->fr_type == fr_special ||
									 event->fr_type == fr_skip || /* 修正跳转时打断不成功的BUG 2018.04.20 add by suifeng* */
				                     event->fr_type == fr_refuse)) {
					response->type = CLEAN_ANSWER;
				}
				else {
					put_anser_in_wait_queue(event);
				}
            } else {
                log_info("lower priority words, ignore!\n");
                clean_wait_queue(event);
            }
        }
    } else {
    	if (event->context.priority >= BREAK_PRIORITY) {
            clean_wait_queue(event);
    	    if (event->text_type == text_break && timeval_sub(request->start, event->play_end_time) < 500) {
                log_debug("play id: %d, current id: %d", event->play_response_id, event->context.response_id);
                log_debug("play priority: %d, current priority: %d", event->cur_priority, event->context.priority);
                log_info("break continue, ignore!");
            } else {
                log_debug("play id: %d, current id: %d", event->play_response_id, event->context.response_id);
                log_debug("play priority: %d, current priority: %d", event->cur_priority, event->context.priority);
                log_info("high priority, answer!");
                response->type = CLEAN_ANSWER;
            }
    	} else /* 在播放结束的时候，过滤掉结束之前说的结果  */
        if (sa_info.text_type == text_words && timeval_compare(request->end, event->play_end_time) == 0) {
        	/* 如果这个结果是AI说话之后开始说的，先缓存这个结果 */
            if ((event->fr_type == fr_question1 || event->fr_type == fr_question2 || event->fr_type == fr_sure) &&
            		event->context.priority >= GENERAL_PRIORITY && timeval_compare(request->start, event->play_start_time) == 1) {
            	put_anser_in_wait_queue(event);
            } else {
                log_info("play end, ignore front the end word!\n");
                clean_wait_queue(event);
            }
        } else {
            if (event->system_status == s_before_call ||
            		(event->fr_type == fr_time_all) || /* 一通话到最长时间，直接结束 add by suifeng2017.12.01 */
                    (event->context.response_id == 0 && event->fr_type != fr_sil_timeout) || /*modify by suifeng2016.11.10*/
                    event->context.response_id == 1 || event->context.response_id == 2) {
                log_info("start or before start, answer!");
            	response->type = READ_ANSWER;
            } else {
#if 0
                ret = yfs_is_ars_busy();
                if (ret > 0) {
                    event->wait_cnt++;
                    if (((event->context.priority >= GENERAL_PRIORITY - 1) && event->wait_cnt >= SEC_0_5) ||
                            (event->context.priority > TIMEOUT_PRIORITY && event->wait_cnt >= SEC_1_0) ||
                            (event->context.priority == TIMEOUT_PRIORITY && event->system_status == s_running && event->wait_cnt >= 2)
                            ) {/*modify by suifeng2017.04.20*/
                        log_info("[info] ars is busy! already waited some time, then answer!\n");
                        event->wait_cnt = 0;
                        if (event->context.priority < GENERAL_PRIORITY && event->wait_queue.w_id > 0) {
                        	get_anser_from_wait_queue(event);
                        }
						response->type = READ_ANSWER;
                    } else if (event->context.priority == TIMEOUT_PRIORITY && event->system_status == s_start) {/*add by suifeng2017.04.20*/
                        start_talk_handle(event);
                        response->type = READ_ANSWER;
                    } else {
                    	log_info("[info] ars is busy! busy status: %d, wait!\n", ret);
                    	put_anser_in_wait_queue(event);
                    }
                } else {
#endif
                    if (event->context.priority < GENERAL_PRIORITY && event->wait_queue.w_id > 0) {
                    	get_anser_from_wait_queue(event);
                    }
                    response->type = READ_ANSWER;
                //}
                /* add by suifeng 2017.05.25 */
                if (response->type == READ_ANSWER &&
                		event->prev_fr_type != fr_question_notfind &&
						event->prev_fr_type != fr_other_notfind &&
						event->prev_fr_type != fr_sil_timeout &&
						event->prev_fr_type != fr_ans_timeout &&
                        (event->fr_type == fr_question_notfind ||
                        event->fr_type == fr_other_notfind ||
                        event->fr_type == fr_sil_timeout ||
                        event->fr_type == fr_ans_timeout) &&
                        timeval_sub(request->end, event->play_end_time) < event->times_nfr_ignore) {/* modify by suifeng 2017.09.15 */
                	log_info("play just a minate, ignore!\n");
                    response->type = KEEP_ANSWER;
                }
            }
        }
    }

//    if (event->fr_type == fr_question1 == KEEP_ANSWER &&
//            (strcmp(event->context.cur_context, "success") == 0 || strcmp(event->context.cur_context, "bye") == 0)) {
//        if (event->wait_queue.w_id > 0) {
//            get_anser_from_wait_queue(event);
//        } else {
//            ret = sure_answer(event);
//        }
//        response->type = READ_ANSWER;
//    }

    event->prev_fr_type = event->fr_type;

    if (response->type != KEEP_ANSWER) {
        event->wait_cnt = 0;
        if (event->notclean_waitqueue == 0) {
            clean_wait_queue(event);
        } else {
            event->notclean_waitqueue = 0;
        }

        memcpy(response->word, event->word.word[0], sizeof(response->word));

        response->context = event->context;

        if (strcmp(event->context.next_context, "#") == 0 || event->fr_type == fr_time_all) {
            log_info("****** HANGUP *******\n");
            response->type = CLOSE_ANSWER;
            event->system_status = s_hangup;
        } else if (event->context.next_context[0] == '@') {
            log_info("****** SKIP_OPERATOR_ANSWER *******\n");
            response->type = SKIP_OPERATOR_ANSWER;
            strcpy(response->extension, event->context.next_context + 1);
			//event->system_status = s_hangup;
        } else if (event->context.next_context[0] == '&') {
            log_info("******  SKIP_IVR_ANSWER *******\n");
            response->type = SKIP_IVR_ANSWER;
            strcpy(response->extension, event->context.next_context + 1);
			//event->system_status = s_hangup;
        }
         else if (!strncmp(event->context.next_context,"#9#" ,3) ||
        		 event->fr_type == fr_time_ai2human) {
            log_info("****** SYBOML %s \n*******\n", event->context.next_context);
			
			if (!strncmp(event->context.next_context,"#9#" ,3)) {
				strcpy(response->context.next_context, event->context.next_context + 3);
				//printf("next context:%s\n", event->context.next_context);
			}

			if (event->fr_type == fr_time_ai2human) {
				event->ivr_cnt++;

			}

            response->type = SKIP_IVR_ANSWER;
			strcpy(response->extension, "9");
        }
        else if (event->fr_type == fr_skip_session_success) {
            response->type = SKIP_IVR_ANSWER;
 			strcpy(response->extension, "8");
        }
		

        if (sa_info.system_status == s_hangup && event->context.hangup_status != 0) {
            sa_info.hangup_status = event->context.hangup_status;
        }

        if (event->context.response_id == event->play_response_id) {
            /* say a words times more than 3 times, then continue next flow  */
            if (++event->this_word_say_times >= 3) {
                log_info("say a words times more than 3 times, then continue next flow");
                event->this_word_say_times = 0;
                strcpy(response->word, "0");
                strcpy(response->context.record, "0");
                strcpy(response->context.next_context, "!");
            }
        } else {
            event->this_word_say_times = 0;
        }
    }

    return;
}

static int samantic_init(sqlite3 **db , char *dbname, event_t * sa_info)
{
    //char cmd[128];
    /*remove log*/
	/*
    if (get_log_path() != NULL) {
       memset(cmd, 0, sizeof(cmd));
       sprintf(cmd, "rm -f %s/replace.log %s/pinyin.log ", get_log_path(), get_log_path());
       system(cmd);
    }
	*/
    int ret = 0;

    sa_info->last_id = -1;

    log_info("Initialise sqlite3 database: %s", dbname);
    ret = init_sqlite3_db(&sa_info->db, dbname);
    if (ret == -1) {
		log_info("Initialise sqlite3 database %s failed!", dbname);
        sa_info->db = NULL;
        return ret;
    }

    ret = get_score_threshold(sa_info);
    if (ret < 0) {
        return ret;
    }
    get_ef_level_item(sa_info);//add bby pxj 20180205
    ret = get_init_arg(sa_info);
    if (ret < 0) {
        return ret;
    }

    get_custom_record(&custom);
#if 0
    /* modify by suifeng 2017.10.17, for add auto correct word */
    char *word_correct_arg = NULL;
    if (config_parser_getstring_fromfile(AI_CFG_PATH, "word_correct", &word_correct_arg) == 0) {
    	if (0 == strcmp("disable", word_correct_arg)) {
    		log_info("word correct disable");
    	} else if (0 == strcmp("enable", word_correct_arg)) {
    		log_info("word correct enable");
    		sa_info.correct.enable_flag = 1;
    	} else {
    		return 0;
    	}

    	sa_info.correct.enable_flag = 1;

    	/* correct init */
		sa_info.correct.init = correct_init;
		sa_info.correct.correct_free = correct_free;
		sa_info.correct.correct = similarity;
		if (sa_info.correct.init) {
			sa_info.correct.init();
		}
		//free(word_correct_arg);
    } else {
    	log_error("config_parser_getstring_fromfile(%s, word correct) fail\r\n",AI_CFG_PATH);
    }
#endif
    log_debug("Semantic initialise sucess!");

    return 0;
}

/* flag = 1, add front to src , flag = 0, add end to src */
static void add_answer(event_t *src, event_t *add, int flag)
{
    char tmp[512];
    if (flag == 0) {
        sprintf(tmp, "%s|%s", src->word.word[0], add->word.word[0]);
    } else {
        sprintf(tmp, "%s|%s", add->word.word[0], src->word.word[0]);
    }
    strcpy(src->word.word[0], tmp);
}

static int get_question_ignore_context(event_t *event)
{
    int ret = 0;
    sprintf(event->context.tmp_context, "%s.qign", event->context.cur_context);
    ret = sql_get_response_by_context_only(event, event->context.tmp_context);
    if (ret == 1) {
        event->fr_type = fr_qign;
    }
    return ret;
}

static int get_question_any_context(event_t *event)
{
    int ret = 0;
    sprintf(event->context.tmp_context, "%s.qany", event->context.cur_context);
    ret = sql_get_response_by_context_only(event, event->context.tmp_context);
    if (ret == 1) {
        event->fr_type = fr_qany;
    }
    return ret;
}

static int get_question_response_from_current_context(event_t *event)
{
    int ret = 0;
    sprintf(event->context.tmp_context, "%s.spec", event->context.cur_context);
    event->context.context = event->context.tmp_context;
    ret = sql_get_response(event);
    if (ret == 1) {
        event->fr_type = fr_special;
    }
    return ret;
}

static int get_question_response_from_question_context(event_t *event)
{
    int ret = 0;
    strcpy(event->context.tmp_context, ".question");
    event->context.context = event->context.tmp_context;
    ret = sql_get_response(event);
    if (ret == 1) {
        event->fr_type = fr_question1;
    }
    return ret;
}

static int get_response_from_second_question_context(event_t *event)
{
    int ret = 0;
    strcpy(event->context.tmp_context, "..question");
    event->context.context = event->context.tmp_context;
    ret = sql_get_response(event);
    if (ret == 1) {
        event->fr_type = fr_question2;
    }
    return ret;
}


static int get_why(event_t *event)
{
    srand(time(0));
    sprintf(event->context.tmp_context, ".why%d", rand() % 5);
    return sql_get_response_by_context_only(event, event->context.tmp_context);
}


static int invite(event_t *event, char *context)
{
    int ret = 0, ret1 = 0;
    event_t why = *event;

    ret1 = get_why(&why);
    strcpy(event->context.tmp_context, context);
    ret = sql_get_response_by_context_only(event, event->context.tmp_context);

    if (ret == 1 && ret1 == 1) {
        add_answer(event, &why, 1);
    }

    return ret;
}

static int invite_handle(event_t *event)
{
    return invite(event, "primary");
}

static int invite_middle_handle(event_t *event)
{
    return invite(event, "middle");
}

static int invite_high_handle(event_t *event)
{
    return invite(event, "high");
}

static int busy_handle(event_t *event)
{
    int ret = 0;
    strcpy(event->context.tmp_context, "busy");
    ret = sql_get_response_by_context_only(event, event->context.tmp_context);
    if (ret == 1) {
        event->fr_type = fr_busy;
    }
    return ret;
}

static int skip_session_fail_handle(event_t *event)
{
    int ret = 0;
    sprintf(event->context.tmp_context, ".skip_session_fail");
    event->context.context = event->context.tmp_context;
    ret = sql_get_public_response_by_context_only(event);
    if (ret == 1) {
        event->fr_type = fr_skip_session_fail;
    }
    return ret;
}

static int skip_session_success_handle(event_t *event)
{
    int ret = 0;
//    sprintf(event->context.tmp_context, "%s.skip_session_sucsses", event->context.cur_context);
//    ret = sql_get_response_by_context_only(event, event->context.tmp_context);
    sprintf(event->context.tmp_context, ".skip_session_sucsses");
    event->context.context = event->context.tmp_context;
    ret = sql_get_public_response_by_context_only(event);
    if (ret == 1) {
        event->fr_type = fr_skip_session_success;
    }
    return ret;
}



static int refuse_answer(event_t *event)
{
    int ret = 0;
    sprintf(event->context.tmp_context, "%s.refuse", event->context.cur_context);
    ret = sql_get_response_by_context_only(event, event->context.tmp_context);
    if (ret == 1) {
        event->fr_type = fr_refuse;
    }
    return ret;
}

static int sure_answer(event_t *event)
{
    int ret = 0;
    sprintf(event->context.tmp_context, "%s.sure", event->context.cur_context);
    ret = sql_get_response_by_context_only(event, event->context.tmp_context);
    if (ret == 1) {
        event->fr_type = fr_sure;
    }
    return ret;
}

static int negtive_answer(event_t *event)
{
    int ret = 0;
    sprintf(event->context.tmp_context, "%s.negative", event->context.cur_context);
    ret = sql_get_response_by_context_only(event, event->context.tmp_context);
    if (ret == 1) {
        event->fr_type = fr_negtive;
    }
    return ret;
}

static int neuter_answer(event_t *event)
{
    int ret = 0;
    sprintf(event->context.tmp_context, "%s.neuter", event->context.cur_context);
    ret = sql_get_response_by_context_only(event, event->context.tmp_context);
    if (ret == 1) {
        event->fr_type = fr_neuter;
    }
    return ret;
}

static int get_any_answer(event_t *event)
{
    int ret = 0;
    sprintf(event->context.tmp_context, "%s.any", event->context.cur_context);
    ret = sql_get_response_by_context_only(event, event->context.tmp_context);
    if (ret == 1) {
        event->fr_type = fr_any;
    }
    return ret;
}

static int get_auto_answer(event_t *event)
{
    int ret = 0;
    sprintf(event->context.tmp_context, "%s.auto", event->context.cur_context);
    ret = sql_get_response_by_context_only(event, event->context.tmp_context);
    if (ret == 1) {
        event->fr_type = fr_auto;
    }
    return ret;
}

static int get_auto0_answer(event_t *event)
{
    int ret = 0;
    sprintf(event->context.tmp_context, "%s.auto0", event->context.cur_context);
    ret = sql_get_response_by_context_only(event, event->context.tmp_context);
    if (ret == 1) {
        event->fr_type = fr_auto0;
    }
    return ret;
}

int get_time_all_answer(event_t *event)
{
	int ret = 0;

    if (is_phone_timeout(event->call_start, event->phone_time_all)) {
        sprintf(event->context.tmp_context, ".phone_time_all");
        event->context.context = event->context.tmp_context;
        ret = sql_get_public_response_by_context_only(event);
        if (ret == 1) {
        	event->fr_type = fr_time_all;
        }
    }

    return ret;
}

static int inconvenient_answer(event_t *event)
{
    event->continue_event = evt_sa_send_mesg;
    return 1;
}

static int get_again_over(event_t *event)
{
    sprintf(event->context.tmp_context, ".over_again");
    return sql_get_response_by_context_only(event, event->context.tmp_context);
}

static int again_answer(event_t *event)
{
    int ret = 0;
    event_t tmp = *event;

    ret = sql_get_by_id(event, event->play_response_id);

    if (ret == 1 && get_again_over(&tmp) == 1) {
        add_answer(event, &tmp, 1);
    }

    if (ret == 1) {
        event->fr_type = fr_again;
    }

    return ret;
}

static int purpose_test(event_t *event)
{
    int ret = 0;

    if (is_purpose_type(event->context.cur_context)) {
        strcpy(event->context.tmp_context, event->context.cur_context);
        ret = sql_get_unused_purpose(event->db, event->context.tmp_context);
        if (ret == 1) {
            ret = sql_get_response_by_context_only(event, event->context.tmp_context);
        }
    } else if (event->total_score >= event->score_threshold.score_p) {
        strcpy(event->context.tmp_context, event->context.cur_context);
        ret = sql_get_unused_purpose(event->db, event->context.tmp_context);
        if (ret == 1) {
            ret = sql_get_response_by_context_only(event, event->context.tmp_context);
        }
    }

    return ret;
}

static int word_add_invite(event_t *event)
{
    int ret = 0;
    event_t tmp_event = *event;

    ret = purpose_test(&tmp_event);
    if (ret == 1 && event->context.question_flag == 0) {
    //if (ret == 1 && strcmp(event->context.next_context, "success") != 0 && strcmp(event->context.next_context, "bye") != 0) {
//        add_answer(&tmp_event, event, 1);
//        *event = tmp_event;
//    	event->wait_play_id_queue = tmp_event.context.response_id;
        log_info("add invite!");
        event->wait_queue.w_fr_type = event->fr_type;
        event->wait_queue.w_id = tmp_event.context.response_id;
        event->wait_queue.w_priority = tmp_event.context.priority;
        event->notclean_waitqueue = 1;
    }

    return 1;
}


static int get_skip_answer(event_t *event)
{
    int ret = 0;
    event_t skip = *event;
    int time_limits = SKIP_TIMES_LIMITED;

    /**/
    while (skip.context.skip_condition[0] != '0' && skip.context.skip_to[0] != '0' && (--time_limits) > 0) {
        log_debug("skip condition: [%s]", skip.context.skip_condition);
        log_debug("skip to: [%s]", skip.context.skip_to);
        if (skip.context.skip_condition[0] == '.' ? is_context_used(skip.context.skip_condition + 1) : is_context_used(skip.context.skip_condition)) {
            sprintf(skip.context.tmp_context, "%s", skip.context.skip_to);
            ret = sql_get_response_by_context_only(&skip, skip.context.tmp_context);
            if (ret != 1) {/* 解决语境不存在时死循环的问题 modify by suifeng 2017.06.16*/
            	log_error("skip context not exist!");
            	break;
            }
        } else {
            log_debug("break");
            break;
        }
    }
    if (ret == 1) {
        log_info("skip");
        *event = skip;
        event->fr_type = fr_skip; /* 修正跳转时导致的超时不回答的问题 （2018.04.20 add by suifeng） */
        clean_wait_queue(event); //跳转成功后，清掉缓存队列，以免产生BUG（2016.10.10 add by suifeng）

        /* 跳转成功后，判断该语境是否使用，如果使用，则取下一个没有使用的主流程（2016.11.17 add by suifeng） */
//        printf("next_context:%s\n", event->context.next_context);
        if (is_purpose_type(event->context.next_context)) {
            log_info("is purpose");
            ret = sql_get_unused_purpose(event->db, event->context.next_context);
            if (ret == 1) {
                ret = sql_get_response_by_context_only(event, event->context.next_context);
            }
        } else if (is_flow_type(event->db, event->context.next_context) == 1) {
            ret = sql_get_next_notused_response(event, event->context.next_context);
        }
    }

    return ret;
}

static int ask_handle(event_t *event)
{
    int ret = 0;
	event_t tmp = *event;

	log_info("[ask handle] find the question");

    /* 忽略单字，双字 */
    if (strlen(event->word.word[event->word.cur_word]) < 6) {
        log_info("word size < 2, not effect word(%s), len(%d)",
                event->word.word[event->word.cur_word],
                (int)strlen(event->word.word[event->word.cur_word]));
        event->fr_type = fr_1wd_ignore;
        return 2;
    }

    /* 先在当前语境下查找 */
    if (ret != 1) {
        log_info("get from current context...");
    	ret = get_question_response_from_current_context(event);
    }

    /* 是否忽略此问题 */
	if (ret != 1) {
	    log_info("get from question ignore context...");
		ret = get_question_ignore_context(&tmp);
		if (ret == 1) {
            *event = tmp;
            return ret;
		}
	}

    if (ret != 1) {
        /* get question answer from question context */
        log_info("get from question context...");
        ret = get_question_response_from_question_context(event);
    }

    if (ret != 1) {
        /* get question answer from question context */
        log_info("get from second question context...");
        ret = get_response_from_second_question_context(event);
    }

    if (ret == 1) {
        if (event->system_status == s_running) {
            if (++event->question_cnt > 1 && event->ai_status != ai_play) {
                event->question_cnt = 0;
            }
        }
        //event->total_score += count_score(event->question_value, 1.0);
        if (event->question_cnt == 0) {
            word_add_invite(event);
        }
    }

    if (ret != 1) {
        if (event->answer_type == type_inconvenient) {
            ret = inconvenient_answer(event);
        } else if (event->answer_type == type_again) {
            ret = again_answer(event);
        }
    }

    if (ret != 1) {
        log_info("get from auto0 answer...");
    	ret = get_auto0_answer(event);
    }

    /* 获取问题统一回答 */
    if (ret != 1) {
        log_info("get from question any context...");
        ret = get_question_any_context(event);
    }

    if (ret != 1) {
        log_info("get from any answer...");
        ret = get_any_answer(event);
    }

    return ret;
}

#if 0
static int is_single_word(char * word)
{
	char * pos = NULL;

    /* 忽略单字  - english */
	//if ((pos = strstr(word, " ")) != NULL  && strstr(pos + 1, " ") != NULL) {
	//	return 0;
	//}

    return 0;
}
#endif

/***************************************************************
 * Description: 问题回答事件
 *              返回值: -1 异常; 0 没有结果; 1 有结果
***************************************************************/
static int answer_handle(event_t *event)
{
//    float weight;
    int ret = 0;
	event_t tmp = *event;
//	int score = 0;


    if (event->answer_type == type_null) {

	    /* 忽略单字 */

    	//printf("word len: %d\n", strlen(event->word.word[event->word.cur_word]));

        if (strlen(event->word.word[event->word.cur_word]) < 6) {
            log_info("word size < 2, not effect word(%s), len(%d)",
                    event->word.word[event->word.cur_word],
                    (int)strlen(event->word.word[event->word.cur_word]));
            event->fr_type = fr_1wd_ignore;
            return 2;
        }

  //  	if (is_single_word(event->word.word[event->word.cur_word])) {
  //          event->fr_type = fr_1wd_ignore;
 //           log_info("word size < 2, not effect word(%s)", event->word.word[event->word.cur_word]);
 //           return 2;
//    	}

	
        /* 先在当前语境下查找 */
    	if (ret != 1) {
    	    log_info("get from current context...");
    		ret = get_question_response_from_current_context(event);
    	}

		if (ret == 1) {
			return ret;
		}

        /*is a question ?*/
        if (ret != 1) {
            /* get question answer from question context */
            log_info("get from question context...");
            ret = get_question_response_from_question_context(event);
        }

        if (ret != 1) {
            /* get question answer from question context */
            log_info("get from second question context...");
            ret = get_response_from_second_question_context(event);
        }

        /* 是否忽略此问题 */
    	if (ret == 1) {
    	    log_info("get from question ignore context...");
    		if (get_question_ignore_context(&tmp) == 1) {
				*event = tmp;
			} else {
                if (event->system_status == s_running) {
                    if (++event->question_cnt > 1 && event->ai_status != ai_play) {
                        event->question_cnt = 0;
                    }
                }
                //event->total_score += count_score(event->question_value, 1.0);
                if (event->question_cnt == 0) {
                    word_add_invite(event);
                }
			}
    	}

        if (ret != 1) {
            log_info("get from auto0 answer...");
            ret = get_auto0_answer(event);
        }

        if (ret != 1) {
            log_info("get any answer context...");
            ret = get_any_answer(event);
        }

        return ret;

    } else {
        /* 先在当前语境特殊回答下查找 */
    	if (ret != 1) {
    	    log_info("get from current context...");
        	ret = get_question_response_from_current_context(event);
    	}

        if (ret != 1) {
            /* 按正常流程 走*/
            if (event->answer_type == type_negtive || event->answer_type == type_refuse) {
                if (is_purpose_type(event->context.cur_context) && event->reason == 0) {
                    ret = sql_get_introduce_deep(event->db, event->context.tmp_context);
                }
                if (ret == 1) {
                    ret = sql_get_response_by_context_only(event, event->context.tmp_context);
                    event->reason = 1;
                    return 1;
                } else if (event->answer_type == type_negtive) {
                    ret = negtive_answer(event);
                } else if (event->answer_type == type_refuse) {
                	ret = refuse_answer(event);
                }
            } else if (event->answer_type == type_sure || event->answer_type == type_neuter){
                if (event->answer_type == type_sure) {
                    ret = sure_answer(event);
                } else if (event->answer_type == type_neuter) {
                    ret = neuter_answer(event);
                }

                /* modify by suifeng2016/11/22 */
                if (ret == 1) {
                    /*if (is_flow_type(event->db, event->context.next_context) == 1) {
                        score = event->context.words_score;
                        ret = sql_get_next_notused_response(event, event->context.next_context);
                        event->context.words_score ? 0: (event->context.words_score = score);
                    } else*/ if (is_context_used(event->context.next_context)) {
                        strcpy(event->context.tmp_context, event->context.cur_context);
                        ret = sql_get_next_notused_response(event, event->context.tmp_context);
                    }
                } else /*if (ret != 1 && is_flow_type(event->db, event->context.cur_context) == 1)*/ {
                    event->continue_event = evt_continue_run;
                    log_info("continue");
                    return 1;
                }
            }


            if (ret != 1) {
                if (event->answer_type == type_inconvenient) {
                    ret = inconvenient_answer(event);
                } else if (event->answer_type == type_again) {
                    ret = again_answer(event);
                }
            }

            if (ret != 1) {
                log_info("get from auto0 answer...");
                ret = get_auto0_answer(event);
                if (ret == 1) {
                    return ret;
                }
            }

            if (ret != 1) {
                log_info("get any answer context...");
                ret = get_any_answer(event);
            }
        }

        if (ret != 1) {
            /*当前语境是小语境，则进入上一级语境*/
            if (event->context.cur_context[0] == '.' && event->context.parent_context[0] != '\0') {
                log_info("entry parent context...");
                memset(event->context.cur_context, 0 , sizeof(event->context.cur_context));
                strcpy(event->context.cur_context, event->context.parent_context);
            }

            if (event->answer_type == type_negtive || event->answer_type == type_refuse) {
                if (is_purpose_type(event->context.cur_context) && event->reason == 0) {
                    ret = sql_get_introduce_deep(event->db, event->context.tmp_context);
                }
                if (ret == 1) {
                    ret = sql_get_response_by_context_only(event, event->context.tmp_context);
                    event->reason = 1;
                } else if (event->answer_type == type_negtive) {
                    ret = negtive_answer(event);
                } else if (event->answer_type == type_refuse) {
                    ret = refuse_answer(event);
                }
            } else if (event->answer_type == type_sure || event->answer_type == type_neuter){
                if (event->answer_type == type_sure) {
                    ret = sure_answer(event);
                } else if (event->answer_type == type_neuter) {
                    ret = neuter_answer(event);
                }

                /* modify by suifeng2016/11/22 */
                if (ret == 1) {
                    /*if (is_flow_type(event->db, event->context.next_context) == 1) {
                        score = event->context.words_score;
                        ret = sql_get_next_notused_response(event, event->context.next_context);
                        event->context.words_score ? 0: (event->context.words_score = score);
                    } else*/ if (is_context_used(event->context.next_context)) {
                        strcpy(event->context.tmp_context, event->context.cur_context);
                        ret = sql_get_next_notused_response(event, event->context.tmp_context);
                    }
                } else /*if (ret != 1 && is_flow_type(event->db, event->context.cur_context) == 1)*/ {
                    event->continue_event = evt_continue_run;
                    log_info("continue");
                    return 1;
                }
            }
        }
    }

    return ret;
}


static int wait_handle(event_t *event)
{
    int ret = 0;

    if (++event->timeout < SEC_2_0) {
        return 0;
    }

//    ret = yfs_is_ars_busy();
//
//    if (ret > 0) {
//        event->timeout = 0;
//        log_info("[info] ars is busy! busy: %d", ret);
//        return 2;
//    }

//    event->not_memery_id = 1;

    if (event->timeout == SEC_5_0) {
        event->timeout = 0;
        sprintf(event->context.tmp_context, ".wait0");
        event->context.context = event->context.tmp_context;
        if ((ret = sql_get_public_response_by_context_only(event)) == 1) {
            //event->timeout_cnt++;
        } else {
			//event->not_memery_id = 0;
            log_error("(Walling!) wait not data!!");
        }
    }

    return ret;
}

static int answer_timeout_handle(event_t *event)
{
    int ret = 0;
    char info[128];

	//printf("---out---: %d", event->timeout);

    event->timeout++;

    if (event->timeout == SEC_0_5) {
        if (get_auto0_answer(event) == 1) {
            event->timeout = 0;
            return 1;
        }
    }

    if (event->timeout < SEC_5_0) {
        return ret;
    }

    event->timeout = 0;

    /* success context */
    log_info("answer timeout: %d, context: %s", event->timeout_cnt, event->context.cur_context);
    if (strcmp(event->context.cur_context, "success") == 0 ||
            strcmp(event->context.cur_context, "bye") == 0) {
        event->continue_event = evt_success;
        return 1;
    }

    if ((get_time_ai2human(event) == 1 && event->ivr_cnt == 0) || get_time_all_answer(event) == 1) {
    	return 1;
    }

//    event->not_memery_id = 1;
    ret = get_auto_answer(event);
    if (ret == 1) {
        log_info("auto answer");
        return ret;
    }
//    event->not_memery_id = 0;

    /* 一直没人回答，直接进入挂机 */
    if (event->timeout_cnt >= event->ansout_nub) {
        if(purpose_test(event) == 1 && event->timeout_cnt == event->ansout_nub) {
            event->fr_type = fr_ans_timeout_purpose;
            return 1;
        }

        event->continue_event = evt_ai_hang_up;
        memset(info, 0, sizeof(info));
        sprintf(info, "timeout: score(%d)", event->total_score);
        set_customer_level(0, info);
        set_hangup_status(34);

        return 1;
    }

//    event->not_memery_id = 1;
    sprintf(event->context.tmp_context, ".ansout%d", event->timeout_cnt);
    event->context.context = event->context.tmp_context;
    ret = sql_get_public_response_by_context_only(event);

    if (ret == 1) {
        event->fr_type = fr_ans_timeout;
    } else if (ret == -10) {
    	log_error("has some error!!");
    } else {
		//event->not_memery_id = 0;
        log_error("(Walling!) answer timeout not data!!");
    }

    return ret;
}

/* 查找到超时回答，即为2, 区别普通回答，以便清空 timeout 标志 */
static int silent_timeout_handle(event_t *event)
{
    int ret = 0;

    if (++event->timeout < SEC_2_0) {
        return 0;
    }

    /* 一直没人回答，直接进入发信息 */
    if (event->timeout_cnt > event->timeout_nub && event->timeout == SEC_5_0) {
        event->timeout = 0;
        event->continue_event = evt_ai_hang_up;
        set_customer_level('D', "silent timeout");
        set_hangup_status(35);

        log_info("silent timeout: %d", event->timeout_cnt);
        return 1;
    }

//    event->not_memery_id = 1;

    if (is_hello() && event->timeout_cnt == 0 && event->timeout == SEC_4_5) {
        ret = sql_get_by_id(event, 0);
        event->timeout = 0;
        event->fr_type = fr_sil_timeout;
        log_info("silent timeout: %d", event->timeout_cnt);
        return ret;
    }

    if (event->timeout == SEC_5_0) {
        event->timeout = 0;
        sprintf(event->context.tmp_context, ".timeout%d", event->timeout_cnt - 1);
        event->context.context = event->context.tmp_context;
        if ((ret = sql_get_public_response_by_context_only(event)) == 1) {
            event->fr_type = fr_sil_timeout;
        } else if (ret == -10) {
        	log_error("has some error!!");
        } else {
//            event->not_memery_id = 0;
            log_error("(Walling!) silent timeout not data!!");
        }
        log_info("silent timeout: %d", event->timeout_cnt);
    }

    return ret;
}

static int telphone_handle(event_t *event)
{
    int ret = 0;
    //event->not_memery_id = 1;
    ret = sql_get_by_id(event, 0);
    if (ret != 1) {
        log_error("SQL No Start Data!");
        //event->not_memery_id = 0;
    }
    event->system_status = s_start;

    return ret;
}


static int hang_up_handle(event_t *event)
{
	//char cmd[300];
    memset(&event->word, 0, sizeof(event->word));
    memset(&event->context, 0, sizeof(event->context));
    event->context.next_context[0] = '#';

//    if (event->hangup_status == 760) {
//    	snprintf(cmd, sizeof(cmd) - 1, "rm -rf %s", get_db_path());
//    	system(cmd);
//    }

    return 1;
}

static int continue_run_handle(event_t *event)
{
    int ret = 0;
//    int score = 0;

    /*当前语境是小语境，则进入上一级语境*/
    if (event->context.cur_context[0] == '.' && event->context.parent_context[0] != '\0') {
        log_info("entry parent context...");
        memset(event->context.cur_context, 0 , sizeof(event->context.cur_context));
        strcpy(event->context.cur_context, event->context.parent_context);
    }

    /* 是否是目的 */
    if (is_purpose_type(event->context.cur_context)) {
        strcpy(event->context.tmp_context, event->context.cur_context);
        ret = sql_get_unused_purpose(event->db, event->context.tmp_context);
        if (ret == 1) {
            ret = sql_get_response_by_context_only(event, event->context.tmp_context);
        }
    } else {
		/*找出当前语境肯定回答的下一语境*/
//		sprintf(event->context.tmp_context, "%s.sure", event->context.cur_context);
//		event->context.context = event->context.tmp_context;
//		ret = sql_get_response_by_context_only(event);


        strcpy(event->context.tmp_context, event->context.cur_context);
        ret = sql_get_next_notused_response(event, event->context.tmp_context);

		if (ret != 1) {
	        sprintf(event->context.tmp_context, "%s.any", event->context.cur_context);
	        ret = sql_get_response_by_context_only(event, event->context.tmp_context);
		}

//		if (ret == 1) {
//			/* 判断当前流程是否走过，走过则跳过走下一未走流程  */
//			ret = sql_get_next_notused_response(event);
//		}
    }

    if (ret != 1) {
        log_error("******* (Walling) continue data error! ********");
        ret = -1;
    }

    return ret;
}

static int before_call_handle(event_t *event)
{
    int ret = 0;

    sprintf(event->context.tmp_context, ".before_call");
    event->context.context = event->context.tmp_context;
    ret = sql_get_public_response(event);
    //log_debug("sql_get_public_response ret:%d", ret);
    if ((ret != 1) && (ret >= 0)) {
        ret = 2;
    }

    return ret;
}

static int sa_event_handle(event_t *info)
{
    int ret = 0;

    do {
        //printf("[event] context: %s, system status: %d", sa_info.context.cur_context, sa_info.system_status);
    	//log_info("event: %d", info->event);
        switch (info->event) {
        case evt_null:
            log_info("no event!");
            ret = 2;
            break;

        case evt_pre_answer:
            ret = answer_handle(info);
            break;

        case evt_pre_ask:
            ret = ask_handle(info);
            break;

        case evt_ai_play:

            break;
        case evt_ignore:
            ret = 2;
            log_info("event ignore!");
            break;

        case evt_wait:
            ret = wait_handle(info);
            break;

        case evt_ai_answer_timeout:
            ret = answer_timeout_handle(info);
            break;

        case evt_ai_silent_timeout:
            ret = silent_timeout_handle(info);
            break;

        case evt_ai_telphone:
            ret = telphone_handle(info);
            break;

        case evt_start_talk:
            ret = start_talk_handle(info);
            break;

        case evt_ai_hang_up:
            ret = hang_up_handle(info);
            break;

        case evt_sa_send_mesg:
            ret = busy_handle(info);
            break;

        case evt_sa_invite:
            ret = invite_handle(info);
            break;

        case evt_sa_invite_middle:
            ret = invite_middle_handle(info);
            break;

        case evt_sa_invite_high:
            ret = invite_high_handle(info);
            break;

        case evt_continue_run:
            ret = continue_run_handle(info);
            break;

        case evt_before_call:
            ret = before_call_handle(info);
            break;

        case evt_success:
            /* modify by suifeng 2017.05.26 */
//            ret = 1;
//            log_debug(" reponse_ignore: %d", info->reponse_ignore);
//            if (info->ai_status == ai_play) {
//                info->reponse_ignore = yfs_is_ars_busy() + 1;
//            } else {
//                if (--info->reponse_ignore <= 0) {
                    ret = sure_answer(info);
//                }
//            }
            break;
        case evt_ai_skip_session_fail:
        	ret = skip_session_fail_handle(info);
        	break;
        case evt_ai_skip_session_success:
        	ret = skip_session_success_handle(info);
        	break;
        default:
        	ret = 2;
        	log_error("not define event(%d)!", info->event);
        	break;
        }

        if (ret == -ERROR_GET_TABLE) {
        	info->continue_event = evt_ai_hang_up;
        	info->hangup_status = 761;
        } else if (ret != 1 && info->event == evt_pre_ask &&
                info->answer_type != type_question &&
                 info->answer_type != type_null) {
            info->continue_event = evt_pre_answer;
        } else {
            if (ret == 1 && info->context.next_context[0] == '!' && info->word.word[0][0] == '0') {
                /* go to parent context and run continue */
                log_info("go to parent context");
                memset(info->context.cur_context, 0 , sizeof(info->context.cur_context));
                strcpy(info->context.cur_context, info->context.parent_context);
                info->continue_event = evt_continue_run;
            }
        }
        clean_event(info);

    } while (info->event != evt_null);

    return ret;
}

static int pinyin_analyzer_proc(event_t *event)
{
    int ret = 0;
    int i = 0;
    char pinyin[256];
    char cmd[256];
    int ret1 = 0;
    char temp[256];

    for (i = 0; i < event->word.word_nub; i++) {
        event->word.cur_word = i;
        memset(pinyin, 0, sizeof(pinyin));
        ret1 = word_to_pinyin((unsigned char*)event->word.word[i], pinyin);
        //printf("word     : [%s]", event->word.word[i]);
        memset(cmd, 0, sizeof(cmd));
//        if (get_log_path() == NULL) {
            sprintf(cmd, "echo '1:%s' >> pinyin.log", event->word.word[i]);
//        } else {
//            sprintf(cmd, "echo '1:%s' >> %s/pinyin.log", event->word.word[i], get_log_path());
//        }
        system(cmd);
        if (ret1 != 0) {
            continue;
        }
        memset(temp, 0,sizeof(temp));
        ret1 = normative_by_pinyin(pinyin, temp, event->db);
        //printf("pinyin   : [%s]", pinyin);
        //printf("py->word : [%s]", temp);
        memset(cmd, 0, sizeof(cmd));
//        if (get_log_path() == NULL) {
            sprintf(cmd, "echo '2:%s' >> pinyin.log", temp);
//        } else {
//            sprintf(cmd, "echo '2:%s' >> %s/pinyin.log", temp, get_log_path());
//        }
        system(cmd);

        if (ret1 != 0) {
            continue;
        }

#if 0
        /* word analyze */
        ret = set_envent(&sa_info, 0, event->db, event->word.word[i]);
        printf("type:%d",event->answer_type);
        printf("event:%d",event->event);

        /* event handle */
        ret = sa_event_handle(&sa_info);
        if (ret == 1) {
            printf("set_accent success!");
            break;
        }
#endif
    }

    return ret;
}


#if 0
int sa_pre_process(event_t *info)
{
    int i = 0, ret = 0;

    for (i = 0; i < info->word.word_nub; i++) {
        ret = set_envent(info, 0, info->db, info->word.word[i]);
        if (ret == 0) {
            log_debug("i:%d",i);
            break;
        }
        if (info->answer_type != type_null) {
            if (ret == 3 && ret <= strlen(info->word.word[i]) / 3) {
                continue;
            }
            log_debug("i2:%d",i);
            break;
        }
    }

    return 0;
}

int sa_word_replace(event_t *event)
{
    int i = 0;
    char cmd[512];

    for (i = 0; i < event->word.word_nub; i++) {
        log_debug("原词：[%s]", event->word.word[i]);
        if (get_log_path() == NULL) {
            sprintf(cmd, "echo '%s' >> replace.log", event->word.word[i]);
        } else {
            sprintf(cmd, "echo '%s' >> '%s/replace.log'", event->word.word[i], get_log_path());
        }
        system(cmd);
        if (normative_word(event->word.word[i], event->word.word[i], event->db) == 0) {
            log_debug("替换：[%s]", event->word.word[i]);
        }
    }

    return 0;
}
#endif


int get_time_ai2human(event_t *event)
{
	int ret = 0;

    if (is_phone_timeout(event->call_start, event->time_ai2human)) {
        sprintf(event->context.tmp_context, ".time_ai2human");
        event->context.context = event->context.tmp_context;
        ret = sql_get_public_response_by_context_only(event);
        if (ret == 1) {
        	event->fr_type = fr_time_ai2human;
        }
    }

    return ret;
}


/***************************************************************
 *
***************************************************************/
int set_context_name(context_t *context)
{
    if(context == NULL || context->next_context[0] == '\0') {
        return -1;
    }

    log_info("Play start, update context");

    sa_info.ai_status = ai_play;

    /* change current context */
    if (context->next_context[0] == '!') {
        /* go to parent context and run continue */
        log_info("go to parent context");
        memset(context->cur_context, 0 , sizeof(context->cur_context));
        strcpy(context->cur_context, context->parent_context);
        sa_info.context = *context;
    } else if (context->next_context[0] == '>') {
        log_info("entry wait mode...");
        sa_info.system_status = s_wait;
    } else if (context->next_context[0] == '^') {
        log_info("entry previous context ...");
        memset(context->cur_context, 0 , sizeof(context->cur_context));
        strcpy(context->cur_context, context->previous_context);
        sa_info.context = *context;
    } else if (context->next_context[0] == '#') {
        //memcpy(context->cur_context, context->next_context, CONTEXT_SIZE);
        sa_info.context = *context;
    //} else if (is_purpose_type(context->cur_context) && !is_purpose_type(context->next_context) && context->next_context[0] != '.') {

    } else if (context->next_context[0] != '0') {
        memcpy(context->cur_context, context->next_context, CONTEXT_SIZE);
        strcpy(context->previous_context, sa_info.context.cur_context);
        sa_info.context = *context;
    }

    sa_info.context.context = sa_info.context.cur_context;
    strcpy(context->cur_context, sa_info.context.cur_context);//?

    if (sa_info.not_memery_id == 0) {
        log_info("memery this id");
        sa_info.play_response_id = context->response_id;
        if (sa_info.fr_type != fr_question_notfind) {
            if (sa_info.qustion_notfind_times != 0 && sa_info.question_notfind_floor > 1) {
                if (++sa_info.question_notfind_cur_floor > sa_info.question_notfind_floor) {
                    sa_info.question_notfind_cur_floor = 1;
                }
            }
            sa_info.qustion_notfind_times = 0;
        }
        sa_info.text_type = text_words;
    } else if (sa_info.not_memery_id == 2) { /* break tips */
        log_info("break, not memery this id");
        sa_info.not_memery_id = 0;
        if (sa_info.fr_type != fr_question_notfind) {
            if (sa_info.qustion_notfind_times != 0 && sa_info.question_notfind_floor > 1) {
                if (++sa_info.question_notfind_cur_floor > sa_info.question_notfind_floor) {
                    sa_info.question_notfind_cur_floor = 1;
                }
            }
            sa_info.qustion_notfind_times = 0;
        }
        sa_info.text_type = text_break;
    } else {
        log_info("tips, not memery this id");
        sa_info.not_memery_id = 0;
        sa_info.text_type = text_tips;
    }

    /* 解决记忆ID与当前ID重合时，不缓存的BUG  add by suifeng (2017.07.13) */
    sa_info.last_id = context->response_id;
    /* timeout times count, add by suifeng 2016.10.19 */
    if (sa_info.timeout_flag == 1) {
        sa_info.timeout_flag = 0;
        sa_info.timeout_cnt++;
    } else {
        sa_info.timeout_cnt = 0;
    }

    if (context->hangup_status != 0) {
        sa_info.hangup_status = context->hangup_status;
        log_info("hangup_status: %d", sa_info.hangup_status);
    }

    /* 记录正在播放的优先级 */
    sa_info.cur_priority = sa_info.context.priority;
    /* 记录当前播放语句的分数 */
    //sa_info.question_value = sa_info.context.words_score;//暂时用不上

    /* 获取当前时间 */
    gettimeofday(&sa_info.play_start_time, NULL);
//    struct tm *tm_time = localtime(&sa_info.play_start_time.tv_sec);
//    log_info("play time:%d/%d/%d %d:%d:%d.%d\n",
//                        tm_time->tm_year+1900, tm_time->tm_mon+1, tm_time->tm_mday,
//                        tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec, (int)sa_info.play_start_time.tv_usec/1000);

    //printf("cur context: [%s]\n", sa_info.context.cur_context);
    set_used_floor(sa_info.context.cur_context, sa_info.context.level);

    /* 如果已经走到目标语境，则记录，以方便后续跳转  add by SuiFeng 2016.11.29 */
    if (is_purpose_type(sa_info.context.cur_context)) {
        sa_info.purpose_flag = 1;
    }
    log_debug("current context: [%s]", sa_info.context.cur_context);

    return 0;
}


/* ******************************
 * return: -1 error, 0 success
 * *****************************/
int set_semantic_analyzer(SA_OPERATION_TYPE opt_type, sa_set_para_t *para)
{
	int ret = 0;

	switch (opt_type) {
		case SET_DB_PATH:
			ret = set_db_path(para->db_path);
			break;
		case SET_UP_CONTEXT:
			ret = set_context_name(&para->context);
			break;
		case SET_PLAY_END_STATUS:
			ret = set_end_status();
			break;
		case SET_CUSTOM_INFO:
			ret = set_custom_info(para->custom_info);
			break;
		case SET_LOG_PATH:
			ret = set_log_path(para->log_path);
			break;
		case SET_MICON:
			/* modify by suifeng 20171013 --  used add log */
			ret = set_micon(para->micon);
			break;
		case SET_AI_VERSION:
			ret = set_ai_version(para->ai_version);
			break;
		case SET_SA_INFO:
			ret = sa_info_init();
			break;
		default:
			break;
	}

	return ret;
}


int get_semantic_analyzer(SA_OPERATION_TYPE opt_type, sa_get_para_t *para)
{
	int ret = 0;

	switch (opt_type) {
		case GET_LEVEL:
			ret = get_customer_level(&para->level);
			break;
		case GET_HANGUP_STATUS:
			ret = get_hangup_status(&para->hg_status);
			break;
		case GET_HANGUP_DESC:
			ret = get_hangup_desc(para->hg_desc);
			break;
		case GET_RECORD_FILE:
			ret = get_record_file(para->p_to_record.filename, para->p_to_record.sum, para->p_to_record.word);
			break;
		case GET_DB_NAME:
			ret = get_db_name(para->db_name);
			break;
		case GET_AI_VERSION_OF_DB_DEPEND:
			ret = get_ai_version_of_db_depend(para->ai_version_of_db_depend);
			break;
		case GET_INDUSTRY:
			ret = get_industry(para->industry);
			break;
		case GET_AREA:
			ret = get_area(para->area);
			break;
		default:
			break;
	}

	return ret;
}


int semantic_analyzer(semantic_request_t *request, semantic_answer_t *response)
{
    int ret = 0;
    int i = 0;

	format_request(&sa_info, request);

    if (sa_info.event == evt_ai_183 || sa_info.event == evt_ai_telphone) {
        /* initial */
        if (sa_info.db == NULL) {
            ret = samantic_init(&sa_info.db, get_db_path(), &sa_info);
            if (ret < 0) {
                log_error("Initialise semantic analyzer failed!");
                sa_info.system_status = s_before_call;
                sa_info.event = evt_ai_hang_up;
                sa_info.hangup_status = 760; /* module not find */
            } else {
            	if (sa_info.event == evt_ai_183) {
					sa_info.system_status = s_before_call;
            	} else {
            		sa_info.system_status = s_start;
            		gettimeofday(&sa_info.call_start, NULL);
            	}
            }
        }
    }

	/* is hang up, not effect any request */
    if (sa_info.system_status == s_hangup
		   	|| sa_info.system_status == s_null
			|| (sa_info.system_status == s_session
			   	&& sa_info.event != evt_ai_skip_session_fail)) {
		response->type = KEEP_ANSWER;
        return 0;
    }

    if (sa_info.db == NULL) {
        	log_error("db is null!");
            sa_info.event = evt_ai_hang_up;
            sa_info.hangup_status = 760; /* module not find */
    }

    /* event handle */
    if (sa_info.event == evt_null) {
        for (i = 0; i < sa_info.word.word_nub; i++, sa_info.word.cur_word = i) {
            /* word analyze */
            ret = set_envent(&sa_info, 0, sa_info.db, sa_info.word.word[i]);
            log_info("type:%d",sa_info.answer_type);
            log_info("event:%d",sa_info.event);

        	/* 先进行一次修正，将结果存起来待用，且只修正第一个词（概率最高）  modify by LuoHuaMing(2017.11.03) */
            if (sa_info.correct.correct && i == 0) {
                memset(sa_info.corrected_word, 0, sizeof(sa_info.corrected_word));
                ret = sa_info.correct.correct("./correct/correct.db", sa_info.word.word[0],
                		sa_info.corrected_word, &sa_info.correct_distance);

                response->correct_distance = sa_info.correct_distance;
                if (strlen(sa_info.corrected_word) > 0) {
                    log_info("corrected_word:%s", sa_info.corrected_word);
                    memset(response->corrected_word, 0, sizeof(response->corrected_word));
                    sa_info.corrected_word[sizeof(sa_info.corrected_word)-1] = '\0';
                    snprintf(response->corrected_word, sizeof(response->corrected_word), "%s", sa_info.corrected_word);
                    //snprintf(sa_info.word.word[i], sizeof(sa_info.word.word[i]), "%s", sa_info.corrected_word);
                }
            }

            /* event handle */
            ret = sa_event_handle(&sa_info);
            if (ret == 1) {
                break;
            }
        }
        if(ret < 0)// get db v = null
        {
            sa_info.event = evt_ai_hang_up;
            sa_info.hangup_status = 760; /* module not find */
        }
        /* 在词修正结果参与匹配的情况下，如果匹配不上，用纠正后的词再次进行匹配  add by LuoHuaMing(2017.11.03) */
        if (sa_info.correct.enable_flag ==1 && strlen(sa_info.corrected_word) > 0) {
        	snprintf(sa_info.word.word[0], sizeof(sa_info.word.word[0]), "%s", sa_info.corrected_word);
        	ret = sa_event_handle(&sa_info);
        }
    } else {
        /* event handle */
        ret = sa_event_handle(&sa_info);
    }

    if (ret == 0) {
        pinyin_analyzer_proc(&sa_info);
    }

    /* no result and not playing */
    if (ret != 1 && sa_info.ai_status != ai_play) {
        if (sa_info.wait_queue.w_id > 0) {
            ret = get_anser_from_wait_queue(&sa_info);
        } else if (ret == 0 && sa_info.word.word_nub > 0) {
            ret = sa_score_process(&sa_info);
        }
    }

    if (ret == 1) {
        sa_info.timeout = 0;
        get_skip_answer(&sa_info);
        format_response(&sa_info, request, response);
//    } else if (ret == -10) {
//    	log_error("has some error!!");
    } else {
        response->type = KEEP_ANSWER;
    }

    if (response->type != KEEP_ANSWER) {
    	wildcard_replace(response);
    	update_system(&sa_info);
    	sa_info.total_score = count_score(sa_info.total_score, sa_info.context.words_score, sa_info.context.wight);
    	log_info("Total score: %d, System status: %d",
			   	sa_info.total_score, sa_info.system_status);
    	log_info("Current context: %s", response->context.cur_context);
    	log_info("Next context: %s", response->context.next_context);
    	log_info("Answer word: %s", response->word);
    	log_info("Answer record: %s", response->context.record);
    }

    return 0;
}


