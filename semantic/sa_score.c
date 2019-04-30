/*
 * sa_score.c
 *
 *  Created on: 2016年6月22日
 *      Author: 云分身
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "semantic_analyzer.h"
#include "sa_sql.h"
#include "sa_suport.h"
#include "sa_score.h"
#include "sa_correct.h"
#include "common_type.h"


YFS_LOG_LEVEL_E g_debug = DBG_ALL;
YFS_LOG_RECORD_E g_record ;

//extern int yfs_asr_tts_text(const char *text, char * out_file);

struct custom_info custom = {
        'D',
};

int set_custom_info(custom_info_t custom_info)
{
    log_info("name:%s, appellation:%s", custom_info.name, custom_info.appellation);

	custom.c_info = custom_info;

    return 0;
}

int get_init_arg(event_t *event)
{
    int i = 0;

    sql_get_one_data_by_name(event->db, "REQ_PUBLIC_DATA", "SCORE",&event->time_ai2human, ".time_ai2human", 1);
    event->time_ai2human == 0 ? event->time_ai2human = 30 : 0; //
    log_info("time of ai to human: %d", event->time_ai2human);

    sql_get_one_data_by_name(event->db, "REQ_PUBLIC_DATA", "SCORE",&event->notfind_nub, ".notfind_n", 1);
    event->notfind_nub == 0 ? event->notfind_nub = 2 : 0; 
    log_info("notfind number: %d", event->notfind_nub);

    sql_get_one_data_by_name(event->db, "REQ_PUBLIC_DATA", "SCORE",&event->ansout_nub, ".ansout_n", 1);
    event->ansout_nub == 0 ? event->ansout_nub = 2 : 0; 
    log_info("ansout number: %d", event->ansout_nub);

    sql_get_one_data_by_name(event->db, "REQ_PUBLIC_DATA", "SCORE",&event->timeout_nub, ".timeout_n", 1);
    event->timeout_nub == 0 ? event->timeout_nub = 2 : 0; 
    log_info("timeout number: %d", event->timeout_nub);

    sql_get_one_data_by_name(event->db, "REQ_PUBLIC_DATA", "SCORE",&event->question_notfind_floor, ".question_notfind_floor", 1);
    event->question_notfind_floor == 0 ? event->question_notfind_floor = 1 : 0; 
    event->question_notfind_floor > Q_NOTFIND_MAX_FLOOR ? event->question_notfind_floor = Q_NOTFIND_MAX_FLOOR : 0;
    event->question_notfind_cur_floor = 1;
    log_info("question notfind floor: %d", event->question_notfind_floor);

    for (i = 0; i < event->question_notfind_floor; i++) {
        event->context.words_score = 0;
        if (i == 0) {
            sprintf(event->context.tmp_context, ".question_notfind_n");
        } else {
            sprintf(event->context.tmp_context, ".question_notfind_n%d", i + 1);
        }

        sql_get_one_data_by_name(event->db, "REQ_PUBLIC_DATA", "SCORE",&event->question_notfind_nub[i], event->context.tmp_context, 1);
        event->question_notfind_nub[i] == 0 ? event->question_notfind_nub[i] = 2 : 0;
        log_info("question notfind n%d: %d", i + 1, event->question_notfind_nub[i]);
    }



    /*test*/
