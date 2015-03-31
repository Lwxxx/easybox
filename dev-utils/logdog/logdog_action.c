#include <stdlib.h>
#include <unistd.h>
#include <pcre.h>

#include "eb_global.h"
#include "logdog_action.h"
#include "doglog.h"

GLOBAL int logdog_match(char* line, char* pattern)
{
	pcre* re;
	const char* error;
	int   err_offset;
	int   rc;
	int   ret = LDG_NOT_MATCH;
	int   ovector[OVECCOUNT];

	re = pcre_compile(pattern, 0, &error, &err_offset, NULL);
	if (NULL == re)
	{
		LDG_ERR("compile pcre pattern failed!\n");
		return LDG_NOT_MATCH;
	}

	rc = pcre_exec(re, NULL, line, strlen(line),
				   0, 0, ovector, OVECCOUNT);

	if (rc >= 0)
	{
		ret = LDG_MATCH;
	}

	pcre_free(re);
	return ret;
}

GLOBAL void logdog_do(char* cmd)
{
	if (NULL == cmd) {
		return;
	}

	if (0 == fork()) { // child process
		if (0 != system(cmd))
		{
			LDG_LOG("run command %s failed\n", cmd);
		}
		else
		{
			LDG_LOG("run command %s success\n", cmd);
		}

		exit(0);
	}
}
