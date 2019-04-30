/*
 * sa_sql.c
 *
 *  Created on: 2016楠�?6閺�?23閺�?
 *      Author: 娴滄垵鍨庨煬?
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <pcre.h>
#include <assert.h>
#include "semantic_analyzer.h"
#include "sa_suport.h"
#include "sa_sql.h"
#include "common_type.h"

#define DEFAULT_FIELD_NUB    11
#define DEFAULT_FIELD "ANSWER, NAME, NEXTNAME, SKIP_CONDITION, SCORE, LEVEL, PRIORITY, RECORD, SKIP_TO, WEIGHT, QUESTION"

static char db_path[256];
static char log_path[256];
static char dalog_micon[256];

int set_log_path(char *path)
{
	if (path == NULL) {
		log_error("log path null!");
		return -1;
	}
    if (strlen(path) > sizeof(log_path) - 1) {
        log_error("log path long than 256!");
        return -1;
    }
    strcpy(log_path, path);
    return 0;
}


char *get_log_path(void)
{
    if (log_path[0] == '\0') {
        return NULL;
    }
    return log_path;
}

int set_micon(char *micon)
{
	if (micon == NULL) {
		log_error("micon null!");
		return -1;
	}
    if (strlen(micon) > sizeof(dalog_micon) - 1) {
        log_error("micon long than 256!");
        return -1;
    }
    strcpy(dalog_micon, micon);
    return 0;
}

char *get_micon(void)
{
	return dalog_micon;
}

int get_industry(char *industry)
{
	strcpy(industry, sa_info.industry);
//	log_info("industry: %s", industry);
//	*industry = sa_info.industry;
	return 0;
}

int get_area(char *area)
{
	strcpy(area, sa_info.area);
//	log_info("area: %s", area);
//	*area = sa_info.area;
	return 0;
}

void clean_log_path(void)
{
    memset(log_path, 0, sizeof(log_path));
}

int set_db_path(char *path)
{
	if (path == NULL) {
		log_error("db path null!");
		return -1;
	}
    if (strlen(path) > sizeof(db_path)) {
        log_error("db path long than 256!");
        return -1;
    }
    strcpy(db_path, path);
    log_debug("db path %s", db_path);

    return 0;
}

char *get_db_path(void)
{
    if (db_path[0] == '\0') {
        return NULL;
    }
    return db_path;
}

void clean_db_path(void)
{
    memset(db_path, 0, sizeof(db_path));
}

int word_separate(char (*pack)[512], int *nub, char *word)
{
    char *rst = NULL;
    char *p = word;
    int i = 0;

    while (1) {

        rst = strchr(p, '|');

        if (rst != NULL) {
            *rst = '\0';
            strcpy(pack[i++], p);
            p = rst + 1;
//            strcpy(pack[i++], rst + 1);
        } else {
            strcpy(pack[i], p);
            break;
        }
    }
    *nub = i + 1;

    return 0;
}

/***************************************************************
 *
***************************************************************/
int analy_filename(char (*filename)[FILENAME_SIZE], int *sum, char *word)
{
    int len;
    int i = 0;
    int pf = *sum;
    int pfi = 0;
//    char file_name[10][1024];
    char buf[FILENAME_SIZE];

    if(word == NULL || word[0] == '\0'){
        return -1;
    }
    if(filename == NULL || sum == NULL){
        return -1;
    }


    len = strlen(word);
//    printf("len %d", len);

    log_info("flag");

    for(i=0; i<len; i++){
       filename[pf][pfi] = word[i];
       if(filename[pf][pfi] == '|') {
            filename[pf][pfi] = '\0';
            memset(buf,0x0,sizeof(buf));
			if (get_db_path() == NULL) {
				sprintf(buf,"%s/record/%s.wav", HOME, filename[pf]);
			} else {
				sprintf(buf,"%s/record/%s.wav", get_db_path(), filename[pf]);
			}
            sprintf(filename[pf],"%s",buf);

//            printf("mp3: %s", filename[pf]);
            pfi = -1;
            pf++;
       }
       pfi++;
       if(pf == FILE_MAX_COUNT){
            break;
       }
    }
    filename[pf][pfi+1] = '\0';
    memset(buf,0x0,sizeof(buf));

    if (filename[pf][0] == '@') {
    	log_info("file:[%s]", (char *)filename[pf] + 1);
    	sprintf(buf,"%s%s/%s%s",AI_VOICES_DIR, get_micon(), &filename[pf][1], TTS_FORMAT);
    } else {
		if (get_db_path() == NULL) {
			sprintf(buf,"%s/record/%s.wav", HOME, filename[pf]);
		} else {
			sprintf(buf,"%s/record/%s.wav", get_db_path(), filename[pf]);
		}
    }
    sprintf(filename[pf],"%s",buf);

    *sum = pf+1;
//    printf("mp3: %s, sum: %d", filename[pf], *sum);


    return 0;
}

int get_record_file(char (*filename)[FILENAME_SIZE], int *sum, char *record)
{
    int ret = -1;
    char word_pack[FILE_MAX_COUNT][WORD_SIZE];
    int nub = 0;
    int i = 0;


    if(record == NULL || record[0] == '\0'){
        return -1;
    }
    if(strlen(record) > 512){
    	record[512] = '\0';
    }

    word_separate(word_pack, &nub, record);

    *sum = 0;
    for (i = 0; i < nub; i++) {
		analy_filename(filename, sum, word_pack[i]);
		ret = 0;
    }

    for(i = 0; i < *sum; i++) {
        log_debug("mp3: %s|", filename[i]);
    }
    log_debug("");
    return ret;
}