//    strcpy(sa_info.ai_version_of_db_depend, "1.2");
//    strcpy(sa_info.db_name, "test_template");

    sql_get_one_data_by_name(sa_info.db, "DATABASE_INFO", "INFO",sa_info.db_name, "ProjectName", 0);
    sql_get_one_data_by_name(sa_info.db, "DATABASE_INFO", "INFO",sa_info.ai_version_of_db_depend, "VersionNumber", 0);
    sql_get_one_data_by_name(sa_info.db, "DATABASE_INFO", "INFO",sa_info.industry, "Industry", 0);
    sql_get_one_data_by_name(sa_info.db, "DATABASE_INFO", "INFO",sa_info.area, "Area", 0);

    log_info("db_name: %s", sa_info.db_name);
    log_info("ai_version_of_db_depend: %s", sa_info.ai_version_of_db_depend);
    if (strlen(sa_info.area) == 0) {
    	snprintf(sa_info.area, sizeof(sa_info.area) - 1, "None");
    } else {
    	if (strchr(sa_info.area, ' ') != NULL) {
    		log_error("db Area set error: has sapce!");
    		return -1;
    	}
    }
    if (strlen(sa_info.industry) == 0) {
    	snprintf(sa_info.industry, sizeof(sa_info.industry) - 1, "None");
    } else {
    	if (strchr(sa_info.industry, ' ') != NULL) {
    		log_error("db Industry set error: has sapce!");
    		return -1;
    	}
    }

    log_info("Area: %s", sa_info.area);
    log_info("Industry: %s", sa_info.industry);

    sql_get_one_data_by_name(event->db, "REQ_PUBLIC_DATA", "SCORE",&event->phone_time_all, ".phone_time_all", 1);
    event->phone_time_all == 0 ? event->phone_time_all = 10 * 60 : 0;
    log_info("phone time of all: %ds", event->phone_time_all);
    /* add by suifeng 2017.09.15 */
    sql_get_one_data_by_name(event->db, "REQ_PUBLIC_DATA", "SCORE",&event->times_nfr_ignore, ".times_nfr_ignore", 1);
    event->times_nfr_ignore == 0 ? event->times_nfr_ignore = 2000 : 0;
    log_info("times_nfr_ignore: %dms", event->times_nfr_ignore);

    return 0;
}

//add by pxj 20180205
void set_default_e_level_item(event_t *event)
{
	event->score_threshold.e_level_item[0] = STA_IS_BUSY_NOW;
	event->score_threshold.e_level_item[1] = STA_NOT_ANSWER;
	event->score_threshold.e_level_item[2] = STA_SIP_487;
	event->score_threshold.e_level_item[3] = STA_NOT_CONNECTED;
	event->score_threshold.e_level_item[4] = 500;
	event->score_threshold.e_level_item[5] = 0;
}

void set_default_f_level_item(event_t *event)
{
	event->score_threshold.f_level_item[0] = STA_NOT_EXIST;
	event->score_threshold.f_level_item[1] = STA_POWER_OFF;
	event->score_threshold.f_level_item[2] = STA_OUT_OF_SERVICE;
	event->score_threshold.f_level_item[3] = 0;
}

int get_ef_level_array(char *string, int *array, int size)
{
	char str[100];
	int i = 0;
	int len = strlen(string);

	if(!string ||  len <= 0 || len > sizeof(str))
	{
		return -1;
	}
	memcpy(str, string, len);
	char delims[] = ",";
	char *result = NULL;
	result = strtok(str, delims );
	while( result != NULL ) {
		//       printf( "result is \"%s\"\n", result );
		if(i > (size - 1))
			break;
		array[i] = atoi(result);
		i++;
		result = strtok( NULL, delims );
	}
	return 0;
}

/*add  by pxj 20180205 增加E类F类用户自定义功能*/
//select * from REQ_PUBLIC_DATA where NAME = ".itemF";
//INSERT INTO REQ_PUBLIC_DATA(ID,NAME,CONDITION,SCORE) VALUES ("999",".itemF","F类","36,37");
//可分类36 37 38 39 40 41 42 43 44 45 487 500,默认36,37,39 ->f,其它为e
int get_ef_level_item(event_t *event)
{
    int  ret = 0;
    char str_item[100];

    memset(str_item,'\0', sizeof(str_item));
    sql_get_one_data_by_name(event->db, "REQ_PUBLIC_DATA", "SCORE",str_item, ".itemF", 0);
    if (strlen(str_item) > 0) {
    ret = get_ef_level_array(str_item,event->score_threshold.f_level_item, sizeof(&event->score_threshold.f_level_item)/sizeof(int));
		if (ret != 0) {
			set_default_f_level_item(event);
			log_info("get itemF = null,set default\n");
		}
    }
    else {
		set_default_f_level_item(event);
		log_info("get itemF = null,set default\n");
    }
	return 0;
}

