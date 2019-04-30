#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sqlcipher/sqlite3.h>

#include "sa_sql.h"


/***************************************************************
 *
***************************************************************/
static int cmp_word_by_reg(char *src, char *key, int itype)
{
    int i = 0;
    char buf[32][1024];
    int pb = 0;
    int pk = 0;
    char *pre = NULL;
    char *current = NULL;
    char temp[128];

    if(src == NULL || src[0] == '\0'){
        fprintf(stderr,"[%s][%d]word is empty!\n",__FILE__, __LINE__);
        return -2; 		/* modify by suifeng 2017.07.19 */
    }
    if(key == NULL || key[0] == '\0'){
        fprintf(stderr,"[%s][%d]key_word is empty!\n",__FILE__, __LINE__);
        return -1;
    }

//    printf("src %s, len %d, key %s, len %d\n", src, strlen(src), key, strlen(key));

    memset(buf,0x0,sizeof(buf));
    while(1){
        if(key[pk] == '\0'){
            buf[i][pb] = '\0';
            if(key[pk-1] != ' '){
                i++;
            }
            break;
        }
        buf[i][pb] = key[pk];
        if(buf[i][pb] == ' '){
            buf[i][pb] = '\0';
            if(pb != 0){
                i++;
            }
            pb = 0;
            pk++;
            continue;
        }
        pb++;
        pk++;
    }

//    printf("buf: %s, len %d, i %d\n", buf[0], strlen(buf), i);

    memset(temp,0x0,sizeof(temp));
    for(pb = 0; pb < i; pb++){
        current = strstr(src,buf[pb]);
        if(buf[pb][0] != '*'){
            if(current < pre || current == NULL){
//                printf("flag\n");
                return -1;
            }
        }
        if(buf[pb][0] == '*'){
            strcat(temp,buf[pb]);
            strcat(temp,buf[pb]);
        }
        strcat(temp,buf[pb]);
        pre = current + strlen(buf[pb]);
    }
    pk = strlen(temp);


//    fprintf(stderr,"src:[%s],key:[%s],len:[%d],pk:[%d]",src,key,strlen(src),pk);
    if(strlen(src) == pk)
    {
        log_info("完全匹配");
        return 0;
    }
    else
    {
        if (strlen(key) == 3) {
            if(itype != 4){
//                printf("falg2\n", pk);
                return -1;
            }
        }
//        printf("----pk len: %d\n", pk);
        return pk;
    }
}

/***************************************************************
 *
***************************************************************/
static int replace_word(char *result, char *key, char *word)
{   
    int i = 0;
    char buf[1024];
    int pw = 0;
    int pb = 0;
    int len = 0; 
    int word_len = 0;
    char *word_adr = NULL;
    int ae_len = 0;

    if(result == NULL || result[0] == '\0'){
        return -1;
    }
    if(key == NULL || key[0] == '\0'){
        return -1;
    }
    if(word == NULL || word[0] == '\0'){
        return -1;
    }

    len = strlen(result);
    word_len = strlen(word);
    if((word_adr=strstr(result,key)) != NULL){
        ae_len = word_adr - result;
        for(i=0; i<len; i++){
            buf[pb] = result[i];
            if(i == ae_len){
                for(pw=0; pw<word_len; pw++){
                    buf[pb] = word[pw];
                    pb++;
                }
                pb = pb - 1;
                i = i + strlen(key) - 1;
            }
            pb++;
        }
        buf[pb] = '\0';
    }
    else{
        return -1;
    }
    sprintf(result,"%s",buf);
    return 0;
}