/***************************************************************
 *
***************************************************************/
int word_to_filename(char (*filename)[FILENAME_SIZE], int *sum, char *word)
{
    char **result = NULL;
    char *err_msg = NULL;
    int rows = 0;
    int colums = 0;
    int ret = -1;
    char sql[4096];
    sqlite3 *db;

    char word_pack[FILE_MAX_COUNT][WORD_SIZE];
    int nub = 0;
    int i = 0;

//    ret = sqlite3_open(DBHOME,&db);
//    if(ret != 0){
//        return -1;
//    }
    db = sa_info.db;

//    printf("word to file");

    if(word == NULL || word[0] == '\0'){
        return -1;
    }
    if(strlen(word) > 4048){
        word[4048] = '\0';
    }

    word_separate(word_pack, &nub, word);

//    printf("nub: %d", nub);

    *sum = 0;
    for (i = 0; i < nub; i++) {

        memset(sql,0x0,sizeof(sql));
        sprintf(sql,"select SPATH from WORD_TO_SOUND where WORD = '%s'",word_pack[i]);
        ret = sqlite3_get_table(db,sql,&result,&rows,&colums,&err_msg);

        if(rows > 0){
    //        sprintf(filename,"%s",result[1]);
//            printf("result %s",result[1]);
            analy_filename(filename, sum, result[1]);
//            sqlite3_free_table(result);
    //        sqlite3_close(db);

            ret = 0;
        }
        sqlite3_free_table(result);
    }


    for(i = 0; i < *sum; i++) {
        log_debug("mp3: %s|", filename[i]);
    }
    log_debug("");

//    sqlite3_free_table(result);
//    sqlite3_close(db);
    return ret;
}


/***************************************************************
 * Description: 璁＄畻褰撳墠璇绾у埆
 *              鍏ュ弬锛歝har *src鍨嬭澧冨悕瀛�
 *              鍑哄弬: int *sum_dot褰撳墠璇绾у埆
 *              杩斿洖鍊�: -1 澶辫触锛� 0 鎴愬姛
 * History:
***************************************************************/
int count_dot(char *src, int *sum_dot)
{
    int len = strlen(src);

    if(len < 1){
        *sum_dot = 1;
        return -1;
    }
    *sum_dot = 0;
    for(; len > -1; len--){
        if(src[len] == '.'){
            *sum_dot = *sum_dot + 1;
        }
    }

    return 0;
}

/***************************************************************
 * Description: 鐠囥儱鍤遍弫鎵暏閺夈儲鐎柅鐘虫殶閹诡喖绨卞锝呭灟鐞涖劏鎻蹇撳毐閺�?
 *              娴ｈ法鏁ょ拠瀛樻:閸愬懐鏁ら崙鑺ユ殶閿涘奔绗夐崗浣筋啅鐞氼偄顦婚柈銊ㄧ殶閻�?
 * History:
***************************************************************/
#if 0
static void sqlite3_regexp( sqlite3_context *context, int argc, sqlite3_value **argv ) {
    assert(argc == 2);
    char *pcre_err = NULL;
    int pcre_erroffset, pcre_errcode;
    pcre *pattern = pcre_compile2((const char *) sqlite3_value_text(argv[0]), PCRE_UTF8, &pcre_errcode, (const char **) &pcre_err, &pcre_erroffset, NULL);
    if( pattern == NULL ) {
        if( pcre_errcode == 21 ) {
            sqlite3_result_error_nomem(context);
        } else {
            sqlite3_result_error(context, pcre_err, strlen(pcre_err));
        }
    } else {
        const unsigned char *subject = sqlite3_value_text(argv[1]);
        pcre_errcode = pcre_exec(pattern, NULL, (const char *) subject, strlen((const char *) subject), 0, 0, NULL, 0);
        if( pcre_errcode < 0 ) {
            char *errmsg;
            switch( pcre_errcode ) {
                case PCRE_ERROR_NOMATCH:
                    sqlite3_result_int(context, 0);
                    break;
                case PCRE_ERROR_NOMEMORY:
                    sqlite3_result_error_nomem(context);
                    break;
                default:
                    errmsg = sqlite3_mprintf("pcre_exec: Error code %d\n", pcre_errcode);
                    if( errmsg == NULL ) {
                        sqlite3_result_error_nomem(context);
                    } else {
                        sqlite3_result_error(context, errmsg, strlen(errmsg));
                    }
                    sqlite3_free(errmsg);
            }
        } else {
            sqlite3_result_int(context, 1);
        }
        pcre_free(pattern);
    }
}
#endif
int compare_regexp(char *pattern, char *word)
{
    int sum_dot = 0;
    int ret = 0;
    char str_num;
    int pre_inum = 0;
    int back_inum = 0;
    char *pattern_name = NULL;
    char *remain_name = NULL;
    int len;

    if(pattern == NULL || word == NULL){
        return -1;
    }
    if(pattern[0] == '\0' || word == '\0'){
        return -1;
    }

    str_num = pattern[1];
    if(str_num > 0){
        pre_inum = str_num - 48;
    }
    if(pre_inum == 0 && pattern[2] == ']'){
        ret = count_dot(word,&sum_dot);
        if(ret == 0){
            if(sum_dot == 0){
                return 1;
            }
        }
        return 0;
    }

    str_num = pattern[3];
    if(str_num > 0){
        back_inum = str_num - 48;
    }

    pattern_name = pattern + 5;
    len = strlen(pattern_name);
    if(strncmp(word,pattern_name,len) == 0){
        remain_name = word + len;
        if(remain_name[0] == '\0'){
            if(pre_inum == 0){
                return 1;
            }
        }
        if(remain_name[0] != '\0'){
            ret = count_dot(remain_name,&sum_dot);
            if(sum_dot == pre_inum || sum_dot == back_inum){
                return 1;
            }
        }
    }
    return 0;
}

static void sqlite3_regexp(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    unsigned char *pattern = NULL;
    unsigned char *word = NULL;
    int ret = 0;

    pattern = (unsigned char *)sqlite3_value_text(argv[0]);
    word = (unsigned char *)sqlite3_value_text(argv[1]);
    ret = compare_regexp((char *)pattern, (char *)word);
    if(ret == 1){
        sqlite3_result_int(context,1);
    }
    else{
        sqlite3_result_int(context,0);
    }
}

static int test_db(sqlite3 *db)
{
    char buf[16];
    return sql_get_one_data_by_name(db, "REQ_PUBLIC_DATA", "SCORE", buf, ".scoreA", 1);
}

