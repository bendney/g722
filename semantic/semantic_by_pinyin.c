#include <stdio.h>
#include <string.h>
#include <sqlcipher/sqlite3.h>

#include "pinyin.h"

int set_accent(char *src, char *result, int step)
{
    char buf[8];
    int i = 0;
    int len = 0;
    int pb = 0;

    if(src == NULL || src[0] == '\0'){
        return -1;
    }
    len = strlen(src);
    memset(buf,0x0,sizeof(buf));
    if(step == 0){
        sprintf(buf,"%s",src);
    }
    if(step == 1){
        if(src[0] == 'n'){
            buf[0] = 'l';
        }
        else if(src[0] == 'l'){
            buf[0] = 'n';
        }
    }
    if(step == 2){
        if(src[0] == 'h'){
            buf[0] = 'f';
        }
        else if(src[0] == 'f'){
            buf[0] = 'h';
        }
    }
    if(step == 3){
        if(src[0] == 'z' || src[0] == 'c' || src[0] == 's'){
            buf[0] = src[0];
            if(src[1] == 'h'){
                buf[1] = src[2];
                i = 3;
            }
            else{
                buf[1] = 'h';
                i = 1;
            }
            pb = 2;
        }
    }
    for(; i<len; i++,pb++){
        buf[pb] = src[i];
    }
    //printf("i:[%d],pb:[%d]--buf[1]:[%c]\n",i,pb,buf[pb]);
    if(step == 4){
        if(buf[pb-1] == 'n'){
            if(buf[pb-2] == 'a' || buf[pb-2] == 'e' || buf[pb-2] == 'i' || buf[pb-2] == 'o'){
                buf[pb] = 'g';
                buf[pb+1] = '\0';
            }
            else{
                buf[pb] = '\0';
            }
        }
        else if(buf[pb-1] == 'g'){
            if(buf[pb-2] == 'n'){
                buf[pb-1] = '\0';
            }
        }
        else{
            buf[pb] = '\0';
        }
    }
    sprintf(result,"%s",buf);
    return 0;
}

int pub_reg_cmp_pingying(char *src, char *key, char **coord)
{
    int i = 0;
    char buf[32][8];
    int pb = 0;
    int pk = 0; 
    char *p_current = NULL;
    int sum_match = 0;
    int step = 0;
    char accent[10];
    int len_accent = 0;
    int pre_len = -1;
    int current_len = -1;

    if(src == NULL || src[0] == '\0'){
        return -1;
    }
    if(key == NULL || key[0] == '\0'){
        return -1;
    }

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

    for(pb=0; pb<i; pb++){
        for(step=0; step<5; step++){
            memset(accent,0x0,sizeof(accent));
            set_accent(buf[pb],accent,step);
            p_current = strstr(src, accent);
            if(p_current != NULL){
                current_len = p_current -src;
                len_accent = strlen(accent);
                if(p_current - src == 0){
                    if(src[p_current-src+len_accent] == '\0' || src[p_current-src+len_accent] == ' '){
                        break;
                    }
                }
                else{
                    if(src[(p_current-src)-1] == ' '){
                        if(src[(p_current-src)+len_accent] == '\0' || src[(p_current-src)+len_accent] == ' '){
                            break;
                        }
                    }
                }
                p_current = NULL;
            }
        }
        if(p_current != NULL){
            if(*coord != NULL){
                if(p_current == *coord){
                }
                else{
                    if(current_len > pre_len){
                        sum_match++;
                        *coord = p_current;
                        pre_len = current_len;
                    }
                }
            }
            else{
                if(current_len > pre_len){
                    sum_match++;
                    *coord = p_current;
                    pre_len = current_len;
                }
            }
        }
    }
    //printf("sum_match:[%d]---src:[%s]----key:[%s]\n",sum_match, src, key);
    return sum_match;
}

int compare_answer_by_pingying(char *sql, char *src, char **current_p1,sqlite3 *db)
{
    sqlite3_stmt *statement;
    int ret = 0;
    char *condition = NULL;
    //char *word = NULL;
    int cur_count = 0;
    int max_count = 0;
	char *current_p = NULL;

    if(sql == NULL || sql[0] == '\0'){
        return -1;
    }

    ret = sqlite3_prepare(db,sql,-1,&statement,NULL);
    if(ret != SQLITE_OK){
        sqlite3_finalize(statement);
        fprintf(stderr,"prepare error!\n");
        return -1;
    }
    while(sqlite3_step(statement) == SQLITE_ROW){
		current_p = (char *)current_p1;
        sqlite3_column_int(statement,0); 
        condition = (char *)sqlite3_column_text(statement,1);
        //word = (char *)sqlite3_column_text(statement,2);
        cur_count = pub_reg_cmp_pingying(src,condition,&current_p);
        if(cur_count > max_count){
            max_count = cur_count;
        }
    }
    sqlite3_finalize(statement);
	
	current_p1 = (char **)current_p;
    return max_count; 
}

