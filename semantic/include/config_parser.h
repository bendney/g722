#ifndef __YFS_CONFIG_PARSER_H
#define __YFS_CONFIG_PATSER_H

typedef struct yfs_config_dict_tag{
    char *key;
    char *value;
    struct yfs_config_dict_tag *next;
}YFS_CONFIG_DICT_S;

/*
* @descript :�������ļ��м���������Ϣ
* @args :
*   config_file :�����ļ���(���)
*   dict        :���ڴ��������Ϣ�Ľṹ�壬����ҪԤ�ȷ����ڴ�(����)
* @return :
*   �ɹ�        :0
*   ʧ��        :-1
*/
int config_parser_load(const char *config_file, YFS_CONFIG_DICT_S **dict);

/*
* @descript:���ַ����ķ�ʽ��ȡ��������
* @args:
*   dict    :������Ϣ�Ӹýṹ���л�ȡ(���)
*   key     :��Ҫ��ȡ��������Ϣ�ļ�(���)
* @return:
*   �ɹ�    :����������Ϣ
*   ʧ��    :����NULL
*/
char *config_parser_getstring(const YFS_CONFIG_DICT_S *dict, const char *key);

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
int config_parser_getint(const YFS_CONFIG_DICT_S *dict, const char *key, int *value);

/*
* @descript:�ͷ����ڴ��������Ϣ�Ľṹ��
* @args:
*   dict    :���������Ϣ�Ľṹ��
* @return:
*/
void config_parser_unload(YFS_CONFIG_DICT_S *dict);

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
int config_parser_getstring_fromfile(const char *config_file, const char *key, char **value);

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
int config_parser_getint_fromfile(const char *config_file, const char *key, int *value);



#endif