/***************************************************************
 * Description: 閸掓繂顫愰崠鏍ㄦ殶閹诡喖绨�
 *              閸忋儱寮�: sqlite3 **db閸ㄥ鏆熼幑顔肩氨閹稿洭鎷�
 *                    char *dbname閸ㄥ顩﹂幍鎾崇磻閻ㄥ嫭鏆熼幑顔肩氨 婵″倹鐏夋稉铏光敄娴ｈ法鏁�
 *                                姒涙顓婚弫鐗堝祦鎼�?
 *              閸戝搫寮�: sqlite3 **db閺佺増宓佹惔鎾村瘹闁�?
 *              鏉╂柨娲栭崐?: -1 婢惰精瑙﹂敍?0 閹存劕濮�
 * History:
***************************************************************/
int init_sqlite3_db(sqlite3 **db, char *dbname)
{
    int ret = 0;
    char buf[256];
//  struct timeval tp1, tp2;

    memset(buf, 0, sizeof(buf));
    if(dbname == NULL || dbname[0] == '\0'){
        sprintf(buf, "%s/%s", HOME, DB_NAME);
    }
    else{
        sprintf(buf, "%s/%s", dbname, DB_NAME);
    }
//  gettimeofday(&tp1, NULL);
    ret = sqlite3_open(buf, db);
    if(ret != 0){
        log_error("open sqlite3 %s failed, error number: %d\n", buf, ret);
        return -1;
    }

//#ifdef SQL_KEY
    ret = sqlite3_key(*db, "#R@rX#gq2WNxPtlE", strlen("#R@rX#gq2WNxPtlE"));
    ret = test_db(*db);
    if (ret != 0) {
		//log_debug("no key");
        sqlite3_close(*db);
        ret = sqlite3_open(buf, db);
        if(ret != 0){
            log_error("open sqlite3");
            return -1;
        }
    }

   //log_debug("key ret: %d", ret);
//#endif
    sqlite3_create_function(*db, "regexp", 2, SQLITE_UTF8, 0, sqlite3_regexp, 0, 0);

    //gettimeofday(&tp2, NULL);

    //log_info("sql init time: %dms\n", timeval_sub(tp1, tp2));
   
	log_debug("init_sqlite3_db %s sucess\n",buf);

    return 0;
}


/***************************************************************
 * add by suifeng (2016.9.18)
***************************************************************/
int ai_insert_node(ai_address_buf **head, ai_address_buf *node)
{
//    int find_flag = 0;
    ai_address_buf *p = NULL;
    ai_address_buf *prev_p = NULL;

    if (*head == NULL) {
        node->next = NULL;
        *head = node;
        //printf("insert head\n");
        return 0;
    }

    for (p = *head; p != NULL; prev_p = p, p = p->next) {
        if (node->cur_address < p->cur_address) {
            if (prev_p == NULL) { /*insert front of head*/
                node->next = *head;
                *head = node;
                //printf("insert front of head\n");
            } else { /* insert front of p */
                //printf("prev_order:%d\n", prev_p->order);
                prev_p->next = node;
                node->next = p;
                //printf("insert middle\n");
            }

            return 0;
        } else if (node->cur_address == p->cur_address) {
            return -1;
        }
    }

    /* insert end of line */
    prev_p->next = node;
    node->next = NULL;

    //printf("insert end\n");

    return 0;
}

#if 0
/***************************************************************
 *
***************************************************************/
ai_address_buf *ai_insert_node(ai_address_buf *head, ai_address_buf *node)
{
    ai_address_buf *tnode = NULL;
    ai_address_buf *pre_node = NULL;

    if(head == NULL){
        head = node;
        node->next = NULL;
        return head;
    }
    tnode = head;
    while(tnode != NULL){
        if(node->cur_address > tnode->cur_address){
            if(pre_node == NULL){
                node->next = head;
                head = node;
                return head;
            }
            node->next = tnode;
            pre_node->next = node;
            return head;
        }
//        else if(node->cur_address < tnode->cur_address){
//            pre_node = tnode;
//            tnode = tnode->next;
//            return head;
//        } else {
//            return head;
//        }
    }
    pre_node->next = node;
    node->next = NULL;
    return head;
}
#endif

#if 0
int pub_reg_cmp_word(char *src, char *key , int order)
{
    int i = 0;
    char buf[32][128];
    int pb = 0;
    int pk = 0;
    char *p_current = NULL;
    //char *p_before = NULL;
    int sum_word = 0;
    int sum_match = 0;
    int sum_order = 0;
    ai_address_buf *head = NULL;
    ai_address_buf *node = NULL;
    ai_address_buf *next_node = NULL;
    int sum_node = 0;

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
    sum_word = i;

    for(pb=0; pb<i; pb++){
        p_current = strstr(src,buf[pb]);
        if(p_current != NULL){
            printf("src:[%s]  buf:[%s]\n",src,buf[pb]);
            node = (ai_address_buf *)malloc(sizeof(ai_address_buf));
            node->order = pb;
            node->cur_address = p_current;
            p_current = NULL;
            if (ai_insert_node(head, node) == 0) {
                sum_match++;
            }
        }
    }

    if(sum_match != i){
        return 0;
    }

    node = head;
    while(node != NULL){
        printf("[node]:[order]: [%s]:[%d]\n",node->cur_address,node->order);
        node = node->next;
    }

    if(head != NULL){
        node = head;
        next_node = head->next;
        sum_node++;
        while(next_node != NULL){
            printf("pre:[%s]:[%d]---next:[%s][%d]\n",node->cur_address,node->order, next_node->cur_address,next_node->order);
            if(node->order == (next_node->order + 1)){
                sum_order++;
            }
            node = node->next;
            next_node = next_node->next;
        }
    }
    if(i == 1 && sum_match == 1){
        node = head;
        while(node != NULL){
            next_node = node->next;
            free(node);
            node = next_node;
        }
        if(strlen(buf[0]) == 3){
            if(strlen(src) != 3){
                return 0;
            }
        }
        return 4;
    }
    node = head;
    while(node != NULL){
        next_node = node->next;
        free(node);
        node = next_node;
    }
    return (sum_match*10)/sum_word + (sum_order*10)/sum_word;
}
#endif