static int Cancel_Symbol(char *res, char *src)
{
#if 1 //从后往前查找，只会去掉最后面的标点符号
	char symbol[32][4]={"，", "。", "？", "！" ,"、"};
	//char symbols[64] = ", . ? ! ， 。 ？ ！ 、";
	int nums = 5;
	char buf[4];
	int len = 0;
	int i = 0;
	//int symbol_len = 0;

	if(res == NULL || res[0] == '\0' || src == NULL){
		return -1;
	}

	len = strlen(res);
	if(len < 3){
		//src[0] = '\0';
		return 0;  /* modify by suifeng (2018.06.13) */
	}

	buf[0] = res[len - 3];
	buf[1] = res[len - 2];
	buf[2] = res[len - 1];
	buf[3] = '\0';

//	printf("buf:(%s), len:%d\n", buf, strlen(buf));

	for(i=0; i<nums; i++){
//		symbol_len = strlen(symbol[i]);
//		printf("symbol len: %d\n", symbol_len);
//		if (symbol_len == 1) {
//			buf[0] = res[len - 1];
//			buf[1] = '\0';
//			printf("symbol: %d\n", buf);
//			if(strcmp(symbol[i],buf) == 0){
//				printf("this symbol: %d\n", symbol[i]);
//				res[len-1] = '\0';
//				break;
//			}
//		} else if (symbol_len == 3) {
//			buf[0] = res[len - 3];
//			buf[1] = res[len - 2];
//			buf[2] = res[len - 1];
//			buf[3] = '\0';
//			if(strcmp(symbol[i],buf) == 0){
//				res[len-3] = '\0';
//				break;
//			}
//		}
		if(strcmp(symbol[i],buf) == 0){
			res[len-3] = '\0';
			break;
		}
	}

	/*add by xyq
	 * 去除倒数第二个标点符号
	 * */
	len = strlen(res);
	if(len < 3){
		//src[0] = '\0';
		return 0; /* modify by suifeng (2018.07.19) */
	}

	buf[0] = res[len - 3];
	buf[1] = res[len - 2];
	buf[2] = res[len - 1];
	buf[3] = '\0';

	for(i=0; i<nums; i++){
		if(strcmp(symbol[i],buf) == 0){
			res[len-3] = '\0';
			break;
		}
	}
	sprintf(src,"%s",res);
#else
	char symbol[32][4]={"，", " ",
			"。",
			"？",
			"！",
			"、",
			","};
	int nums = 6;
//	char buf[3];
	int len = 0;
	int i = 0;
	char * p;

	if(res == NULL || res[0] == '\0' || src == NULL){
		return -1;
	}


	len = strlen(res);
	if(len < 2){
		//src[0] = '\0';
		return -1;
	}

	for(i=0; i<nums; i++){
		p = strstr(src, symbol[i]);
		if( p != NULL && (strlen(src) - (p - src) == strlen(symbol[i]))){
			p[0] = '\0';
			break;
		}
	}
#endif

	return 0;
}


int analy_word(char (*word)[WORD_SIZE], int *sum, char *src)
{
    int len;
    int i = 0;
    int pf = 0;
    int pfi = 0;

    if(src == NULL || src[0] == '\0'){
        return -1;
    }
    if(word == NULL || sum == NULL){
        return -1;
    }


    len = strlen(src);
    for(i = 0; i < len; i++){
        word[pf][pfi] = src[i];
        if(word[pf][pfi] == '|' || word[pf][pfi] == '|'){
            word[pf][pfi] = '\0';
            pf++;
            pfi = -1;
        }
        pfi++;
        if(pf == WORD_MAX_COUNT){
            break;
        }
    }
//    word[pf][pfi+1] = ',';
    word[pf][pfi+1] = '\0';
    *sum = pf+1;
//    printf("len %d\n",strlen(word[0]));
    return 0;
}


