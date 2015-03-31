#include <stdio.h>
#include <stdlib.h>

#include "eb_global.h"
#include "logdog_config.h"
#include "logdog_action.h"
#include "file_watcher.h"
#include "doglog.h"

IMPORT struct logdog_config ldg_cfg;

#define test_begin(module) \
	printf("\n========================= Test %-6s =========================\n", module)

#define test_end()\
	printf("========================= END =================================\n")

void TEST_config()
{
	test_begin("config");
	logdog_init_config();
	logdog_load_config("./logdog.json");
	logdog_dump_config();
	logdog_destory_config();
	test_end();
}

void show_line(char* line)
{
	printf("%s", line);
}

void TEST_watch()
{
	test_begin("watch");
	//file_watch("test.log", show_line);
	test_end();
}

void TEST_match()
{
	test_begin("match");

	if (LDG_MATCH == logdog_match("abc", "^a.*?"))
	{
		printf("abc match ^a.*?\n");
	}
	else
	{
		printf("abc not match ^a.*?\n");
	}

	if (LDG_NOT_MATCH == logdog_match("bcg_,", ".*g.*_$"))
	{
		printf("bcg_ not match .*g.*_$\n");
	}
	else
	{
		printf("bcg_ match .*g.*_$\n");
	}

	test_end();
}

void TEST_do()
{
	test_begin("do");
	logdog_do("echo \"hello\" > hello.txt");
	test_end();
}


int main(int argc, char *argv[])
{
	TEST_config();
	TEST_watch();
	TEST_match();
	TEST_do();

	return 0;
}
