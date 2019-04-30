/*
 * sa_api.c
 *
 *  Created on: 2016年10月31日
 *      Author: suifeng
 */

#include <string.h>

#include "semantic_analyzer.h"

static char ai_version[24];

int set_ai_version(char *version)
{
    if (version == NULL || version[0] == 0) {
        return -1;
    }

    strcpy(ai_version, version);

    return 0;
}

char* get_ai_version(void)
{
    if (ai_version[0] == 0) {
        return NULL;
    }

    return ai_version;
}

int get_db_name(char *name)
{
    if (name == NULL) {
        return -1;
    }

    strcpy(name, sa_info.db_name);

    return 0;
}

int get_ai_version_of_db_depend(char *version)
{
    if (version == NULL) {
        return -1;
    }

    strcpy(version, sa_info.ai_version_of_db_depend);

    return 0;
}