int normative_by_pinyin(char *src, char *result, sqlite3 *db)
{
    char sql[128];
    int count_step[5];
    int cur_count_step[5];
    int id = 0;
    char *name = NULL;
    char *word = NULL;
    sqlite3_stmt *statement;
    int ret = 0;
    int i = 0;
    int max_id = 0;
    char *current_p1 = NULL;
 
    if(src == NULL || src[0] == '\0'){
        return -1;
    }
    
    memset(sql,0x0,sizeof(sql));
    sprintf(sql,"select distinct NAME, ID from NORMATIVE_PINGYING where LEVEL = 0 order by ID asc");
    ret = sqlite3_prepare(db,sql,-1,&statement,NULL);
    if(ret != SQLITE_OK){
        sqlite3_finalize(statement);
        fprintf(stderr,"prepare error!\n");
        return -1;
    }
    memset(count_step,0x0,sizeof(count_step));
    while(sqlite3_step(statement) == SQLITE_ROW){
        current_p1 = NULL;
        name = (char *)sqlite3_column_text(statement,0);
        id = sqlite3_column_int(statement,1);
        //word = (char *)sqlite3_column_text(statement,2);
        memset(cur_count_step,0x0,sizeof(cur_count_step));
        memset(sql,0x0,sizeof(sql));
        sprintf(sql,"select ID,CONDITION from NORMATIVE_PINGYING where NAME = '%s' and LEVEL = 2",name);
        cur_count_step[0] = compare_answer_by_pingying(sql,src,&current_p1,db);
        memset(sql,0x0,sizeof(sql));
        sprintf(sql,"select ID,CONDITION from NORMATIVE_PINGYING where NAME = '%s' and LEVEL = 3",name);
        cur_count_step[1] = compare_answer_by_pingying(sql,src,&current_p1,db);
        //printf("cur_count_step[0]:[%d]-----cur_count_step[1]:[%d]\n",cur_count_step[0],cur_count_step[1]);
        if(cur_count_step[0] < 1 && cur_count_step[1] < 1 ){
            continue;
        }
        if(cur_count_step[1] < 1){
            continue;
        }
        memset(sql,0x0,sizeof(sql));
        sprintf(sql,"select ID,CONDITION from NORMATIVE_PINGYING where NAME = '%s' and LEVEL = 1",name);
        cur_count_step[2] = compare_answer_by_pingying(sql,src,&current_p1,db);
        memset(sql,0x0,sizeof(sql));
        sprintf(sql,"select ID,CONDITION from NORMATIVE_PINGYING where NAME = '%s' and LEVEL = 4",name);
        cur_count_step[3] = compare_answer_by_pingying(sql,src,&current_p1,db);
        memset(sql,0x0,sizeof(sql));
        sprintf(sql,"select ID,CONDITION from NORMATIVE_PINGYING where NAME = '%s' and LEVEL = 5",name);
        cur_count_step[4] = compare_answer_by_pingying(sql,src,&current_p1,db);
        if(cur_count_step[1] > count_step[1]){
            if(count_step[0] < 1 || count_step[1] < 1){
                for(i=0; i<5; i++){
                    count_step[i] = cur_count_step[i];
                }
                max_id = id;
            }
        }
    }
    sqlite3_finalize(statement);

    if(count_step[0] < 1 && count_step[1] < 1){
        //result[0] = '\0';
        return 0;
    }
    if(count_step[1] == 0){
        if(count_step[2] < 1 && count_step[3] < 1 && count_step[4] < 1){
            //result[0] = '\0';
            return 0;
        }
    }
    
    memset(sql,0x0,sizeof(sql));
    sprintf(sql,"select WORD from NORMATIVE_PINGYING where ID = %d",max_id);
    ret = sqlite3_prepare(db,sql,-1,&statement, NULL);
    if(ret != SQLITE_OK){
        sqlite3_finalize(statement);
        return 0;
    }
    if(sqlite3_step(statement) == SQLITE_ROW){
        word = (char *)sqlite3_column_text(statement,0);
    }
    if(word != NULL || word[0] != '\0'){
        sprintf(result,"%s",word);
    } else {
        sqlite3_finalize(statement);
        return 0;
    }
    sqlite3_finalize(statement);
    return 1;
}
