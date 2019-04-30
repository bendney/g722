/*
 * sa_suport.c
 *
 *  Created on: 2016年6月22日
 *      Author: 云分身
 */

#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include "logger.h"

#define CONTEXT0_FLOOR  1
#define CONTEXT0    "jieshao"
#define CONTEXT1_FLOOR  3
#define CONTEXT1    "weizhi"
#define CONTEXT2_FLOOR  3
#define CONTEXT2    "jiage"
#define CONTEXT3_FLOOR  3
#define CONTEXT3    "huxing"
#define CONTEXT4_FLOOR     3
#define CONTEXT4    "primary"
#define CONTEXT_NUB     5

#define MAX_FLOOR   9
#define MAX_CONTEX  30

typedef enum {
    ct_null,
    ct_introduce,
    ct_purpose,
    ct_redeem,
}context_type;

struct context_queue {
    char name[64];
    int used_floor;
    int max_floor;
    context_type ct_type;
    int floor_used[MAX_FLOOR][2];
};

static int context_cnt = 0;
static struct context_queue context_queue[MAX_CONTEX];
//static struct context_queue flow_context_queue[MAX_CONTEX];

static int abstract_main_context(char *name, char *context)
{
    char *pos = NULL;

    /* get main context */
    memset(context, 0, 64);

    if (name[0] == '.') {
        pos = strchr(name + 1, '.');
        if (pos == NULL) {
            strcpy(context, name + 1);
        } else {
            strncpy(context, name + 1, pos - name - 1);
        }
    } else {
        pos = strchr(name, '.');
        if (pos == NULL) {
            strcpy(context, name);
        } else {
            strncpy(context, name, pos - name);
        }
    }

    return 0;
}

static int get_context_postion(char *name)
{
    int i = 0;

    for (i = 0; i < context_cnt; i++) {
        if (strcmp(name, context_queue[i].name) == 0) {
            return i;
        }
    }

    return -1;
}

static int get_floor_postion(struct context_queue *context, char type)
{
    int i = 0;

    for (i = 0; i < context->used_floor; i++) {
        if (context->floor_used[i][0] == type) {
            return i;
        }
    }

    return -1;
}

static int set_floor(struct context_queue *context, char type)
{
    int postion = -1;

    if (type >= '1' && type <= '1' + MAX_FLOOR) {
        context->ct_type = ct_introduce;
    } else if (type >= 'A' && type <= 'A' + MAX_FLOOR) {
        context->ct_type = ct_purpose;
    } else if (type >= 'a' && type <= 'a' + MAX_FLOOR) {
        context->ct_type = ct_redeem;
    }

    postion = get_floor_postion(context, type);
    if (postion < 0) {
        if (context->used_floor == MAX_FLOOR) {
            log_error("floor is max(%d)!", context->used_floor);
            return -1;
        }
        context->floor_used[context->used_floor][0] = type;
        context->floor_used[context->used_floor][1]++;
        context->used_floor++;
    } else {
        context->floor_used[postion][1]++;
    }

    return 0;
}

int set_used_floor(char *name, char type)
{
    int ret = 0;
    char context_name[64];
    int postion = -1;


    if (type == '0' || name == NULL) {
        //printf("type is 0 or name is NULL\n");
        return 0;
    }

    if (type == '@') {
        //printf("type is @\n");
        return 0;
    }

    /* find the main context */
//    printf("context src: [%s]\n", name);
    abstract_main_context(name, context_name);
//    printf("context dst: [%s]\n", context_name);

    /* the context is used ? */
    postion = get_context_postion(context_name);

    if (postion < 0) {
        if (context_cnt == MAX_CONTEX) {
            log_error("context is max(%d)!", context_cnt);
            return -1;
        }
        strcpy(context_queue[context_cnt].name, context_name);
        postion = context_cnt;
        context_cnt++;
    }

    ret = set_floor(&context_queue[postion], type);


    return ret;
}

int clear_context_info(void)
{
    memset(context_queue, 0, sizeof(context_queue));
    context_cnt = 0;

    return 0;
}

/* return: -1 not used, >= 0 context position */
int get_context_status(char *context_name)
{
    int i = 0;

//    printf("context count: %d\n", context_cnt);

    for (i = 0; i < context_cnt; i++) {
//        printf("queue: %s\n", context_queue[i].name);
        if (strcmp(context_name, context_queue[i].name) == 0) {
//            printf("context positon: %d\n", i);
            return i;
        }
    }

    return -1;
}

/*  */
int is_context_used(char *context_name)
{
    char *context = context_name;

    if (context_name[0] == '.') {
        context = context_name + 1;
    }

    if (get_context_status(context) < 0) {
//        printf("context:[%s] ",context_name);
//        printf("not used\n");
        return 0;
    }
//    printf("used\n");
    return 1;
}

#if 0
int is_flow_context(char *context_name)
{
    char *context = context_name;

    if (context_name[0] == '.') {
        context = context_name + 1;
    }

    if (get_context_status(context) < 0) {
//        printf("context:[%s] ",context_name);
//        printf("not used\n");
        return 0;
    }
//    printf("used\n");
    return 1;
}
#endif

int get_context_used_floor(char *context_name)
{
    int pos;

    pos = get_context_status(context_name);
    if (pos < 0) {
        return 0;
    } else {
        return context_queue[pos].used_floor;
    }

    return 0;
}

int get_purpose_status(char *context)
{
    int i = 0;

    for (i = 0; i < context_cnt; i++) {
        if (context_queue[i].ct_type == ct_purpose) {
            if (strcmp(context, context_queue[i].name) == 0) {
                return i;
            }
        }
    }

    return -1;
}

int is_purpose_type(char *context)
{
    int i = 0;

    for (i = 0; i < context_cnt; i++) {
        if (context_queue[i].ct_type == ct_purpose) {
            if (strcmp(context, context_queue[i].name) == 0) {
                return 1;
            }
        }
    }

    return 0;
}


