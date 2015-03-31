#ifndef __LOGDOG_CONFIG_H__
#define __LOGDOG_CONFIG_H__

#include <limits.h>

#include "list.h"
#include "json.h"
#include "json_object_private.h"

#ifndef PATH_MAX
#define LDG_PATH_MAX PATH_MAX
#else
#define LDG_PATH_MAX 256
#endif // PATH_MAX

#define LDG_MATCH_MAX 256
#define LDG_DO_MAX 64

struct logdog_md {
	char match[LDG_MATCH_MAX];
	char cmd[LDG_DO_MAX];
};

struct logdog_entry {
	char filename[LDG_PATH_MAX];
	int  actions_count;
	struct logdog_md* match_do;
};

struct logdog_config {
	int count;
	struct logdog_entry* entry;
};

void logdog_init_config(void);
void logdog_load_config(char* path);
void logdog_destory_config(void);
void logdog_dump_config(void);

#endif // __LOGDOG_CONFIG_H__
