#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "doglog.h"
#include "file_watcher.h"

void file_watch(char* filename, void (*callback)(char* line))
{
	FILE* fp = NULL;
	long  pos = 0;
	char  line_buf[LDG_LINE_MAX] = {0};

	// skip old part of the file
	fp = fopen(filename, "r");
	if (NULL == fp)
	{
		LDG_ERR("read file %s failed: %s\n", filename, strerror(errno));
		return;
	}
	fseek(fp , 0, SEEK_END);
	pos = ftell(fp);
	fclose(fp);

	// watch the file and run callback
	while (1)
	{
		fp = fopen(filename, "r");
		if (NULL != fp) {

			fseek(fp , pos, SEEK_SET);
			while (fgets(line_buf, LDG_LINE_MAX, fp)) {
				callback(line_buf);
				memset(line_buf, 0, LDG_LINE_MAX);
				pos = ftell(fp);
			}

			fclose(fp);
		}

		sleep(LDG_CHECK_INTERVAL);
	}
}
