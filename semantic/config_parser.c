#include <stdio.h>
#include <json-c/json.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "logger.h"
#include "common_type.h"
#include "config_parser.h"

static unsigned long get_file_size(const char *file)
{
    unsigned long filesize = 0;
    struct stat file_stat = {0};
    if (NULL == file)
        return 0;

    if (stat(file, &file_stat) < 0)
        return 0;
    else
        filesize = file_stat.st_size;

    return filesize;
}

static int read_from_file(const char *file, char *buff, unsigned int buff_len)
{
    FILE *fp;
    int tmp = 0;
    size_t nbytes = 0;

    if ((fp = fopen(file, "r")) == NULL) {
        log_error("open config file failed\n");
        return -1;
    }

    while (!feof(fp)) {
        nbytes = fread(buff + tmp, 1, buff_len, fp);
        if (ferror(fp)) {
            log_error("an error occured when read from config file\n");
            clearerr(fp);
            fclose(fp);
            return -1;
        }
//        log_debug("nbytes :%u\n", nbytes);
        tmp += nbytes;
    }
    fclose(fp);
    return 0;
}

static int parse_config_content(const char *cfg_buff, YFS_CONFIG_DICT_S **dict)
{
    struct json_object *json_obj = NULL;
    struct json_object_iter iter;
    YFS_CONFIG_DICT_S *head = NULL;
    
    if (NULL == cfg_buff) {
        return -1;
    }

    json_obj = json_tokener_parse(cfg_buff);
    json_object_object_foreachC(json_obj, iter) {
        const char *val = json_object_get_string(iter.val);
        YFS_CONFIG_DICT_S *node = (YFS_CONFIG_DICT_S*)malloc(sizeof(YFS_CONFIG_DICT_S));
        if (NULL == node){
            json_object_put(json_obj);
            return -1;
        }
        
        node->key = (char *)malloc(strlen(iter.key)+1);
        if (NULL == node->key){
            SAFE_FREE(node);
            json_object_put(json_obj);
            return -1;
        }

        node->value = (char *)malloc(strlen(val)+1);
        if (NULL == node->value){
            SAFE_FREE(node->key);
            SAFE_FREE(node);
            json_object_put(json_obj);
            return -1;
        }
        
        strncpy(node->key, iter.key, strlen(iter.key)+1);
        strncpy(node->value, val, strlen(val)+1);
        node->next = NULL;
        if (NULL == *dict)
            *dict = node;
        else {
            (*dict)->next = node;
            *dict = (*dict)->next;
        }
        if (NULL == head)
            head = *dict;
    }
    *dict = head;

    json_object_put(json_obj);
    
    return 0;
}

static int string_is_digit(char *str)
{
    char *tmp = str;
    while (*tmp != '\0') {
        if (*tmp < '0' || *tmp > '9')
            return -1;
        ++tmp;
    }

    return 0;
}

/*
* @descript :�������ļ��м���������Ϣ
* @args :
*   config_file :�����ļ���(���)
*   dict        :���ڴ��������Ϣ�Ľṹ��(����)
* @return :
*   �ɹ�        :0
*   ʧ��        :-1
*/
int  config_parser_load(const char *config_file, YFS_CONFIG_DICT_S **dict)
{
    char *cfg_file_buf = NULL;
    unsigned long filesize = 0;

    if (NULL == config_file) {
        log_error("config file is NULL\n");
        return -1;
    }

    if ((filesize = get_file_size(config_file)) == 0) {
        log_error("config file is empty\n");
        return -1;
    }

    if ((cfg_file_buf = (char *)malloc(filesize+1)) == NULL) {
        log_error("malloc config file buff failed\n");
        return -1;
    }

    if (read_from_file(config_file, cfg_file_buf, filesize) != 0) {
        log_error("read from config failed\n");
        SAFE_FREE(cfg_file_buf);
        return -1;
    }

    if (parse_config_content(cfg_file_buf, dict) != 0) {
        log_error("parse config buff failed\n");
        SAFE_FREE(cfg_file_buf);
        return -1;
    }

    SAFE_FREE(cfg_file_buf);
    
    return 0;
}

/*
* @descript:���ַ����ķ�ʽ��ȡ��������
* @args:
*   dict    :������Ϣ�Ӹýṹ���л�ȡ(���)
*   key     :��Ҫ��ȡ��������Ϣ�ļ�(���)
* @return:
*   �ɹ�    :����������Ϣ
*   ʧ��    :����NULL
*/
char * config_parser_getstring(const YFS_CONFIG_DICT_S *dict, const char *key)
{
    if (dict == NULL) {
        log_error("dict is null \n");
        return NULL;
    }
    while (dict != NULL) {
        if (strncmp(key, dict->key, strlen(dict->key)+1) == 0) {
            return dict->value;
        }
        dict = dict->next;
    }

    return NULL;
}