int get_score_threshold(event_t *event)
{
    int ret = 0;

    ret = sql_get_one_data_by_name(event->db, "REQ_PUBLIC_DATA", "SCORE",&event->score_threshold.score_a, ".scoreA", 1);
    if (ret < 0) {
        return ret;
    }
    event->score_threshold.score_a == 0 ? event->score_threshold.score_a = 30 : 0;
    log_info("A score threshold: %d", event->score_threshold.score_a);

    sql_get_one_data_by_name(event->db, "REQ_PUBLIC_DATA", "SCORE",&event->score_threshold.score_b, ".scoreB", 1);
    event->score_threshold.score_b == 0 ? event->score_threshold.score_b = 50 : 0;
    log_info("B score threshold: %d", event->score_threshold.score_b);

    sql_get_one_data_by_name(event->db, "REQ_PUBLIC_DATA", "SCORE",&event->score_threshold.score_c, ".scoreC", 1);
    log_info("C score threshold: %d", event->score_threshold.score_c);

    sql_get_one_data_by_name(event->db, "REQ_PUBLIC_DATA", "SCORE",&event->score_threshold.score_d, ".scoreD", 1);
    log_info("D score threshold: %d", event->score_threshold.score_d);

    sql_get_one_data_by_name(event->db, "REQ_PUBLIC_DATA", "SCORE",&event->score_threshold.score_p, ".scoreP", 1);
    log_info("P score threshold: %d", event->score_threshold.score_p);
    event->score_threshold.score_p == 0 ? event->score_threshold.score_p = 60 : 0;
    log_info("P score threshold: %d", event->score_threshold.score_p);

    return 0;
}

int get_custom_record(struct custom_info *custom)
{
	//char buf[64];
	//int ret = 0;
	//int ret2 = 0;
	char path[128] = {0};
	char tmp[256] = {0};
	int i = 0;

	memset(custom->record, 0, sizeof(custom->record));
//	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path) - 1, "%s%s",AI_VOICES_DIR,get_micon());
	log_info("path: %s", path);

#if 1

//	if (custom->c_info.name[0] != '\0') {
//		memset(tmp, 0, sizeof(tmp));
//		snprintf(tmp, sizeof(tmp) - 1, "%s/name%s", path, TTS_FORMAT);
//		ret = yfs_asr_tts_text(custom->c_info.name, tmp);
//		if (ret < 0) {
//			log_error("tts error");
//		} else {
//			snprintf(custom->record[0], sizeof(custom->record[0]) - 1, "@name");
//		}
//	}

	for (i = 0; i < OPTION_SIZE; i++) {
		if (custom->c_info.option[i][0] != '\0') {
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp) - 1, "%s/option%d%s", path, i, TTS_FORMAT);
			log_debug("file:%s", tmp);
			/*
			ret = yfs_asr_tts_text(custom->c_info.option[i], tmp);
			if (ret < 0) {
				log_error("tts error");
			} else {
			*/
				snprintf(custom->record[i], sizeof(custom->record[i]) - 1, "@option%d", i);
			//}
		}
	}

//	if (custom->c_info.option1[0] != '\0') {
//		memset(tmp, 0, sizeof(tmp));
//		snprintf(tmp, sizeof(tmp) - 1, "%s/option1%s", path, TTS_FORMAT);
//		log_debug("file:%s", tmp);
//		ret = yfs_asr_tts_text(custom->c_info.option1, tmp);
//		if (ret < 0) {
//			log_error("tts error");
//		} else {
//			snprintf(custom->record[1], sizeof(custom->record[1]) - 1, "@option1");
//		}
//	}
//
//	if (custom->c_info.option2[0] != '\0') {
//		memset(tmp, 0, sizeof(tmp));
//		snprintf(tmp, sizeof(tmp) - 1, "%s/option2%s", path, TTS_FORMAT);
//		log_debug("file:%s", tmp);
//		ret = yfs_asr_tts_text(custom->c_info.option2, tmp);
//		if (ret < 0) {
//			log_error("tts error");
//		} else {
//			snprintf(custom->record[2], sizeof(custom->record[2]) - 1, "@option2");
//		}
//	}
//
//	if (custom->c_info.option3[0] != '\0') {
//		memset(tmp, 0, sizeof(tmp));
//		snprintf(tmp, sizeof(tmp) - 1, "%s/option3%s", path, TTS_FORMAT);
//		log_debug("file:%s", tmp);
//		ret = yfs_asr_tts_text(custom->c_info.option3, tmp);
//		if (ret < 0) {
//			log_error("tts error");
//		} else {
//			snprintf(custom->record[3], sizeof(custom->record[3]) - 1, "@option3");
//		}
//	}
//
//	if (custom->c_info.option4[0] != '\0') {
//		memset(tmp, 0, sizeof(tmp));
//		snprintf(tmp, sizeof(tmp) - 1, "%s/option4%s", path, TTS_FORMAT);
//		log_debug("file:%s", tmp);
//		ret = yfs_asr_tts_text(custom->c_info.option4, tmp);
//		if (ret < 0) {
//			log_error("tts error");
//		} else {
//			snprintf(custom->record[4], sizeof(custom->record[4]) - 1, "@option4");
//		}
//	}
//
//	if (custom->c_info.option5[0] != '\0') {
//		memset(tmp, 0, sizeof(tmp));
//		snprintf(tmp, sizeof(tmp) - 1, "%s/option%s", path, TTS_FORMAT);
//		log_debug("file:%s", tmp);
//		ret = yfs_asr_tts_text(custom->c_info.option5, tmp);
//		if (ret < 0) {
//			log_error("tts error");
//		} else {
//			snprintf(custom->record[5], sizeof(custom->record[5]) - 1, "@option5");
//		}
//	}

