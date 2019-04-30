/***************************************************************
 * Author: MaJi
 * Date: 2016-05-26
 * History:
 * Description: 语义分析相关函数
***************************************************************/ 
#ifndef __SEMANTIC__H
#define __SEMANTIC__H

#include <sys/time.h>

/* version */
//#define VERSION_ONE
#define VERSION_TWO

#define WORD_SIZE 512 
#define FILENAME_SIZE 256
#define FILE_MAX_COUNT 30
#define EXTENSION_SIZE  10

#define ERROR_ANSWER -1
#define VALID_ANSWER 0
#define READ_ANSWER 1
#define CLOSE_ANSWER 2
#define CLEAN_ANSWER 3
#define KEEP_ANSWER 4
#define SKIP_OPERATOR_ANSWER   5
#define SKIP_IVR_ANSWER        6

#define MAX_QUESTION 30
#define UPDATE_ALL  2
#define UPDATE_ONE  1
#define MATCH_MAX_COUNT 100
#define WITH_SPEECH     1
#define WITH_CONNECT    2
#define WITH_TIMEOUT    3
#define WITH_SESSION    4
#define WITH_SESSION_FAIL    5
#define WITH_183		6
#define WITH_PRE_SESSION    7

#define CONTEXT_NAME_SIZE 128

#define CONTEXT_NUMBER      10
#define CONTEXT_SIZE        64

#define FILE_MAX_COUNT  30
#define WORD_MAX_COUNT  12

#define YFS_HANGUP_DESC_LEN     128

#define MEDIA_PLAY_CONTEXT_INTERVAL     100 /*time out ms*/
#define SEC_0_5         (int)((float)500 / MEDIA_PLAY_CONTEXT_INTERVAL)
#define SEC_1_0         (int)((float)1000 / MEDIA_PLAY_CONTEXT_INTERVAL)
#define SEC_1_5         (int)((float)1500 / MEDIA_PLAY_CONTEXT_INTERVAL)
#define SEC_2_0         (int)((float)2000 / MEDIA_PLAY_CONTEXT_INTERVAL)
#define SEC_2_5         (int)((float)2500 / MEDIA_PLAY_CONTEXT_INTERVAL)
#define SEC_3_0         (int)((float)3000 / MEDIA_PLAY_CONTEXT_INTERVAL)
#define SEC_3_5         (int)((float)3500 / MEDIA_PLAY_CONTEXT_INTERVAL)
#define SEC_4_0         (int)((float)4000 / MEDIA_PLAY_CONTEXT_INTERVAL)
#define SEC_4_5         (int)((float)4500 / MEDIA_PLAY_CONTEXT_INTERVAL)
#define SEC_5_0         (int)((float)5000 / MEDIA_PLAY_CONTEXT_INTERVAL)
#define SEC_5_5         (int)((float)5500 / MEDIA_PLAY_CONTEXT_INTERVAL)
#define SEC_6_0         (int)((float)6000 / MEDIA_PLAY_CONTEXT_INTERVAL)
#define SEC_6_5         (int)((float)6500 / MEDIA_PLAY_CONTEXT_INTERVAL)
#define SEC_7_0         (int)((float)7000 / MEDIA_PLAY_CONTEXT_INTERVAL)
#define SEC_7_5         (int)((float)7500 / MEDIA_PLAY_CONTEXT_INTERVAL)
#define SEC_8_0         (int)((float)8000 / MEDIA_PLAY_CONTEXT_INTERVAL)
#define SEC_8_5         (int)((float)8500 / MEDIA_PLAY_CONTEXT_INTERVAL)
#define SEC_9_0         (int)((float)9000 / MEDIA_PLAY_CONTEXT_INTERVAL)
#define SEC_9_5         (int)((float)9500 / MEDIA_PLAY_CONTEXT_INTERVAL)
#define SEC_10          (int)((float)10000 / MEDIA_PLAY_CONTEXT_INTERVAL)

#define NAME_SIZE		128
#define OPTION_SIZE		13

typedef struct _context 
{
    char *context; /*语境指针*/
    char cur_context[CONTEXT_SIZE]; /*保存当前语境*/
    char parent_context[CONTEXT_SIZE]; /*保存当前大语境*/
    char previous_context[CONTEXT_SIZE]; /*保存上一级语境*/
    char context_name[CONTEXT_SIZE]; /*保存从数据库里面读出来的当前语境*/
    char next_context[CONTEXT_SIZE]; /*保存从数据库里读出来下一语境*/
    char tmp_context[CONTEXT_SIZE]; /*中间语境，用于查询*/
    char skip_condition[CONTEXT_SIZE]; /*跳转条件*/
    char skip_to[CONTEXT_SIZE]; /*跳转到哪里*/
    int  response_id; /*记录查询到的语句ID*/
    int  words_score; /*记录查询到的语句分数*/
    double wight;
    int  question_flag;
    char level;
    int  hangup_status;
    int  priority;
    char record[512];
} context_t;


