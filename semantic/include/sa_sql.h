/*
 * sa_sql.h
 *
 *  Created on: 2016年6月30日
 *      Author: 云分身
 */

#ifndef TEST_SA_SA_SQL_H_
#define TEST_SA_SA_SQL_H_

#include "semantic_analyzer.h"

#define ERROR_GET_TABLE		10
#define ERROR_GET_DB		1

typedef struct address_buf
{
    int order;
    char *cur_address;
    struct address_buf *next;
}ai_address_buf;

int init_sqlite3_db(sqlite3 **db, char *dbname);
int sql_get_response(event_t *event);
int sql_get_response_top_context(event_t *event);
int sql_get_response_by_context_only(event_t *event, const char *context);
int sql_get_by_id(event_t *event, int id);
int sql_get_unused_purpose(sqlite3 *db, char *context);
int sql_get_introduce_deep(sqlite3 *db, char *context);
int sql_get_unused_context(sqlite3 *db, char *context);
int sql_get_next_notused_response(event_t *event, char* context);
int sql_get_hangup_stauts(event_t *event, int id);
int sql_get_base_name(sqlite3 *db, char *name, char *record);
int sql_get_appeliation(sqlite3 *db, char *word, char *record);
int sql_get_record(sqlite3 *db, char *table, char *word, char *record);
int sql_get_data_by_id(sqlite3 *db, char *table, char *field, char *data, int id);
/* flag = 0, get char
 * flag = 1, get int
 * */
int sql_get_one_data_by_name(sqlite3 *db, char *table, char *field, void *data, char *name, int flag);
int sql_get_public_response_by_context_only(event_t *event);
int sql_get_public_by_id(event_t *event, int id);
int sql_get_public_response(event_t *event);

int is_flow_type(sqlite3 *db, char *context);

int get_record_file(char (*filename)[FILENAME_SIZE], int *sum, char *record);
char *get_db_path(void);
void clean_db_path(void);
char *get_log_path(void);
void clean_log_path(void);
char *get_micon(void);
int set_micon(char *micon);
int get_industry(char *industry);
int get_area(char *area);

#endif /* TEST_SA_SA_SQL_H_ */