/* add by suifeng (2016.9.18) */
int find_key_and_record(ai_address_buf **head, char *src, char *key, int key_order)
{
    char *p_current = NULL;
    int ret = -1;
    ai_address_buf *node = NULL;

    p_current = strstr(src, key);

    if(p_current != NULL){
        node = (ai_address_buf *)malloc(sizeof(ai_address_buf));
        node->order = key_order;
        node->cur_address = p_current;
        ret = ai_insert_node(head, node);
        if (ret == -1) {/* position is same, find continue */
           free(node);
           fprintf(stderr, "position is same, find continue\n");
           ret = find_key_and_record(head, p_current + 3, key, key_order);
        }
    }

    return ret;
}

#if 1
/***************************************************************
 * Description: 閻犲洢鍎遍崵閬嶅极閹殿喗鏆忛柡澶堝劥琚欓柡瀣姇閼荤喖骞嶈椤㈡垵顫㈤敐鍛仧閻炴稏鍔忛幓顏勵嚕閿�?
 *              闁稿繈鍎卞?: char *src闁搞劌顑嗗顖涚閿�?
 *                    char *key闁搞劌顑呴崣褔鏌ㄩ璺ㄦГ
 *              閺夆晜鏌ㄥú鏍磹閿�?: -1 鐎殿喖鍊搁悥鍫曟晬閿�?0 闁哄啰濮甸弳鐔煎箲椤曞棛骞�1 闁瑰瓨鍔曟慨?
 * History:
***************************************************************/
int pub_reg_cmp_word(char *src, char *key, int use_order)
{
    int i = 0;
    char buf[32][128];
    int pb = 0;
    int pk = 0;
//    char *p_current = NULL;
    int sum_word = 0;
    int sum_match = 0;
    int sum_order = 0;
    ai_address_buf *head = NULL;
    ai_address_buf *node = NULL;
    ai_address_buf *next_node = NULL;
    int slen = 0;
    int ret = 0;
	int dlen = 0;  /* add- by mj 2016.9.21 */
	char *p_before = NULL; /* add by mj 2016.10.15 */

    if(src == NULL || src[0] == '\0'){
        return -1;
    }
    if(key == NULL || key[0] == '\0'){
        return -1;
    }

    slen = strlen(src);

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
    sum_word = i;

    /* modify by suifeng (2016.9.18) */
    for(pb = 0; pb < sum_word; pb++) {
	    dlen = dlen + strlen(buf[pb]); /* add- by mj 2016.9.21 */
	    /* #add by mj 2016.10.15 */
	    if(pb == 0){
	        p_before = strstr(src,buf[pb]);
	    }
	    else{
	        if(p_before != NULL){
	            p_before = strstr(p_before,buf[pb]);
	        }
	    }

	    if(p_before == NULL){
	        ret = find_key_and_record(&head, src, buf[pb], pb);
	    }
	    else{
	        ret = find_key_and_record(&head,p_before,buf[pb],pb);
	    }
	    /* #add end */
        if (ret == 0) {
            p_before = strstr(src,buf[pb]); /* add by mj 2016.10.15 */
            sum_match++;
        }
    }

//    fprintf(stderr, "src: %s\n", src);
//    for(node = head; node != NULL; node = node->next) {
//        fprintf(stderr, "node: %s, older: %d\n", buf[node->order], node->order);
//    }

    if(sum_match != sum_word){
        node = head;
        while(node != NULL){
            next_node = node->next;
            free(node);
            node = next_node;
        }
        //printf("not match all\n");
        return 0;
    }

    /* is order ? */ /* modify by suifeng (2016.10.10) */
    if(head != NULL){
        for(node = head, sum_order = 0; node->next != NULL; node = node->next) {
            if (node->order + 1 == node->next->order) {
                sum_order++;
            }
        }
        if(sum_word > 1){ /* modify by mj 2016 10 17 */
            sum_order + 1 == sum_word ? sum_order++ : 0;
        }
    }

    //printf("sum: %d, order: %d\n", sum_word, sum_order);

    if(sum_word == 1 && sum_match == 1){
        node = head;
        while(node != NULL){
            next_node = node->next;
            free(node);
            node = next_node;
        }
        if(strlen(buf[0]) == 3){
            if(strlen(src) != 3){
                return 0;
            }
        }

        return ((sum_match+dlen)*10)/slen + (sum_order*10)/sum_word; /* modify by mj 2016-10-10 */
    }

    node = head;
    while(node != NULL){
        next_node = node->next;
        free(node);
        node = next_node;
    }

    /* add by suifeng (2016.9.14) */
    //printf("sum_word: %d, sum_order: %d", sum_word, sum_order);
    if(use_order == 1){
        if(sum_order == sum_word){
            log_info("is order\n");
            return ((sum_match+dlen)*10)/slen + (sum_order*10)/sum_word; /* add- by mj 2016.9.21 */
        }
        else{
            log_info("not order\n");
            return -1;
        }
    }

    return ((sum_match+dlen)*10)/slen + (sum_order*10)/sum_word; /* add- by mj 2016.9.21 */
}
#endif

/***************************************************************
 *
***************************************************************/
int recompare_answer(char *sql, char *src, sqlite3 *db, int *result_id)
{
    sqlite3_stmt *statement;
    int ret = 0;
    char *condition = NULL;
    int max_score = 0;
    int cur_score = 0;
    int max_id = 0;
    int id = 0;

    if(sql == NULL || sql[0] == '\0'){
        log_error("sql empty!!");
        return -1;
    }

    ret = sqlite3_prepare(db,sql,-1,&statement,NULL);
    if(ret != SQLITE_OK){
        sqlite3_finalize(statement);
        log_error("prepare error!");
        return -1;
    }

    while(sqlite3_step(statement) == SQLITE_ROW){
        condition = (char *)sqlite3_column_text(statement,0);
        id = sqlite3_column_int(statement,1);
        if((cur_score = pub_reg_cmp_word(src, condition, 0)) > 0){
            if(cur_score > max_score){
                max_score = cur_score;
                max_id = id;
            }
        }
    }

    if(max_id > 0){
        sqlite3_finalize(statement);
        *result_id = max_id;
        return max_score;
    }

    sqlite3_finalize(statement);
    return 0;
}

