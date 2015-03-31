#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "eb_global.h"
#include "logdog_config.h"
#include "logdog_action.h"
#include "file_watcher.h"
#include "doglog.h"


#define DFT_CONFIG_FILE "logdog_config.json"
#define DFT_LOG_FILE    "logdog.log"

IMPORT struct logdog_config ldg_cfg;

LOCAL void show_usage(void)
{
	char usage[] = "Usage:\n\n"
		"  logdog [options]\n\n"
		"  -c conig_file    set config file\n"
		"  -l log_file      set logdog log file\n"
		"  -i interval      set file check interval\n"
		"  -d               run as daemon\n"
		"  -h               show help infomation\n";

	printf("%s\n", usage);
	exit(1);
}

LOCAL void* run_dog(void *arg)
{
	struct logdog_entry* entry;
	int actions_count = 0;
	int i = 0;

	if (NULL == arg)
	{
		LDG_ERR("invalid logdog entry!\n");
	}
	else
	{
		entry = (struct logdog_entry*)arg;
	}

	LDG_LOG("run logdog for file: %s\n", entry->filename);
	actions_count = entry->actions_count;

	void (*check_and_do)(char* line) =
	({
		void __check_and_do(char* line)
		{
			struct logdog_md* match_do = NULL;
			for (i = 0; i < actions_count; ++i)
			{
				match_do = &(entry->match_do)[i];
				if (LDG_MATCH == logdog_match(line, match_do->match))
				{
					logdog_do(match_do->cmd);
				}
			}
		}
		__check_and_do;
	});

	file_watch(entry->filename, check_and_do);

	pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
	int i           = 0;
	int ret         = 0;
	int arg_val     = 0;
	int daemon_flag = 0;
	int dog_count   = 0;
	char config_file[LDG_PATH_MAX] = {0};
	char log_file[LDG_PATH_MAX]    = {0};
	pthread_t* thread_id           = NULL;
	struct logdog_entry* entry     = NULL;


	// set default file path
	sprintf(config_file, "%s", DFT_CONFIG_FILE);
	sprintf(log_file, "%s", DFT_LOG_FILE);

	// parse command line options
	while (EOF != (arg_val = getopt(argc, argv, "c:l:i:dh")))
	{
		switch (arg_val)
		{
		case 'c':
			memset(config_file, 0, LDG_PATH_MAX);
			sprintf(config_file, optarg, LDG_PATH_MAX);
			break;
		case 'l':
			memset(log_file, 0, LDG_PATH_MAX);
			sprintf(log_file, optarg, LDG_PATH_MAX);
			break;
		case 'i':
			//+TODO: support config check interval
			break;
		case 'd':
			daemon_flag = 1;
			break;
		case 'h':
			show_usage();
			break;
		default:
			break;
		}
	}

	if (1 == daemon_flag)
	{
		//+TODO run as daemon
	}

	// load config json
	logdog_load_config(config_file);

	// create n dog threads
	dog_count = ldg_cfg.count;
	thread_id = (pthread_t*)malloc(dog_count * sizeof(pthread_t));
	if (NULL == thread_id)
	{
		LDG_ERR("allocate memory for thread failed: %s\n", strerror(errno));
		goto out;
	}

	for (i = 0; i < dog_count; ++i)
	{
		entry = &(ldg_cfg.entry[i]);
		ret = pthread_create(&(thread_id[i]), NULL, run_dog, (void *)entry);
		if (0 != ret)
		{
			LDG_ERR("create thread failed: %s\n", strerror(errno));
			goto out;
		}

	}

	for (i = 0; i < dog_count; ++i)
	{
		pthread_join(thread_id[i], NULL);
	}

out:
	logdog_destory_config();
	if (NULL != thread_id) free(thread_id);

	return 0;
}