#else
    ret = sql_get_base_name(sa_info.db, custom->c_info.name, custom->record);

	if (custom->c_info.appellation[0] != '\0' && strcmp(custom->c_info.appellation, "nul") != 0) {
	    ret2 = sql_get_appeliation(sa_info.db, custom->c_info.appellation, buf);
	} else {
	    ret2 = sql_get_appeliation(sa_info.db, custom->c_info.gender, buf);
	}
	if (ret == 1) {
		if (ret2 == 1) {
			strcat(custom->record, "|");
			strcat(custom->record, buf);
		}
	} else if (ret2 == 1) {
		strcat(custom->record, buf);
	}
#endif
//	log_debug("name:%s, record: [%s]", custom->c_info.name, custom->record);

	return 0;
}

void set_customer_level(char level, char *info)
{
    if (level >= 'A' && level <= 'F') {
        custom.level = level;
    }
    if (info != NULL) {
        strcpy(custom.info, info);
    }
}

int get_hangup_status(int *status)
{
    log_debug("last ai status: [%d]", *status);
    if (sa_info.hangup_status != 0) {
        *status = sa_info.hangup_status;
    } else {
//        switch ((SA_HANGUP_STATUS)*status) {
//        case STA_SIP_480:
//        case STA_SIP_486:
//        case STA_SIP_487:
//        case STA_SIP_503:
//            sa_info.hangup_status = *status;
//            break;
//        }

        /* all of status move of 100 is network abnormal -- modify by suifeng20161103 */
        if (*status >= NETWORK_ABNORMAL_LOWER) {
            sa_info.hangup_status = *status;
        }

//        if (*status == 487) {
//            *status = STA_REJECT;
//        }
    }

    return 0;
}

int set_hangup_status(int status)
{
    sa_info.hangup_status = status;
    return 0;
}

int set_hangup_desc(char *desc)
{
    strcpy(sa_info.hangup_desc, desc);
    return 0;
}

int get_hangup_desc(char *desc)
{
    strcpy(desc, sa_info.hangup_desc);
    return 0;
}

int sa_info_init(void)
{
    /* init */
    custom.level = 'D';
    memset(custom.info, 0, sizeof(custom.info));
	if (sa_info.db){
    	sqlite3_close(sa_info.db);
		sa_info.db = NULL;
	}

    if (sa_info.hangup_status == 760) {
        //char cmd[SIZE_512]= {0};
        //snprintf(cmd,sizeof(cmd)-1, "rm -rf %s", get_db_path());
		//log_info("sa_info_init cmd %s",cmd);
		//system(cmd);
    }

	/* modify by suifeng 2017.10.17, for add auto correct word */
    if (sa_info.correct.correct_free) {
    	sa_info.correct.correct_free();
    }

    memset(&sa_info, 0, sizeof(sa_info));
//  clean_main_context_status();
    clear_context_info();
    clean_db_path();
    clean_log_path();

    return 0;
}