/***************************************************************
 * Description: 閻犲洢鍎遍崵閬嶅极閹殿喗鏆忛柡澶堝劜鐎垫粓鎮¤椤掓粓宕氬▎蹇曠闁告牕缍婇崢銈囩磼閹惧浜�
 *              闁稿繈鍎卞?: char *sql闁搞劌鐛眖l閻犲浂鍘艰ぐ?
 *                    char *src闁搞劌顑嗗顖涚閿�?
 *                    sqlite3 *db闁搞劌顑呯紞瀣礈瀹ュ嫬鈻忛柣鈧妿濞堟垿寮悧鍫濈ウ閹艰揪鎷�
 *              閺夆晜鏌ㄥú鏍磹閿�?: -1 鐎殿喖鍊搁悥鍫曟晬閿�?0 闁哄啰濮甸弳鐔煎箲椤曞棛骞㈠鍫嗗倻鑹�0 闁告牕缍婇崢銈嗘償閿旇姤浠樺Δ鍌涱焽濞堟厲D
 * History:
***************************************************************/
int compare_answer(char *sql, char *src, sqlite3 *db, int *result_id)
{
    sqlite3_stmt *statement;
    int ret = 0;
    char *condition = NULL;
    int max_score = 0;
    int cur_score = 0;
    int max_id = 0;
    int id = 0;
    char t_sql[1024];
    int t_score = 0;
    int t_id = 0;

    int usr_order = 0;
    char *key = NULL;

    if(sql == NULL || sql[0] == '\0'){
        log_error("sql empty!!");
        return -1;
    }

    ret = sqlite3_prepare(db,sql,-1,&statement,NULL);
    if(ret != SQLITE_OK){
        sqlite3_finalize(statement);
        log_error("prepare error!");
        return -1;
    }

    while(sqlite3_step(statement) == SQLITE_ROW){
        condition = (char *)sqlite3_column_text(statement,0);
        id = sqlite3_column_int(statement,1);
        /* add to support '*' */
        if (strcmp(condition, "*") == 0) {
            *result_id = id;
            sqlite3_finalize(statement);
            return 20;
        }

//        printf("src: %s, condition: %s\n", src, condition);

        /* use order find (add by suifeng 2016.9.14) */
        key = condition;
        if (condition != NULL && condition[0] == '>') {
            //printf("condition: %s\n", condition);
            while(++key) {
                if (key[0] != ' ');
                    break;
            }
            //printf("use order\n");
            usr_order = 1;
        } else {
            usr_order = 0;
        }

        if((cur_score = pub_reg_cmp_word(src, key, usr_order)) > 0){
            if(cur_score > max_score){
                max_score = cur_score;
                max_id = id;
            }
        }
    }
    //printf("********************************************\n");

    if(max_score == 4){
        if(strlen(src) > 12 ){
           memset(t_sql,0x0,sizeof(t_sql));
           sprintf(t_sql,"select CONDITION, ID from REQ_ANS_DATA where NAME regexp '[0]'");
           t_score = recompare_answer(t_sql,src,db,&t_id);
        }
    }

    if(max_id > 0){
        if(t_score > max_score){
            *result_id = t_id;
            sqlite3_finalize(statement);
            return t_score;
        }
        sqlite3_finalize(statement);
        *result_id = max_id;
        return max_score;
    }

    sqlite3_finalize(statement);
    return 0;
}

/***************************************************************
 * Description: 閺屻儲澹橀崗鎶芥暛鐎�?
 *              鏉╂柨娲栭崐?: -1 闁挎瑨顕ら敍?0 婢惰精瑙﹂敍? 1 閹存劕濮�
 * History:
***************************************************************/
int find_key_word(char *word, char **result, int rows, int colums, int *key_row)
{
    int i = 0;
    int j = 0, len;
    char tmp[512];
    char *rst = NULL;


    for(i = 1; i <= rows; i++) {
        /* 閸忋劏鐦濋崠褰掑帳 */
        rst = strstr(word, result[i * colums]);
        if (rst != NULL) {
            *key_row = i;
            return 1;
        }

        /* 閸掑棜鐦濋崠褰掑帳  */
        len = strlen(result[i * colums]);
        for(j = 0; j < len; j++) {
            if (result[i * colums][j] == ' ') {
                memcpy(tmp, result[i * colums], j);
                tmp[j] = '\0';
                rst = strstr(word, tmp);
                if (rst != NULL) {
                    if (len <= j + 2) {
                        log_error("data format error!!!");
                        return -1;
                    }
                    strcpy(tmp, result[i * colums] + j + 1);
                    rst = strstr(word, tmp);
                    if (rst != NULL) {
                        *key_row = i;
                        return 1;
                    } else {
                        break;
                    }
                } else {
                   break;
                }
            }
        }
    }

    return 0;
}

/* flag = 0, get char
 * flag = 1, get int
 * */
int sql_get_one_data_by_name(sqlite3 *db, char *table, char *field, void *data, char *name, int flag)
{
    char sql[256];
    char **result = NULL;
    int rows = 0;
    int colums = 0;
    char *err_msg = NULL;

    memset(sql,0x0,sizeof(sql));
    sprintf(sql,"select %s from %s where NAME = '%s'",field, table, name);
	//printf("sql: [%s]\n", sql);
    sqlite3_get_table(db, sql, &result, &rows, &colums, &err_msg);
    if (err_msg != NULL) {
        sqlite3_free_table(result);
        log_info("sqlite3 get table error: %s", err_msg);
        return -ERROR_GET_DB;
    }

    if(rows == 0){
        sqlite3_free_table(result);
        return 0;
    }

    if (flag == 0) {
        strcpy(data, result[1]);
    } else if (flag == 1) {
        *((int *)data) = atoi(result[1]);
    }

    sqlite3_free_table(result);

    return 0;
}



int sql_get_data_by_id(sqlite3 *db, char *table, char *field, char *data, int id)
{
    char sql[1024];
    char **result = NULL;
    int rows = 0;
    int colums = 0;
    char *err_msg = NULL;
//    int i = 0;

    memset(sql,0x0,sizeof(sql));
    sprintf(sql,"select %s from %s where ID = %d",field, table, id);
    sqlite3_get_table(db, sql, &result, &rows, &colums, &err_msg);
    if (err_msg != NULL) {
        sqlite3_free_table(result);
        log_error("sqlite3_get_table: %s", err_msg);
        return -ERROR_GET_TABLE;
    }

    if(rows == 0){
        sqlite3_free_table(result);
        return 0;
    }

    strcpy(data, result[1]);

    sqlite3_free_table(result);

    return 1;
}