/*
* @descript:���ַ����ķ�ʽֱ�Ӵ������ļ��л�ȡ�������ݣ�
* @args:
*   config_file :�����ļ���(���)
*   key         :��Ҫ��ȡ��������Ϣ�ļ�(���)
*   value       :���ݼ���ȡ��ֵ(����)��ʹ�ú���Ҫ�ͷ�
* @return:
*   �ɹ�        :0
*   ʧ��        :-1
*/
int  config_parser_getstring_fromfile(const char *config_file, const char *key, char **value)
{
    char *cfg_file_buf = NULL;
    unsigned long filesize = 0;
    struct json_object *json_obj = NULL;
    struct json_object *value_obj = NULL;
    char *tmp_value = NULL;
    size_t value_len = 0;

    if (NULL == config_file) {
        log_error("config file is NULL");
        return -1;
    }

    if ((filesize = get_file_size(config_file)) == 0) {
        log_error("config file is empty");
        return -1;
    }
//  log_debug("file size :%d\n", filesize);
    
    if ((cfg_file_buf = (char *)malloc(filesize+1)) == NULL) {
        log_error("malloc config file buff failed");
        return -1;
    }
    
	memset(cfg_file_buf, 0, filesize+1);

    if (read_from_file(config_file, cfg_file_buf, filesize) != 0) {
        log_error("read from config failed");
        SAFE_FREE(cfg_file_buf);
        return -1;
    }
//  log_debug("json_str :%s\n", cfg_file_buf);
    if ((json_obj = json_tokener_parse(cfg_file_buf)) == NULL) {
        log_error("parser json failed");
        SAFE_FREE(cfg_file_buf);
        return -1;
    }
    
    if (!json_object_object_get_ex(json_obj, key, &value_obj)) {
        log_error("get state failed");
        json_object_put(json_obj);
        SAFE_FREE(cfg_file_buf);
        return -1;
    }

    tmp_value = (char *)json_object_get_string(value_obj);
    value_len = strlen(tmp_value)+1;
    *value = (char *)malloc(value_len);
    if ((*value) == NULL) {
        log_error("malloc value failed");
        json_object_put(json_obj);
        SAFE_FREE(cfg_file_buf);
        return -1;
    }
    snprintf(*value, value_len, "%s", tmp_value);
//  log_debug("value :%s\n", *value);

    json_object_put(json_obj);
    SAFE_FREE(cfg_file_buf);
    
    return 0;
}

/*
* @descript:���ַ����ķ�ʽֱ�Ӵ������ļ��л�ȡ��������
* @args:
*   config_file :�����ļ���(���)
*   key         :��Ҫ��ȡ��������Ϣ�ļ�(���)
*   value       :���ݼ���ȡ��ֵ(����)
* @return:
*   �ɹ�        :0
*   ʧ��        :-1
*/
int  config_parser_getint_fromfile(const char *config_file, const char *key, int *value)
{
    char *tmp_val = NULL;
    if ( config_parser_getstring_fromfile(config_file, key, &tmp_val) != 0) {
        log_error("get string config from file failed\n");
        return -1;
    }

    if (string_is_digit(tmp_val))
        return -1;

    *value = atoi(tmp_val);
    SAFE_FREE(tmp_val);

    return 0;
}

/*
* @descript:�����͵ķ�ʽ��ȡ��������
* @args:
*   dict    :������Ϣ�Ӹýṹ���л�ȡ(���)
*   key     :��Ҫ��ȡ��������Ϣ�ļ�(���)
*   value   :������Ϣ�ɴ˷���(����)
* @return:
*   �ɹ�    :0
*   ʧ��    :-1
*/
int  config_parser_getint(const YFS_CONFIG_DICT_S *dict, const char *key, int *value)
{
    char *tmp_val =  config_parser_getstring(dict, key);

    if (string_is_digit(tmp_val))
        return -1;
    
    *value = atoi(tmp_val);
    return 0;
}

/*
* @descript:�ͷ����ڴ��������Ϣ�Ľṹ��
* @args:
*   dict    :���������Ϣ�Ľṹ��
* @return:
*/
void  config_parser_unload(YFS_CONFIG_DICT_S *dict)
{
    while (dict != NULL) {
        YFS_CONFIG_DICT_S *tmp = dict;
        
        if (tmp->key != NULL)
            SAFE_FREE(tmp->key);
        if (tmp->value != NULL)
            SAFE_FREE(tmp->value);
        
        dict = tmp->next;
        SAFE_FREE(tmp);
    }
}
