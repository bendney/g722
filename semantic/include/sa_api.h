/*
 * sa_api.h
 *
 *  Created on: 2016年10月31日
 *      Author: suifeng
 */

#ifndef SA_API_H_
#define SA_API_H_

int set_ai_version(char *version);
char* get_ai_version(void);
int get_db_name(char *name);
int get_ai_version_of_db_depend(char *version);


#endif /* SA_API_H_ */
