#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "proc.h"

static struct mem_info_collection mem_data;



struct mem_info_collection* get_mem_data(void)
{
	return &mem_data;
}


int collect_memory_info(void)
{
	FILE* proc_fd = NULL;

	proc_fd = fopen(PROC_MEMINFO, "r");
	if (NULL == proc_fd)
	{
		fprintf(stderr, "open %s failed!\n", PROC_MEMINFO);
		return -1;
	}

	/* MemTotal:        xxxxxxx kB
	 * MemFree:         xxxxxxx kB
	 * Buffers:           xxxxx kB
	 * Cached:          xxxxxxx kB
	 */
	fscanf(proc_fd, "MemTotal: %32llu kB\n", &(mem_data.total));
	fscanf(proc_fd, "MemFree: %32llu kB\n", &(mem_data.free));
	fscanf(proc_fd, "Buffers: %32llu kB\n", &(mem_data.buffer));
	fscanf(proc_fd, "Cached: %32llu kB\n", &(mem_data.cached));
	fclose(proc_fd);

	mem_data.used = mem_data.total - mem_data.free - mem_data.buffer - mem_data.cached;

	return 0;
}