int sql_get_default_data(event_t *event, char *table, int id)
{
    char sql[1024];
    char **result = NULL;
    int rows = 0;
    int colums = 0;
    char *err_msg = NULL;
    int i = 0;

    memset(sql,0x0,sizeof(sql));
    sprintf(sql,"select %s from %s where ID = %d",DEFAULT_FIELD, table, id);
    sqlite3_get_table(event->db, sql, &result, &rows, &colums, &err_msg);
    if (err_msg != NULL) {
        sqlite3_free_table(result);
        log_error("sqlite3_get_table: %s", err_msg);
        return -ERROR_GET_TABLE;
    }

    if(rows == 0){
        sqlite3_free_table(result);
        return 0;
    }

    for (i = 0; i < DEFAULT_FIELD_NUB; i++) {
        switch (i) {
        case 0:
            sprintf(event->word.word[0],"%s",result[i + DEFAULT_FIELD_NUB]);
            break;
        case 1:
            sprintf(event->context.context_name,"%s",result[i + DEFAULT_FIELD_NUB]);
            break;
        case 2:
            sprintf(event->context.next_context,"%s",result[i + DEFAULT_FIELD_NUB]);
            /* 濡傛灉鏄皬璇锛屽垯灏嗚褰曞ぇ璇璁板綍涓嬫潵锛岃繖鏍峰氨鍙互鎵惧埌鍥炲幓鐨勮矾 */
            //if (event->context.next_context[0] == '.' && event->context.cur_context[0] != '.' && event->context.cur_context[0] != '\0') {
            if (event->context.cur_context[0] != '\0' && strchr(event->context.cur_context, '.') == NULL) {
                memset(event->context.parent_context, 0, sizeof(event->context.parent_context));
                strcpy(event->context.parent_context, event->context.cur_context);
            }
            break;
        case 3:
            sprintf(event->context.skip_condition,"%s",result[i + DEFAULT_FIELD_NUB]);
            break;
        case 4:
            event->context.words_score = atoi(result[i + DEFAULT_FIELD_NUB]);
            log_debug("words score: %d", event->context.words_score);
            break;
        case 5:
            if (result[i + DEFAULT_FIELD_NUB][1] == '\0') {
                event->context.level = result[i + DEFAULT_FIELD_NUB][0];
                event->context.hangup_status = 0; /* add by siyi 2016.11.17 */
                log_debug("words level : %c", event->context.level);
            } else {
                event->context.hangup_status = atoi(result[i + DEFAULT_FIELD_NUB]);
                log_debug("hang up status : %d", event->context.hangup_status);
            }
            break;
        case 6:
            event->context.priority = atoi(result[i + DEFAULT_FIELD_NUB]);
            log_debug("priority: %d", event->context.priority);
            break;
        case 7:
            strcpy(event->context.record, result[i + DEFAULT_FIELD_NUB]);
            log_debug("record : %s", event->context.record);
            break;
        case 8:
            strcpy(event->context.skip_to, result[i + DEFAULT_FIELD_NUB]);
            break;
        case 9:
            event->context.wight = atof(result[i + DEFAULT_FIELD_NUB]);
            log_debug("wight : %.1f", event->context.wight);
            break;
        case 10:
            event->context.question_flag = atoi(result[i + DEFAULT_FIELD_NUB]);
            log_debug("question flag : %d", event->context.question_flag);
            break;
        }
    }
    log_debug("id : %d", id);

    sqlite3_free_table(result);
    event->context.response_id = id;

    return 1;
}

int sql_get_public_by_id(event_t *event, int id)
{
    int ret = 0;
    ret = sql_get_default_data(event, "REQ_PUBLIC_DATA", id);
    return ret;
}

int sql_get_by_id(event_t *event, int id)
{
    int ret = 0;
    ret = sql_get_default_data(event, "REQ_ANS_DATA", id);
    return ret;
}

int sql_get_public_response(event_t *event)
{
    char sql[1024];
    int ret = 0;
    int id = 0;
    int score = 0;

    if (event->db == NULL) {
        log_fatal("db is not open!");
        return -1;
    }

    memset(sql, 0x0, sizeof(sql));
    sprintf(sql,"select CONDITION, ID from REQ_PUBLIC_DATA where NAME = '%s' order by ID asc", event->context.context);
    score = compare_answer(sql, event->word.word[event->word.cur_word], event->db, &id);
    if (score > 0) {
        ret = sql_get_public_by_id(event, id);
    }

    return ret;
}

int sql_get_response(event_t *event)
{
    char sql[1024];
    int ret = 0;
    int id = 0;
    int score = 0;

    if (event->db == NULL) {
        log_fatal("db is not open!");
        return -1;
    }

    memset(sql, 0x0, sizeof(sql));
    sprintf(sql,"select CONDITION, ID from REQ_ANS_DATA where NAME = '%s' order by ID asc", event->context.context);//damon db need check
    score = compare_answer(sql, event->word.word[event->word.cur_word], event->db, &id);
    if (score > 0) {
        ret = sql_get_by_id(event, id);
    }

    return ret;
}

int sql_get_response_top_context(event_t *event)
{
    char sql[1024];
    int ret = 0;
    int id = 0;
    int score = 0;

    if (event->db == NULL) {
        log_fatal("db is not open!");
        return -1;
    }

    memset(sql, 0x0, sizeof(sql));
    sprintf(sql,"select CONDITION, ID from REQ_ANS_DATA where NAME regexp '[0]' order by ID asc");

    score = compare_answer(sql, event->word.word[event->word.cur_word], event->db, &id);
    if (score > 0) {
        ret = sql_get_by_id(event, id);
    }

    return ret;
}

