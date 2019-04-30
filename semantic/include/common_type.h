#ifndef __YFS_COMMON_TYPE_H
#define __YFS_COMMON_TYPE_H

/*************************************************    
Authors:       �����
Description:   ���峣����������
History:       
   1. Date:             2016/5/24
      Author:           wandexin
     Modification:      ����

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

#define SEM_FILE_BASE_COUNT         256 //�������ļ�����,��������ʶ����������ʱ�ļ�

/*�������ʱ��*/
#define COMPILE_DATE_TIME (__TIME__" "__DATE__)//hh:mm:ss mm dd yyyy
/*�������*/
#define SOFTWARE_NAME    "robotvoip" 
/*����汾��: �����汾��,�ΰ汾��,�޶��汾��,SVN revision�ĸ��������,*/
/*�ú�ֻ������ǰ������,������������Ҫ�ֹ��޸�,���һ�����Զ���SVN��ȡ*/
/* �˴�ÿ�η����汾Ҫ�ֶ�����  */
#define VERSION "1.1.0"

/*�汾˵��*/
#define VERSION_DES "Bate"  /*Bate or Release*/

/*svn�汾��,��makefile���ɵ�svn_version.c��*/
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
* @Description:��ȡ��ǰai����Ŀ¼
* @return �ɹ�:!NULL ʧ��:NULL
*/
char *get_ai_work_dir();

/**  
* @Description:���˰汾��Ϣ
* @return �� 
*/
void yfs_display_software_version();

int get_ai_sip_domain(char *domain, int d_len);

#endif
