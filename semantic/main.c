#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "semantic_analyzer.h"

struct info 
{
    int status;
    char info[256];
};

struct info info[6] =
{
	{30, "邀约成功，请发短信"},
	{31, "客户要求发短信，请跟踪"},
	{32, "问题未能回答，请发短信并回电"},
	{33, "客户不方便接听，请发短信，可再次加入拔打计划"},
	{34, "回答超时挂机"},
	{35, "静默超时挂机"}
};


//YFS_LOG_LEVEL_E g_debug = DBG_ALL;
YFS_LOG_RECORD_E g_record;

int yfs_is_ars_busy(void)
{
    return 0;
}

int yfs_asr_tts_text(const char *text, char * out_file)
{
	return 0;
}

custom_info_t cst_info = {
		"赵子龙",
		"先生",
};


int main(int argc, char *argv[])
{
//	char databasePath[128] = "/usr/src/mms/Man-machine-switching/bin/jqr";
	char databasePath[128] = "/usr/src/mms/Man-machine-switching/bin/dyxgx170627";

    fd_set read_set;
    int cnt = 0;
    struct timeval tmval;
    //int i = 0;
    int templet = 0;

    sa_set_para_t inParams;
    sa_get_para_t outParams;

    semantic_request_t request;
    semantic_answer_t answer;

    inParams.db_path = databasePath;

    if (argc > 2 && getopt(argc,argv,"t:") != -1) {
        inParams.db_path = optarg;
        templet = 1;
    }

	/*
	if (inParams.db_path == NULL) {
        if (config_parser_getstring_fromfile(AI_CFG_PATH, "default_template_dir", &inParams.db_path)){
            log_error("config_parser_getstring_fromfile(%s, default_template_dir) fail\r\n",AI_CFG_PATH);
            exit(0);
        }
    }
	*/

    memset(request.word, 0x0, sizeof(request.word));
    request.type = WITH_183;

    set_semantic_analyzer(SET_DB_PATH, &inParams);

    //inParams.custom_info = cst_info;
    //set_semantic_analyzer(SET_CUSTOM_INFO, &inParams);

    //strcpy(inParams.ai_version, "1.2.2");
    //set_semantic_analyzer(SET_AI_VERSION, &inParams);

    int keyboard = open("/dev/tty", O_RDONLY | O_NONBLOCK);

    while (1) {
        semantic_analyzer(&request, &answer);
        if (strcmp(request.word, "quit") == 0) {
			break;
		}

		if (answer.type == CLOSE_ANSWER) {
			printf("Close answer content %s \n", answer.word);
			printf("Close answer record_file %s \n", answer.context.record);
            break;
        } else if (answer.type == READ_ANSWER) {
			printf("answer content %s \n", answer.word);
			printf("answer record_file %s \n", answer.context.record);
		}

		/* Update session context as next round params */
        if (answer.type != KEEP_ANSWER) {
        	inParams.context = answer.context;
        	set_semantic_analyzer(SET_UP_CONTEXT, &inParams);
        	set_semantic_analyzer(SET_PLAY_END_STATUS, NULL);
        }

        tmval.tv_sec = 0;
        tmval.tv_usec = 400000;

        FD_ZERO(&read_set);
        FD_SET(keyboard, &read_set);
        cnt = select(keyboard + 1, &read_set, NULL, NULL, &tmval);
        if (cnt > 0) {
            if (FD_ISSET(keyboard, &read_set)) {
                memset(request.word, 0x0, sizeof(request.word));
                read(keyboard, request.word, sizeof(request.word));
                if (request.word[strlen(request.word) - 1] == '\n') {
                    request.word[strlen(request.word) - 1] = '\0';
                }
                printf("用户输入: %s\n", request.word);

                request.type = WITH_SPEECH;
            }
        } else {
            request.type = WITH_TIMEOUT;
        }

		gettimeofday(&request.start, NULL);
		gettimeofday(&request.end, NULL);
    }

    /*
    printf("ai version: %s\n", get_ai_version());
	*/
    get_semantic_analyzer(GET_AI_VERSION_OF_DB_DEPEND, &outParams);
    printf("Dialog database version: %s\n", outParams.ai_version_of_db_depend);
    get_semantic_analyzer(GET_DB_NAME, &outParams);
    printf("Dialog database name : %s\n", outParams.db_name);

    printf("------------------------\n");

    outParams.hg_status = 0;
    get_semantic_analyzer(GET_HANGUP_STATUS, &outParams);
    int score = get_semantic_analyzer(GET_LEVEL, &outParams);
    printf("[Custom Info] score: %d\n", score);
    printf("[Custom Info] level: %c\n", outParams.level);
    printf("[Custom Info] hangup: %d\n", outParams.hg_status);
#if 0
    for (i = 0; i < sizeof(info) / sizeof(info[0]); i++) {
        if (outParams.hg_status == info[i].status) {
            printf(" hang up reason: %s\n", info[i].info);
        }
    }
#endif
    printf("------------------------\n");

    if (templet == 0) {
        //log_info("word path: %s\n", inParams.db_path);
        //free(inParams.db_path);
    }

    return 0;
}