/***************************************************************
 *
***************************************************************/
int normative_word(char *result, char *src, sqlite3 *db)
{   
    //int i = 0;
    char sql[256];
    int ret = 0; 
    sqlite3_stmt *statement = NULL;
    char *condition = NULL;
    char *word = NULL;
    char buf[526];

    if(src == NULL || src[0] == '\0'){
        return -1;
    }

//    printf("src: %s\n", src);

    memset(buf,0x0,sizeof(buf));
    sprintf(buf,"%s",src);

    memset(sql,0x0,sizeof(sql));
    sprintf(sql,"select ID, CONDITION, WORD from NORMATIVE_WORD order by ID asc");

    ret = sqlite3_prepare(db,sql,-1,&statement, NULL);
    if(ret != SQLITE_OK){
        sqlite3_finalize(statement);
        return -1;
    }

    while(sqlite3_step(statement) == SQLITE_ROW){
        sqlite3_column_int(statement,0);
//        printf("****id[%d]*****\n",id);
        condition = (char *)sqlite3_column_text(statement,1);
        word = (char *)sqlite3_column_text(statement,2);
        ret = replace_word(buf,condition,word);
        if(ret != -1){
            sprintf(result,"%s",buf);
        }
    }
    sprintf(result,"%s",buf);

//    printf("result: %s\n", result);
    sqlite3_finalize(statement);
    return 0;

}

#if 0
/***************************************************************
 *
***************************************************************/
int judge_word(char *src, char *type, int itype, sqlite3 *db)
{   
    char sql[1024];
    int id = 0;
    char *name = NULL;
    char *cur_type = NULL;
    int cur_itype = 0;
    int ret = 0; 
    sqlite3_stmt *statement;
    int max_id = 0;
    int max_len = 0;
    int current_len = 0;

    memset(sql,0x0,sizeof(sql));
    sprintf(sql,"select ID, NAME, TYPE, ITYPE from CMP_NO");
    ret = sqlite3_prepare(db,sql,-1,&statement,NULL);
    if(ret != SQLITE_OK){
        sqlite3_finalize(statement);
        log_error("prepare error");
        return -1;
    }

    while(sqlite3_step(statement) == SQLITE_ROW){
        id = sqlite3_column_int(statement,0);
        name = (char *)sqlite3_column_text(statement,1);
        cur_type = (char *)sqlite3_column_text(statement,2);
        cur_itype = sqlite3_column_int(statement,3);

        current_len=cmp_word_by_reg(src,name,cur_itype);
		
		/* add by suifeng 2017.07.19 */
		if (current_len == -2) 
			break;
		
		if(current_len == 0){
            max_id = id;
            sprintf(type,"%s",cur_type);
            itype = cur_itype;
            max_len = current_len;
            break;
		} else{
            if(current_len > max_len){
                max_id = id;
                max_len = current_len;
                sprintf(type,"%s",cur_type);
                itype = cur_itype;
            }
        }
    }

    sqlite3_finalize(statement);
    return max_len;
}
#endif



