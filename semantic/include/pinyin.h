#ifndef PINYIN_H_
#define PINYIN_H_

int set_accent(char *src, char *result, int step);
int pub_reg_cmp_pingying(char *src, char *key, char **coord);
int compare_answer_by_pingying(char *sql, char *src, char **current_p1,sqlite3 *db);
int normative_by_pinyin(char *src, char *result, sqlite3 *db);

#endif
