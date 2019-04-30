/*
 * sa_suport.h
 *
 *  Created on: 2016年6月22日
 *      Author: 云分身
 */

#ifndef INCLUDE_SA_SUPORT_H_
#define INCLUDE_SA_SUPORT_H_

int set_used_floor(char *name, char type);
int clear_context_info(void);

int is_purpose_type(char *context);
int get_purpose_status(char *context);

int get_context_status(char *context_name);
int get_context_used_floor(char *context_name);
int is_context_used(char *context_name);

#endif /* INCLUDE_SA_SUPORT_H_ */


