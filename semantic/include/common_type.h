#ifndef __YFS_COMMON_TYPE_H
#define __YFS_COMMON_TYPE_H

/*************************************************    
Authors:       万德新
Description:   定义常用数据类型
History:       
   1. Date:             2016/5/24
      Author:           wandexin
     Modification:      初稿

    2. ...  
*************************************************/

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif


#define YFS_FILE_NAME_SIZE      256
#define YFS_PHONE_NUMBER_SIZE   64
#define YFS_IP_STRING_SIZE      64
#define YFS_ID_STRING_SIZE      32
#define YFS_USER_NAME_SIZE      32
#define YFS_USER_PWD            16
#define YFS_URL_SIZE            1024
#define YFS_ENCRIPT_STR_SIZE    30
#define SHA256_STR_LEN          65
#define YFS_LOG_FILE_LEN        256
#define YFS_LOCATION_LEN        64
#define YFS_CMD_LEN             1024
#define YFS_AI_WORK_PATH_SIZE   32
#define AI_CFG_PATH             "/robotvoip/etc/ai_cfg.txt"
#define AI_VOICES_DIR	        "/robotvoip/var/ai_voices/"

#define SEM_FILE_BASE_COUNT         256 //产生的文件个数,用于语言识别后产生的临时文件

/*软件编译时间*/
#define COMPILE_DATE_TIME (__TIME__" "__DATE__)//hh:mm:ss mm dd yyyy
/*软件名称*/
#define SOFTWARE_NAME    "robotvoip" 
/*软件版本号: 由主版本号,次版本号,修订版本号,SVN revision四个部分组成,*/
/*该宏只定义了前三部分,这三个部分需要手工修改,最后一部分自动从SVN获取*/
/* 此处每次发布版本要手动更新  */
#define VERSION "1.1.0"

/*版本说明*/
#define VERSION_DES "Bate"  /*Bate or Release*/

/*svn版本号,由makefile生成到svn_version.c中*/
extern int g_nSvnVersion;




#define SAFE_FREE(ptr) \
{ \
    if (NULL != ptr) { \
        free(ptr); \
        ptr = NULL; \
    } \
}

extern char g_encript_user[];
extern char g_encript_pwd[];
//extern char g_location[];

extern int memory_Info(char* Info, int len);

/**  
* @Description:获取当前ai工作目录
* @return 成功:!NULL 失败:NULL
*/
char *get_ai_work_dir();

/**  
* @Description:输了版本信息
* @return 无 
*/
void yfs_display_software_version();

int get_ai_sip_domain(char *domain, int d_len);

#endif