/***************************************************************
 *
***************************************************************/
int get_customer_level_blk(char *level)
{
    int total_score = sa_info.total_score;

    /* 增加这个是为了防止没有通过语义模块直接就挂机的情况导致评价错误 2916.12.2 随风 */
    score_threshold_t score_threshold;
    score_threshold.score_a = sa_info.score_threshold.score_a ? sa_info.score_threshold.score_a : 30;
    score_threshold.score_b = sa_info.score_threshold.score_b ? sa_info.score_threshold.score_b : 50;
    score_threshold.score_c = sa_info.score_threshold.score_c ? sa_info.score_threshold.score_c : 0;
    score_threshold.score_d = sa_info.score_threshold.score_d ? sa_info.score_threshold.score_d : 0;

    /* set level */
    if ((strcmp(sa_info.context.cur_context, "success") == 0 || sa_info.hangup_status == STA_SUCCESS) && sa_info.total_score >= score_threshold.score_a) {
        set_customer_level('A', "邀约成功");
    } else if (sa_info.total_score >= score_threshold.score_b || (strcmp(sa_info.context.cur_context, "success") == 0 || sa_info.hangup_status == STA_SUCCESS)) {
        set_customer_level('B', "大于等于50分");
    } else if ( /* 增加F类，无用号码  -- 2017.03.10 suifeng */
            sa_info.hangup_status == STA_NOT_EXIST ||
            sa_info.hangup_status == STA_IDD_SERVICE ||
            sa_info.hangup_status == STA_ADD_0 ||
            sa_info.hangup_status == STA_NOT_ADD_0 ||
            sa_info.hangup_status == STA_OUT_OF_SERVICE ||
            sa_info.hangup_status == STA_POWER_OFF ||
            sa_info.hangup_status == STA_BARRING_OF_INCOMING) {
        set_customer_level('F', "无用的号码");
    } else if (
            /* all of not before call -- modify by suifeng20170803 */
            sa_info.call_start.tv_sec == 0 && sa_info.call_start.tv_usec == 0
//            sa_info.hangup_status == STA_POWER_OFF ||
//            sa_info.hangup_status == STA_NOT_CONNECTED ||
//            sa_info.hangup_status == STA_OUT_OF_SERVICE ||
//            sa_info.hangup_status == STA_IS_BUSY_NOW ||
//            sa_info.hangup_status == STA_NOT_ANSWER ||
//            sa_info.hangup_status == STA_REJECT ||
            /* all of status move of 100 is network abnormal -- modify by suifeng20161103 */
            /* sa_info.hangup_status >= NETWORK_ABNORMAL_LOWER */) {

        set_customer_level('E', "可再次拨打");
    } else if (sa_info.total_score > score_threshold.score_c) {
        set_customer_level('C', "大于0分");
    } else {
        set_customer_level('D', "0分");
        //set_hangup_status(0);
    }

    *level = custom.level;

    log_debug("cur_context: %s", sa_info.context.cur_context);
    log_info("level: %c, score: %d", *level, sa_info.total_score);

    return total_score;
}

int get_customer_level(char *level)
{
    int total_score = sa_info.total_score;
    int i = 0;
    int ret = 1;
    /* 增加这个是为了防止没有通过语义模块直接就挂机的情况导致评价错误 2916.12.2 随风 */
    score_threshold_t score_threshold;
    score_threshold.score_a = sa_info.score_threshold.score_a ? sa_info.score_threshold.score_a : 30;
    score_threshold.score_b = sa_info.score_threshold.score_b ? sa_info.score_threshold.score_b : 50;
    score_threshold.score_c = sa_info.score_threshold.score_c ? sa_info.score_threshold.score_c : 0;
    score_threshold.score_d = sa_info.score_threshold.score_d ? sa_info.score_threshold.score_d : 0;

    /* set level */
    if ((strcmp(sa_info.context.cur_context, "success") == 0 || sa_info.hangup_status == STA_SUCCESS) && sa_info.total_score >= score_threshold.score_a) {
        set_customer_level('A', "邀约成功");
    } else if (sa_info.total_score >= score_threshold.score_b || (strcmp(sa_info.context.cur_context, "success") == 0 || sa_info.hangup_status == STA_SUCCESS)) {
        set_customer_level('B', "大于等于50分");
    }
    else if ( /* 增加F类，无用号码  -- 2017.03.10 suifeng *//*modify by pxj 20180205 report_data*/
            sa_info.hangup_status == STA_NOT_EXIST ||
            sa_info.hangup_status == STA_IDD_SERVICE ||
            sa_info.hangup_status == STA_ADD_0 ||
            sa_info.hangup_status == STA_NOT_ADD_0 ||
            sa_info.hangup_status == STA_OUT_OF_SERVICE ||
            sa_info.hangup_status == STA_POWER_OFF ||
            sa_info.hangup_status == STA_BARRING_OF_INCOMING ||
				(sa_info.call_start.tv_sec == 0 && sa_info.call_start.tv_usec == 0) ||
				sa_info.hangup_status == STA_IS_BUSY_NOW ||
				sa_info.hangup_status == STA_NOT_ANSWER ||
				sa_info.hangup_status == STA_SIP_487 ||
				sa_info.hangup_status == STA_NOT_CONNECTED ||
				sa_info.hangup_status == 500 )//pxj Unable to connect
    {
    	//modify by pxj 20180209 先判断F类，否则为E类
		for(i = 0; i < (sizeof(sa_info.score_threshold.f_level_item)/sizeof(int)); i++){
			if(sa_info.score_threshold.f_level_item[i] == 0)
			{
				ret = 1;
				break;
			}
			if(sa_info.score_threshold.f_level_item[i] == sa_info.hangup_status){
				ret = 0;
				set_customer_level('F', "自定义F类别");
				log_debug("is F level ,status = %d\n",sa_info.hangup_status);
				break;
			}
		}
		if(ret == 1)
		{
			set_customer_level('E', "自定义E类别");
			log_debug("is E level ,status = %d\n",sa_info.hangup_status);
		}
    }/*modify end*/
    else if (sa_info.total_score > score_threshold.score_c) {
        set_customer_level('C', "大于0分");
    } else {
        set_customer_level('D', "0分");
        //set_hangup_status(0);
    }

    *level = custom.level;

    log_debug("cur_context: %s", sa_info.context.cur_context);
    log_info("level: %c, score: %d", *level, sa_info.total_score);

    return total_score;
}