/***************************************************************
 *
***************************************************************/
int set_envent(event_t *semantic_info,int flag ,sqlite3 *db, char *word)
{
    char result[526];
//    char type[32];
    int itype = 0;
    char sql[1024];
    //int id = 0;
    char *name = NULL;
    //char *cur_type = NULL;
    char cur_itype = 0;
    int ret = 0;
    sqlite3_stmt *statement = NULL;
    //int max_evt_id = 0;
    int max_evt_len = -1;
    //int max_type_id = 0;
    int max_type_len = -1;
    int current_len = -1;
    int ask_itype = 0;
    int evt_flag = -1;
    int type_flag = -1;
//    int i = 0;
    char cmd[512];

    if (word == NULL || word[0] == '\0') {
        return -1;
    }

    if(Cancel_Symbol(word, word) == -1){
    	printf("Cancel_Symbol error\n");
        return -1;
    }

    memset(result,0x0,sizeof(result));
    log_debug("原词： [%s]", word);

    memset(cmd, 0, sizeof(cmd));
    if (get_log_path() == NULL) {
		sprintf(cmd, "echo \"%s : %s\" >> speech.txt",
				semantic_info->record_file, word);
    } else {
    	char * pos = NULL;
    	pos = strrchr(semantic_info->record_file, '/');
    	if (pos == NULL) {
    		sprintf(cmd, "echo \"%s : %s\" >> %s/%s_speech.txt;  echo \"%s : %s\" >> speech.txt",
    				semantic_info->record_file, word, get_log_path(), get_micon(),
					semantic_info->record_file, word);
    	} else {
    		sprintf(cmd, "echo \"%s : %s\" >> %s/%s_speech.txt;  echo \"%s : %s\" >> speech.txt",
    				pos + 1, word, get_log_path(), get_micon(),
					pos + 1, word);

    	}
    }
    system(cmd);
    normative_word(result, word, db);
    strcpy(word, result);
    log_debug("替换： [%s]", result);


    memset(sql,0x0,sizeof(sql));
    sprintf(sql,"select ID, NAME, TYPE, ITYPE from CMP_NO");
    ret = sqlite3_prepare(db,sql,-1,&statement,NULL);
    if(ret != SQLITE_OK){
        sqlite3_finalize(statement);
        return -1;
    }

    semantic_info->event = evt_pre_answer;
    semantic_info->answer_type = type_null;

    /* 大写转成小写  －－ 2017.08.10 suifeng */
    //int i = 0;
    //while(word[i++])
    	//word[i] = tolower(word[i]);

    while(sqlite3_step(statement) == SQLITE_ROW){
        sqlite3_column_int(statement,0);
        name = (char *)sqlite3_column_text(statement,1);
        (char *)sqlite3_column_text(statement,2);
        cur_itype = sqlite3_column_int(statement,3);

//            printf("result %s, len %d\n", result, strlen(result));
        current_len=cmp_word_by_reg(result,name,cur_itype);
        /* modify by mj 2016.10.26 */
        /* modify start */
		
		/* add by suifeng 2017.07.19 */
		if (current_len == -2) 
			break;
		
        if(cur_itype == 4){
            if(current_len == 0){
                ask_itype = 4;
                evt_flag = 1;
                itype =  0;
                type_flag = 1;
            }
            else if(current_len > 0){
                if(current_len > max_evt_len && evt_flag != 1){
                    //max_evt_id = id;
                    max_evt_len = current_len;
                    ask_itype = cur_itype;
                }
            }
        }else{
            if(current_len == 0){
                itype = cur_itype;
                type_flag = 1;
                ask_itype = 1;
                evt_flag = 1;
            }
            else if (current_len > 0){
                if(current_len > max_type_len && type_flag != 1){/* modify by mj 2016/11/18 */
                    //max_type_id = id;
                    max_type_len = current_len;
                    itype = cur_itype;
                }
            }
        }
        if(evt_flag == 1 && type_flag == 1){
            break;
        }
        /*
        if(current_len == 0){
            if(cur_itype == 4){
                ask_itype = 4;
            }
            max_id = id;
            memset(type,0x0,sizeof(type));
            sprintf(type,"%s",cur_type);
            if(cur_itype != 4){
                itype = cur_itype;
                max_len = current_len;
            }
            //if(ask_itype == 4){
                break;
            //}
        }
        else if(current_len > 0){
            if(cur_itype == 4){
                ask_itype = 4;
            }
            else if(current_len > max_len){
                max_id = id;
                max_len = current_len;
                memset(type,0x0,sizeof(type));
                sprintf(type,"%s",cur_type);
                //if(cur_itype != 4){
                itype = cur_itype;
                //}
            }
        }
        */
        /* modify end */
    }

    semantic_info->answer_type = (answer_type_t)itype;

    if(ask_itype == 4){
        semantic_info->event = evt_pre_ask;
        if (semantic_info->answer_type == type_null) {
            semantic_info->answer_type = type_question;
        }
    }
    else
    {
        semantic_info->event = evt_pre_answer;
    }

#if 0
    if (itype > 4) {
        semantic_info->event = evt_pre_answer;
    }
    printf("itype: %d, current len: %d\n",itype, current_len);
#endif

    sqlite3_finalize(statement);


    return max_type_len;
}


