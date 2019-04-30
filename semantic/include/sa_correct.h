/*
 * sa_correct.h
 *
 *  Created on: 2017年10月17日
 *      Author: listen
 */

#ifndef INCLUDE_SA_CORRECT_H_
#define INCLUDE_SA_CORRECT_H_

int correct_init(void);
void correct_free(void);
int similarity(char *db_path, char* src, char *result, float *distance);


#endif /* INCLUDE_SA_CORRECT_H_ */