int sql_is_skip_response(event_t *event)
{
    char sql[1024];
    char **result = NULL;
    char *err_msg = NULL;
    int rows = 0;
    int colums = 0;
    int ret = 0;
//    int id = 0;

    if (event->db == NULL) {
        log_fatal("db is not open!");
        return -1;
    }

    memset(sql, 0x0, sizeof(sql));
    sprintf(sql,"select CONDITION, ID from REQ_ANS_DATA where NAME = '%s'",event->context.context);
    sqlite3_get_table(event->db, sql, &result, &rows, &colums, &err_msg);

    if (rows > 0) {
        if (is_context_used(result[2])) {
            ret = 1;
        }
    }
    sqlite3_free_table(result);

    return ret;
}

/* just get response by context only */
int sql_get_public_response_by_context_only(event_t *event)
{
    char sql[1024];
    char **result = NULL;
    char *err_msg = NULL;
    int rows = 0;
    int colums = 0;
    int ret = 0;
    int id = 0;

    if (event->db == NULL) {
        log_fatal("db is not open!");
        return -1;
    }

    memset(sql, 0x0, sizeof(sql));
    sprintf(sql,"select CONDITION, ID from REQ_PUBLIC_DATA where NAME = '%s'",event->context.context);//damon db need check

    sqlite3_get_table(event->db, sql, &result, &rows, &colums, &err_msg);
    if (err_msg != NULL) {
        sqlite3_free_table(result);
        log_error("sqlite3_get_table: %s", err_msg);
        return -ERROR_GET_TABLE;
    }

    if (rows > 0) {
        id = atoi(result[3]);
        ret = sql_get_public_by_id(event, id);
    }
    sqlite3_free_table(result);

    return ret;
}

/* just get response by context only */
int sql_get_response_by_context_only(event_t *event, const char *context)
{
    char sql[1024];
    char **result = NULL;
    char *err_msg = NULL;
    int rows = 0;
    int colums = 0;
    int ret = 0;
    int id = 0;

    if (event->db == NULL) {
        log_fatal("db is not open!");
        return -1;
    }

//    printf("context[%s]\n", event->context.context);

    memset(sql, 0x0, sizeof(sql));
    sprintf(sql,"select CONDITION, ID from REQ_ANS_DATA where NAME = '%s'", context);

    sqlite3_get_table(event->db, sql, &result, &rows, &colums, &err_msg);
    if (err_msg != NULL) {
        sqlite3_free_table(result);
        log_error("sqlite3_get_table: %s", err_msg);
        return -ERROR_GET_TABLE;
    }

//    printf("rows: %d\n", rows);
    if (rows > 0) {
        id = atoi(result[3]);
        ret = sql_get_by_id(event, id);
//        printf("id: %d\n", id);
    } else {
		log_info("sql no this context(%s) data!\n", context);
    }
    sqlite3_free_table(result);

    return ret;
}


int sql_get_context_floor(sqlite3 *db, char *context)
{
    char sql[1024];
    char **result = NULL;
    char *err_msg = NULL;
    int rows = 0;
    int colums = 0;
    int ret = 0;

    if (db == NULL) {
        log_fatal("db is not open!");
        return -1;
    }

    memset(sql, 0x0, sizeof(sql));
    sprintf(sql,"select FLOOR from REQ_ANS_FLOW where NAME = '%s'",context);
    sqlite3_get_table(db, sql, &result, &rows, &colums, &err_msg);

    if (rows > 0) {
        ret = atoi(result[1]);
        log_debug("floor: %d", ret);
    } else {
        log_error("SQL table REQ_ANS_FLOW no context(%s) data!", context);
    }
    sqlite3_free_table(result);

    return ret;
}

int is_flow_type(sqlite3 *db, char *context)
{
    char sql[1024];
    char **result = NULL;
    char *err_msg = NULL;
    int rows = 0;
    int colums = 0;
    int i = 0;

    if (db == NULL) {
        log_fatal("db is not open!");
        return -1;
    }

    memset(sql, 0x0, sizeof(sql));
    sprintf(sql,"select NAME from REQ_ANS_FLOW where LEVEL = 1 order by ID asc");

    sqlite3_get_table(db, sql, &result, &rows, &colums, &err_msg);

    if (rows > 0) {
        for (i = 1; i <= rows; i++) {
//            log_info("context: [%s], result: [%s]", context, result[i]);
            if (strcmp(context, result[i]) == 0) {
                sqlite3_free_table(result);
                log_info("is flow type");
                return 1;
            }
        }
    } else {
        log_error("SQL table REQ_ANS_FLOW no data!");
    }
    sqlite3_free_table(result);

    return 0;
}

int sql_get_unused_context(sqlite3 *db, char *context)
{
    char sql[1024];
    char **result = NULL;
    char *err_msg = NULL;
    int rows = 0;
    int colums = 0;
    int ret = 0;
    int i = 0;

    if (db == NULL) {
        log_fatal("db is not open!");
        return -1;
    }

    memset(sql, 0x0, sizeof(sql));
    sprintf(sql,"select NAME from REQ_ANS_FLOW where LEVEL = 1 order by ID asc");

    sqlite3_get_table(db, sql, &result, &rows, &colums, &err_msg);

    if (rows > 0) {
        for (i = 1; i <= rows; i++) {
            if (strcmp(context, result[i]) == 0) {
                break;
            }
        }

        /* is top context */
        if (i != rows + 1) {
//            if (is_just_used(context)) {
//                ret = 0;
//            } else {
                for (i = 1; i <= rows; i++) {
    //                printf("sql context: %s, rows: %d", result[i], rows);
                    if (get_context_status(result[i]) < 0) {
                        log_debug("find a not used context: %s", result[i]);
                        break;
                    }
                }
                if (i != rows + 1) {
    //                printf("context: %s", result[i]);
                    strcpy(context, result[i]);
                    ret = 1;
//                    printf("ret 1\n");
                } else {
//                    printf("ret 2\n");
                    ret = 2;
                }
//            }
        } else {
            ret = 0;
        }
    } else {
        ret = 0;
        log_error("SQL table REQ_ANS_FLOW no data!");
    }
    sqlite3_free_table(result);

    return ret;
}