typedef struct semantic_answer
{
    char word[WORD_SIZE];
    int type;
	char record_file[FILENAME_SIZE];
    context_t context;
    struct timeval start;
    struct timeval end;
    int duration;
    char extension[EXTENSION_SIZE];
    char corrected_word[WORD_SIZE];/* modify by suifeng 2017.10.17, for add auto correct word */
    float correct_distance;
} semantic_answer_t;


typedef struct _custom_info
{
    char name[32];
    char gender[8];
    char appellation[32];
    char option[OPTION_SIZE][NAME_SIZE];
    char option2[64];
    char option3[64];
    char option4[64];
    char option5[64];
} custom_info_t;

typedef enum 
{
	SET_DB_PATH,
	SET_UP_CONTEXT,
	SET_PLAY_END_STATUS,
	SET_CUSTOM_INFO,
	SET_LOG_PATH,
	SET_MICON,
	SET_AI_VERSION,
	SET_SA_INFO,
	GET_HANGUP_STATUS,
	GET_HANGUP_DESC,
	GET_LEVEL,
	GET_RECORD_FILE,
	GET_DB_NAME,
	GET_AI_VERSION_OF_DB_DEPEND,
	GET_AREA,
	GET_INDUSTRY,
} SA_OPERATION_TYPE;

typedef enum 
{
    STA_SUCCESS             = 30, /*成功，请发短信*/
    STA_SEND_MSG            = 31, /*客户要求发短信，请跟踪*/
    STA_QES_NOTFIND         = 32, /*问题未能回答，请发短信并回电*/
    STA_BUSY                = 33, /*客户不方便接听，请发短信*/
    STA_ANS_TIMEOUT         = 34, /*回答超时挂机*/
    STA_SIL_TIMEOUT         = 35, /*静默超时挂机*/
    STA_NOT_EXIST           = 36, /*空号*/
    STA_POWER_OFF           = 37, /*关机*/
    STA_NOT_CONNECTED       = 38, /*无法接通*/
    STA_OUT_OF_SERVICE      = 39, /*停机*/
    STA_IS_BUSY_NOW         = 40, /*正在通话中 -- 占线*/
    STA_BARRING_OF_INCOMING = 41, /*呼入限制*/
    STA_IDD_SERVICE         = 42, /*国际长途*/
    STA_NOT_ANSWER          = 43, /*无人接听*/
    STA_ADD_0               = 44, /*外地号码请加0*/
    STA_NOT_ADD_0           = 45, /*本地号码请不要加0*/
    STA_REJECT              = 46, /*拒接*/
    STA_REFUSE              = 47, /*不感兴趣*/
    STA_DIAL_EXTENSION      = 48, /*请拨分机*/
    STA_NO_TIME             = 49, /*没有时间*/
    STA_MAINCALL_PUASE      = 55, /*主叫暂停服务*/
    STA_SIP_480             = 480,/*Temporarily Unavailable -- 转义为 无人接听*/
    STA_SIP_486             = 486,/*Busy Here -- 转义为 占线*/
    STA_SIP_487             = 487,/*Request Terminated 请求被BYE或者CANCEL所终止 -- 转义为 拒接*/
    STA_SIP_503             = 503,/*Service Unavailable */
} SA_HANGUP_STATUS;

typedef union
{
	char *db_path; /* set db path */
	context_t context; /* set context */
	custom_info_t custom_info; /* set customer info */
	char *log_path; /* set log path */
	char ai_version[24]; /* current bin version, such as 1.2 */
	char *micon; /* set micon */
} sa_set_para_t;


typedef union
{
	struct
   	{
		char (*filename)[FILENAME_SIZE]; /* output filenames */
		int *sum; /* output files number */
		char *word; /* input word */
	} p_to_record; /* word to record parameter */
	char level; /* customer level */
	int hg_status; /* hang up status */
	char hg_desc[YFS_HANGUP_DESC_LEN]; /*  hang up describe add by SuiFeng 2016.11.25 */
	char db_name[64]; /* template name */
	char ai_version_of_db_depend[24]; /* such as 1.2 */
    char industry[36];
    char area[36];
} sa_get_para_t;

typedef semantic_answer_t semantic_request_t;

int set_semantic_analyzer(SA_OPERATION_TYPE opt_type, sa_set_para_t *para);

int get_semantic_analyzer(SA_OPERATION_TYPE opt_type, sa_get_para_t *para);

int semantic_analyzer(semantic_request_t *request, semantic_answer_t *response);

#endif
