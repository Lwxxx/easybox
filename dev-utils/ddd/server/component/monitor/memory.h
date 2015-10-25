#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <inttypes.h>

// used = buffers + cached
// buffers
// cached
// total
struct mem_info_collection
{
	uint64_t total;
	uint64_t free;
	uint64_t used;
	uint64_t buffer;
	uint64_t cached;
};


#ifdef __cplusplus
extern "C"
{
#endif

struct mem_info_collection* get_mem_data(void);
int collect_memory_info(void);

#ifdef __cplusplus
}
#endif

#endif // __MEMORY_H__