int sql_get_unused_purpose(sqlite3 *db, char *context)
{
    char sql[1024];
    char **result = NULL;
    char *err_msg = NULL;
    int rows = 0;
    int colums = 0;
    int ret = 0;
    int i = 0;

    if (db == NULL) {
        log_fatal("db is not open!");
        return -1;
    }

    memset(sql, 0x0, sizeof(sql));
    sprintf(sql,"select NAME from REQ_ANS_FLOW where LEVEL = 2 order by ID asc");

    sqlite3_get_table(db, sql, &result, &rows, &colums, &err_msg);

    if (rows > 0) {
        for (i = 1; i <= rows; i++) {
            if (get_purpose_status(result[i]) < 0) {
                break;
            }
        }
        if (i != rows + 1) {
            strcpy(context, result[i]);
            ret = 1;
        } else {
            strcpy(context, result[rows]);
            ret = 1;
            //ret = 2;
        }
    } else {
        ret = 0;
        log_error("SQL table REQ_ANS_FLOW no data!");
    }
    sqlite3_free_table(result);

    return ret;
}

int sql_get_introduce_deep(sqlite3 *db, char *context)
{
    int i = 0;
    char sql[1024];
    char **result = NULL;
    char *err_msg = NULL;
    int rows = 0;
    int colums = 0;
    int ret = 0;
    char name[128];

    if (db == NULL) {
        log_fatal("db is not open!");
        return -1;
    }

    memset(sql, 0x0, sizeof(sql));
    sprintf(sql,"select NAME, FLOOR from REQ_ANS_FLOW where FLOOR > 1 order by ID asc");

    sqlite3_get_table(db, sql, &result, &rows, &colums, &err_msg);

    if (rows > 0) {
        memset(name, 0, sizeof(name));
        strcat(name, ".");
        for (i = 1; i <= rows; i++) {
            log_debug("%s floor: %d", result[i * 2], atoi(result[i * 2 + 1]));
            if (get_context_used_floor(result[i * 2]) < atoi(result[i * 2 + 1])) {
                strcat(name, result[i * 2]);
                strcat(name, "&");
                ret = 1;
            }
        }
        log_debug("name: %s", name);
    } else {
        log_error("SQL table REQ_ANS_FLOW no FLOOR > 1 data!");
    }

    if (ret == 1) {
        strcpy(context, name);
    }

    sqlite3_free_table(result);

    return ret;

}

int sql_get_next_notused_response(event_t *event, char *context)
{
    int ret = 0;

    log_debug("get a unused flow! current context: %s", event->context.cur_context);
    if (is_context_used(context)) {

        if (event->purpose_flag) { /* 濡傛灉宸茬粡璧板埌鐩爣璇锛屽垯鍦ㄧ洰鏍囪澧冧腑璺宠浆 add by SuiFeng 2016.11.29 */
            ret = sql_get_unused_purpose(event->db, context);
        } else {
            ret = sql_get_unused_context(event->db, context);
            if (ret == 2 || ret == 0) {
                ret = sql_get_unused_purpose(event->db, context);
            }
        }

        if (ret == 1) {
            ret = sql_get_response_by_context_only(event, context);
        } else {
            log_error("error!!! not find a not used flow!!");
        }
    } else {
        log_info("context: %s is not used", context);
    }

    return ret;
}

int sql_get_base_name(sqlite3 *db, char *name, char *record)
{
    char sql[1024];
    int ret = 0;
    char *word = NULL;
    sqlite3_stmt *statement;

    if (db == NULL) {
        log_fatal("db is not open!");
        return -1;
    }

    memset(sql, 0x0, sizeof(sql));

    /* get base name */

    memset(sql,0x0,sizeof(sql));
    sprintf(sql,"select WORD, RECORD from BASE_NAME_TABLE");

    ret = sqlite3_prepare(db, sql, -1, &statement, NULL);
    if (ret != SQLITE_OK) {
        sqlite3_finalize(statement);
        return -1;
    }

    while(sqlite3_step(statement) == SQLITE_ROW){
        word = (char *)sqlite3_column_text(statement, 0);
        if (strstr(name, word) != NULL) {
        	sprintf(record, "%s", sqlite3_column_text(statement, 1));
        	ret = 1;
        	break;
        }
    }
    sqlite3_finalize(statement);

    return ret;
}

int sql_get_appeliation(sqlite3 *db, char *word, char *record)
{
    char sql[1024];
    int ret = 0;
    char *sword = NULL;
    sqlite3_stmt *statement;

    if (db == NULL) {
        log_fatal("db is not open!");
        return -1;
    }

    memset(sql, 0x0, sizeof(sql));

    /* get */
    memset(sql,0x0,sizeof(sql));
    sprintf(sql,"select WORD, RECORD from APPELLATION_TABLE");

    ret = sqlite3_prepare(db, sql, -1, &statement, NULL);
    if (ret != SQLITE_OK) {
        sqlite3_finalize(statement);
        return -1;
    }

    while(sqlite3_step(statement) == SQLITE_ROW){
        sword = (char *)sqlite3_column_text(statement, 0);
        log_debug("sql word: [%s]", sword);
        if (strstr(word, sword) != NULL) {
        	sprintf(record, "%s", sqlite3_column_text(statement, 1));
        	ret = 1;
        	break;
        }
    }

    sqlite3_finalize(statement);

    return ret;
}

int sql_get_record(sqlite3 *db, char *table, char *word, char *record)
{
    char sql[1024];
    int ret = 0;
    char *sword = NULL;
    sqlite3_stmt *statement;

    if (db == NULL) {
        log_fatal("db is not open!");
        return -1;
    }

    memset(sql, 0x0, sizeof(sql));

    /* get */
    memset(sql,0x0,sizeof(sql));
    sprintf(sql,"select WORD, RECORD, from %s", table);

    ret = sqlite3_prepare(db, sql, -1, &statement, NULL);
    if (ret != SQLITE_OK) {
        sqlite3_finalize(statement);
        return -1;
    }

    while(sqlite3_step(statement) == SQLITE_ROW){
        sword = (char *)sqlite3_column_text(statement, 0);
        if (strstr(word, sword) != NULL) {
        	sprintf(record, "%s", sqlite3_column_text(statement, 1));
        	ret = 1;
        	break;
        }
    }
    sqlite3_finalize(statement);

    return ret;
}