int count_score(int total, int value, float weight)
{
    int score = (float)total * weight + value;

    #ifndef CONFIG_TEST
    printf("(total)%d * (weight)%.2f + (value)%d = (score)%d\n", total, weight, value, score);
    #endif

    return score;
}


/***************************************************************
 * Description: 得分判断，决定是否触发后续事件
 *              返回值:0 没有结果; 1 有结果
***************************************************************/
int score_judge(event_t *event)
{
    int ret = 0;

//    printf("total score: %d", event->total_score);
//    if (event->total_score >= 60) {
//        event->continue_event = evt_sa_invite;
//        ret = 1;
//    } else {
//        ret = 0;
//    }

    return ret;
}


/***************************************************************
 * Description: 没有结果进入得分处理程序
 *              返回值: -1 异常; 0 没有结果; 1 有结果
***************************************************************/
int sa_score_process(event_t *event)
{
    int ret = 0;

    log_info("[sa_score_process] total score: %d， word: %s, answer type: %d", event->total_score, event->word.word[0], event->answer_type);

    if ((get_time_ai2human(event) == 1 && event->ivr_cnt == 0) || get_time_all_answer(event) == 1) {
    	return 1;
    }

    if (event->answer_type == type_question) {
        if (event->qustion_notfind_times >= event->question_notfind_nub[event->question_notfind_cur_floor - 1]) {
            event->qustion_notfind_times = 0;
            if (++event->question_notfind_cur_floor > event->question_notfind_floor) {
                event->question_notfind_cur_floor = 1;
            }
        }
//        event->not_memery_id = 1;
        if (event->question_notfind_cur_floor == 1) {
            sprintf(event->context.tmp_context, ".question.notfind%d", event->qustion_notfind_times);
        } else {
            sprintf(event->context.tmp_context, ".question.notfind_%d_%d", event->question_notfind_cur_floor, event->qustion_notfind_times);
        }
        ret = sql_get_response_by_context_only(event, event->context.tmp_context);

        if (ret == 1) {
            event->fr_type = fr_question_notfind;
        } else if (ret == -10) {
        	log_error("has some error!!");
        } else {
//            event->not_memery_id = 0;
            log_error("******* (Walling) .question.notfind type no data, please add! ********");
        }

     } else {
         if (event->noting_notfind_times == event->notfind_nub) {
             event->noting_notfind_times = 0;
         }
//         event->not_memery_id = 1;
         sprintf(event->context.tmp_context, ".notfind%d", event->noting_notfind_times);
         event->context.context = event->context.tmp_context;
         ret = sql_get_public_response_by_context_only(event);

         if (ret == 1) {
             event->fr_type = fr_other_notfind;
         } else if (ret == -10) {
         	log_error("has some error!!");
         } else {
//             event->not_memery_id = 0;
             log_error("******* (Walling) .notfind type no data, please add! ********");
         }
     }

    return ret;
}

