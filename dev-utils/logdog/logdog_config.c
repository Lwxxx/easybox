#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "eb_global.h"
#include "logdog_config.h"
#include "doglog.h"

GLOBAL struct logdog_config ldg_cfg;


/**
 * @brief init logdog config
 */
GLOBAL void logdog_init_config(void)
{
	memset(&ldg_cfg, 0, sizeof(struct logdog_config));
}

/**
 * @brief load logdog config from a json string
 */
LOCAL void __logdog_load_config_by_json(char* str)
{
	json_object *cfg_json = NULL;
	json_object *logdog_array_obj = NULL;
	json_object *logdog_obj = NULL;
	json_object *file_obj = NULL;
	json_object *actions_array_obj = NULL;
	json_object *actions_obj = NULL;
	json_object *match_obj = NULL;
	json_object *do_obj = NULL;
	const char* tmp_str = NULL;
	int config_count = 0;
	int actions_count = 0;
	int parse_failed = 0;
	int i, j;

	if (NULL == str)
	{
		LDG_ERR("json string is NULL\n");
		goto out;
	}

	cfg_json = json_tokener_parse(str);
	if (NULL == cfg_json)
	{
		LDG_ERR("parse json string failed!\n");
		goto out;
	}

	json_object_object_get_ex(cfg_json, "logdog", &logdog_array_obj);
	config_count  = array_list_length(json_object_get_array(logdog_array_obj));
	ldg_cfg.count = config_count;
	ldg_cfg.entry = (struct logdog_entry*)calloc
		(config_count, sizeof(struct logdog_entry));
	if (NULL == ldg_cfg.entry)
	{
		LDG_ERR("allocate logdog config failed\n");
		parse_failed = 1;
		goto out;
	}

	// iterate the array, creat config list
	for (i = 0; i < config_count; ++i)
	{
		logdog_obj = json_object_array_get_idx(logdog_array_obj, i);
		json_object_object_get_ex(logdog_obj, "file", &file_obj);
		json_object_object_get_ex(logdog_obj, "actions", &actions_array_obj);
		tmp_str = json_object_get_string(file_obj);
		if (strlen(tmp_str) < LDG_PATH_MAX)
		{
			memcpy(&(ldg_cfg.entry[i].filename), tmp_str, LDG_PATH_MAX);
		}
		else
		{
			LDG_ERR("filename too long: %s", tmp_str);
		}

		actions_count = array_list_length(json_object_get_array(actions_array_obj));
		ldg_cfg.entry[i].actions_count = actions_count;
		ldg_cfg.entry[i].match_do = (struct logdog_md*)calloc
			(actions_count, sizeof(struct logdog_md));
		if (NULL == ldg_cfg.entry[i].match_do)
		{
			parse_failed = 1;
			goto out;
		}

		for (j = 0; j < actions_count; ++ j)
		{
			actions_obj = json_object_array_get_idx(actions_array_obj, j);
			json_object_object_get_ex(actions_obj, "match", &match_obj);
			json_object_object_get_ex(actions_obj, "do", &do_obj);

			tmp_str = json_object_get_string(match_obj);
			if (strlen(tmp_str) < LDG_MATCH_MAX)
			{
				memcpy(&(ldg_cfg.entry[i].match_do[j].match), tmp_str, LDG_MATCH_MAX);
			}
			else
			{
				LDG_ERR("match rules too long: %s\n", tmp_str);
			}

			tmp_str = json_object_get_string(do_obj);
			if (strlen(tmp_str) < LDG_DO_MAX)
			{
				memcpy(&(ldg_cfg.entry[i].match_do[j].cmd), tmp_str, LDG_DO_MAX);
			}
			else
			{
				LDG_ERR("do rules too long: %s\n", tmp_str);
			}
		}
	}

out:
	if (NULL != cfg_json) json_object_put(cfg_json);
	if (NULL != logdog_array_obj) json_object_put(logdog_array_obj);
	if (NULL != logdog_obj) json_object_put(logdog_obj);
	if (NULL != file_obj) json_object_put(file_obj);
	if (NULL != actions_array_obj) json_object_put(actions_array_obj);
	if (NULL != actions_obj) json_object_put(actions_obj);
	if (NULL != match_obj) json_object_put(match_obj);
	if (NULL != do_obj) json_object_put(do_obj);
	if (1 == parse_failed)
	{
		logdog_destory_config();
	}
}

/**
 * @brief load logdog config form a json file
 */
GLOBAL void logdog_load_config(char* path)
{
	FILE* fp = NULL;
	char* json_buf = NULL;
	int flen = 0;

	if (NULL == path)
	{
		LDG_ERR("config file path is NULL\n");
		goto out;
	}

	fp = fopen(path, "r");
	if (NULL == fp)
	{
		LDG_ERR("open config file %s failed: %s\n", path, strerror(errno));
		goto out;
	}

	fseek(fp, 0L, SEEK_END);
	flen = ftell(fp);

	json_buf = (char*)calloc(1, flen+1);
	if (NULL == json_buf)
	{
		LDG_ERR("alloc json buf failed: %s\n", strerror(errno));
		goto out;
	}

	fseek(fp, 0L, SEEK_SET);
	if (flen != fread(json_buf, 1, flen, fp))
	{
		LDG_ERR("read config file failed: %s\n", strerror(errno));
		goto out;
	}

	json_buf[flen] = '\0';

	__logdog_load_config_by_json(json_buf);

out:
	if (NULL != fp) fclose(fp);
	if (NULL != json_buf) free(json_buf);
}

/**
 * @brief free config data
 */
GLOBAL void logdog_destory_config(void)
{
	int count = ldg_cfg.count;
	int i;

	for (i = 0; i < count; ++i)
	{
		if (NULL != ldg_cfg.entry[i].match_do)
		{
			free(ldg_cfg.entry[i].match_do);
		}
	}

	if (NULL != ldg_cfg.entry)
	{
		free(ldg_cfg.entry);
	}
}

/**
 * @brief dump all logdog config data (for debug)
 */
GLOBAL void logdog_dump_config(void)
{
	int i, j;

	for (i = 0; i < ldg_cfg.count; ++i)
	{
		printf("-----------------------------------------------\n");
		printf("file: %s\n", ldg_cfg.entry[i].filename);
		for (j = 0; j < ldg_cfg.entry[i].actions_count; ++j)
		{
			printf("\tmatch: %s\n", ldg_cfg.entry[i].match_do[j].match);
			printf("\tdo   : %s\n", ldg_cfg.entry[i].match_do[j].cmd);
		}
	}
	printf("-----------------------------------------------\n");
}
