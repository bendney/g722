#ifndef __YFS_CONFIG_PARSER_H
#define __YFS_CONFIG_PATSER_H

typedef struct yfs_config_dict_tag{
    char *key;
    char *value;
    struct yfs_config_dict_tag *next;
}YFS_CONFIG_DICT_S;

/*
* @descript :从配置文件中加载配置信息
* @args :
*   config_file :配置文件名(入参)
*   dict        :用于存放配置信息的结构体，不需要预先分配内存(出参)
* @return :
*   成功        :0
*   失败        :-1
*/
int config_parser_load(const char *config_file, YFS_CONFIG_DICT_S **dict);

/*
* @descript:以字符串的方式获取配置内容
* @args:
*   dict    :配置信息从该结构体中获取(入参)
*   key     :需要获取的配置信息的键(入参)
* @return:
*   成功    :返回配置信息
*   失败    :返回NULL
*/
char *config_parser_getstring(const YFS_CONFIG_DICT_S *dict, const char *key);

/*
* @descript:以整型的方式获取配置内容
* @args:
*   dict    :配置信息从该结构体中获取(入参)
*   key     :需要获取的配置信息的键(入参)
*   value   :配置信息由此返回(出参)
* @return:
*   成功    :0
*   失败    :-1
*/
int config_parser_getint(const YFS_CONFIG_DICT_S *dict, const char *key, int *value);

/*
* @descript:释放用于存放配置信息的结构体
* @args:
*   dict    :存放配置信息的结构体
* @return:
*/
void config_parser_unload(YFS_CONFIG_DICT_S *dict);

/*
* @descript:以字符串的方式直接从配置文件中获取配置内容
* @args:
*   config_file :配置文件名(入参)
*   key         :需要获取的配置信息的键(入参)
*   value       :根据键获取的值(出参)
* @return:
*   成功        :0
*   失败        :-1
*/
int config_parser_getstring_fromfile(const char *config_file, const char *key, char **value);

/*
* @descript:以字符串的方式直接从配置文件中获取配置内容
* @args:
*   config_file :配置文件名(入参)
*   key         :需要获取的配置信息的键(入参)
*   value       :根据键获取的值(出参)
* @return:
*   成功        :0
*   失败        :-1
*/
int config_parser_getint_fromfile(const char *config_file, const char *key, int *value);



#endif
