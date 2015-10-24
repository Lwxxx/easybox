#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "proc.h"
#include "cpu.h"

static struct cpu_info_collection cpu_data;
static uint64_t user_time = 0;
static uint64_t nice_time = 0;
static uint64_t system_time = 0;
static uint64_t idle_time = 0;
static uint64_t iowait_time = 0;
static uint64_t irq_time = 0;
static uint64_t softirq_time = 0;
static uint64_t steel_time = 0;
static uint64_t guest_time = 0;
static uint64_t guest_nice_time = 0;

struct cpu_info_collection* get_cpu_data(void)
{
	return &cpu_data;
}

int collect_cpu_info(void)
{
	FILE* proc_fd = NULL;
	uint64_t curr_user_time = 0;
	uint64_t curr_nice_time = 0;
	uint64_t curr_system_time = 0;
	uint64_t curr_idle_time = 0;
	uint64_t curr_iowait_time = 0;
	uint64_t curr_irq_time = 0;
	uint64_t curr_softirq_time = 0;
	uint64_t curr_steel_time = 0;
	uint64_t curr_guest_time = 0;
	uint64_t curr_guest_nice_time = 0;
	uint64_t user_period = 0;
	uint64_t nice_period = 0;
	uint64_t system_period = 0;
	uint64_t idle_period = 0;
	uint64_t iowait_period = 0;
	uint64_t irq_period = 0;
	uint64_t softirq_period = 0;
	uint64_t steel_period = 0;
	uint64_t guest_period = 0;
	uint64_t guest_nice_period = 0;
	uint64_t total_period = 0;
	uint64_t busy_period = 0;

	proc_fd = fopen(PROC_STAT, "r");
	if (NULL == proc_fd)
	{
		fprintf(stderr, "open %s failed!\n", PROC_STAT);
		return -1;
	}

	/* cpu  155592 653 36037 899284 46888 0 827 0 0 0
	 *      (1)  user
	 *      (2)  nice
	 *      (3)  system
	 *      (4)  idle
	 *      (5)  iowait
	 *      (6)  irq
	 *      (7)  softirq
	 *      (8)  steel
	 *      (9)  guest
	 *      (10) guest_nice
	 */
	fscanf(proc_fd,
		   "%s %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu",
		   cpu_data.total_info.name, &curr_user_time, &curr_nice_time,
		   &curr_system_time, &curr_idle_time, &curr_iowait_time,
		   &curr_irq_time, &curr_softirq_time, &curr_steel_time,
		   &curr_guest_time, &curr_guest_nice_time);
	fclose(proc_fd);

	/* calculate period*/
	user_period = curr_user_time - user_time;
	nice_period = curr_nice_time - nice_time;
	system_period = curr_system_time - system_time;
	idle_period = curr_idle_time - idle_time;
	iowait_period = curr_iowait_time - iowait_time;
	irq_period = curr_irq_time - irq_time;
	softirq_period = curr_softirq_time - softirq_time;
	steel_period = curr_steel_time - steel_time;
	guest_period = curr_guest_time - guest_time;
	guest_nice_period = curr_guest_nice_time - guest_nice_time;

	/* store current cpu time data */
	user_time = curr_user_time;
	nice_time = curr_nice_time;
	system_time = curr_system_time;
	idle_time = curr_idle_time;
	iowait_time = curr_iowait_time;
	irq_time = curr_irq_time;
	softirq_time = curr_softirq_time;
	steel_time = curr_steel_time;
	guest_time = curr_guest_time;
	guest_nice_time = curr_guest_nice_time;

	/* calculate cpu usage */
	total_period = user_period + nice_period + system_period + idle_period +
				   iowait_period + irq_period + softirq_period + steel_period;
	busy_period = total_period - idle_period - iowait_period;
	cpu_data.total_info.usage = ((double)busy_period/(double)total_period) * 100;

	return 0;
}
