#ifndef __DDD_MONITOR_CPU_H__
#define __DDD_MONITOR_CPU_H__

#include <inttypes.h>

// define a cpu count limit to make memort management easier
#define CPU_MAX 32

#define CPU_NAME_MAX 8

// cpu info of each core
struct cpu_info
{
	char name[CPU_NAME_MAX];
	double usage;
};

// cpu load averge
struct cpu_load_averge
{
	double avg_1min;
	double avg_5min;
	double avg_15min;
};

// cpu info collection
struct cpu_info_collection
{
	int cpu_count;
	struct cpu_info total_info;
	struct cpu_info cpu_list[CPU_MAX];
	struct cpu_load_averge load;
};

#ifdef __cplusplus
extern "C"
{
#endif

int collect_cpu_info(void);
struct cpu_info_collection* get_cpu_data(void);

#ifdef __cplusplus
}
#endif


#endif // __DDD_MONITOR_CPU_H__
